add_library(MiuiUpdate
  src/MiuiUpdate/MiuiCrypto.cc
  src/MiuiUpdate/MiuiFindUpdate.cc
  src/MiuiUpdate/MiuiParseVersion.cc
)

add_library(Downloader
  src/Downloader/Downloader.cc
  src/Downloader/WriteFunc.cc
)

target_include_directories(MiuiUpdate
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(Downloader
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(MiuiUpdate
  PRIVATE openssl::openssl
  PRIVATE CURL::libcurl
  PRIVATE nlohmann_json::nlohmann_json
)

target_link_libraries(Downloader
  PRIVATE CURL::libcurl
)