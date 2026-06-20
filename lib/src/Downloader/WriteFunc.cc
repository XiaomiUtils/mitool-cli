#include <Downloader/Downloader.h>

#include <fstream>

size_t Downloader::writeCallback(void *contents, size_t size, size_t nmemb,
                                   void *userp) {
  std::ofstream *ofs = static_cast<std::ofstream *>(userp);
  size_t totalSize = size * nmemb;
  ofs->write(static_cast<char *>(contents), totalSize);
  return totalSize;
}