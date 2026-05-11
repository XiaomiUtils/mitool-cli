#include "MiuiKeys.h"
#include "MiuiUpdate.h"

#include <cpr/cpr.h>
#include <cpr/util.h>
#include <nlohmann/json.hpp>

#include <format>
#include <string>

using json = nlohmann::json;

json requestJson = {
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

static std::string buildOsVersion(const DeviceData &data) {
  const std::string versionBlock =
      std::string(data.version.osVer) + std::to_string(data.version.major);

  std::string fullVersion;
  if (data.version.osVer[0] == 'V') {
    fullVersion = "MIUI-";
  }

  fullVersion +=
      std::vformat("{}.{}.{}.{}.{}{}{}{}",
                   std::make_format_args(
                       versionBlock, data.version.minor, data.version.patch,
                       data.version.build, data.version.androidLetter,
                       data.version.deviceLetter, data.version.regionLetter,
                       data.version.carier));

  return fullVersion;
}

std::string MiuiUpdater::requestForUpdate(const std::string &requestData) {
  auto encrypted = aes128cbcEncrypt(requestJson.dump());
  const auto encoded = base64Encrypt(encrypted);

  // URL encode
  const auto escaped = cpr::util::urlEncode(encoded);

  // POST body
  const std::string postBody = "q=" + (std::string)escaped + "&t=&s=1";

  // Request
  auto response = cpr::Post(
      cpr::Url{MIUI_OTA_URL},

      cpr::Header{{"clientId", "MITUNES"},
                  {"Connection", "Keep-Alive"},
                  {"Accept-Encoding", "identity"},
                  {"Content-Type", "application/x-www-form-urlencoded"}},

      cpr::UserAgent{"MiTunes_UserAgent_v3.0"},

      cpr::Body{postBody});

  // Network error
  if (response.error.code != cpr::ErrorCode::OK) {
    return "N/A";
  }

  // Response decode
  const auto decodedResponse = cpr::util::urlDecode(response.text);

  auto decryptedBase64 = base64Decrypt((std::string)decodedResponse);

  return aes128cbcDecrypt(decryptedBase64);
}

std::string MiuiUpdater::getLatestUpdate(const std::string &device,
                                         const std::string &version) {

  if (!parseVersion(version))
    return "";

  requestJson["d"] = device + data.regionFull;
  requestJson["c"] = data.codebase;
  requestJson["bv"] = data.bv;
  requestJson["v"] = buildOsVersion(data);

  return requestForUpdate(requestJson.dump());
}
