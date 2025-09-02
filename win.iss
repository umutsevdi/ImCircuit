; ------------------------------------------------------------------------------
; File: i18n/build.sh
; Created: 08/22/25
; Author: Umut Sevdi
; Description: Generates the Windows Installer.
;
; Project: logic-circuit-simulator-2
; License: 
; GNU GENERAL PUBLIC LICENSE
; ------------------------------------------------------------------------------

; Change the following variable to path to the source code.
#define Source "C:\Users\user\source\repos\logic-circuit-simulator-2"

#define LcsName "Logic Circuit Simulator"
#define LcsDescription "Free and Open Source Logic Circuit Simulator."
#define LcsVersion "0.0.2"
#define LcsPublisher "Umut Sevdi"
#define LcsURL "https://umutsevdi.com/"
#define LcsExeName "LogicCircuitSimulator.exe"
#define LcsAssocName LcsName + " File"
#define LcsAssocExt ".lcs"
#define LcsAssocKey StringChange(LcsAssocName, " ", "") + LcsAssocExt
#define AppCopyright="Copyright (C) 2024-2025 Umut Sevdi"
[Setup]
AppId={{F8F8F40F-329B-4F1D-86A0-BC654325E25E}
AppName={#LcsName}

AppVersion={#LcsVersion}
AppVerName={#LcsName} - {#LcsVersion}
AppPublisher={#LcsPublisher}
AppPublisherURL={#LcsURL}
AppSupportURL={#LcsURL}
AppContact={#LcsPublisher}
AppComments={#LcsDescription}
AppCopyright={#AppCopyright}
AppUpdatesURL={#LcsURL}
DefaultDirName={autopf}\{#LcsName}
DisableDirPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
ChangesAssociations=yes
DisableProgramGroupPage=yes
LicenseFile={#Source}\LICENSE
PrivilegesRequired=lowest
OutputDir={#Source}\build
OutputBaseFilename=Logic Circuit Simulator Installer
SetupIconFile={#Source}\build\package\win32\Logic Circuit Simulator\bin\LogicCircuitSimulator.ico
UninstallDisplayIcon={#Source}\build\package\win32\Logic Circuit Simulator\bin\LogicCircuitSimulator.ico
UninstallDisplayName={#LcsName}
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#Source}\build\release\{#LcsExeName}"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#Source}\build\release\glfw3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#Source}\build\release\iconv-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#Source}\build\release\intl-8.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#Source}\build\release\libcurl.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#Source}\build\release\zlib1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#Source}\build\package\win32\Logic Circuit Simulator\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#LcsName}"; Filename: "{app}\bin\{#LcsExeName}"
Name: "{autodesktop}\{#LcsName}"; Filename: "{app}\bin\{#LcsExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#LcsExeName}"; Description: "{cm:LaunchProgram,{#StringChange(LcsName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

