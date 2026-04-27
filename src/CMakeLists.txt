add_executable(xmota
  main.cc
 )

target_link_libraries(xmota
  PRIVATE openssl::openssl
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE cpr::cpr
 )