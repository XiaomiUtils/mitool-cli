#include <print>

#include <CLI/CLI.hpp>

namespace mitool::app {

void registerFindCommand(CLI::App &app);
void registerInstallCommand(CLI::App &app);
void registerUtilsCommand(CLI::App &app);

} // namespace mitool::app

namespace {

constexpr const char *kAsciiArt =
    " __  __ ___ _____ ___   ___  _           ____ _     ___ \n"
    "|  \\/  |_ _|_   _/ _ \\ / _ \\| |         / ___| |   |_ _|\n"
    "| |\\/| || |  | || | | | | | | |   _____| |   | |    | | \n"
    "| |  | || |  | || |_| | |_| | |__|_____| |___| |___ | | \n"
    "|_|  |_|___| |_| \\___/ \\___/|_____|     \\____|_____|___|\n";

} // namespace

int main(int argc, char *argv[]) {
  std::println("{}", kAsciiArt);

  CLI::App app{"A simple utility for finding, downloading, and installing "
               "firmware updates"};

  mitool::app::registerFindCommand(app);
  mitool::app::registerInstallCommand(app);
  mitool::app::registerUtilsCommand(app);

  CLI11_PARSE(app, argc, argv);

  return 0;
}
