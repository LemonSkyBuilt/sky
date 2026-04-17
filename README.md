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

## Development Workflow

- Git hosting and CI: `GitHub + GitHub Actions`
- Main CI workflow: [`.github/workflows/ci.yml`](.github/workflows/ci.yml)
- Workflow guide: [`docs/development-workflow.md`](docs/development-workflow.md)
- Minimal closed-loop guide: [`docs/minimal-closed-loop.md`](docs/minimal-closed-loop.md)

## Local Infra

Requirements:
- `Docker Desktop`
- `docker compose`

Files:
- Compose file: [`deploy/docker-compose.yml`](deploy/docker-compose.yml)
- Environment template: [`deploy/.env.example`](deploy/.env.example)
- Start script: [`scripts/dev-up.ps1`](scripts/dev-up.ps1)
- Stop script: [`scripts/dev-down.ps1`](scripts/dev-down.ps1)

Start the local dependencies:

```powershell
./scripts/dev-up.ps1
```

Start the local dependencies with Redpanda Console:

```powershell
./scripts/dev-up.ps1 -WithConsole
```

Stop the local dependencies:

```powershell
./scripts/dev-down.ps1
```

Stop the local dependencies and delete local data volumes:

```powershell
./scripts/dev-down.ps1 -DeleteData
```

Default local endpoints:
- `PostgreSQL`: `localhost:5432`
- `Redpanda Kafka API`: `localhost:19092`
- `Redpanda Schema Registry`: `localhost:18081`
- `Redpanda HTTP Proxy`: `localhost:18082`
- `Redpanda Admin API`: `localhost:19644`
- `Redpanda Console`: `http://localhost:8086` when started with `-WithConsole`

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

