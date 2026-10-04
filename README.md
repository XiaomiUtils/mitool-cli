# MiTool-CLI

A fast, cross-platform CLI utility for searching, downloading, and flashing Xiaomi firmware.

![C++](https://img.shields.io/badge/C%2B%2B-20-blue)
![Platform](https://img.shields.io/badge/platform-windows%20%7C%20linux-brightgreen)
[![License: GPLv3](https://img.shields.io/badge/license-GPLv3-green)](https://opensource.org/license/gpl-3.0)
![GitHub Repo stars](https://img.shields.io/github/stars/XiaomiUtils/mitool-cli?style=social)

## Features

- Search firmware by device codename and OS version
- Cross-platform: Windows and Linux
- Simple one-command build with Conan + CMake

**Planned:**
- Download firmware directly from Xiaomi servers
- Flash firmware to device via fastboot/ADB

## Usage

```bash
mitool find --os-version <version> --device <codename> [--download]
mitool utils --parse-version <version>
mitool utils --verify-md5 <file> <md5>
mitool install --file <firmware.zip> [--md5 <md5>] [--mode auto|recovery|fastboot]
```

**Example:**
```bash
mitool find --os-version OS2.0.5.0.VMUMIXM --device xun
```

## Commands

| Command | Argument | Description |
|---------|----------|-------------|
| `find` | `--os-version` | OS version currently installed on the device |
| `find` | `--device` | Device codename |
| `find` | `--download` | Download the found firmware to `rom_downloads/` |
| `utils` | `--parse-version` | Parse a firmware version string |
| `utils` | `--verify-md5` | Check a file against an expected MD5 (`<file> <md5>`) |
| `install` | `--file` | Path to the firmware file |
| `install` | `--md5` | Expected MD5 of the file (optional) |
| `install` | `--mode` | `auto` (default), `recovery` or `fastboot` — flashing itself is not implemented yet |

## Build

**Requirements:** C++20 compiler, [Conan](https://conan.io), CMake

```bash
# Install Conan
pip install conan

# Detect build profile
conan profile detect

# Install dependencies
conan install . -of=build --build=missing

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
cmake --build build
```

The binary will be available at `build/bin/mitool`.

## License

Distributed under the [GNU GPLv3](https://opensource.org/license/gpl-3.0) license.
