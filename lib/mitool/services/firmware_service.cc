module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#ifdef _WIN32
#undef min
#undef max
#endif

module mitool.services.firmware;

import mitool.core.error;
import mitool.core.types;
import mitool.utils.version_parser;

using Json = nlohmann::json;

namespace {

constexpr const char *kMiuiOtaUrl =
    "http://update.miui.com/updates/miotaV3.php";
constexpr const char *kCaBundleFile = "cacert.pem";
constexpr int kAesBlockSize = 16;

constexpr std::array<unsigned char, 16> kMiuiCryptoKey = {
    0x6D, 0x69, 0x75, 0x69, 0x6F, 0x74, 0x61, 0x76,
    0x61, 0x6C, 0x69, 0x64, 0x65, 0x64, 0x31, 0x31};
constexpr std::array<unsigned char, 16> kMiuiCryptoIv = {
    0x30, 0x31, 0x30, 0x32, 0x30, 0x33, 0x30, 0x34,
    0x30, 0x35, 0x30, 0x36, 0x30, 0x37, 0x30, 0x38};

const Json kRequestTemplate = {
    {"a", 0},
    {"c", ""},
    {"b", "F"},
    {"d", ""},
    {"g", "00000000000000000000000000000000"},
    {"i", "0000000000000000000000000000000000000000000000000000000000000000"},
    {"isR", "0"},
    {"f", "1"},
    {"l", "en_US"},
    {"n", ""},
    {"sys", "0"},
    {"unlock", "0"},
    {"r", "CN"},
    {"sn", "0x00000000"},
    {"v", ""},
    {"bv", ""},
    {"id", ""}};

using mitool::core::Error;
using mitool::core::ErrorCode;
using mitool::core::Result;

std::unexpected<Error> makeError(ErrorCode code, std::string message) {
  return std::unexpected(Error{code, std::move(message)});
}

// ---------------------------------------------------------------------------
// Crypto: base64 + AES-128-CBC
// ---------------------------------------------------------------------------

using BioChain = std::unique_ptr<BIO, decltype(&BIO_free_all)>;
using CipherContext =
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

Result<std::vector<unsigned char>> base64Decode(std::string_view text) {
  if (text.empty()) {
    return makeError(ErrorCode::CryptoError, "base64Decode: empty input");
  }

  BIO *mem = BIO_new_mem_buf(text.data(), static_cast<int>(text.size()));
  BIO *b64 = BIO_new(BIO_f_base64());
  if (!mem || !b64) {
    BIO_free(mem);
    BIO_free(b64);
    return makeError(ErrorCode::CryptoError,
                     "base64Decode: failed to create BIO");
  }

  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO_push(b64, mem);
  const BioChain chain(b64, &BIO_free_all);

  std::vector<unsigned char> buffer(text.size());
  int totalLen = 0;
  while (true) {
    const int len = BIO_read(b64, buffer.data() + totalLen,
                             static_cast<int>(buffer.size()) - totalLen);
    if (len <= 0) {
      break;
    }
    totalLen += len;
  }

  buffer.resize(static_cast<std::size_t>(totalLen));
  return buffer;
}

Result<std::string> base64Encode(const std::vector<unsigned char> &data) {
  BIO *b64 = BIO_new(BIO_f_base64());
  BIO *mem = BIO_new(BIO_s_mem());
  if (!mem || !b64) {
    BIO_free(mem);
    BIO_free(b64);
    return makeError(ErrorCode::CryptoError,
                     "base64Encode: failed to create BIO");
  }

  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO_push(b64, mem);
  const BioChain chain(b64, &BIO_free_all);

  if (BIO_write(b64, data.data(), static_cast<int>(data.size())) <= 0 ||
      BIO_flush(b64) != 1) {
    return makeError(ErrorCode::CryptoError, "base64Encode: write failed");
  }

  BUF_MEM *bufferPtr = nullptr;
  BIO_get_mem_ptr(mem, &bufferPtr);
  if (!bufferPtr) {
    return makeError(ErrorCode::CryptoError,
                     "base64Encode: failed to read BIO buffer");
  }
  return std::string(bufferPtr->data, bufferPtr->length);
}

Result<std::vector<unsigned char>> aes128cbcEncrypt(std::string_view data) {
  const CipherContext ctx(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free);
  if (!ctx) {
    return makeError(ErrorCode::CryptoError,
                     "aes128cbcEncrypt: failed to create context");
  }

  if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_128_cbc(), nullptr,
                         kMiuiCryptoKey.data(), kMiuiCryptoIv.data()) != 1) {
    return makeError(ErrorCode::CryptoError,
                     "aes128cbcEncrypt: initialization error");
  }

  std::vector<unsigned char> ciphertext(data.size() + kAesBlockSize);
  int outLen1 = 0;
  int outLen2 = 0;

  if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &outLen1,
                        reinterpret_cast<const unsigned char *>(data.data()),
                        static_cast<int>(data.size())) != 1) {
    return makeError(ErrorCode::CryptoError,
                     "aes128cbcEncrypt: encryption error (Update)");
  }

  if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + outLen1, &outLen2) !=
      1) {
    return makeError(ErrorCode::CryptoError,
                     "aes128cbcEncrypt: encryption error (Final)");
  }

  ciphertext.resize(static_cast<std::size_t>(outLen1 + outLen2));
  return ciphertext;
}

