add_executable(mitool
  Main.cc
  FindCommand.cc
)

target_link_libraries(mitool
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE fmt::fmt
  PRIVATE CLI11::CLI11
  PRIVATE CURL::libcurl
  PRIVATE MiuiUpdate
  PRIVATE Downloader
)
