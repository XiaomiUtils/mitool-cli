#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <print>
#include <string>

#include <CLI/CLI.hpp>

import mitool.core.error;
import mitool.services.install;

namespace {

struct InstallOptions {
  std::string file;
  std::string md5;
  mitool::services::InstallMode mode = mitool::services::InstallMode::Auto;
};

void runInstall(const InstallOptions &options) {
  const mitool::services::InstallRequest request{
      .firmware_path = options.file,
      .expected_md5 = options.md5,
      .mode = options.mode,
  };

  const auto installed = mitool::services::installFirmware(request);
  if (!installed) {
    std::println(stderr, "[ERROR] {}", installed.error().message);
    std::exit(1);
  }
  std::println("[INFO] Firmware installed successfully");
}

} // namespace

namespace mitool::app {

void registerInstallCommand(CLI::App &app) {
  using mitool::services::InstallMode;

  auto *installCmd =
      app.add_subcommand("install", "Install a firmware package on a device");

  auto options = std::make_shared<InstallOptions>();

  const std::map<std::string, InstallMode> modes{
      {"auto", InstallMode::Auto},
      {"recovery", InstallMode::Recovery},
      {"fastboot", InstallMode::Fastboot},
  };

  installCmd->add_option("--file", options->file, "Path to the firmware file")
      ->required();
  installCmd->add_option("--md5", options->md5,
                         "Expected MD5 of the firmware file (optional)");
  installCmd->add_option("--mode", options->mode, "Installation mode")
      ->transform(CLI::CheckedTransformer(modes, CLI::ignore_case));

  installCmd->callback([options]() { runInstall(*options); });
}

} // namespace mitool::app
