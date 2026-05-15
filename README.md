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
mitool --os-version <version> --device <codename>
```

**Example:**
```bash
mitool --os-version OS2.0.5.0.VMUMIXM --device xun
```

## Arguments

| Argument | Type | Description |
|----------|------|-------------|
| `--os-version` | parameter | OS version currently installed on the device |
| `--device` | parameter | Device codename |
| `--no-banner` | flag | Disable ASCII startup banner |

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
