[CmdletBinding()]
param(
    [switch]$WithConsole
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$composeFile = Join-Path $repoRoot "deploy\docker-compose.yml"

if (-not (Test-Path $composeFile)) {
    throw "Compose file not found: $composeFile"
}

if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    throw "Docker CLI is not installed or not on PATH."
}

$composeArgs = @("compose", "-f", $composeFile, "up", "-d")
if ($WithConsole) {
    $composeArgs += "--profile"
    $composeArgs += "console"
}

Write-Host "Starting local infrastructure from $composeFile..."
docker @composeArgs

Write-Host ""
Write-Host "Local services:"
Write-Host "  PostgreSQL:      localhost:5432"
Write-Host "  Redpanda Kafka:  localhost:19092"
Write-Host "  Schema Registry: localhost:18081"
Write-Host "  HTTP Proxy:      localhost:18082"
Write-Host "  Admin API:       localhost:19644"
if ($WithConsole) {
    Write-Host "  Redpanda UI:     http://localhost:8086"
}
