#include <cstdio>
#include <cstdlib>
#include <memory>
#include <print>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

import mitool.core.error;
import mitool.core.types;
import mitool.utils.hash_verifier;
import mitool.utils.version_parser;

namespace {

struct UtilsOptions {
  std::string version;
  std::vector<std::string> md5_args; // {файл, ожидаемый MD5}
};

[[noreturn]] void exitWithError(const mitool::core::Error &error) {
  std::println(stderr, "[ERROR] {}", error.message);
  std::exit(1);
}

std::string getOsName(const std::string &osName) {
  return (osName == "OS") ? "HyperOS" : "MIUI";
}

void printVersionInfo(const mitool::core::VersionData &data) {
  std::println("OS Name:       {}", getOsName(data.os_name));
  std::println("OS Ver:        {}", data.os_ver);
  std::println("Android:       {:.1f} ({})", data.android_v, data.android_l);
  std::println("Region:        {} ({})", data.region, data.region_full);
  std::println("Device:        {}", data.device);
  std::println("SOTA:          {}", data.sota);
  std::println("Version:       {}", data.full_ver);
}

void runParseVersion(const std::string &version) {
  const auto parsed = mitool::utils::parseVersion(version);
  if (!parsed) {
    exitWithError(parsed.error());
  }
  printVersionInfo(*parsed);
}

void runVerifyMd5(const std::vector<std::string> &args) {
  const auto verified = mitool::utils::verifyMd5(args[0], args[1]);
  if (!verified) {
    exitWithError(verified.error());
  }
  std::println("[INFO] MD5 matches: {}", args[1]);
}

} // namespace

namespace mitool::app {

void registerUtilsCommand(CLI::App &app) {
  auto *utilsCmd =
      app.add_subcommand("utils", "Helper tools: version parser, hash check");

  auto options = std::make_shared<UtilsOptions>();

  utilsCmd->add_option("--parse-version", options->version,
                       "Parse a firmware version string and show its details");
  utilsCmd
      ->add_option("--verify-md5", options->md5_args,
                   "Check a file against an expected MD5: <file> <md5>")
      ->expected(2);
  utilsCmd->require_option(1);

  utilsCmd->callback([options]() {
    if (!options->version.empty()) {
      runParseVersion(options->version);
    } else {
      runVerifyMd5(options->md5_args);
    }
  });
}

} // namespace mitool::app
