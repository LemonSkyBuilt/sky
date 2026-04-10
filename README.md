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

## Import Steps

1. Install `JDK 21`.
2. Open the repository root in `IntelliJ IDEA`.
3. Import the root `pom.xml`.
4. Use IDEA bundled Maven or a local Maven installation to sync dependencies.

## Development Workflow

- Git hosting and CI: `GitHub + GitHub Actions`
- Main CI workflow: [`.github/workflows/ci.yml`](.github/workflows/ci.yml)
- Workflow guide: [`docs/development-workflow.md`](docs/development-workflow.md)

## Notes

- The current workspace provides a minimal Maven multi-module skeleton.
- `java` and `mvn` were not available in the shell environment during initialization, so command-line build verification has not been executed yet.
