module;

#include <string>
#include <vector>

export module mitool.core.types;

export namespace mitool::core {

struct DeviceData {
  std::string codename;
  std::string os_region;
  std::string os_version;
  std::string sn;
  std::string imei;
};

struct VersionData {
  std::string full_ver;
  std::string os_name;
  int os_ver;
  int major;
  int minor;
  int patch;
  std::string android_l; // letter
  double android_v;      // version
  std::string device;
  std::string region;
  std::string region_full;
  std::string carrier;
  std::string sota;
};

struct UpdateInfo {
  std::string device;
  std::string current_rom;
  std::string latest_rom;
  std::string android_ver;
  std::string file_size;
  std::string applicable_from;
  std::string md5;
  std::string file_name;
  std::vector<std::string> changelog;
  std::vector<std::string> mirror_list;
};

} // namespace mitool::core
