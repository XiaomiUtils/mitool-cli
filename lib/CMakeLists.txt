add_library(mitool_lib STATIC)

target_sources(mitool_lib
  PUBLIC FILE_SET mitool_public_modules TYPE CXX_MODULES
  FILES
    mitool/core/error.ixx
    mitool/core/types.ixx

    mitool/services/firmware_service.ixx
    mitool/services/install_service.ixx

    mitool/utils/hash_verifier.ixx
    mitool/utils/version_parser.ixx

  PRIVATE
    mitool/services/firmware_service.cc
    mitool/services/install_service.cc

    mitool/utils/hash_verifier.cc
    mitool/utils/version_parser.cc
)

target_link_libraries(mitool_lib
  PRIVATE openssl::openssl
  PRIVATE CURL::libcurl
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE re2::re2)

target_compile_features(mitool_lib PUBLIC cxx_std_23)
