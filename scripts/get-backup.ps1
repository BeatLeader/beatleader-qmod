Write-Output "Pulling playlists from Quest"

if (Test-Path Backup/Playlists) {
    rm -r Backup/Playlists
}
if (Test-Path Backup/PlaylistBackups) {
    rm -r Backup/PlaylistBackups
}

adb pull /sdcard/ModData/com.beatgames.beatsaber/Mods/PlaylistManager/Playlists/ Backup/Playlists/ | Out-Null
if ($LASTEXITCODE -ne 0) {
    mkdir Backup/Playlists | Out-Null
}

adb pull /sdcard/ModData/com.beatgames.beatsaber/Mods/PlaylistManager/PlaylistBackups/ Backup/PlaylistBackups/ | Out-Null
if ($LASTEXITCODE -ne 0) {
    mkdir Backup/PlaylistBackups | Out-Null
}