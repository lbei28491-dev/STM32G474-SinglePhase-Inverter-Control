param(
    [ValidateSet('build', 'rebuild')]
    [string]$Action = 'build'
)

$ErrorActionPreference = 'Stop'

$workspaceRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$projectPath = Join-Path $workspaceRoot 'MDK-ARM\1.uvprojx'
$logPath = Join-Path $workspaceRoot 'MDK-ARM\vscode_build.log'
$buildSwitch = if ($Action -eq 'rebuild') { '-r' } else { '-b' }

$uv4Candidates = @(
    $env:UV4_PATH,
    $env:KEIL_PATH,
    'D:\Keil_v5\UV4\UV4.exe',
    'C:\Keil_v5\UV4\UV4.exe',
    (Join-Path $env:ProgramFiles 'Keil_v5\UV4\UV4.exe'),
    (Join-Path ${env:ProgramFiles(x86)} 'Keil_v5\UV4\UV4.exe')
) | Where-Object { $_ -and (Test-Path -LiteralPath $_) }

$uv4Path = $uv4Candidates | Select-Object -First 1

if (-not $uv4Path) {
    throw "Keil uVision not found. Checked common installation paths for UV4.exe."
}

if (-not (Test-Path -LiteralPath $projectPath)) {
    throw "Keil project not found: $projectPath"
}

New-Item -ItemType Directory -Path (Split-Path -Parent $logPath) -Force | Out-Null

$process = Start-Process `
    -FilePath $uv4Path `
    -ArgumentList @($buildSwitch, $projectPath, '-j0', '-o', $logPath) `
    -Wait `
    -PassThru `
    -WorkingDirectory $workspaceRoot `
    -WindowStyle Hidden

if (Test-Path -LiteralPath $logPath) {
    Get-Content -LiteralPath $logPath -Encoding Default
}

if ($process.ExitCode -ne 0) {
    throw "Keil $Action failed with exit code $($process.ExitCode)."
}

