add_executable(xmota
  main.cc
 )

target_link_libraries(xmota
  PRIVATE openssl::openssl
  PRIVATE cpr::cpr
  PRIVATE nlohmann_json::nlohmann_json
 )