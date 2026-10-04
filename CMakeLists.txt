cmake_minimum_required(VERSION 3.30)

project(MiTool-CLI
  LANGUAGES CXX
  VERSION 0.0.1
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CACERT_URL "https://curl.se/ca/cacert.pem")
set(CACERT_FILE "${CMAKE_BINARY_DIR}/bin/cacert.pem")

if(NOT EXISTS "${CACERT_FILE}")
    message(STATUS "Downloading cacert.pem...")
    file(DOWNLOAD "${CACERT_URL}" "${CACERT_FILE}"
        STATUS CACERT_DOWNLOAD_STATUS
        TIMEOUT 30
    )
    list(GET CACERT_DOWNLOAD_STATUS 0 CACERT_DOWNLOAD_STATUS_CODE)
    if(NOT CACERT_DOWNLOAD_STATUS_CODE EQUAL 0)
        list(GET CACERT_DOWNLOAD_STATUS 1 CACERT_DOWNLOAD_ERROR)
        message(FATAL_ERROR "Failed to download cacert.pem: ${CACERT_DOWNLOAD_ERROR}")
    endif()
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(CLI11 REQUIRED)
find_package(RE2 REQUIRED)

add_subdirectory(lib)
add_subdirectory(app)
