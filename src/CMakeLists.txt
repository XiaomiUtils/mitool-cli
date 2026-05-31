add_library(MiuiUpdate
  MiuiUpdate/MiuiCrypto.cc
  MiuiUpdate/MiuiFindUpdate.cc
  MiuiUpdate/MiuiParseVersion.cc
)

add_executable(mitool
  Main.cc
)

target_link_libraries(MiuiUpdate
  PRIVATE openssl::openssl
  PRIVATE CURL::libcurl
  PRIVATE nlohmann_json::nlohmann_json
 )

target_link_libraries(mitool
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE fmt::fmt
  PRIVATE CLI11::CLI11
  PRIVATE MiuiUpdate
 )
