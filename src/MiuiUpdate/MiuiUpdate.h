#pragma once

#include <string>
#include <cstdint>
#include <vector>

struct MiuiVersion {
  char osVer[4];
  std::uint16_t major;
  std::uint16_t minor;
  std::uint16_t patch;
  std::uint16_t build;
  char androidLetter;
  char deviceLetter[3];
  char regionLetter[3];
  char carier[3];
};

struct DeviceData {
  MiuiVersion version;

  std::string regionFull;
  std::string device;

  double codebase;

  char bv[8];
};

class MiuiUpdater {
private:
  DeviceData data;

private:
  bool parseVersion(const std::string &version);

  std::vector<std::uint8_t> base64Decrypt(const std::string &data);
  std::string base64Encrypt(std::vector<std::uint8_t> &data);

  std::vector<std::uint8_t> aes128cbcEncrypt(const std::string &data);
  std::string aes128cbcDecrypt(std::vector<uint8_t> &data);

  std::string requestForUpdate(const std::string &requestData);

public:
  std::string getLatestUpdate(const std::string &device, const std::string &version);
};
