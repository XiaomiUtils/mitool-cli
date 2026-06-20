#include <CLI/CLI.hpp>
#include <fmt/base.h>

static constexpr const char *ASCII_ART =
    " __  __ ___ _____ ___   ___  _           ____ _     ___ \n"
    "|  \\/  |_ _|_   _/ _ \\ / _ \\| |         / ___| |   |_ _|\n"
    "| |\\/| || |  | || | | | | | | |   _____| |   | |    | | \n"
    "| |  | || |  | || |_| | |_| | |__|_____| |___| |___ | | \n"
    "|_|  |_|___| |_| \\___/ \\___/|_____|     \\____|_____|___|\n";

void registrateFindcommand(CLI::App &app);

int main(int argc, char *argv[]) {
  fmt::println(ASCII_ART);

  CLI::App app{"A simple utility for finding, downloading, and installing "
               "firmware updates"};

  registrateFindcommand(app);

  CLI11_PARSE(app, argc, argv);

  return 0;
}
