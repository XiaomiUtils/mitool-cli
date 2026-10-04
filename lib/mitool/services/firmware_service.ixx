module;

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string_view>

export module mitool.services.firmware;

import mitool.core.error;
import mitool.core.types;

export namespace mitool::services {

// Called during download: bytes received and total bytes
// (total == 0 if size is unknown).
using ProgressCallback =
    std::function<void(std::int64_t downloaded, std::int64_t total)>;

// Searches for the latest available OTA firmware for the device.
core::Result<core::UpdateInfo> findLatestUpdate(std::string_view device,
                                                std::string_view osVersion);

// Downloads the firmware from info into the directory and returns the file path.
core::Result<std::filesystem::path>
downloadFirmware(const core::UpdateInfo &info,
                 const std::filesystem::path &directory,
                 const ProgressCallback &onProgress = {});

// TODO: search for Fastboot firmwares.

} // namespace mitool::services
