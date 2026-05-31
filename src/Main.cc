#include "MiuiUpdate/MiuiUpdate.h"

#include <CLI/CLI.hpp>
#include <fmt/base.h>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

using json = nlohmann::json;

static constexpr const char *ASCII_ART =
    "__  __ ___ _____ ___   ___  _           ____ _     ___ \n"
    "|  \\/  |_ _|_   _/ _ \\ / _ \\| |         / ___| |   |_ _|\n"
    "| |\\/| || |  | || | | | | | | |   _____| |   | |    | | \n"
    "| |  | || |  | || |_| | |_| | |__|_____| |___| |___ | | \n"
    "|_|  |_|___| |_| \\___/ \\___/|_____|     \\____|_____|___|\n";

struct LatestUpdateInfo {
  std::string device;
  std::string currentRom;
  std::string latestRom;
  std::string androidVer;
  std::string fileSize;
  std::string applicableFrom;
  std::string md5;
  std::vector<std::string> changelog;
};

static std::string safeGet(const json &obj, const std::string &key) {
  if (!obj.is_null() && obj.contains(key) && obj.at(key).is_string()) {
    return obj.at(key).get<std::string>();
  }
  return "N/A";
}

static LatestUpdateInfo parseUpdateInfo(const std::string &jsonStr) {
  const json data = json::parse(jsonStr);

  const json &latestRom = data["LatestRom"];
  const json &currentRom = data["CurrentRom"];
  const json &incrementRom = data["IncrementRom"];

  std::vector<std::string> changelog;
  if (latestRom.contains("changelog")) {
    const json &cl = latestRom.at("changelog");
    if (cl.contains("System") && cl.at("System").contains("txt")) {
      const json &txt = cl.at("System").at("txt");
      if (txt.is_array()) {
        for (const auto &entry : txt) {
          if (entry.is_string()) {
            changelog.push_back(entry.get<std::string>());
          }
        }
      }
    }
  }

  if (changelog.empty()) {
    changelog.push_back("N/A");
  }

  return LatestUpdateInfo{
      .device = safeGet(latestRom, "device"),
      .currentRom = safeGet(currentRom, "version"),
      .latestRom = safeGet(latestRom, "version"),
      .androidVer = safeGet(latestRom, "codebase"),
      .fileSize = safeGet(latestRom, "filesize"),
      .applicableFrom = safeGet(incrementRom, "versionForApply"),
      .md5 = safeGet(latestRom, "md5"),
      .changelog = changelog,
  };
}

static void printUpdateInfo(const LatestUpdateInfo &info) {
  static constexpr const char *SEP =
      "├─────────────────────────────────────────────────────────────────";

  fmt::println(
      "┌─────────────────────────────────────────────────────────────────");
  fmt::println("│  MIUI OTA — Update Information");
  fmt::println("{}", SEP);
  fmt::println("│  Device          :  {}", info.device);
  fmt::println("│  Current ROM     :  {}", info.currentRom);
  fmt::println("│  Latest ROM      :  {}", info.latestRom);
  fmt::println("│  Android version :  {}", info.androidVer);
  fmt::println("│  File size       :  {}", info.fileSize);
  fmt::println("│  Applicable from :  {}", info.applicableFrom);
  fmt::println("│  MD5             :  {}", info.md5);
  fmt::println("{}", SEP);
  fmt::println("│  Changelog:");
  for (std::size_t i = 0; i < info.changelog.size(); ++i) {
    fmt::println("│    {}. {}", i + 1, info.changelog[i]);
  }
  fmt::println(
      "└─────────────────────────────────────────────────────────────────");
}

int main(int argc, char *argv[]) {
  std::string osVersion;
  std::string deviceCodename;
  bool noBannerFlag{false};

  CLI::App app{"A simple utility for finding, downloading, and installing "
               "firmware updates"};

  app.add_option("--os-version", osVersion,
                 "Specify the OS version currently installed on the device")->required();
  app.add_option("--device", deviceCodename, "Specify the device codename")->required();
  app.add_flag("--no-banner", noBannerFlag, "Disable ASCII startup banner");

  CLI11_PARSE(app, argc, argv);

  if (!noBannerFlag) {
    fmt::println("{}", ASCII_ART);
  }

  std::transform(osVersion.begin(), osVersion.end(), osVersion.begin(), ::toupper);
  std::transform(deviceCodename.begin(), deviceCodename.end(), deviceCodename.begin(), ::tolower);

  MiuiUpdater updater;
  const std::string jsonResponse =
      updater.getLatestUpdate(deviceCodename, osVersion);

  LatestUpdateInfo info;
  try {
    info = parseUpdateInfo(jsonResponse);
  } catch (const json::parse_error &e) {
    fmt::println("[ERROR] Failed to parse server response: {}", e.what());
    return 1;
  } catch (const json::out_of_range &e) {
    fmt::println("[ERROR] Unexpected response structure: {}", e.what());
    return 1;
  }

  printUpdateInfo(info);

  return 0;
}
