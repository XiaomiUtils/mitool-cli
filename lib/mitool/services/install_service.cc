module;

#include <expected>
#include <filesystem>
#include <format>
#include <string_view>

module mitool.services.install;

import mitool.core.error;
import mitool.utils.hash_verifier;

namespace {

std::string_view toString(mitool::services::InstallMode mode) {
  using mitool::services::InstallMode;
  switch (mode) {
  case InstallMode::Auto:
    return "auto";
  case InstallMode::Recovery:
    return "recovery";
  case InstallMode::Fastboot:
    return "fastboot";
  }
  return "unknown";
}

} // namespace

namespace mitool::services {

core::Result<void> installFirmware(const InstallRequest &request) {
  if (!std::filesystem::exists(request.firmware_path)) {
    return std::unexpected(
        core::Error{core::ErrorCode::IoError,
                    std::format("Firmware file '{}' does not exist",
                                request.firmware_path.string())});
  }

  if (!request.expected_md5.empty()) {
    const auto verified =
        utils::verifyMd5(request.firmware_path, request.expected_md5);
    if (!verified) {
      return std::unexpected(verified.error());
    }
  }

  return std::unexpected(core::Error{
      core::ErrorCode::NotImplemented,
      std::format("Installation in '{}' mode is not implemented yet",
                  toString(request.mode))});
}

} // namespace mitool::services
