[CmdletBinding()]
param(
    [switch]$DeleteData
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

$composeArgs = @("compose", "-f", $composeFile, "down")
if ($DeleteData) {
    $composeArgs += "--volumes"
}

Write-Host "Stopping local infrastructure from $composeFile..."
docker @composeArgs