Result<std::string> aes128cbcDecrypt(const std::vector<unsigned char> &data) {
  const CipherContext ctx(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free);
  if (!ctx) {
    return makeError(ErrorCode::CryptoError,
                     "aes128cbcDecrypt: failed to create context");
  }

  if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_128_cbc(), nullptr,
                         kMiuiCryptoKey.data(), kMiuiCryptoIv.data()) != 1) {
    return makeError(ErrorCode::CryptoError,
                     "aes128cbcDecrypt: initialization error");
  }

  std::vector<unsigned char> plaintext(data.size() + kAesBlockSize);
  int outLen1 = 0;
  int outLen2 = 0;

  if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &outLen1, data.data(),
                        static_cast<int>(data.size())) != 1) {
    return makeError(ErrorCode::CryptoError,
                     "aes128cbcDecrypt: decryption error (Update)");
  }

  if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + outLen1, &outLen2) !=
      1) {
    return makeError(
        ErrorCode::CryptoError,
        "aes128cbcDecrypt: decryption error (Final) - invalid key/IV?");
  }

  return std::string(plaintext.begin(), plaintext.begin() + outLen1 + outLen2);
}

// ---------------------------------------------------------------------------
// HTTP (libcurl)
// ---------------------------------------------------------------------------

using CurlHandle = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;
using CurlString = std::unique_ptr<char, decltype(&curl_free)>;

class CurlHeaders {
public:
  CurlHeaders() = default;
  CurlHeaders(const CurlHeaders &) = delete;
  CurlHeaders &operator=(const CurlHeaders &) = delete;

  ~CurlHeaders() {
    if (list_) {
      curl_slist_free_all(list_);
    }
  }

  void append(const char *value) {
    if (curl_slist *next = curl_slist_append(list_, value)) {
      list_ = next;
    }
  }

  curl_slist *get() const { return list_; }

private:
  curl_slist *list_ = nullptr;
};

std::size_t writeToString(void *contents, std::size_t size, std::size_t nmemb,
                          void *userp) {
  static_cast<std::string *>(userp)->append(static_cast<char *>(contents),
                                            size * nmemb);
  return size * nmemb;
}

std::size_t writeToStream(void *contents, std::size_t size, std::size_t nmemb,
                          void *userp) {
  auto *stream = static_cast<std::ofstream *>(userp);
  const std::size_t writeSize = size * nmemb;
  stream->write(static_cast<const char *>(contents),
                static_cast<std::streamsize>(writeSize));
  return stream->good() ? writeSize : 0;
}

