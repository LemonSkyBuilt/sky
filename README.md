# Trading Platform

This repository contains the initial monorepo skeleton for a trading platform built with `Java + C++`.

## Java Workspace

- IDE: `IntelliJ IDEA`
- JDK: `Temurin 21`
- Build tool: `Maven`
- Project root to open in IDEA: repository root

## Current Java Modules

- `java-services/common-starter`
- `java-services/gateway-service`
- `java-services/market-service`
- `java-services/order-service`
- `java-services/account-service`
- `java-services/portfolio-service`
- `java-services/risk-service`
- `java-services/backtest-service`
- `java-services/audit-service`

## Current C++ Modules

- `cpp-services/common`
- `cpp-services/matching-engine`
- `cpp-services/risk-core`
- `cpp-services/market-replay-engine`

## Import Steps

1. Install `JDK 21`.
2. Open the repository root in `IntelliJ IDEA`.
3. Import the root `pom.xml`.
4. Use IDEA bundled Maven or a local Maven installation to sync dependencies.

## Local C++ Build

Requirements:
- `CMake 3.20+`
- A C++20-capable compiler such as `g++` or `clang++`
- `Ninja`

Commands:

```bash
cmake -S cpp-services -B build/cpp -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/cpp --parallel
ctest --test-dir build/cpp --output-on-failure
```

## Notes

- The current workspace provides a minimal Maven multi-module skeleton plus a minimal C++ CMake workspace.
- The C++ services currently expose bootstrap executables and `ctest` self-check coverage so CI can validate the native build path.
- `java` and `mvn` were not available in the shell environment during initialization, so command-line Java build verification has not been executed here.
