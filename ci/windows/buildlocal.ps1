# Local build - Run from Dissolve topdir

$env:JV_DIR = "$($HOME)\build\jv"
echo $env:JV_DIR
$env:Qt5_DIR = "C:\Qt\5.13.1\mingw73_64"
echo $env:Qt5_DIR
$env:MINGW_DIR = $env:Qt5_DIR
iscc.exe /O.\ .\ci\windows\jv.iss
$exe = Get-ChildItem  .\*.exe
echo "Executable installer is "$exe.Name
innoextract.exe $exe.Name -d $exe.BaseName
mv "$($exe.BaseName)\app\bin\*" $exe.BaseName
mv "$($exe.BaseName)\app" ./
#rm -Force "$($exe.BaseName)\app"
$zip = $exe.BaseName + ".zip"
zip -r $zip $exe.BaseName