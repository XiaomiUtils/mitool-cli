#pragma once

#include <curl/curl.h>

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// TODO:
// 1. Add progress bar support
// 2. Add header printing functionality (e.g. file size, content type, etc.)
// 3. Rewrite the logic (optional) to remove the inclusion of <curl/curl.h>

class Downloader {
private:
  CURL *curl;

private:
  void initCurl();
  void cleanupCurl();

  static size_t writeCallback(void *contents, size_t size, size_t nmemb,
                              void *userp);

public:
  Downloader();
  ~Downloader();

  bool downloadFile(const std::string &url, const std::string &outputPath);
  bool downloadFile(const std::string &url, const std::string &filename, const fs::path &outputPath);
};