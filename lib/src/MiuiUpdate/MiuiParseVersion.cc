#include <MiuiUpdate/MiuiUpdate.h>

#include <cstdint>
#include <cstring>
#include <regex>
#include <string>
#include <unordered_map>

static double detectAndroidVersion(const char letter) {
  switch (letter) {
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

static std::string detectRegion(const char region[3]) {
  static const std::unordered_map<std::string, std::string> regionMap = {
      {"MI", "_global"},    {"EU", "_eea_global"}, {"RU", "_ru_global"},
      {"TW", "_tw_global"}, {"ID", "_id_global"},  {"IN", "_in_global"},
      {"CN", ""},
  };

  auto it = regionMap.find(std::string(region, 2));
  return it != regionMap.end() ? it->second : "";
}

static std::string detectBv(const char osVer[4], std::uint16_t major) {
  if (osVer[0] != 'V') {
    return "";
  }
  if (major <= 13) {
    return std::to_string(major);
  }
  return "";
}

bool MiuiUpdater::parseVersion(const std::string &input) {
  static const std::regex re(
      R"(^([A-Z]+)(\d+)\.(\d+)\.(\d+)\.(\d+)\.([A-Z])([A-Z])([A-Z])([A-Z]{2})(.*)$)");

  std::smatch match;
  if (!std::regex_match(input, match, re)) {
    return false;
  }

  std::strncpy(data.version.osVer, match[1].str().c_str(),
               sizeof(data.version.osVer) - 1);
  data.version.osVer[sizeof(data.version.osVer) - 1] = '\0';

  const uint16_t rawMajor = static_cast<uint16_t>(std::stoi(match[2].str()));
  data.version.major = rawMajor;
  data.version.minor = static_cast<uint16_t>(std::stoi(match[3].str()));
  data.version.patch = static_cast<uint16_t>(std::stoi(match[4].str()));
  data.version.build = static_cast<uint16_t>(std::stoi(match[5].str()));

  data.version.androidLetter = match[6].str()[0];

  // deviceLetter = match[7] + match[8] → "CD"
  const std::string deviceStr = match[7].str() + match[8].str();
  std::strncpy(data.version.deviceLetter, deviceStr.c_str(),
               sizeof(data.version.deviceLetter) - 1);
  data.version.deviceLetter[sizeof(data.version.deviceLetter) - 1] = '\0';

  // regionLetter = match[9] → "MI"
  std::strncpy(data.version.regionLetter, match[9].str().c_str(),
               sizeof(data.version.regionLetter) - 1);
  data.version.regionLetter[sizeof(data.version.regionLetter) - 1] = '\0';

  // carrier = match[10] → "XM"
  std::strncpy(data.version.carier, match[10].str().c_str(),
               sizeof(data.version.carier) - 1);
  data.version.carier[sizeof(data.version.carier) - 1] = '\0';

  // DeviceData fields
  data.codebase = detectAndroidVersion(data.version.androidLetter);
  data.regionFull = detectRegion(data.version.regionLetter);

  const std::string bv = detectBv(data.version.osVer, rawMajor);
  std::strncpy(data.bv, bv.c_str(), sizeof(data.bv) - 1);
  data.bv[sizeof(data.bv) - 1] = '\0';

  return true;
}
