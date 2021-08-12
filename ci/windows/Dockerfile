#FROM mcr.microsoft.com/windows/servercore:ltsc2019
FROM mcr.microsoft.com/windows/servercore:1803
LABEL Description="Windows Server Core 1803 / Qt 5.13.1 / Chocolatey"

# Disable crash dialog for release-mode runtimes
RUN reg add "HKLM\SOFTWARE\Microsoft\Windows\Windows Error Reporting" /v Disabled /t REG_DWORD /d 1 /f
RUN reg add "HKLM\SOFTWARE\Microsoft\Windows\Windows Error Reporting" /v DontShowUI /t REG_DWORD /d 1 /f

# Install choco for psuedo package manager
RUN @powershell -NoProfile -ExecutionPolicy Bypass -Command \
    $Env:chocolateyVersion = '0.10.14' ; \
    $Env:chocolateyUseWindowsCompression = 'false' ; \
    "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))" && SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"
RUN choco install -y windows-sdk-10-version-1803-all
RUN choco install -y unzip --version 6.0 && unzip -v
RUN choco install -y zip --version 3.0 && zip -v
RUN choco install -y cmake
RUN choco install -y ninja
RUN choco install -y winflexbison
RUN choco install -y git
RUN choco install -y innosetup
RUN choco install -y 7zip
RUN choco install -y innounp

# Install Qt5 5.13.1
COPY qtifwsilent.qs C:\\qtifwsilent.qs
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command \
    $ErrorActionPreference = 'Stop'; \
    $Wc = New-Object System.Net.WebClient ; \
    $Wc.DownloadFile('http://download.qt.io/archive/qt/5.13/5.13.1/qt-opensource-windows-x86-5.13.1.exe', 'C:\qt.exe') ; \
    Echo 'Downloaded qt-opensource-windows-x86-5.13.1.exe' ; \
    $Env:QT_INSTALL_DIR = 'C:\\Qt' ; \
    Start-Process C:\qt.exe -ArgumentList '--verbose --script C:/qtifwsilent.qs' -NoNewWindow -Wait ; \
    Remove-Item C:\qt.exe -Force ; \
    Remove-Item C:\qtifwsilent.qs -Force
ENV QTDIR C:\\Qt\\5.13.1\\mingw73_64

# Execute build script
ENTRYPOINT  ["powershell"]
CMD ["-NoProfile", "-ExecutionPolicy Bypass", "-File entrypoint.ps1"]