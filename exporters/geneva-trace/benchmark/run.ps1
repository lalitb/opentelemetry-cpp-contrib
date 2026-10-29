# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

$ErrorActionPreference = 'Stop'
$output = Join-Path $PSScriptRoot 'results'
New-Item -ItemType Directory -Force $output | Out-Null
$session = "OtelGenevaBench-$PID"
$etl = Join-Path $output 'events.etl'
$provider = '{8DAD5A91-836E-4FBC-94D7-9ABACCD34BE5}'
try {
    & logman create trace $session -p $provider 0xffffffffffffffff 5 -bs 64 -nb 128 256 -o $etl -ow -ets |
        Tee-Object (Join-Path $output 'session.txt')
    if ($LASTEXITCODE -ne 0) { throw 'ETW session creation failed; run this script elevated.' }
    try {
        $timings = & (Join-Path $PSScriptRoot 'build\Release\geneva_etw_benchmark.exe') 2>&1
        $benchmarkExit = $LASTEXITCODE
        $timings | Set-Content -Encoding UTF8 (Join-Path $output 'timings.txt')
        $timings
    } finally {
        & logman stop $session -ets | Tee-Object -Append (Join-Path $output 'session.txt')
        if ($LASTEXITCODE -ne 0) { throw 'ETW session stop failed.' }
    }
    if ($benchmarkExit -ne 0) { throw "Benchmark exited with code $benchmarkExit." }
    & tracerpt $etl -of CSV -o (Join-Path $output 'events.csv') `
        -summary (Join-Path $output 'summary.txt') -y 2>&1 |
        Tee-Object (Join-Path $output 'decode.txt')
    if ($LASTEXITCODE -ne 0) { throw 'ETL decoding failed.' }
    $summary = Get-Content -Raw (Join-Path $output 'summary.txt')
    $expected = [regex]::Match(($timings -join "`n"), 'Expected ETW events: (\d+)').Groups[1].Value
    $countPattern = "(?im)^\|\s*$expected\s+.*$([regex]::Escape($provider))\|"
    if (!$expected -or $summary -notmatch $countPattern -or
        $summary -notmatch 'Total Events\s+Lost\s+0\b') {
        throw 'ETW capture count/loss validation failed.'
    }
    "Verified $expected benchmark-provider events; zero lost." |
        Set-Content -Encoding UTF8 (Join-Path $output 'validation.txt')
    '0' | Set-Content (Join-Path $output 'exit-code.txt')
} catch {
    $_ | Out-String | Set-Content (Join-Path $output 'error.txt')
    '1' | Set-Content (Join-Path $output 'exit-code.txt')
    exit 1
}
