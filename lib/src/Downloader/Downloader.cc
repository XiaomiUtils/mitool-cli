#include <Downloader/Downloader.h>

#include <curl/curl.h>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

Downloader::Downloader() {
  initCurl();
}

Downloader::~Downloader() {
  cleanupCurl();
}

void Downloader::initCurl() {
  curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

void Downloader::cleanupCurl() {
  if (curl) {
    curl_easy_cleanup(curl);
  }
}

static void createDirectoryIfNotExists(const fs::path &path) {
  if (!fs::exists(path)) {
    fs::create_directories(path);
  }
}

bool Downloader::downloadFile(const std::string &url, const std::string &outputPath) {
  createDirectoryIfNotExists(fs::path(outputPath).parent_path());
  std::ofstream ofs(outputPath, std::ios::binary);
  if (!ofs) {
    return false;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    throw std::runtime_error(std::format("CURL error: {}", curl_easy_strerror(res)));
  }
  ofs.close();

  return res == CURLE_OK;
}

bool Downloader::downloadFile(const std::string &url, const std::string &filename, const fs::path &outputPath) {
  createDirectoryIfNotExists(outputPath);
  std::ofstream ofs((outputPath / filename).string(), std::ios::binary);
  if (!ofs) {
    return false;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      throw std::runtime_error(std::format("CURL error: {}", curl_easy_strerror(res)));
  }
  }
  ofs.close();

  return res == CURLE_OK;
}