int reportProgress(void *clientp, curl_off_t dlTotal, curl_off_t dlNow,
                   curl_off_t /*ulTotal*/, curl_off_t /*ulNow*/) {
  const auto *callback =
      static_cast<const mitool::services::ProgressCallback *>(clientp);
  if (callback && *callback) {
    (*callback)(dlNow, dlTotal);
  }
  return 0;
}

Result<std::string> requestForUpdate(const std::string &requestData) {
  auto encrypted = aes128cbcEncrypt(requestData);
  if (!encrypted) {
    return std::unexpected(encrypted.error());
  }
  auto encoded = base64Encode(*encrypted);
  if (!encoded) {
    return std::unexpected(encoded.error());
  }

  const CurlHandle curl(curl_easy_init(), &curl_easy_cleanup);
  if (!curl) {
    return makeError(ErrorCode::NetworkError,
                     "Failed to initialise curl handle");
  }

  const CurlString escaped(
      curl_easy_escape(curl.get(), encoded->c_str(),
                       static_cast<int>(encoded->length())),
      &curl_free);
  if (!escaped) {
    return makeError(ErrorCode::NetworkError, "curl_easy_escape failed");
  }
  const std::string postData = std::format("q={}&t=&s=1", escaped.get());

  CurlHeaders headers;
  headers.append("clientId: MITUNES");
  headers.append("Connection: Keep-Alive");
  headers.append("Accept-Encoding: identity");
  headers.append("Content-Type: application/x-www-form-urlencoded");

  std::string response;
  curl_easy_setopt(curl.get(), CURLOPT_URL, kMiuiOtaUrl);
  curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "MiTunes_UserAgent_v3");
  curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
  curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, postData.c_str());
  curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE,
                   static_cast<long>(postData.size()));
  curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
  curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeToString);
  curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);

  const CURLcode status = curl_easy_perform(curl.get());
  if (status != CURLE_OK) {
    return makeError(
        ErrorCode::NetworkError,
        std::format("OTA request failed: {}", curl_easy_strerror(status)));
  }

  if (response.empty()) {
    return makeError(ErrorCode::NoResponse,
                     "The OTA server returned an empty response");
  }

  const auto decoded = base64Decode(response);
  if (!decoded) {
    return std::unexpected(decoded.error());
  }
  return aes128cbcDecrypt(*decoded);
}

Result<void>
downloadToFile(const std::string &url, const std::filesystem::path &path,
               const mitool::services::ProgressCallback &onProgress) {
  const CurlHandle curl(curl_easy_init(), &curl_easy_cleanup);
  if (!curl) {
    return makeError(ErrorCode::NetworkError,
                     "Failed to initialise curl handle");
  }

  // First attempt with system certificates, second with cacert.pem.
  CURLcode status = CURLE_OK;
  for (const bool useCaBundle : {false, true}) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
      return makeError(
          ErrorCode::IoError,
          std::format("Cannot open '{}' for writing", path.string()));
    }

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeToStream);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, reportProgress);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, &onProgress);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    if (useCaBundle) {
      curl_easy_setopt(curl.get(), CURLOPT_CAINFO, kCaBundleFile);
    }

    status = curl_easy_perform(curl.get());
    output.close();
    if (status == CURLE_OK) {
      if (output.fail()) {
        return makeError(ErrorCode::IoError,
                         std::format("Failed to write '{}'", path.string()));
      }
      return {};
    }
  }

  return makeError(ErrorCode::NetworkError,
                   std::format("Failed to download {}: {}", url,
                               curl_easy_strerror(status)));
}

// ---------------------------------------------------------------------------
// Parsing server response
// ---------------------------------------------------------------------------

const Json *findChild(const Json *parent, const char *key) {
  if (parent == nullptr || !parent->is_object()) {
    return nullptr;
  }
  const auto it = parent->find(key);
  return it != parent->end() ? &*it : nullptr;
}

std::string safeGet(const Json *object, const char *key) {
  const Json *value = findChild(object, key);
  if (value != nullptr && value->is_string()) {
    return value->get<std::string>();
  }
  return "N/A";
}

