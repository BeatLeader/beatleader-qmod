Param(
    [Parameter(Mandatory=$false)]
    [Switch] $self,

    [Parameter(Mandatory=$false)]
    [Switch] $all,

    [Parameter(Mandatory=$false)]
    [String] $custom="",

    [Parameter(Mandatory=$false)]
    [Switch] $file,

    [Parameter(Mandatory=$false)]
    [Switch] $help,

    [Parameter(Mandatory=$false)]
    [Switch] $excludeHeader
)

if ($help -eq $true) {
    if ($excludeHeader -eq $false) {
        echo "`"Start-Logging`" - Logs Beat Saber using `"adb logcat`""
        echo "`n-- Arguments --`n"
    }

    echo "-Self `t`t Only Logs your mod and Crashes"
    echo "-All `t`t Logs everything, including logs made by the Quest itself"
    echo "-Custom `t Specify a specific logging pattern, e.g `"custom-types|questui`""
    echo "`t`t NOTE: The paterent `"AndriodRuntime|CRASH`" is always appended to a custom pattern"
    echo "-File `t`t Saves the output of the log to `"logcat.log`""

    exit
}

$timestamp = Get-Date -Format "MM-dd HH:mm:ss.fff"
$bspid = adb shell pidof com.beatgames.beatsaber
$command = "adb logcat -T `"$timestamp`""

if ($all -eq $false) {
    while ([string]::IsNullOrEmpty($bspid)) {
        Start-Sleep -Milliseconds 100
        $bspid = adb shell pidof com.beatgames.beatsaber
    }

    $command += "--pid $bspid"
}

if ($self -eq $true) {
    $command += " | Select-String -pattern `"(bl|AndroidRuntime|CRASH)`""
} elseif ($custom -ne "") {
    $pattern = "(" + $custom + "|AndriodRuntime|CRASH)"
    $command += " | Select-String -pattern `"$pattern`""
}
elseif ($all -eq $false) {
    $command += " | Select-String -pattern `"(QuestHook|modloader|AndroidRuntime|CRASH)`""
}

if ($file -eq $true) {
    $command += " | Out-File -FilePath $PSScriptRoot\logcat.log"
}

echo "Logging using Command `"$command`""
Invoke-Expression $command