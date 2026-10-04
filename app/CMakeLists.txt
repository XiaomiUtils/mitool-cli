add_executable(mitool)

target_sources(mitool
  PRIVATE
    main.cc
    commands/find_command.cc
    commands/install_command.cc
    commands/utils_command.cc
)

target_link_libraries(mitool
  PRIVATE CLI11::CLI11
  PRIVATE mitool_lib
)

target_compile_features(mitool PUBLIC cxx_std_23)