std::vector<std::string> collectStrings(const Json *array) {
  std::vector<std::string> result;
  if (array == nullptr || !array->is_array()) {
    return result;
  }
  for (const auto &entry : *array) {
    if (entry.is_string()) {
      result.push_back(entry.get<std::string>());
    }
  }
  return result;
}

Result<mitool::core::UpdateInfo> parseUpdateInfo(const std::string &text) {
  const Json data = Json::parse(text, nullptr, false);
  if (data.is_discarded()) {
    return makeError(ErrorCode::ParseError,
                     "The OTA server returned malformed JSON");
  }

  const Json *latestRom = findChild(&data, "LatestRom");
  if (latestRom == nullptr || !latestRom->is_object()) {
    return makeError(ErrorCode::NoUpdateFound,
                     "No update found: the server response has no LatestRom");
  }
  const Json *currentRom = findChild(&data, "CurrentRom");
  const Json *incrementRom = findChild(&data, "IncrementRom");

  std::vector<std::string> changelog = collectStrings(
      findChild(findChild(findChild(latestRom, "changelog"), "System"), "txt"));
  if (changelog.empty()) {
    changelog.push_back("N/A");
  }

  return mitool::core::UpdateInfo{
      .device = safeGet(latestRom, "device"),
      .current_rom = safeGet(currentRom, "version"),
      .latest_rom = safeGet(latestRom, "version"),
      .android_ver = safeGet(latestRom, "codebase"),
      .file_size = safeGet(latestRom, "filesize"),
      .applicable_from = safeGet(incrementRom, "versionForApply"),
      .md5 = safeGet(latestRom, "md5"),
      .file_name = safeGet(latestRom, "filename"),
      .changelog = std::move(changelog),
      .mirror_list = collectStrings(findChild(&data, "MirrorList")),
  };
}

// "bv" field: MIUI version (V12, V13), otherwise empty
std::string detectRequestBv(const mitool::core::VersionData &version) {
  if (version.os_name.starts_with('V') && version.os_ver <= 13) {
    return std::to_string(version.os_ver);
  }
  return "";
}

} // namespace

namespace mitool::services {

core::Result<core::UpdateInfo> findLatestUpdate(std::string_view device,
                                                std::string_view osVersion) {
  if (device.empty()) {
    return makeError(ErrorCode::InvalidArgument,
                     "Device codename must not be empty");
  }

  const auto version = utils::parseVersion(osVersion);
  if (!version) {
    return std::unexpected(version.error());
  }

  Json request = kRequestTemplate;
  request["d"] = std::string(device) + version->region_full;
  request["c"] = version->android_v;
  request["bv"] = detectRequestBv(*version);
  request["v"] = version->full_ver;

  const auto response = requestForUpdate(
      request.dump(-1, ' ', false, Json::error_handler_t::replace));
  if (!response) {
    return std::unexpected(response.error());
  }
  return parseUpdateInfo(*response);
}

core::Result<std::filesystem::path>
downloadFirmware(const core::UpdateInfo &info,
                 const std::filesystem::path &directory,
                 const ProgressCallback &onProgress) {
  if (info.mirror_list.empty()) {
    return makeError(ErrorCode::NoUpdateFound,
                     "The update info contains no download mirrors");
  }

  std::error_code errorCode;
  std::filesystem::create_directories(directory, errorCode);
  if (errorCode) {
    return makeError(ErrorCode::IoError,
                     std::format("Cannot create directory '{}': {}",
                                 directory.string(), errorCode.message()));
  }

  const std::string url = std::format("{}/{}/{}", info.mirror_list.front(),
                                      info.latest_rom, info.file_name);
  const std::filesystem::path target =
      directory / std::format("{}_{}.zip", info.device, info.latest_rom);

  const auto downloaded = downloadToFile(url, target, onProgress);
  if (!downloaded) {
    return std::unexpected(downloaded.error());
  }
  return target;
}

} // namespace mitool::services
