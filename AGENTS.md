# GTProxy - Agent Guidelines

## Project Overview

GTProxy is a C++23 network proxy for Growtopia that enables packet debugging and modification.
Built with CMake 3.24+, Conan 2.0+ package manager, and Google Test for testing.
Supports Windows, GNU/Linux, and macOS.

## Build Commands

### Prerequisites

```bash
pip install "conan>2.0"
git clone --recurse-submodules https://github.com/ZTzTopia/GTProxy.git
```

### Configure and Build

```bash
# Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Platform Notes

- **MSVC (Windows):** Uses `/EHsc` for exception handling
- **GCC/Clang:** Uses `-fexceptions`
- Conan packages are automatically fetched during CMake configure

## Test Commands

Tests use Google Test framework. Test executable: `build/tests/GTProxy_tests`

```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run all tests directly
./build/tests/GTProxy_tests

# Run a single test by name
./build/tests/GTProxy_tests --gtest_filter=ByteStreamTest.WriteAndReadBasicTypes

# Run all tests in a suite
./build/tests/GTProxy_tests --gtest_filter=ByteStreamTest.*

# List all available tests
./build/tests/GTProxy_tests --gtest_list_tests
```

## Project Structure

```
src/
├── core/           # Application core, config, logging, scheduler, web server
├── network/        # ENet client/server connections
├── packet/         # Packet types, encoding/decoding, event handling
│   ├── game/       # Game-specific packet handlers
│   └── message/    # Message packet types
├── world/          # World entity and state management
├── player/         # Player entity and state management
├── command/        # In-proxy command system
│   └── commands/   # Individual command implementations
├── scripting/      # Lua scripting engine (sol2)
│   └── bindings/   # Lua API bindings
├── event/          # Event dispatcher system
└── utils/          # Utilities (ByteStream, TextParse, hash, random)

lib/                # External libs (enet, cpp-httplib) as Git submodules
tests/              # Google Test unit tests
scripts/            # Lua scripts for extending proxy
resources/          # Runtime resources (SSL certs)
```

## Code Style Guidelines

### Naming Conventions

| Element           | Style                | Example                                   |
|-------------------|----------------------|-------------------------------------------|
| Classes/Structs   | PascalCase           | `Core`, `ByteStream`, `Config`            |
| Functions/Methods | snake_case           | `get_config`, `is_connected`              |
| Member variables  | snake_case_          | `config_`, `running_`, `peer_`            |
| Local variables   | snake_case           | `sleep_duration`, `packet_data`           |
| Enums (unscoped)  | SCREAMING_SNAKE_CASE | `NET_MESSAGE_UNKNOWN`, `PACKET_FLAG_NONE` |
| Enum classes      | PascalCase values    | `Result::Success`, `Result::Failed`       |
| Namespaces        | lowercase            | `core`, `network`, `packet`               |
| Template params   | PascalCase           | `LengthType`, `Func`                      |

### Header Guards

Always use `#pragma once` instead of include guards.

### Include Order

1. Own header (in .cpp files)
2. Standard library headers (`<vector>`, `<string>`, `<memory>`)
3. External library headers (`<enet/enet.h>`, `<spdlog/spdlog.h>`)
4. Project headers with relative paths (`"../packet/packet_types.hpp"`)

### Formatting

- **Indentation:** 4 spaces (no tabs)
- **Braces:** Same line for class/function declarations
- **Brace initialization:** Prefer `Type name{ value }` over `Type name = value`
- **Line length:** No strict limit, but keep readable

```cpp
// Brace initialization examples
const auto begin{ static_cast<const std::byte*>(ptr) };
ByteStream bs{};
int port{ 16999 };
```

### Types and Attributes

- Use `[[nodiscard]]` on getters and functions where ignoring return is likely a bug
- Use `final` on classes not intended for inheritance
- Use `override` on all virtual method overrides
- Prefer `std::byte` over `char`/`unsigned char` for binary data
- Prefer `std::span<const std::byte>` for non-owning byte ranges
- Prefer `std::string_view` for non-owning string parameters
- Use explicit integer types: `std::uint8_t`, `std::uint16_t`, `std::uint32_t`

### Error Handling

- Throw `std::runtime_error` with brace initialization for fatal errors
- Use early returns for error conditions
- Use `std::optional` for fallible operations that may not have a value
- Return `bool` for simple success/failure operations

```cpp
if (!host_) {
    throw std::runtime_error{ "Failed to create host" };
}

if (data.size() < 4) {
    spdlog::warn("Malformed packet (size {})", data.size());
    return false;
}

const auto decoded{ decoder_.decode(data) };
if (!decoded.has_value()) {
    return;
}
```

### Logging

Use spdlog for all logging. Available levels: `trace`, `debug`, `info`, `warn`, `error`.

```cpp
spdlog::info("Connected to server at {}:{}", host, port);
spdlog::warn("Received malformed packet (size {})", data.size());
spdlog::error("Failed to initialize: {}", error_msg);
```

### Memory Management

- Use `std::unique_ptr` for exclusive ownership
- Use `std::shared_ptr` only when shared ownership is required
- Use references for non-owning parameters
- Store references as member variables when lifetime is guaranteed

## Creating New Packet Structures

If a required packet is not defined, follow these steps:

1.  **Define Packet ID:** Add a new enum value to `PacketId` in `src/packet/packet_id.hpp`.
2.  **Map Packet ID:** Add the mapping to `VARIANT_FUNCTION_MAP` (for variant strings) or `GAME_PACKET_MAP` (for game packets) in `src/packet/packet_id.hpp`.
3.  **Create Packet Struct:** Define a new struct in the appropriate file (e.g., `src/packet/game/world.hpp`) that inherits from `VariantPacket<PacketId::PacketName>`.
4.  **Implement Serialization:** Override `read(const Payload& payload)` for decoding and `write()` for encoding.
5.  **Register Packet:** Register the new packet in `src/packet/register_packets.hpp` using `registry.register_packet<packet::game::PacketStructName>()`.

## Lua Scripting

See [scripts/AGENTS.md](scripts/AGENTS.md) for Lua scripting guidelines. Scripts in `scripts/` are loaded automatically at startup.

## Dependencies

Managed by Conan 2.0+:

| Library     | Purpose                    |
|-------------|----------------------------|
| eventpp     | Event dispatching          |
| fmt         | String formatting          |
| glaze       | JSON serialization         |
| spdlog      | Logging                    |
| lua + sol2  | Lua scripting              |
| LibreSSL    | TLS/SSL support            |
| magic_enum  | Enum reflection            |
| gtest       | Unit testing               |

Git submodules (in `lib/`):

| Library     | Purpose                    |
|-------------|----------------------------|
| enet        | UDP networking             |
| cpp-httplib | HTTP server                |
