$ErrorActionPreference='Stop'
if(Get-Process 'Stalker2-Win64-Shipping' -ErrorAction SilentlyContinue){throw 'Exit STALKER 2 before rollback.'}
$record=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'deployment.json') -Raw | ConvertFrom-Json
$target=Join-Path $record.game 'nvngx.dll_dlssnr.dll'
$backup=Join-Path $PSScriptRoot 'before-direct-update\nvngx.dll_dlssnr.dll'
if((Get-FileHash -LiteralPath $target).Hash.ToLower() -ne $record.candidate){throw 'Game forwarder changed; preserving it.'}
if((Get-FileHash -LiteralPath $backup).Hash.ToLower() -ne $record.previous_candidate){throw 'Backup hash mismatch.'}
$journalPath=Join-Path $record.game '.nr-studio\installation.json'
$journal=Get-Content -LiteralPath $journalPath -Raw | ConvertFrom-Json
$entry=@($journal.Files | Where-Object Name -eq 'nvngx.dll_dlssnr.dll')
if($entry.Count -ne 1 -or $entry[0].Installed -ne $record.candidate){throw 'Installation journal changed; preserving it.'}
Copy-Item -LiteralPath $backup -Destination $target -Force
Copy-Item -LiteralPath $backup -Destination (Join-Path $record.game '.nr-studio\nvngx.dll_dlssnr.dll.new') -Force
$entry[0].Installed=$record.previous_candidate
$journal | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $journalPath -Encoding UTF8
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'before-direct-update\deployment.json') -Destination (Join-Path $PSScriptRoot 'deployment.json') -Force
Write-Output 'Previous validated kernel runtime restored. NR settings preserved.'
