#pragma once

#include <string>
#include <cstdint>

struct MiuiVersion {
  char os_ver[3];
  uint16_t major;
  uint16_t minor;
  uint16_t patch;
  char android_letter;
  char device_letter[3];
  char region_letter[3];
  char carier[3];
};

struct DeviceData {
  MiuiVersion version;
  std::string region_full;
  double codebase;
};