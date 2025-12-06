[Setup]
AppName=CactusViewer
AppVersion=2.2.2
DefaultDirName={autopf}\CactusViewer
DefaultGroupName=CactusViewer
OutputBaseFilename=CactusViewer_Setup
Compression=lzma
SolidCompression=yes
PrivilegesRequiredOverridesAllowed=dialog
InfoBeforeFile=Cactus intro.txt
InfoAfterFile=Cactus outro.txt
AppPublisherURL=https://wassimulator.com
WizardStyle=modern
DisableProgramGroupPage=no
DisableDirPage=no
OutputDir=bin

[Files]
Source: "bin\CactusViewer.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\CactusViewer"; Filename: "{app}\CactusViewer.exe"
Name: "{commondesktop}\CactusViewer"; Filename: "{app}\CactusViewer.exe"; Tasks: desktopicon

[Tasks]
Name: desktopicon; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"
Name: setDefault; Description: "Make Cactus Viewer the default image viewer."; GroupDescription: "Configuration:"
Name: addContext; Description: "Add 'Open with CactusViewer' to right-click context menu."; GroupDescription: "Configuration:"

[Registry]
Root: HKCR; Subkey: "CactusViewer"; ValueType: string; ValueName: ""; ValueData: "Image File" ; Tasks: addContext
Root: HKCR; Subkey: "CactusViewer\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\CactusViewer.exe"" ""%1""" ; Tasks: addContext
Root: HKCR; Subkey: "*\shell\OpenWithCactusViewer"; ValueType: string; ValueName: ""; ValueData: "Open with CactusViewer" ; Tasks: addContext
Root: HKCR; Subkey: "*\shell\OpenWithCactusViewer\command"; ValueType: string; ValueName: ""; ValueData: """{app}\CactusViewer.exe"" ""%1""" ; Tasks: addContext
Root: HKCR; Subkey: "*\shell\OpenWithCactusViewer"; ValueType: string; ValueName: "Icon"; ValueData: """{app}\CactusViewer.exe"",0" ; Tasks: addContext

Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".3fr"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".ari"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".arw"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".avci"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".avcs"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".avif"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".avifs"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".bay"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".bmp"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".cap"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".cr2"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".cr3"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".crw"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".cur"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".dcr"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".dcs"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".dds"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".dib"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".dng"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".drf"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".eip"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".erf"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".exif"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".fff"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".gif"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".heic"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".heics"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".heif"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".heifs"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".hif"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".ico"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".icon"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".iiq"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".jfif"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".jpe"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".jpeg"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".jpg"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".jxr"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".k25"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".kdc"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".mef"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".mos"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".mrw"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".nef"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".nrw"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".orf"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".ori"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".pef"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".png"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".ppm"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".ptx"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".pxn"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".raf"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".raw"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".rle"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".rw2"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".rwl"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".sr2"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".srf"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".srw"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".tif"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".tiff"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".wdp"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".webp"; ValueData: "" ; Tasks: setDefault
Root: HKA; Subkey: "Software\Classes\Applications\CactusViewer\SupportedTypes"; ValueType: string; ValueName: ".x3f"; ValueData: "" ; Tasks: setDefault

