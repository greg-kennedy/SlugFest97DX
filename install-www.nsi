; install.nsi
;

;--------------------------------

; The name of the installer
Name "SlugFest '97 DX"

; The file to write
OutFile "install.exe"

; The default installation directory
InstallDir $PROGRAMFILES\SlugFest97DX

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\SlugFest97DX" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Install Types
InstType "Full"
InstType "DX + Classic"
InstType "DX Only"

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "SlugFest '97 DX (required)"

  SectionIn RO 1 2 3
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; The Windows executable, and helper SDL DLLs
  File "SlugFest97DX.exe"
  File "SDL.dll"
  File "SDL_image.dll"
  File "SDL_mixer.dll"
  File "libpng12-0.dll"
  File "zlib1.dll"

  ; Base files from the CD
  File "readme.html"

  ; Data files follow
  CreateDirectory $INSTDIR\data
  SetOutPath $INSTDIR\data
  File /r "data\"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\SlugFest97DX "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SlugFest97DX" "DisplayName" "SlugFest '97 DX"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SlugFest97DX" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SlugFest97DX" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SlugFest97DX" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  ; Write shortcuts
  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\SlugFest '97 DX"
  CreateShortCut "$SMPROGRAMS\SlugFest '97 DX\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\SlugFest '97 DX\SlugFest '97 DX.lnk" "$INSTDIR\SlugFest97DX.exe" "" "$INSTDIR\SlugFest97DX.exe" 0
  CreateShortCut "$SMPROGRAMS\SlugFest '97 DX\Readme.lnk" "$INSTDIR\readme.html" "" "$INSTDIR\readme.html" 0
  
SectionEnd

Section "SlugFest '97 (Classic)"
  SectionIn 1 2

  ; Data files follow
  CreateDirectory $INSTDIR\classic
  SetOutPath $INSTDIR\classic
  File /r "classic\"

  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\SlugFest '97 DX\SlugFest '97 (Classic).lnk" "$INSTDIR\classic\SlugFest.exe" "" "$INSTDIR\classic\SlugFest.exe" 0
  CreateShortCut "$SMPROGRAMS\SlugFest '97 DX\SlugFest '97 (Classic) Help.lnk" "$INSTDIR\classic\SlugFest.hlp" "" "$INSTDIR\classic\SlugFest.hlp" 0

SectionEnd

Section "'Making Of' Video"
  SectionIn 1

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Data files follow
  File "makingof.mpg"

  CreateShortCut "$SMPROGRAMS\SlugFest '97 DX\Making of SlugFest '97.lnk" "$INSTDIR\makingof.mpg" "" "$INSTDIR\makingof.mpg" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SlugFest97DX"
  DeleteRegKey HKLM SOFTWARE\SlugFest97DX

  ; Remove files and uninstaller
  ;  marginally safer than just rmdir /r $instdir
  RMDir /r "$INSTDIR\data"
  RMDir /r "$INSTDIR\classic"
  Delete "$INSTDIR\SlugFest97DX.exe"
  Delete "$INSTDIR\SDL.dll"
  Delete "$INSTDIR\SDL_image.dll"
  Delete "$INSTDIR\SDL_mixer.dll"
  Delete "$INSTDIR\libpng12-0.dll"
  Delete "$INSTDIR\zlib1.dll"
  Delete "$INSTDIR\readme.html"
  Delete "$INSTDIR\makingof.mpg"
  Delete "$INSTDIR\uninstall.exe"

  ; addl generated files
  Delete "$INSTDIR\stdout.txt"
  Delete "$INSTDIR\stderr.txt"
  Delete "$INSTDIR\config.ini"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\SlugFest '97 DX\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\SlugFest '97 DX"
  RMDir "$INSTDIR"

SectionEnd
