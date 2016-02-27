;NSIS Modern User Interface version 1.63
;Basic Example Script
;Written by Joost Verburg

!define MUI_PRODUCT "DDakzi" ;Define your own software name here

!include "MUI.nsh"

;--------------------------------
;Configuration

  ;General
  Name	"DDakzi"
  OutFile "Setup_DDakzi.exe"

  ;Folder selection page
  InstallDir "$PROGRAMFILES\${MUI_PRODUCT}"
  
  ;Remember install folder
  InstallDirRegKey HKLM "Software\${MUI_PRODUCT}" ""
  
;--------------------------------
;Variables

  Var MUI_TEMP
  Var STARTMENU_FOLDER
  
  ;Remember the Start Menu Folder
  ;!define MUI_STARTMENUPAGE_VARIABLE "DDakzi"
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${MUI_PRODUCT}" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !define TEMP $R0

  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
  
;--------------------------------
;Modern UI Configuration

;  !define MUI_LICENSEPAGE
;  !define MUI_COMPONENTSPAGE
  !define MUI_DIRECTORYPAGE
  
  !define MUI_ABORTWARNING

;  !define MUI_UNINSTALLER
;  !define MUI_UNCONFIRMPAGE
  
;--------------------------------
;Pages
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
    
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  
;--------------------------------
;Language Strings

  ;Description
  LangString DESC_SecCopyUI ${LANG_ENGLISH} "Copy the modern.exe file to the application folder."

;--------------------------------
;Data
  
;  LicenseData "${NSISDIR}\Contrib\Modern UI\License.txt"

;--------------------------------
;Installer Sections

Section "modern.exe" SecCopyUI

  ;ADD YOUR OWN STUFF HERE!
  
  FindWindow $R0 "Hello:)"
  
  IntCmp $R0 0 skipclosehellowin

; WM_CLOSE 메시지를 보낸다.  
  SendMessage $R0 16 0 0
  Sleep 500

  skipclosehellowin:

  SetOutPath "$INSTDIR\Skinz"
  File /r "runtime\Skinz\*.*" 

  SetOutPath "$INSTDIR"
  File "runtime\DDakzi.exe"
  File "runtime\XWheel.dll"
  File "runtime\readme.txt"
    
  ;Store install folder
  WriteRegStr HKLM "Software\${MUI_PRODUCT}" "" $INSTDIR

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\DDakzi.lnk" "$INSTDIR\DDakzi.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\ReadMe.lnk" "$INSTDIR\readme.txt"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  	CreateShortCut "$SMSTARTUP\DDakzi.lnk" "$INSTDIR\DDakzi.exe"
  	
  !insertmacro MUI_STARTMENU_WRITE_END
  
  ;Create uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DDakzi" "DisplayName" "DDakzi (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DDakzi" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  

  IfFileExists "$PROGRAMFILES\HelloWin\memos.dat" OldMemos OldMemosSkip
  OldMemos:
  MessageBox MB_YESNO|MB_ICONQUESTION "이전버젼의 메모를 발견했습니다. 가져오시겠습니까?" IDNO OldMemosSkip
  CopyFiles "$PROGRAMFILES\HelloWin\memos.dat" "$INSTDIR"
  
  OldMemosSkip: 

  IfFileExists "$PROGRAMFILES\HelloWin\UNWISE.EXE" OldUninstall OldUninstallSkip
  OldUninstall:
  MessageBox MB_YESNO|MB_ICONQUESTION "이전버젼을 삭제하시겠습니까 ? (권장)" IDNO OldMemosSkip
  ExecWait "$PROGRAMFILES\HelloWin\UNWISE.EXE"
  
  OldUninstallSkip:  
  
  MessageBox MB_YESNO|MB_ICONQUESTION "readme.txt 를 읽어보시겠습니까 ?" IDNO SkipReadme
  ExecShell "open" '"$INSTDIR\readme.txt"'

  SkipReadme:

  MessageBox MB_YESNO|MB_ICONQUESTION "딱지를 실행할까요 ?" IDNO SkipExec
  Exec "$INSTDIR\DDakzi.exe"
  SkipExec:
  
SectionEnd

;Display the Finish header
;Insert this macro after the sections if you are not using a finish page
;!insertmacro MUI_SECTIONS_FINISHHEADER

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCopyUI} $(DESC_SecCopyUI)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN STUFF HERE!

  Delete "$INSTDIR\DDakzi.exe"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR\Skinz\"
  RMDir "$INSTDIR"

  ;Remove shortcut
  ReadRegStr ${TEMP} HKLM "Software\${MUI_PRODUCT}" "Start Menu Folder"
  
  StrCmp ${TEMP} "" noshortcuts
  
    Delete "$SMPROGRAMS\${TEMP}\DDakzi.lnk"
    Delete "$SMPROGRAMS\${TEMP}\ReadMe.lnk" 
    Delete "$SMPROGRAMS\${TEMP}\Uninstall.lnk"
  	Delete "$SMSTARTUP\DDakzi.lnk"
  	
    RMDir "$SMPROGRAMS\${TEMP}" ;Only if empty, so it won't delete other shortcuts
  
  noshortcuts:

  DeleteRegKey /ifempty HKLM "Software\${MUI_PRODUCT}"
  
SectionEnd