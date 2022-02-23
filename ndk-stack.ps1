if (Test-Path "./ndkpath.txt")
{
    $NDKPath = Get-Content ./ndkpath.txt
} else {
    $NDKPath = $ENV:ANDROID_NDK_HOME
}



$stackScript = "$NDKPath/ndk-stack"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $stackScript += ".cmd"
}

if ($args.Count -eq 0) {
    Get-Content ./log.txt | & $stackScript -sym ./build/debug/ > log_unstripped.log
} else {
    Get-Content $args[0] | & $stackScript -sym ./build/debug/ > "$($args[0])_unstripped.txt"
}
