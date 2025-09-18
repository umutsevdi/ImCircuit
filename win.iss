; ------------------------------------------------------------------------------
; File: i18n/build.sh
; Created: 08/22/25
; Author: Umut Sevdi
; Description: Generates the Windows Installer.
;
; Project: umutsevdi/imcircuit
; License: 
; GNU GENERAL PUBLIC LICENSE
; ------------------------------------------------------------------------------

; Change the following variable to path to the source code.
#define Source "C:\Users\user\source\repos\imcircuit"
#define Build Source + "\build"
#define icName "ImCircuit"
#define icDescription "Free and open source logic circuit simulator."
#define icVersion "0.1.0"
#define icAuthor "Umut Sevdi"
#define icURL "https://imcircuit.com/"
#define icExe "ImCircuit.exe"
#define icAssoc icName + " File"
#define icAssocExt ".imcircuit"
#define icAssocKey StringChange(icAssoc, " ", "") + icAssocExt
#define icCopyright "Copyright (C) 2024-2025 Umut Sevdi"
[Setup]
AppId={{F8F8F40F-329B-4F1D-86A0-BC654325E25E}
AppName={#icName}

AppVersion={#icVersion}
AppVerName={#icName} - {#icVersion}
AppPublisher={#icAuthor}
AppPublisherURL={#icURL}
AppSupportURL={#icURL}
AppContact={#icAuthor}
AppComments={#icDescription}
AppCopyright={#icCopyright}
AppUpdatesURL={#icURL}
DefaultDirName={autopf}\{#icName}
DisableDirPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
ChangesAssociations=yes
DisableProgramGroupPage=yes
LicenseFile={#Source}\LICENSE
PrivilegesRequired=lowest
OutputDir={#Build}
OutputBaseFilename=ImCircuit Installer
SetupIconFile={#Build}\package\win32\ImCircuit\imcircuit.ico
UninstallDisplayIcon={#Build}\package\win32\ImCircuit\imcircuit.ico
UninstallDisplayName={#icName}
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#Build}\release\{#icExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Build}\release\glfw3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Build}\release\iconv-2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Build}\release\intl-8.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Build}\release\libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Build}\release\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Build}\package\win32\ImCircuit\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#icName}"; Filename: "{app}\{#icExe}"
Name: "{autodesktop}\{#icName}"; Filename: "{app}\{#icExe}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#icExe}"; Description: "{cm:LaunchProgram,{#StringChange(icName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

