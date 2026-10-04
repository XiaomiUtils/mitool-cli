module;

#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <unordered_map>

#include <re2/re2.h>

module mitool.utils.version_parser;

import mitool.core.error;
import mitool.core.types;

namespace {

double detectAndroidVersion(const char letter) {
  switch (letter) {
  case 'X':
    return 17.0;
  case 'W':
    return 16.0;
  case 'V':
    return 15.0;
  case 'U':
    return 14.0;
  case 'T':
    return 13.0;
  case 'S':
    return 12.0;
  case 'R':
    return 11.0;
  case 'Q':
    return 10.0;
  case 'P':
    return 9.0;
  case 'O':
    return 8.0;
  case 'N':
    return 7.0;
  case 'M':
    return 6.0;
  default:
    return 0.0;
  }
}

std::string detectRegion(std::string_view region) {
  static const std::unordered_map<std::string_view, std::string> regionMap = {
      {"MI", "_global"},    {"EU", "_eea_global"}, {"RU", "_ru_global"},
      {"TW", "_tw_global"}, {"ID", "_id_global"},  {"IN", "_in_global"},
      {"CN", ""},
  };

  const auto it = regionMap.find(region);
  return it != regionMap.end() ? it->second : "";
}

std::string detectBv(std::string_view osName, int osVer) {
  if (osName == "OS") {
    return (osVer == 1) ? "MIUI-V" : "OS";
  }
  if (osVer <= 13) {
    return "MIUI-V";
  }
  return "";
}

} // namespace

namespace mitool::utils {

core::Result<core::VersionData> parseVersion(std::string_view input) {
  static const re2::RE2 kVersionPattern(
      R"(^([A-Z]+)(\d+)\.(\d+)\.(\d+)\.(\d+)\.([A-Z])([A-Z]{2})([A-Z]{2})([A-Z]{2})(?:\.([A-Z]\d+))?$)");

  if (!kVersionPattern.ok()) {
    return std::unexpected(
        core::Error{core::ErrorCode::ParseError,
                    "Invalid version regex: " + kVersionPattern.error()});
  }

  core::VersionData data;

  if (!re2::RE2::FullMatch(input, kVersionPattern, &data.os_name, &data.os_ver,
                           &data.major, &data.minor, &data.patch,
                           &data.android_l, &data.device, &data.region,
                           &data.carrier, &data.sota)) {
    return std::unexpected(core::Error{
        core::ErrorCode::ParseError,
        std::format("Unrecognized version format: '{}' (expected e.g. "
                    "V12.0.16.0.QCDMIXM or OS2.0.3.0.PXCMIXM)",
                    input)});
  }

  data.region_full = detectRegion(data.region);
  data.android_v = detectAndroidVersion(data.android_l[0]);

  const std::string bv = detectBv(data.os_name, data.os_ver);
  data.full_ver = std::format(
      "{}{}.{}.{}.{}.{}{}{}{}", bv, data.os_ver, data.major, data.minor,
      data.patch, data.android_l, data.device, data.region, data.carrier);

  if (!data.sota.empty()) {
    data.full_ver += "." + data.sota;
  }

  return data;
}

} // namespace mitool::utils
