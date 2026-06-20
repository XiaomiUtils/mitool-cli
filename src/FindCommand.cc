#include <MiuiUpdate/MiuiUpdate.h>
#include <Downloader/Downloader.h>

#include <CLI/CLI.hpp>
#include <fmt/base.h>
#include <nlohmann/json.hpp>

#include <exception>
#include <string>


using json = nlohmann::json;

struct LatestUpdateInfo {
  std::string device;
  std::string currentRom;
  std::string latestRom;
  std::string androidVer;
  std::string fileSize;
  std::string applicableFrom;
  std::string md5;
  std::string fileName;
  std::vector<std::string> changelog;
  std::vector<std::string> mirrorList;
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
  const json &mirrors = data["MirrorList"];

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

  std::vector<std::string> mirrorList;
  if (mirrors.is_array()) {
    for (const auto &mirror : mirrors) {
      if (mirror.is_string()) {
        mirrorList.push_back(mirror.get<std::string>());
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
      .fileName = safeGet(latestRom, "filename"),
      .changelog = changelog,
      .mirrorList = mirrorList,
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

inline static std::string buildDownloadUrl(const LatestUpdateInfo &info) {
  return std::format("{}/{}/{}", info.mirrorList.empty() ? "" : info.mirrorList[0], info.latestRom, info.fileName);
}

static void downloadFile(const std::string &url, const std::string &filename) {
  Downloader downloader;
  if (!downloader.downloadFile(url, filename, "rom_downloads")) {
    fmt::println("[ERROR] Failed to download the file from {}", url);
  } else {
    fmt::println("[INFO] Successfully downloaded the file to {}", filename);
  }
}

static void runFind(std::string &deviceCodename, std::string &osVersion, bool download = false) {
  MiuiUpdater updater;
  LatestUpdateInfo info;

  std::transform(osVersion.begin(), osVersion.end(), osVersion.begin(),
                 ::toupper);
  std::transform(deviceCodename.begin(), deviceCodename.end(),
                 deviceCodename.begin(), ::tolower);

  try {
    const std::string jsonStr = updater.getLatestUpdate(deviceCodename, osVersion);
    if (jsonStr.empty()) {
      fmt::println("[ERROR] Failed to retrieve update information. Please check the device codename and OS version.");
      return;
    }
    info = parseUpdateInfo(jsonStr);
    printUpdateInfo(info);

    if (download) {
      const std::string fileName = std::format("{}_{}.zip", info.device, info.latestRom);
      const std::string downloadUrl = buildDownloadUrl(info);
      downloadFile(downloadUrl, fileName);
    }

  } catch (const std::exception &e) {
    fmt::println("[EXCEPTION] {}", e.what());
  }
}

void registrateFindcommand(CLI::App &app) {
  auto findCmd = app.add_subcommand(
      "find", "Find the latest available firmware update for a device");

  static std::string osVersion;
  static std::string device;
  static bool download = false;

  findCmd
      ->add_option("--os-version", osVersion,
                   "Specify the OS version currently installed on the device")
      ->required();
  findCmd->add_option("--device", device, "Specify the device codename")
      ->required();
  findCmd->add_flag(
      "--download", download,
      "Download the latest firmware update for the specified device"); // Not
                                                                       // implemented
                                                                       // yet

  findCmd->callback([&]() { runFind(device, osVersion, download); });
}