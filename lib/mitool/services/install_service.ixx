module;

#include <filesystem>
#include <string>

export module mitool.services.install;

import mitool.core.error;

export namespace mitool::services {

enum class InstallMode {
  Auto,
  Recovery,
  Fastboot,
};

struct InstallRequest {
  std::filesystem::path firmware_path;
  std::string expected_md5; // empty — hash check skipped
  InstallMode mode = InstallMode::Auto;
};

// TODO: firmware itself (Fastboot / Recovery / Mi Assistant) is not 
// implemented yet — currently, only input validation is performed.
core::Result<void> installFirmware(const InstallRequest &request);

} // namespace mitool::services
