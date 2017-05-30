; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "JournalViewer Lite"
#define MyAppVersion "1.8.4"
#define MyAppPublisher "Tristan Youngs"
#define MyAppURL "http://www.projectaten.net"
#define MyAppExeName "JournalViewerLite.exe"

; Locations of bin directories of Qt, GnuWin(32), and MinGW(32)
#define QtDir "C:\Qt\5.7.0\5.7\mingw53_32"
#define GnuWinDir "C:\GnuWin32"
#define MinGWDir "C:\Qt\5.7.0\Tools\mingw530_32"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{3189293A-1CB7-4929-A51A-83EFC4BC2EBF}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes          
LicenseFile=..\license.txt
SetupIconFile=JournalViewerLite.ico
OutputDir=..\..\
OutputBaseFilename=JournalViewerLite-1.8.4
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "..\..\build\bin\JournalViewerLite.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MinGWDir}\bin\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MinGWDir}\bin\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MinGWDir}\bin\libgfortran-3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MinGWDir}\bin\libquadmath-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MinGWDir}\bin\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Xml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5PrintSupport.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\plugins\iconengines\qsvgicon.dll"; DestDir: "{app}\iconengines"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#QtDir}\plugins\imageformats\*.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "C:\Users\axr61803\src\hdf5_build\bin\hdf5.dll"; DestDir: "{app}"
Source: "JournalViewerLite.ico"; DestDir: "{app}"; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\JournalViewerLite.ico"; IconIndex: 0
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\JournalViewerLite.ico"; IconIndex: 0; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\JournalViewerLite.ico"; IconIndex: 0; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
