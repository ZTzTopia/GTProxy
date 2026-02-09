---
description: Extract non-obvious learnings from session to AGENTS.md files to build codebase understanding
---

Extract non-obvious learnings from session to AGENTS.md files to build GTProxy codebase understanding.

## AGENTS.md File Hierarchy

AGENTS.md files exist at multiple levels. When a file is read, all parent AGENTS.md files load automatically:

```
/AGENTS.md                      # Project-wide learnings (build, architecture, packet system)
  └─ scripts/AGENTS.md          # Lua scripting API and patterns
```

## What Counts as a Learning (GTProxy-Specific)

Architecture & Connections:

- Packet registration order dependencies (e.g., packet ID maps before structs)
- ENet peer lifecycle relationships between client/server connections
- Event listener order effects on packet processing pipeline
- ByteStream/Payload interactions in packet encoding/decoding
- World state synchronization with incoming packets

Execution Paths:

- When packet `read()` vs `write()` is called in proxy flow
- Event dispatcher callback execution order across multiple scripts
- Lua script loading order and initialization dependencies
- Scheduler task cancellation behavior during connection events
- Packet modification tracking across event handlers

Configuration & Environment:

- Conan profile requirements for specific platforms
- CMake flags affecting ENet behavior (e.g., threading)
- Lua sol2 version constraints for specific bindings
- GTProxy config defaults that affect packet handling
- Web server port conflicts or TLS certificate requirements

Debugging Breakthroughs:

- Packet ID mismatches causing decode failures
- ENet packet fragmentation with large game packets
- TextParse encoding differences between C++ and Lua
- Memory ownership issues with ByteStream views
- Event handler leaks or duplicate registrations

API/Tool Quirks:

- `ctx:parse()` vs `ctx:parse_packet()` alias behavior
- `send.to_client()` requires packet modification before re-encoding
- Scheduler periodic task return value semantics (true=continue, false=stop)
- Item database lookup by ID vs name behavior differences

Build/Test:

- Conan package cache issues requiring `conan remove '*'`
- Google Test fixture setup for packet serialization tests
- Submodule initialization for ENet/cpp-httplib
- CMake target dependencies affecting link order
- Platform-specific test failures (e.g., socket binding on macOS)

What NOT to Include:

- Documentation already in AGENTS.md files
- Standard C++23, CMake, Conan, or ENet behavior
- Obvious packet structure definitions from comments
- Verbose explanations of well-documented APIs
- Session-specific debugging logs or temporary workarounds

Process:

1. Review Session: Identify discoveries, multi-attempt fixes, unexpected connections
2. Determine Scope: Which GTProxy component does the learning affect?
3. Check Existing: Read relevant AGENTS.md files to avoid duplicates
4. Create/Update: Add 1-3 concise lines per learning at the appropriate level
5. Reference: Link between related files if learning spans multiple areas

Summarize which AGENTS.md files were created/updated and learning count per file.

$ARGUMENTS
