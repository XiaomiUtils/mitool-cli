add_executable(xmota
  main.cc
 )

target_link_libraries(xmota
  PRIVATE cpr::cpr
  PRIVATE nlohmann_json::nlohmann_json
 )