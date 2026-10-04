#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <memory>
#include <print>
#include <string>

#include <CLI/CLI.hpp>

import mitool.core.error;
import mitool.core.types;
import mitool.services.firmware;

namespace {

constexpr const char *kDownloadDirectory = "rom_downloads";
constexpr int kProgressBarWidth = 20;
constexpr double kProgressUpdateIntervalSec = 0.1;

struct FindOptions {
  std::string device;
  std::string os_version;
  bool download = false;
};

[[noreturn]] void exitWithError(const mitool::core::Error &error) {
  std::println(stderr, "[ERROR] {}", error.message);
  std::exit(1);
}

std::string humanSize(std::int64_t bytes) {
  constexpr std::int64_t kilobyte = 1024;
  constexpr std::int64_t megabyte = kilobyte * 1024;
  constexpr std::int64_t gigabyte = megabyte * 1024;

  if (bytes >= gigabyte) {
    return std::format("{:.2f} Gb", static_cast<double>(bytes) / gigabyte);
  }
  if (bytes >= megabyte) {
    return std::format("{:.1f} Mb", static_cast<double>(bytes) / megabyte);
  }
  if (bytes >= kilobyte) {
    return std::format("{:.1f} Kb", static_cast<double>(bytes) / kilobyte);
  }
  return std::format("{} b", bytes);
}

class ProgressPrinter {
public:
  void update(std::int64_t downloaded, std::int64_t total) {
    using Clock = std::chrono::steady_clock;
    const auto now = Clock::now();

    if (std::chrono::duration<double>(now - last_update_).count() <
            kProgressUpdateIntervalSec &&
        downloaded < total) {
      return;
    }
    last_update_ = now;

    const int percent =
        total > 0 ? static_cast<int>((downloaded * 100) / total) : 0;

    const double elapsed =
        std::chrono::duration<double>(now - start_time_).count();
    const double speedMbps = elapsed > 0 ? static_cast<double>(downloaded) /
                                               elapsed / (1024.0 * 1024.0)
                                         : 0.0;

    int filled = 0;
    if (total > 0) {
      filled = static_cast<int>(std::round(static_cast<double>(downloaded) /
                                           static_cast<double>(total) *
                                           kProgressBarWidth));
    }
    filled = std::min(filled, kProgressBarWidth);

    std::string bar = "[";
    for (int i = 0; i < kProgressBarWidth; ++i) {
      if (i < filled - 1) {
        bar += '=';
      } else if (i == filled - 1) {
        bar += '>';
      } else {
        bar += ' ';
      }
    }
    bar += ']';

    std::print("\r{} {:>3}% {:>5.1f} mb/s ({} / {})\033[K", bar, percent,
               speedMbps, humanSize(downloaded), humanSize(total));
    std::fflush(stdout);
  }

private:
  std::chrono::steady_clock::time_point start_time_{
      std::chrono::steady_clock::now()};
  std::chrono::steady_clock::time_point last_update_{};
};

void printUpdateInfo(const mitool::core::UpdateInfo &info) {
  constexpr const char *separator =
      "├─────────────────────────────────────────────────────────────────";

  std::println(
      "┌─────────────────────────────────────────────────────────────────");
  std::println("│  MIUI OTA — Update Information");
  std::println("{}", separator);
  std::println("│  Device          :  {}", info.device);
  std::println("│  Current ROM     :  {}", info.current_rom);
  std::println("│  Latest ROM      :  {}", info.latest_rom);
  std::println("│  Android version :  {}", info.android_ver);
  std::println("│  File size       :  {}", info.file_size);
  std::println("│  Applicable from :  {}", info.applicable_from);
  std::println("│  MD5             :  {}", info.md5);
  std::println("{}", separator);
  std::println("│  Changelog:");
  for (std::size_t i = 0; i < info.changelog.size(); ++i) {
    std::println("│    {}. {}", i + 1, info.changelog[i]);
  }
  std::println(
      "└─────────────────────────────────────────────────────────────────");
}

void downloadUpdate(const mitool::core::UpdateInfo &info) {
  ProgressPrinter progress;
  const auto downloaded = mitool::services::downloadFirmware(
      info, kDownloadDirectory,
      [&progress](std::int64_t now, std::int64_t total) {
        progress.update(now, total);
      });

  std::println("");
  if (!downloaded) {
    exitWithError(downloaded.error());
  }
  std::println("[INFO] Successfully downloaded the file to {}",
               downloaded->string());
}

void runFind(FindOptions options) {
  std::ranges::transform(options.os_version, options.os_version.begin(),
                         [](unsigned char ch) { return std::toupper(ch); });
  std::ranges::transform(options.device, options.device.begin(),
                         [](unsigned char ch) { return std::tolower(ch); });

  const auto info =
      mitool::services::findLatestUpdate(options.device, options.os_version);
  if (!info) {
    exitWithError(info.error());
  }
  printUpdateInfo(*info);

  if (options.download) {
    downloadUpdate(*info);
  }
}

} // namespace

namespace mitool::app {

void registerFindCommand(CLI::App &app) {
  auto *findCmd = app.add_subcommand(
      "find", "Find the latest available firmware update for a device");

  auto options = std::make_shared<FindOptions>();

  findCmd
      ->add_option("--os-version", options->os_version,
                   "Specify the OS version currently installed on the device")
      ->required();
  findCmd
      ->add_option("--device", options->device, "Specify the device codename")
      ->required();
  findCmd->add_flag("--download", options->download,
                    "Download the latest firmware update for the specified "
                    "device");

  findCmd->callback([options]() { runFind(*options); });
}

} // namespace mitool::app
