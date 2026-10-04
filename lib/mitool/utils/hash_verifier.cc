module;

#include <algorithm>
#include <array>
#include <cctype>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>

#include <openssl/evp.h>

module mitool.utils.hash_verifier;

import mitool.core.error;

namespace {

constexpr std::size_t kReadChunkSize = 64 * 1024;
constexpr std::size_t kMd5DigestSize = 16;

using DigestContext = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

std::string toLower(std::string_view text) {
  std::string result(text);
  std::ranges::transform(result, result.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return result;
}

} // namespace

namespace mitool::utils {

core::Result<std::string> computeMd5(const std::filesystem::path &file) {
  std::ifstream input(file, std::ios::binary);
  if (!input.is_open()) {
    return std::unexpected(
        core::Error{core::ErrorCode::IoError,
                    std::format("Cannot open file '{}'", file.string())});
  }

  DigestContext context(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
  if (!context || EVP_DigestInit_ex(context.get(), EVP_md5(), nullptr) != 1) {
    return std::unexpected(core::Error{core::ErrorCode::CryptoError,
                                       "Failed to initialise MD5 context"});
  }

  std::array<char, kReadChunkSize> buffer;
  while (input) {
    input.read(buffer.data(), buffer.size());
    const std::streamsize bytesRead = input.gcount();
    if (bytesRead > 0 &&
        EVP_DigestUpdate(context.get(), buffer.data(),
                         static_cast<std::size_t>(bytesRead)) != 1) {
      return std::unexpected(
          core::Error{core::ErrorCode::CryptoError, "MD5 update failed"});
    }
  }

  if (input.bad()) {
    return std::unexpected(core::Error{
        core::ErrorCode::IoError,
        std::format("Error while reading file '{}'", file.string())});
  }

  std::array<unsigned char, kMd5DigestSize> digest;
  unsigned int digestSize = 0;
  if (EVP_DigestFinal_ex(context.get(), digest.data(), &digestSize) != 1) {
    return std::unexpected(
        core::Error{core::ErrorCode::CryptoError, "MD5 finalisation failed"});
  }

  std::string hex;
  hex.reserve(digestSize * 2);
  for (unsigned int i = 0; i < digestSize; ++i) {
    hex += std::format("{:02x}", digest[i]);
  }
  return hex;
}

core::Result<void> verifyMd5(const std::filesystem::path &file,
                             std::string_view expected) {
  const auto actual = computeMd5(file);
  if (!actual) {
    return std::unexpected(actual.error());
  }

  if (*actual != toLower(expected)) {
    return std::unexpected(
        core::Error{core::ErrorCode::HashMismatch,
                    std::format("MD5 mismatch for '{}': expected {}, got {}",
                                file.string(), toLower(expected), *actual)});
  }
  return {};
}

} // namespace mitool::utils
