# Stop the game before restoring its prior forwarder.
$ErrorActionPreference='Stop'
if(Get-Process 'Stalker2-Win64-Shipping' -ErrorAction SilentlyContinue){throw 'Exit STALKER 2 before rollback.'}
$record=Get-Content (Join-Path $PSScriptRoot 'deployment.json') -Raw | ConvertFrom-Json
$target=Join-Path $record.game 'nvngx.dll_dlssnr.dll'
$backup=Join-Path $PSScriptRoot 'game-backup\nvngx.dll_dlssnr.dll'
if((Get-FileHash -LiteralPath $target).Hash.ToLower() -ne $record.candidate){throw 'Game forwarder changed since deployment; refusing to overwrite it.'}
if((Get-FileHash -LiteralPath $backup).Hash.ToLower() -ne $record.before){throw 'Backup hash mismatch.'}
Copy-Item -LiteralPath $backup -Destination $target -Force
$marker=Join-Path $record.game 'nr-post-opt.enable'
if(Test-Path -LiteralPath $marker){Remove-Item -LiteralPath $marker}
Write-Output 'Original forwarder restored.'
