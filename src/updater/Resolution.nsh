

Var ResDialog
Var ResCombo
Var ResChB


Function ResolutionPage

        !insertmacro MUI_HEADER_TEXT "$(txtResHeader1)" "$(txtResHeader2)"

	nsDialogs::Create 1018
	Pop $ResDialog

	${If} $ResDialog == error
		Abort
	${EndIf}

        ${NSD_CreateLabel} 0 8 50% 12 "$(txtResResolution)"
        Pop $0
        
        ${NSD_CreateComboBox} 0 26 40% 20 ""
        Pop $ResCombo

        System::Alloc 220
        Pop $R0                     # DEVMODE
        IntOp $0 $R0 + 68
        System::Call "*$0(h 220, h 0)"
        IntOp $R1 $R0 + 172         # addr of XXX x YYY
        StrCpy $R2 0                # pocet najitch rozliseni
        StrCpy $R3 0                # iterator
        System::Alloc 320
        Pop $R4                     # pole unikatnich
        StrCpy $7 -1
        StrCpy $8 -1                # rychla cache

loop:
        System::Call "user32::EnumDisplaySettings(p n, i R3, p R0) i .r0"
        StrCmp $0 0 done

        IntOp $R3 $R3 + 1
        System::Call "*$R1(i .r1, i .r2)"

        StrCmp $1 $7 +1 loop2i
        StrCmp $2 $8 +1 loop2i
        GoTo loop
loop2i:
        StrCpy $7 $1
        StrCpy $8 $2
        StrCpy $3 0               # vnitrni iterator
        StrCpy $4 $R4             # ptr do unikatnich

loop2:
        StrCmp $3 $R2 found

        System::Call "*$4(i .r5, i .r6)"
        IntOp $3 $3 + 1
        IntOp $4 $4 + 8
        StrCmp $1 $5 +1 loop2
        StrCmp $2 $6 +1 loop2
        GoTo loop
found:
        ${NSD_CB_AddString} $ResCombo "$1x$2"

        StrCmp $3 40 done
        System::Call "*$4(i r1, i r2)"
        IntOp $R2 $R2 + 1

        GoTo loop
done:

        System::Free $R0
        System::Free $R4

        ${NSD_CB_SelectString} $ResCombo "$ResXx$ResY"
        
        ${NSD_CreateLabel} 0 70 100% 45 "$(txtResNote1)"
        Pop $0
        ${NSD_CreateLabel} 0 120 100% 45 "$(txtResNote2)"
        Pop $0
        ${NSD_CreateLabel} 0 180 100% 30 "$(txtResNote3) '$INSTDIR\krkal.cfg'."
        Pop $0

        ${NSD_CreateCheckBox} 55% 29 45% 14 "$(txtResFullScreen)"
        Pop $ResChB
        ${NSD_SetState} $ResChB $FS_State

	nsDialogs::Show

FunctionEnd



Function ResolutionPageLeave
   ${NSD_GetState} $ResChB $FS_State

   ${NSD_GetText} $ResCombo $0

   ${WordFind} $0 "x" "#" $3
   ${If} $3 <> 2
     MessageBox MB_OK "$(txtResFormatWarning)"
     Abort
   ${EndIf}
   ${WordFind} $0 "x" "+1" $1
   ${WordFind} $0 "x" "+2" $2
   IntOp $1 $1 + 0
   IntOp $2 $2 + 0

   ${If} $1 <= 0
    MessageBox MB_OK "$(txtResFormatWarning)"
    Abort
   ${ElseIf} $2 <= 0
    MessageBox MB_OK "$(txtResFormatWarning)"
    Abort
   ${EndIf}

   StrCpy $ResX $1
   StrCpy $ResY $2
FunctionEnd




Function WriteConfig
  ${If} $LANGUAGE == ${LANG_CZECH}
    StrCpy $9 "cs"
  ${Else}
    StrCpy $9 "en"
  ${EndIf}
  ${If} $FS_State == ${BST_CHECKED}
    StrCpy $8 "1"
  ${Else}
    StrCpy $8 "0"
  ${EndIf}

  ClearErrors  ; want to be a newborn

  StrCpy $R3 "$INSTDIR\krkal.cfg"    ; source name
  StrCpy $R4 "$INSTDIR\krkal.cfg.tmp"    ; temp name

  IfFileExists $R3 +1 RIF_error
  FileOpen $R1 $R3 "r"               ; open source
  FileOpen $R2 $R4 "w"              ; open temp

  RIF_loop:
    FileRead $R1 $0
    IfErrors RIF_leaveloop
    
    StrLen $2 "lang"
    StrCpy $1 $0 $2
    StrCmp $1 "lang" 0 +3
    StrCpy $0 "lang = $9$\r$\n"
    Goto writeLine
    StrLen $2 "game_resx"
    StrCpy $1 $0 $2
    StrCmp $1 "game_resx" 0 +3
    StrCpy $0 "game_resx = $ResX$\r$\n"
    Goto writeLine
    StrLen $2 "game_resy"
    StrCpy $1 $0 $2
    StrCmp $1 "game_resy" 0 +3
    StrCpy $0 "game_resy = $ResY$\r$\n"
    Goto writeLine
    StrLen $2 "game_fullscreen"
    StrCpy $1 $0 $2
    StrCmp $1 "game_fullscreen" 0 +3
    StrCpy $0 "game_fullscreen = $8$\r$\n"
    Goto writeLine
    StrLen $2 "editor_resx"
    StrCpy $1 $0 $2
    StrCmp $1 "editor_resx" 0 +3
    StrCpy $0 "editor_resx = $ResX$\r$\n"
    Goto writeLine
    StrLen $2 "editor_resy"
    StrCpy $1 $0 $2
    StrCmp $1 "editor_resy" 0 +3
    StrCpy $0 "editor_resy = $ResY$\r$\n"
    Goto writeLine
    StrLen $2 "editor_fullscreen"
    StrCpy $1 $0 $2
    StrCmp $1 "editor_fullscreen" 0 +3
    StrCpy $0 "editor_fullscreen = $8$\r$\n"
    Goto writeLine

  writeLine:
    FileWrite $R2 $0
  Goto RIF_loop

  RIF_leaveloop:
    FileClose $R1
    FileClose $R2

    Delete "$R3.old"
    Rename "$R3" "$R3.old"
    Rename "$R4" "$R3"
    Delete "$R3.old"

    ClearErrors
    Goto RIF_out

  RIF_error:
    SetErrors

  RIF_out:
FunctionEnd
