# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=muddy - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to muddy - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "muddy - Win32 Release" && "$(CFG)" != "muddy - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "muddy.mak" CFG="muddy - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "muddy - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "muddy - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "muddy - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "muddy - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\muddy.exe"

CLEAN : 
	-@erase ".\Release\muddy.exe"
	-@erase ".\Release\act_comm.obj"
	-@erase ".\Release\olc_msg.obj"
	-@erase ".\Release\util.obj"
	-@erase ".\Release\comm.obj"
	-@erase ".\Release\obj_prog.obj"
	-@erase ".\Release\db_lang.obj"
	-@erase ".\Release\recycle.obj"
	-@erase ".\Release\martial_art.obj"
	-@erase ".\Release\lang.obj"
	-@erase ".\Release\winstuff.obj"
	-@erase ".\Release\effects.obj"
	-@erase ".\Release\resolver.obj"
	-@erase ".\Release\update.obj"
	-@erase ".\Release\varr.obj"
	-@erase ".\Release\act_info.obj"
	-@erase ".\Release\buffer.obj"
	-@erase ".\Release\skills.obj"
	-@erase ".\Release\word.obj"
	-@erase ".\Release\healer.obj"
	-@erase ".\Release\db_skills.obj"
	-@erase ".\Release\log.obj"
	-@erase ".\Release\save.obj"
	-@erase ".\Release\cmd.obj"
	-@erase ".\Release\quest.obj"
	-@erase ".\Release\olc_mob.obj"
	-@erase ".\Release\olc_clan.obj"
	-@erase ".\Release\lookup.obj"
	-@erase ".\Release\fight.obj"
	-@erase ".\Release\namedp.obj"
	-@erase ".\Release\tables.obj"
	-@erase ".\Release\charset.obj"
	-@erase ".\Release\olc_area.obj"
	-@erase ".\Release\magic.obj"
	-@erase ".\Release\gsn.obj"
	-@erase ".\Release\db_clan.obj"
	-@erase ".\Release\religion.obj"
	-@erase ".\Release\db.obj"
	-@erase ".\Release\mob_prog.obj"
	-@erase ".\Release\note.obj"
	-@erase ".\Release\repair.obj"
	-@erase ".\Release\mob_cmds.obj"
	-@erase ".\Release\olc_word.obj"
	-@erase ".\Release\mem.obj"
	-@erase ".\Release\olc_save.obj"
	-@erase ".\Release\clan.obj"
	-@erase ".\Release\hunt.obj"
	-@erase ".\Release\db_area.obj"
	-@erase ".\Release\interp.obj"
	-@erase ".\Release\olc_mpcode.obj"
	-@erase ".\Release\olc_room.obj"
	-@erase ".\Release\help.obj"
	-@erase ".\Release\hometown.obj"
	-@erase ".\Release\resource.obj"
	-@erase ".\Release\olc_obj.obj"
	-@erase ".\Release\str.obj"
	-@erase ".\Release\spellfn.obj"
	-@erase ".\Release\magic2.obj"
	-@erase ".\Release\regex.obj"
	-@erase ".\Release\auction.obj"
	-@erase ".\Release\ban.obj"
	-@erase ".\Release\flag.obj"
	-@erase ".\Release\olc.obj"
	-@erase ".\Release\special.obj"
	-@erase ".\Release\rating.obj"
	-@erase ".\Release\olc_help.obj"
	-@erase ".\Release\act_move.obj"
	-@erase ".\Release\msg.obj"
	-@erase ".\Release\handler.obj"
	-@erase ".\Release\mlstring.obj"
	-@erase ".\Release\act_wiz.obj"
	-@erase ".\Release\olc_lang.obj"
	-@erase ".\Release\act_obj.obj"
	-@erase ".\Release\strcasecmp.obj"
	-@erase ".\Release\db_class.obj"
	-@erase ".\Release\raffects.obj"
	-@erase ".\Release\string_edit.obj"
	-@erase ".\Release\class.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/muddy.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/muddy.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/muddy.pdb" /machine:I386 /out:"$(OUTDIR)/muddy.exe" 
LINK32_OBJS= \
	"$(INTDIR)/act_comm.obj" \
	"$(INTDIR)/olc_msg.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/comm.obj" \
	"$(INTDIR)/obj_prog.obj" \
	"$(INTDIR)/db_lang.obj" \
	"$(INTDIR)/recycle.obj" \
	"$(INTDIR)/martial_art.obj" \
	"$(INTDIR)/lang.obj" \
	"$(INTDIR)/winstuff.obj" \
	"$(INTDIR)/effects.obj" \
	"$(INTDIR)/resolver.obj" \
	"$(INTDIR)/update.obj" \
	"$(INTDIR)/varr.obj" \
	"$(INTDIR)/act_info.obj" \
	"$(INTDIR)/buffer.obj" \
	"$(INTDIR)/skills.obj" \
	"$(INTDIR)/word.obj" \
	"$(INTDIR)/healer.obj" \
	"$(INTDIR)/db_skills.obj" \
	"$(INTDIR)/log.obj" \
	"$(INTDIR)/save.obj" \
	"$(INTDIR)/cmd.obj" \
	"$(INTDIR)/quest.obj" \
	"$(INTDIR)/olc_mob.obj" \
	"$(INTDIR)/olc_clan.obj" \
	"$(INTDIR)/lookup.obj" \
	"$(INTDIR)/fight.obj" \
	"$(INTDIR)/namedp.obj" \
	"$(INTDIR)/tables.obj" \
	"$(INTDIR)/charset.obj" \
	"$(INTDIR)/olc_area.obj" \
	"$(INTDIR)/magic.obj" \
	"$(INTDIR)/gsn.obj" \
	"$(INTDIR)/db_clan.obj" \
	"$(INTDIR)/religion.obj" \
	"$(INTDIR)/db.obj" \
	"$(INTDIR)/mob_prog.obj" \
	"$(INTDIR)/note.obj" \
	"$(INTDIR)/repair.obj" \
	"$(INTDIR)/mob_cmds.obj" \
	"$(INTDIR)/olc_word.obj" \
	"$(INTDIR)/mem.obj" \
	"$(INTDIR)/olc_save.obj" \
	"$(INTDIR)/clan.obj" \
	"$(INTDIR)/hunt.obj" \
	"$(INTDIR)/db_area.obj" \
	"$(INTDIR)/interp.obj" \
	"$(INTDIR)/olc_mpcode.obj" \
	"$(INTDIR)/olc_room.obj" \
	"$(INTDIR)/help.obj" \
	"$(INTDIR)/hometown.obj" \
	"$(INTDIR)/resource.obj" \
	"$(INTDIR)/olc_obj.obj" \
	"$(INTDIR)/str.obj" \
	"$(INTDIR)/spellfn.obj" \
	"$(INTDIR)/magic2.obj" \
	"$(INTDIR)/regex.obj" \
	"$(INTDIR)/auction.obj" \
	"$(INTDIR)/ban.obj" \
	"$(INTDIR)/flag.obj" \
	"$(INTDIR)/olc.obj" \
	"$(INTDIR)/special.obj" \
	"$(INTDIR)/rating.obj" \
	"$(INTDIR)/olc_help.obj" \
	"$(INTDIR)/act_move.obj" \
	"$(INTDIR)/msg.obj" \
	"$(INTDIR)/handler.obj" \
	"$(INTDIR)/mlstring.obj" \
	"$(INTDIR)/act_wiz.obj" \
	"$(INTDIR)/olc_lang.obj" \
	"$(INTDIR)/act_obj.obj" \
	"$(INTDIR)/strcasecmp.obj" \
	"$(INTDIR)/db_class.obj" \
	"$(INTDIR)/raffects.obj" \
	"$(INTDIR)/string_edit.obj" \
	"$(INTDIR)/class.obj"

"$(OUTDIR)\muddy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\muddy.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\muddy.exe"
	-@erase ".\Debug\util.obj"
	-@erase ".\Debug\comm.obj"
	-@erase ".\Debug\charset.obj"
	-@erase ".\Debug\obj_prog.obj"
	-@erase ".\Debug\update.obj"
	-@erase ".\Debug\mem.obj"
	-@erase ".\Debug\db_skills.obj"
	-@erase ".\Debug\buffer.obj"
	-@erase ".\Debug\db_clan.obj"
	-@erase ".\Debug\skills.obj"
	-@erase ".\Debug\healer.obj"
	-@erase ".\Debug\lang.obj"
	-@erase ".\Debug\handler.obj"
	-@erase ".\Debug\martial_art.obj"
	-@erase ".\Debug\db_area.obj"
	-@erase ".\Debug\namedp.obj"
	-@erase ".\Debug\regex.obj"
	-@erase ".\Debug\save.obj"
	-@erase ".\Debug\str.obj"
	-@erase ".\Debug\tables.obj"
	-@erase ".\Debug\quest.obj"
	-@erase ".\Debug\act_move.obj"
	-@erase ".\Debug\olc_obj.obj"
	-@erase ".\Debug\ban.obj"
	-@erase ".\Debug\spellfn.obj"
	-@erase ".\Debug\olc.obj"
	-@erase ".\Debug\repair.obj"
	-@erase ".\Debug\recycle.obj"
	-@erase ".\Debug\auction.obj"
	-@erase ".\Debug\olc_lang.obj"
	-@erase ".\Debug\religion.obj"
	-@erase ".\Debug\note.obj"
	-@erase ".\Debug\strcasecmp.obj"
	-@erase ".\Debug\olc_mpcode.obj"
	-@erase ".\Debug\mob_cmds.obj"
	-@erase ".\Debug\olc_word.obj"
	-@erase ".\Debug\clan.obj"
	-@erase ".\Debug\msg.obj"
	-@erase ".\Debug\olc_save.obj"
	-@erase ".\Debug\olc_room.obj"
	-@erase ".\Debug\hometown.obj"
	-@erase ".\Debug\string_edit.obj"
	-@erase ".\Debug\db.obj"
	-@erase ".\Debug\magic2.obj"
	-@erase ".\Debug\resource.obj"
	-@erase ".\Debug\varr.obj"
	-@erase ".\Debug\winstuff.obj"
	-@erase ".\Debug\resolver.obj"
	-@erase ".\Debug\act_info.obj"
	-@erase ".\Debug\lookup.obj"
	-@erase ".\Debug\word.obj"
	-@erase ".\Debug\rating.obj"
	-@erase ".\Debug\act_wiz.obj"
	-@erase ".\Debug\act_obj.obj"
	-@erase ".\Debug\flag.obj"
	-@erase ".\Debug\olc_clan.obj"
	-@erase ".\Debug\olc_msg.obj"
	-@erase ".\Debug\olc_help.obj"
	-@erase ".\Debug\db_lang.obj"
	-@erase ".\Debug\olc_area.obj"
	-@erase ".\Debug\fight.obj"
	-@erase ".\Debug\interp.obj"
	-@erase ".\Debug\log.obj"
	-@erase ".\Debug\effects.obj"
	-@erase ".\Debug\mob_prog.obj"
	-@erase ".\Debug\cmd.obj"
	-@erase ".\Debug\mlstring.obj"
	-@erase ".\Debug\hunt.obj"
	-@erase ".\Debug\magic.obj"
	-@erase ".\Debug\help.obj"
	-@erase ".\Debug\db_class.obj"
	-@erase ".\Debug\raffects.obj"
	-@erase ".\Debug\class.obj"
	-@erase ".\Debug\olc_mob.obj"
	-@erase ".\Debug\special.obj"
	-@erase ".\Debug\gsn.obj"
	-@erase ".\Debug\act_comm.obj"
	-@erase ".\Debug\muddy.ilk"
	-@erase ".\Debug\muddy.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/muddy.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/muddy.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/muddy.pdb" /debug /machine:I386 /out:"$(OUTDIR)/muddy.exe" 
LINK32_OBJS= \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/comm.obj" \
	"$(INTDIR)/charset.obj" \
	"$(INTDIR)/obj_prog.obj" \
	"$(INTDIR)/update.obj" \
	"$(INTDIR)/mem.obj" \
	"$(INTDIR)/db_skills.obj" \
	"$(INTDIR)/buffer.obj" \
	"$(INTDIR)/db_clan.obj" \
	"$(INTDIR)/skills.obj" \
	"$(INTDIR)/healer.obj" \
	"$(INTDIR)/lang.obj" \
	"$(INTDIR)/handler.obj" \
	"$(INTDIR)/martial_art.obj" \
	"$(INTDIR)/db_area.obj" \
	"$(INTDIR)/namedp.obj" \
	"$(INTDIR)/regex.obj" \
	"$(INTDIR)/save.obj" \
	"$(INTDIR)/str.obj" \
	"$(INTDIR)/tables.obj" \
	"$(INTDIR)/quest.obj" \
	"$(INTDIR)/act_move.obj" \
	"$(INTDIR)/olc_obj.obj" \
	"$(INTDIR)/ban.obj" \
	"$(INTDIR)/spellfn.obj" \
	"$(INTDIR)/olc.obj" \
	"$(INTDIR)/repair.obj" \
	"$(INTDIR)/recycle.obj" \
	"$(INTDIR)/auction.obj" \
	"$(INTDIR)/olc_lang.obj" \
	"$(INTDIR)/religion.obj" \
	"$(INTDIR)/note.obj" \
	"$(INTDIR)/strcasecmp.obj" \
	"$(INTDIR)/olc_mpcode.obj" \
	"$(INTDIR)/mob_cmds.obj" \
	"$(INTDIR)/olc_word.obj" \
	"$(INTDIR)/clan.obj" \
	"$(INTDIR)/msg.obj" \
	"$(INTDIR)/olc_save.obj" \
	"$(INTDIR)/olc_room.obj" \
	"$(INTDIR)/hometown.obj" \
	"$(INTDIR)/string_edit.obj" \
	"$(INTDIR)/db.obj" \
	"$(INTDIR)/magic2.obj" \
	"$(INTDIR)/resource.obj" \
	"$(INTDIR)/varr.obj" \
	"$(INTDIR)/winstuff.obj" \
	"$(INTDIR)/resolver.obj" \
	"$(INTDIR)/act_info.obj" \
	"$(INTDIR)/lookup.obj" \
	"$(INTDIR)/word.obj" \
	"$(INTDIR)/rating.obj" \
	"$(INTDIR)/act_wiz.obj" \
	"$(INTDIR)/act_obj.obj" \
	"$(INTDIR)/flag.obj" \
	"$(INTDIR)/olc_clan.obj" \
	"$(INTDIR)/olc_msg.obj" \
	"$(INTDIR)/olc_help.obj" \
	"$(INTDIR)/db_lang.obj" \
	"$(INTDIR)/olc_area.obj" \
	"$(INTDIR)/fight.obj" \
	"$(INTDIR)/interp.obj" \
	"$(INTDIR)/log.obj" \
	"$(INTDIR)/effects.obj" \
	"$(INTDIR)/mob_prog.obj" \
	"$(INTDIR)/cmd.obj" \
	"$(INTDIR)/mlstring.obj" \
	"$(INTDIR)/hunt.obj" \
	"$(INTDIR)/magic.obj" \
	"$(INTDIR)/help.obj" \
	"$(INTDIR)/db_class.obj" \
	"$(INTDIR)/raffects.obj" \
	"$(INTDIR)/class.obj" \
	"$(INTDIR)/olc_mob.obj" \
	"$(INTDIR)/special.obj" \
	"$(INTDIR)/gsn.obj" \
	"$(INTDIR)/act_comm.obj"

"$(OUTDIR)\muddy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "muddy - Win32 Release"
# Name "muddy - Win32 Debug"

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\varr.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\act_info.c
DEP_CPP_ACT_I=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	".\fight.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\act_info.obj" : $(SOURCE) $(DEP_CPP_ACT_I) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\act_move.c
DEP_CPP_ACT_M=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\act_move.obj" : $(SOURCE) $(DEP_CPP_ACT_M) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\act_obj.c
DEP_CPP_ACT_O=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	".\fight.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\act_obj.obj" : $(SOURCE) $(DEP_CPP_ACT_O) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\act_wiz.c
DEP_CPP_ACT_W=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	".\db\cmd.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\act_wiz.obj" : $(SOURCE) $(DEP_CPP_ACT_W) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\auction.c
DEP_CPP_AUCTI=\
	{$(INCLUDE)}"\merc.h"\
	".\auction.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\auction.obj" : $(SOURCE) $(DEP_CPP_AUCTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\auction.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\buffer.c
DEP_CPP_BUFFE=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\buffer.obj" : $(SOURCE) $(DEP_CPP_BUFFE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\buffer.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\clan.c
DEP_CPP_CLAN_=\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\clan.obj" : $(SOURCE) $(DEP_CPP_CLAN_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\clan.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\class.c
DEP_CPP_CLASS=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\class.obj" : $(SOURCE) $(DEP_CPP_CLASS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\class.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\const.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\effects.c
DEP_CPP_EFFEC=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\effects.obj" : $(SOURCE) $(DEP_CPP_EFFEC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\fight.c
DEP_CPP_FIGHT=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\quest.h"\
	".\fight.h"\
	{$(INCLUDE)}"\rating.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\fight.obj" : $(SOURCE) $(DEP_CPP_FIGHT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\fight.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\flag.c
DEP_CPP_FLAG_=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\util.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\flag.obj" : $(SOURCE) $(DEP_CPP_FLAG_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\flag.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\handler.c
DEP_CPP_HANDL=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	".\raffects.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\handler.obj" : $(SOURCE) $(DEP_CPP_HANDL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\healer.c
DEP_CPP_HEALE=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\healer.obj" : $(SOURCE) $(DEP_CPP_HEALE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\help.c
DEP_CPP_HELP_=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\help.obj" : $(SOURCE) $(DEP_CPP_HELP_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\hometown.c
DEP_CPP_HOMET=\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\hometown.obj" : $(SOURCE) $(DEP_CPP_HOMET) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\hometown.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hunt.c
DEP_CPP_HUNT_=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\hunt.obj" : $(SOURCE) $(DEP_CPP_HUNT_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\interp.c
DEP_CPP_INTER=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	".\db\cmd.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\interp.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\interp.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\log.c
DEP_CPP_LOG_C=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\log.obj" : $(SOURCE) $(DEP_CPP_LOG_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\log.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lookup.c
DEP_CPP_LOOKU=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\lookup.obj" : $(SOURCE) $(DEP_CPP_LOOKU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lookup.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\magic.c
DEP_CPP_MAGIC=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\magic.obj" : $(SOURCE) $(DEP_CPP_MAGIC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\magic.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\magic2.c
DEP_CPP_MAGIC2=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	".\fight.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\rating.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\magic2.obj" : $(SOURCE) $(DEP_CPP_MAGIC2) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\martial_art.c
DEP_CPP_MARTI=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	".\fight.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\martial_art.obj" : $(SOURCE) $(DEP_CPP_MARTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mem.c
DEP_CPP_MEM_C=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\mem.obj" : $(SOURCE) $(DEP_CPP_MEM_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\merc.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mlstring.c
DEP_CPP_MLSTR=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\mlstring.obj" : $(SOURCE) $(DEP_CPP_MLSTR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mlstring.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mob_cmds.c
DEP_CPP_MOB_C=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	".\mob_cmds.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\mob_cmds.obj" : $(SOURCE) $(DEP_CPP_MOB_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mob_cmds.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mob_prog.c
DEP_CPP_MOB_P=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\regex.h"\
	{$(INCLUDE)}"\merc.h"\
	".\mob_cmds.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	
NODEP_CPP_MOB_P=\
	".\compat\config.h"\
	".\compat\lisp.h"\
	".\compat\syntax.h"\
	

"$(INTDIR)\mob_prog.obj" : $(SOURCE) $(DEP_CPP_MOB_P) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mob_prog.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\namedp.c
DEP_CPP_NAMED=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\namedp.obj" : $(SOURCE) $(DEP_CPP_NAMED) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\namedp.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\note.c
DEP_CPP_NOTE_=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\note.obj" : $(SOURCE) $(DEP_CPP_NOTE_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\obj_prog.c
DEP_CPP_OBJ_P=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\obj_prog.obj" : $(SOURCE) $(DEP_CPP_OBJ_P) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\obj_prog.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\quest.c
DEP_CPP_QUEST=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\quest.obj" : $(SOURCE) $(DEP_CPP_QUEST) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\quest.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\raffects.c
DEP_CPP_RAFFE=\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	".\raffects.h"\
	".\fight.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\raffects.obj" : $(SOURCE) $(DEP_CPP_RAFFE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\raffects.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rating.c
DEP_CPP_RATIN=\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\rating.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\rating.obj" : $(SOURCE) $(DEP_CPP_RATIN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\rating.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\recycle.c
DEP_CPP_RECYC=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\regex.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	
NODEP_CPP_RECYC=\
	".\compat\config.h"\
	".\compat\lisp.h"\
	".\compat\syntax.h"\
	

"$(INTDIR)\recycle.obj" : $(SOURCE) $(DEP_CPP_RECYC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\recycle.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\religion.c
DEP_CPP_RELIG=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\religion.obj" : $(SOURCE) $(DEP_CPP_RELIG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\religion.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\repair.c
DEP_CPP_REPAI=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\repair.obj" : $(SOURCE) $(DEP_CPP_REPAI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\save.c
DEP_CPP_SAVE_=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\save.obj" : $(SOURCE) $(DEP_CPP_SAVE_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\skills.c
DEP_CPP_SKILL=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\skills.obj" : $(SOURCE) $(DEP_CPP_SKILL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\skills.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\special.c
DEP_CPP_SPECI=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	".\fight.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\special.obj" : $(SOURCE) $(DEP_CPP_SPECI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\str.c
DEP_CPP_STR_C=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\str.obj" : $(SOURCE) $(DEP_CPP_STR_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\str.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\string_edit.c
DEP_CPP_STRIN=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\string_edit.obj" : $(SOURCE) $(DEP_CPP_STRIN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\string_edit.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tables.c
DEP_CPP_TABLE=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\tables.obj" : $(SOURCE) $(DEP_CPP_TABLE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tables.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\typedef.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\update.c
DEP_CPP_UPDAT=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	".\fight.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\update.obj" : $(SOURCE) $(DEP_CPP_UPDAT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\update.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\util.c
DEP_CPP_UTIL_=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\util.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\varr.c
DEP_CPP_VARR_=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\varr.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\varr.obj" : $(SOURCE) $(DEP_CPP_VARR_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\act_comm.c
DEP_CPP_ACT_C=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	".\auction.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\act_comm.obj" : $(SOURCE) $(DEP_CPP_ACT_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\comm\resolver.c
DEP_CPP_RESOL=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\log.h"\
	".\comm\resolver.h"\
	

"$(INTDIR)\resolver.obj" : $(SOURCE) $(DEP_CPP_RESOL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\comm\charset.c

"$(INTDIR)\charset.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\comm\comm.c
DEP_CPP_COMM_=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Timeb.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	".\comm\ban.h"\
	".\comm\charset.h"\
	".\comm\resolver.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\db\word.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\comm.obj" : $(SOURCE) $(DEP_CPP_COMM_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\comm\ban.c
DEP_CPP_BAN_C=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	".\comm\ban.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\ban.obj" : $(SOURCE) $(DEP_CPP_BAN_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\word.c
DEP_CPP_WORD_=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\db\word.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\word.obj" : $(SOURCE) $(DEP_CPP_WORD_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db.c
DEP_CPP_DB_C62=\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\rating.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\db.obj" : $(SOURCE) $(DEP_CPP_DB_C62) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db_area.c
DEP_CPP_DB_AR=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\db_area.obj" : $(SOURCE) $(DEP_CPP_DB_AR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db_clan.c
DEP_CPP_DB_CL=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\db_clan.obj" : $(SOURCE) $(DEP_CPP_DB_CL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db_class.c
DEP_CPP_DB_CLA=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\db_class.obj" : $(SOURCE) $(DEP_CPP_DB_CLA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db_lang.c
DEP_CPP_DB_LA=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\db\word.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\db_lang.obj" : $(SOURCE) $(DEP_CPP_DB_LA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db_skills.c
DEP_CPP_DB_SK=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\db_skills.obj" : $(SOURCE) $(DEP_CPP_DB_SK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\gsn.c
DEP_CPP_GSN_C=\
	{$(INCLUDE)}"\typedef.h"\
	".\db\gsn.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\gsn.obj" : $(SOURCE) $(DEP_CPP_GSN_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\lang.c
DEP_CPP_LANG_=\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\varr.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\lang.obj" : $(SOURCE) $(DEP_CPP_LANG_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\msg.c
DEP_CPP_MSG_C=\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\db\db.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\util.h"\
	{$(INCLUDE)}"\varr.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\msg.obj" : $(SOURCE) $(DEP_CPP_MSG_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\resource.c
DEP_CPP_RESOU=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\resource.obj" : $(SOURCE) $(DEP_CPP_RESOU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\spellfn.c
DEP_CPP_SPELL=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\namedp.h"\
	".\db\spellfn.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\spellfn.obj" : $(SOURCE) $(DEP_CPP_SPELL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\cmd.c
DEP_CPP_CMD_C=\
	{$(INCLUDE)}"\typedef.h"\
	".\db\cmd.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\cmd.obj" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_word.c
DEP_CPP_OLC_W=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\db\word.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_word.obj" : $(SOURCE) $(DEP_CPP_OLC_W) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_area.c
DEP_CPP_OLC_A=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_area.obj" : $(SOURCE) $(DEP_CPP_OLC_A) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_clan.c
DEP_CPP_OLC_C=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_clan.obj" : $(SOURCE) $(DEP_CPP_OLC_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_help.c
DEP_CPP_OLC_H=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_help.obj" : $(SOURCE) $(DEP_CPP_OLC_H) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_lang.c
DEP_CPP_OLC_L=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_lang.obj" : $(SOURCE) $(DEP_CPP_OLC_L) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_mob.c
DEP_CPP_OLC_M=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_mob.obj" : $(SOURCE) $(DEP_CPP_OLC_M) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_mpcode.c
DEP_CPP_OLC_MP=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\olc_mpcode.obj" : $(SOURCE) $(DEP_CPP_OLC_MP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_msg.c
DEP_CPP_OLC_MS=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_msg.obj" : $(SOURCE) $(DEP_CPP_OLC_MS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_obj.c
DEP_CPP_OLC_O=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\olc_obj.obj" : $(SOURCE) $(DEP_CPP_OLC_O) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_room.c
DEP_CPP_OLC_R=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_room.obj" : $(SOURCE) $(DEP_CPP_OLC_R) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_save.c
DEP_CPP_OLC_S=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\db\word.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\olc_save.obj" : $(SOURCE) $(DEP_CPP_OLC_S) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc.c
DEP_CPP_OLC_C90=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\olc.obj" : $(SOURCE) $(DEP_CPP_OLC_C90) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\compat\winstuff.c
DEP_CPP_WINST=\
	{$(INCLUDE)}"\compat.h"\
	

"$(INTDIR)\winstuff.obj" : $(SOURCE) $(DEP_CPP_WINST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\compat\regex.c
DEP_CPP_REGEX=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\regex.h"\
	
NODEP_CPP_REGEX=\
	".\compat\lisp.h"\
	".\compat\syntax.h"\
	".\compat\config.h"\
	

"$(INTDIR)\regex.obj" : $(SOURCE) $(DEP_CPP_REGEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\compat\strcasecmp.c
DEP_CPP_STRCA=\
	{$(INCLUDE)}"\compat.h"\
	

"$(INTDIR)\strcasecmp.obj" : $(SOURCE) $(DEP_CPP_STRCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=muddy - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to muddy - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "muddy - Win32 Release" && "$(CFG)" != "muddy - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "muddy.mak" CFG="muddy - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "muddy - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "muddy - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "muddy - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "muddy - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\muddy.exe"

CLEAN : 
	-@erase ".\Release\muddy.exe"
	-@erase ".\Release\act_comm.obj"
	-@erase ".\Release\olc_msg.obj"
	-@erase ".\Release\util.obj"
	-@erase ".\Release\comm.obj"
	-@erase ".\Release\obj_prog.obj"
	-@erase ".\Release\db_lang.obj"
	-@erase ".\Release\recycle.obj"
	-@erase ".\Release\martial_art.obj"
	-@erase ".\Release\lang.obj"
	-@erase ".\Release\winstuff.obj"
	-@erase ".\Release\effects.obj"
	-@erase ".\Release\resolver.obj"
	-@erase ".\Release\update.obj"
	-@erase ".\Release\varr.obj"
	-@erase ".\Release\act_info.obj"
	-@erase ".\Release\buffer.obj"
	-@erase ".\Release\skills.obj"
	-@erase ".\Release\word.obj"
	-@erase ".\Release\healer.obj"
	-@erase ".\Release\db_skills.obj"
	-@erase ".\Release\log.obj"
	-@erase ".\Release\save.obj"
	-@erase ".\Release\cmd.obj"
	-@erase ".\Release\quest.obj"
	-@erase ".\Release\olc_mob.obj"
	-@erase ".\Release\olc_clan.obj"
	-@erase ".\Release\lookup.obj"
	-@erase ".\Release\fight.obj"
	-@erase ".\Release\namedp.obj"
	-@erase ".\Release\tables.obj"
	-@erase ".\Release\charset.obj"
	-@erase ".\Release\olc_area.obj"
	-@erase ".\Release\magic.obj"
	-@erase ".\Release\gsn.obj"
	-@erase ".\Release\db_clan.obj"
	-@erase ".\Release\religion.obj"
	-@erase ".\Release\db.obj"
	-@erase ".\Release\mob_prog.obj"
	-@erase ".\Release\note.obj"
	-@erase ".\Release\repair.obj"
	-@erase ".\Release\mob_cmds.obj"
	-@erase ".\Release\olc_word.obj"
	-@erase ".\Release\mem.obj"
	-@erase ".\Release\olc_save.obj"
	-@erase ".\Release\clan.obj"
	-@erase ".\Release\hunt.obj"
	-@erase ".\Release\db_area.obj"
	-@erase ".\Release\interp.obj"
	-@erase ".\Release\olc_mpcode.obj"
	-@erase ".\Release\olc_room.obj"
	-@erase ".\Release\help.obj"
	-@erase ".\Release\hometown.obj"
	-@erase ".\Release\resource.obj"
	-@erase ".\Release\olc_obj.obj"
	-@erase ".\Release\str.obj"
	-@erase ".\Release\spellfn.obj"
	-@erase ".\Release\magic2.obj"
	-@erase ".\Release\regex.obj"
	-@erase ".\Release\auction.obj"
	-@erase ".\Release\ban.obj"
	-@erase ".\Release\flag.obj"
	-@erase ".\Release\olc.obj"
	-@erase ".\Release\special.obj"
	-@erase ".\Release\rating.obj"
	-@erase ".\Release\olc_help.obj"
	-@erase ".\Release\act_move.obj"
	-@erase ".\Release\msg.obj"
	-@erase ".\Release\handler.obj"
	-@erase ".\Release\mlstring.obj"
	-@erase ".\Release\act_wiz.obj"
	-@erase ".\Release\olc_lang.obj"
	-@erase ".\Release\act_obj.obj"
	-@erase ".\Release\strcasecmp.obj"
	-@erase ".\Release\db_class.obj"
	-@erase ".\Release\raffects.obj"
	-@erase ".\Release\string_edit.obj"
	-@erase ".\Release\class.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/muddy.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/muddy.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/muddy.pdb" /machine:I386 /out:"$(OUTDIR)/muddy.exe" 
LINK32_OBJS= \
	"$(INTDIR)/act_comm.obj" \
	"$(INTDIR)/olc_msg.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/comm.obj" \
	"$(INTDIR)/obj_prog.obj" \
	"$(INTDIR)/db_lang.obj" \
	"$(INTDIR)/recycle.obj" \
	"$(INTDIR)/martial_art.obj" \
	"$(INTDIR)/lang.obj" \
	"$(INTDIR)/winstuff.obj" \
	"$(INTDIR)/effects.obj" \
	"$(INTDIR)/resolver.obj" \
	"$(INTDIR)/update.obj" \
	"$(INTDIR)/varr.obj" \
	"$(INTDIR)/act_info.obj" \
	"$(INTDIR)/buffer.obj" \
	"$(INTDIR)/skills.obj" \
	"$(INTDIR)/word.obj" \
	"$(INTDIR)/healer.obj" \
	"$(INTDIR)/db_skills.obj" \
	"$(INTDIR)/log.obj" \
	"$(INTDIR)/save.obj" \
	"$(INTDIR)/cmd.obj" \
	"$(INTDIR)/quest.obj" \
	"$(INTDIR)/olc_mob.obj" \
	"$(INTDIR)/olc_clan.obj" \
	"$(INTDIR)/lookup.obj" \
	"$(INTDIR)/fight.obj" \
	"$(INTDIR)/namedp.obj" \
	"$(INTDIR)/tables.obj" \
	"$(INTDIR)/charset.obj" \
	"$(INTDIR)/olc_area.obj" \
	"$(INTDIR)/magic.obj" \
	"$(INTDIR)/gsn.obj" \
	"$(INTDIR)/db_clan.obj" \
	"$(INTDIR)/religion.obj" \
	"$(INTDIR)/db.obj" \
	"$(INTDIR)/mob_prog.obj" \
	"$(INTDIR)/note.obj" \
	"$(INTDIR)/repair.obj" \
	"$(INTDIR)/mob_cmds.obj" \
	"$(INTDIR)/olc_word.obj" \
	"$(INTDIR)/mem.obj" \
	"$(INTDIR)/olc_save.obj" \
	"$(INTDIR)/clan.obj" \
	"$(INTDIR)/hunt.obj" \
	"$(INTDIR)/db_area.obj" \
	"$(INTDIR)/interp.obj" \
	"$(INTDIR)/olc_mpcode.obj" \
	"$(INTDIR)/olc_room.obj" \
	"$(INTDIR)/help.obj" \
	"$(INTDIR)/hometown.obj" \
	"$(INTDIR)/resource.obj" \
	"$(INTDIR)/olc_obj.obj" \
	"$(INTDIR)/str.obj" \
	"$(INTDIR)/spellfn.obj" \
	"$(INTDIR)/magic2.obj" \
	"$(INTDIR)/regex.obj" \
	"$(INTDIR)/auction.obj" \
	"$(INTDIR)/ban.obj" \
	"$(INTDIR)/flag.obj" \
	"$(INTDIR)/olc.obj" \
	"$(INTDIR)/special.obj" \
	"$(INTDIR)/rating.obj" \
	"$(INTDIR)/olc_help.obj" \
	"$(INTDIR)/act_move.obj" \
	"$(INTDIR)/msg.obj" \
	"$(INTDIR)/handler.obj" \
	"$(INTDIR)/mlstring.obj" \
	"$(INTDIR)/act_wiz.obj" \
	"$(INTDIR)/olc_lang.obj" \
	"$(INTDIR)/act_obj.obj" \
	"$(INTDIR)/strcasecmp.obj" \
	"$(INTDIR)/db_class.obj" \
	"$(INTDIR)/raffects.obj" \
	"$(INTDIR)/string_edit.obj" \
	"$(INTDIR)/class.obj"

"$(OUTDIR)\muddy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\muddy.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\muddy.exe"
	-@erase ".\Debug\util.obj"
	-@erase ".\Debug\comm.obj"
	-@erase ".\Debug\charset.obj"
	-@erase ".\Debug\obj_prog.obj"
	-@erase ".\Debug\update.obj"
	-@erase ".\Debug\mem.obj"
	-@erase ".\Debug\db_skills.obj"
	-@erase ".\Debug\buffer.obj"
	-@erase ".\Debug\db_clan.obj"
	-@erase ".\Debug\skills.obj"
	-@erase ".\Debug\healer.obj"
	-@erase ".\Debug\lang.obj"
	-@erase ".\Debug\handler.obj"
	-@erase ".\Debug\martial_art.obj"
	-@erase ".\Debug\db_area.obj"
	-@erase ".\Debug\namedp.obj"
	-@erase ".\Debug\regex.obj"
	-@erase ".\Debug\save.obj"
	-@erase ".\Debug\str.obj"
	-@erase ".\Debug\tables.obj"
	-@erase ".\Debug\quest.obj"
	-@erase ".\Debug\act_move.obj"
	-@erase ".\Debug\olc_obj.obj"
	-@erase ".\Debug\ban.obj"
	-@erase ".\Debug\spellfn.obj"
	-@erase ".\Debug\olc.obj"
	-@erase ".\Debug\repair.obj"
	-@erase ".\Debug\recycle.obj"
	-@erase ".\Debug\auction.obj"
	-@erase ".\Debug\olc_lang.obj"
	-@erase ".\Debug\religion.obj"
	-@erase ".\Debug\note.obj"
	-@erase ".\Debug\strcasecmp.obj"
	-@erase ".\Debug\olc_mpcode.obj"
	-@erase ".\Debug\mob_cmds.obj"
	-@erase ".\Debug\olc_word.obj"
	-@erase ".\Debug\clan.obj"
	-@erase ".\Debug\msg.obj"
	-@erase ".\Debug\olc_save.obj"
	-@erase ".\Debug\olc_room.obj"
	-@erase ".\Debug\hometown.obj"
	-@erase ".\Debug\string_edit.obj"
	-@erase ".\Debug\db.obj"
	-@erase ".\Debug\magic2.obj"
	-@erase ".\Debug\resource.obj"
	-@erase ".\Debug\varr.obj"
	-@erase ".\Debug\winstuff.obj"
	-@erase ".\Debug\resolver.obj"
	-@erase ".\Debug\act_info.obj"
	-@erase ".\Debug\lookup.obj"
	-@erase ".\Debug\word.obj"
	-@erase ".\Debug\rating.obj"
	-@erase ".\Debug\act_wiz.obj"
	-@erase ".\Debug\act_obj.obj"
	-@erase ".\Debug\flag.obj"
	-@erase ".\Debug\olc_clan.obj"
	-@erase ".\Debug\olc_msg.obj"
	-@erase ".\Debug\olc_help.obj"
	-@erase ".\Debug\db_lang.obj"
	-@erase ".\Debug\olc_area.obj"
	-@erase ".\Debug\fight.obj"
	-@erase ".\Debug\interp.obj"
	-@erase ".\Debug\log.obj"
	-@erase ".\Debug\effects.obj"
	-@erase ".\Debug\mob_prog.obj"
	-@erase ".\Debug\cmd.obj"
	-@erase ".\Debug\mlstring.obj"
	-@erase ".\Debug\hunt.obj"
	-@erase ".\Debug\magic.obj"
	-@erase ".\Debug\help.obj"
	-@erase ".\Debug\db_class.obj"
	-@erase ".\Debug\raffects.obj"
	-@erase ".\Debug\class.obj"
	-@erase ".\Debug\olc_mob.obj"
	-@erase ".\Debug\special.obj"
	-@erase ".\Debug\gsn.obj"
	-@erase ".\Debug\act_comm.obj"
	-@erase ".\Debug\muddy.ilk"
	-@erase ".\Debug\muddy.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/muddy.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/muddy.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/muddy.pdb" /debug /machine:I386 /out:"$(OUTDIR)/muddy.exe" 
LINK32_OBJS= \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/comm.obj" \
	"$(INTDIR)/charset.obj" \
	"$(INTDIR)/obj_prog.obj" \
	"$(INTDIR)/update.obj" \
	"$(INTDIR)/mem.obj" \
	"$(INTDIR)/db_skills.obj" \
	"$(INTDIR)/buffer.obj" \
	"$(INTDIR)/db_clan.obj" \
	"$(INTDIR)/skills.obj" \
	"$(INTDIR)/healer.obj" \
	"$(INTDIR)/lang.obj" \
	"$(INTDIR)/handler.obj" \
	"$(INTDIR)/martial_art.obj" \
	"$(INTDIR)/db_area.obj" \
	"$(INTDIR)/namedp.obj" \
	"$(INTDIR)/regex.obj" \
	"$(INTDIR)/save.obj" \
	"$(INTDIR)/str.obj" \
	"$(INTDIR)/tables.obj" \
	"$(INTDIR)/quest.obj" \
	"$(INTDIR)/act_move.obj" \
	"$(INTDIR)/olc_obj.obj" \
	"$(INTDIR)/ban.obj" \
	"$(INTDIR)/spellfn.obj" \
	"$(INTDIR)/olc.obj" \
	"$(INTDIR)/repair.obj" \
	"$(INTDIR)/recycle.obj" \
	"$(INTDIR)/auction.obj" \
	"$(INTDIR)/olc_lang.obj" \
	"$(INTDIR)/religion.obj" \
	"$(INTDIR)/note.obj" \
	"$(INTDIR)/strcasecmp.obj" \
	"$(INTDIR)/olc_mpcode.obj" \
	"$(INTDIR)/mob_cmds.obj" \
	"$(INTDIR)/olc_word.obj" \
	"$(INTDIR)/clan.obj" \
	"$(INTDIR)/msg.obj" \
	"$(INTDIR)/olc_save.obj" \
	"$(INTDIR)/olc_room.obj" \
	"$(INTDIR)/hometown.obj" \
	"$(INTDIR)/string_edit.obj" \
	"$(INTDIR)/db.obj" \
	"$(INTDIR)/magic2.obj" \
	"$(INTDIR)/resource.obj" \
	"$(INTDIR)/varr.obj" \
	"$(INTDIR)/winstuff.obj" \
	"$(INTDIR)/resolver.obj" \
	"$(INTDIR)/act_info.obj" \
	"$(INTDIR)/lookup.obj" \
	"$(INTDIR)/word.obj" \
	"$(INTDIR)/rating.obj" \
	"$(INTDIR)/act_wiz.obj" \
	"$(INTDIR)/act_obj.obj" \
	"$(INTDIR)/flag.obj" \
	"$(INTDIR)/olc_clan.obj" \
	"$(INTDIR)/olc_msg.obj" \
	"$(INTDIR)/olc_help.obj" \
	"$(INTDIR)/db_lang.obj" \
	"$(INTDIR)/olc_area.obj" \
	"$(INTDIR)/fight.obj" \
	"$(INTDIR)/interp.obj" \
	"$(INTDIR)/log.obj" \
	"$(INTDIR)/effects.obj" \
	"$(INTDIR)/mob_prog.obj" \
	"$(INTDIR)/cmd.obj" \
	"$(INTDIR)/mlstring.obj" \
	"$(INTDIR)/hunt.obj" \
	"$(INTDIR)/magic.obj" \
	"$(INTDIR)/help.obj" \
	"$(INTDIR)/db_class.obj" \
	"$(INTDIR)/raffects.obj" \
	"$(INTDIR)/class.obj" \
	"$(INTDIR)/olc_mob.obj" \
	"$(INTDIR)/special.obj" \
	"$(INTDIR)/gsn.obj" \
	"$(INTDIR)/act_comm.obj"

"$(OUTDIR)\muddy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "muddy - Win32 Release"
# Name "muddy - Win32 Debug"

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\varr.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\act_info.c
DEP_CPP_ACT_I=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	".\fight.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\act_info.obj" : $(SOURCE) $(DEP_CPP_ACT_I) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\act_move.c
DEP_CPP_ACT_M=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\act_move.obj" : $(SOURCE) $(DEP_CPP_ACT_M) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\act_obj.c
DEP_CPP_ACT_O=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	".\fight.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\act_obj.obj" : $(SOURCE) $(DEP_CPP_ACT_O) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\act_wiz.c
DEP_CPP_ACT_W=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	".\db\cmd.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\act_wiz.obj" : $(SOURCE) $(DEP_CPP_ACT_W) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\auction.c
DEP_CPP_AUCTI=\
	{$(INCLUDE)}"\merc.h"\
	".\auction.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\auction.obj" : $(SOURCE) $(DEP_CPP_AUCTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\auction.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\buffer.c
DEP_CPP_BUFFE=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\buffer.obj" : $(SOURCE) $(DEP_CPP_BUFFE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\buffer.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\clan.c
DEP_CPP_CLAN_=\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\clan.obj" : $(SOURCE) $(DEP_CPP_CLAN_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\clan.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\class.c
DEP_CPP_CLASS=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\class.obj" : $(SOURCE) $(DEP_CPP_CLASS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\class.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\const.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\effects.c
DEP_CPP_EFFEC=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\effects.obj" : $(SOURCE) $(DEP_CPP_EFFEC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\fight.c
DEP_CPP_FIGHT=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\quest.h"\
	".\fight.h"\
	{$(INCLUDE)}"\rating.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\fight.obj" : $(SOURCE) $(DEP_CPP_FIGHT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\fight.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\flag.c
DEP_CPP_FLAG_=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\util.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\flag.obj" : $(SOURCE) $(DEP_CPP_FLAG_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\flag.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\handler.c
DEP_CPP_HANDL=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	".\raffects.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\handler.obj" : $(SOURCE) $(DEP_CPP_HANDL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\healer.c
DEP_CPP_HEALE=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\healer.obj" : $(SOURCE) $(DEP_CPP_HEALE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\help.c
DEP_CPP_HELP_=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\help.obj" : $(SOURCE) $(DEP_CPP_HELP_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\hometown.c
DEP_CPP_HOMET=\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\hometown.obj" : $(SOURCE) $(DEP_CPP_HOMET) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\hometown.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hunt.c
DEP_CPP_HUNT_=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\hunt.obj" : $(SOURCE) $(DEP_CPP_HUNT_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\interp.c
DEP_CPP_INTER=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	".\db\cmd.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\interp.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\interp.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\log.c
DEP_CPP_LOG_C=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\log.obj" : $(SOURCE) $(DEP_CPP_LOG_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\log.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lookup.c
DEP_CPP_LOOKU=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\lookup.obj" : $(SOURCE) $(DEP_CPP_LOOKU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lookup.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\magic.c
DEP_CPP_MAGIC=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\magic.obj" : $(SOURCE) $(DEP_CPP_MAGIC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\magic.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\magic2.c
DEP_CPP_MAGIC2=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	".\fight.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\rating.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\magic2.obj" : $(SOURCE) $(DEP_CPP_MAGIC2) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\martial_art.c
DEP_CPP_MARTI=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	".\fight.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\martial_art.obj" : $(SOURCE) $(DEP_CPP_MARTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mem.c
DEP_CPP_MEM_C=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\mem.obj" : $(SOURCE) $(DEP_CPP_MEM_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\merc.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mlstring.c
DEP_CPP_MLSTR=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\mlstring.obj" : $(SOURCE) $(DEP_CPP_MLSTR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mlstring.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mob_cmds.c
DEP_CPP_MOB_C=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	".\mob_cmds.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\mob_cmds.obj" : $(SOURCE) $(DEP_CPP_MOB_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mob_cmds.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mob_prog.c
DEP_CPP_MOB_P=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\regex.h"\
	{$(INCLUDE)}"\merc.h"\
	".\mob_cmds.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	
NODEP_CPP_MOB_P=\
	".\compat\config.h"\
	".\compat\lisp.h"\
	".\compat\syntax.h"\
	

"$(INTDIR)\mob_prog.obj" : $(SOURCE) $(DEP_CPP_MOB_P) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mob_prog.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\namedp.c
DEP_CPP_NAMED=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\namedp.obj" : $(SOURCE) $(DEP_CPP_NAMED) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\namedp.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\note.c
DEP_CPP_NOTE_=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\note.obj" : $(SOURCE) $(DEP_CPP_NOTE_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\obj_prog.c
DEP_CPP_OBJ_P=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	".\fight.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\obj_prog.obj" : $(SOURCE) $(DEP_CPP_OBJ_P) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\obj_prog.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\quest.c
DEP_CPP_QUEST=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\quest.obj" : $(SOURCE) $(DEP_CPP_QUEST) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\quest.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\raffects.c
DEP_CPP_RAFFE=\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	".\raffects.h"\
	".\fight.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\raffects.obj" : $(SOURCE) $(DEP_CPP_RAFFE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\raffects.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rating.c
DEP_CPP_RATIN=\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\rating.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\rating.obj" : $(SOURCE) $(DEP_CPP_RATIN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\rating.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\recycle.c
DEP_CPP_RECYC=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\regex.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	
NODEP_CPP_RECYC=\
	".\compat\config.h"\
	".\compat\lisp.h"\
	".\compat\syntax.h"\
	

"$(INTDIR)\recycle.obj" : $(SOURCE) $(DEP_CPP_RECYC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\recycle.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\religion.c
DEP_CPP_RELIG=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\religion.obj" : $(SOURCE) $(DEP_CPP_RELIG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\religion.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\repair.c
DEP_CPP_REPAI=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\repair.obj" : $(SOURCE) $(DEP_CPP_REPAI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\save.c
DEP_CPP_SAVE_=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\save.obj" : $(SOURCE) $(DEP_CPP_SAVE_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\skills.c
DEP_CPP_SKILL=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\skills.obj" : $(SOURCE) $(DEP_CPP_SKILL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\skills.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\special.c
DEP_CPP_SPECI=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	".\fight.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\special.obj" : $(SOURCE) $(DEP_CPP_SPECI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\str.c
DEP_CPP_STR_C=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\str.obj" : $(SOURCE) $(DEP_CPP_STR_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\str.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\string_edit.c
DEP_CPP_STRIN=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\string_edit.obj" : $(SOURCE) $(DEP_CPP_STRIN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\string_edit.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tables.c
DEP_CPP_TABLE=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\tables.obj" : $(SOURCE) $(DEP_CPP_TABLE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tables.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\typedef.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\update.c
DEP_CPP_UPDAT=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	".\fight.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\update.obj" : $(SOURCE) $(DEP_CPP_UPDAT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\update.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\util.c
DEP_CPP_UTIL_=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\util.h

!IF  "$(CFG)" == "muddy - Win32 Release"

!ELSEIF  "$(CFG)" == "muddy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\varr.c
DEP_CPP_VARR_=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\varr.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\varr.obj" : $(SOURCE) $(DEP_CPP_VARR_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\act_comm.c
DEP_CPP_ACT_C=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	".\auction.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\act_comm.obj" : $(SOURCE) $(DEP_CPP_ACT_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\comm\resolver.c
DEP_CPP_RESOL=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\log.h"\
	".\comm\resolver.h"\
	

"$(INTDIR)\resolver.obj" : $(SOURCE) $(DEP_CPP_RESOL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\comm\charset.c

"$(INTDIR)\charset.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\comm\comm.c
DEP_CPP_COMM_=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Timeb.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\hometown.h"\
	{$(INCLUDE)}"\quest.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\mob_prog.h"\
	".\comm\ban.h"\
	".\comm\charset.h"\
	".\comm\resolver.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\db\word.h"\
	{$(INCLUDE)}"\resource.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\comm.obj" : $(SOURCE) $(DEP_CPP_COMM_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\comm\ban.c
DEP_CPP_BAN_C=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	".\comm\ban.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\ban.obj" : $(SOURCE) $(DEP_CPP_BAN_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\word.c
DEP_CPP_WORD_=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\db\word.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\word.obj" : $(SOURCE) $(DEP_CPP_WORD_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db.c
DEP_CPP_DB_C62=\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\Sys\Time.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\rating.h"\
	{$(INCLUDE)}"\update.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\db.obj" : $(SOURCE) $(DEP_CPP_DB_C62) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db_area.c
DEP_CPP_DB_AR=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\db_area.obj" : $(SOURCE) $(DEP_CPP_DB_AR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db_clan.c
DEP_CPP_DB_CL=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\db_clan.obj" : $(SOURCE) $(DEP_CPP_DB_CL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db_class.c
DEP_CPP_DB_CLA=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\db_class.obj" : $(SOURCE) $(DEP_CPP_DB_CLA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db_lang.c
DEP_CPP_DB_LA=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\db\word.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\db_lang.obj" : $(SOURCE) $(DEP_CPP_DB_LA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\db_skills.c
DEP_CPP_DB_SK=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\db_skills.obj" : $(SOURCE) $(DEP_CPP_DB_SK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\gsn.c
DEP_CPP_GSN_C=\
	{$(INCLUDE)}"\typedef.h"\
	".\db\gsn.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\gsn.obj" : $(SOURCE) $(DEP_CPP_GSN_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\lang.c
DEP_CPP_LANG_=\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\varr.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\lang.obj" : $(SOURCE) $(DEP_CPP_LANG_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\msg.c
DEP_CPP_MSG_C=\
	{$(INCLUDE)}"\const.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\db\db.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\util.h"\
	{$(INCLUDE)}"\varr.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\msg.obj" : $(SOURCE) $(DEP_CPP_MSG_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\resource.c
DEP_CPP_RESOU=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\compat.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\resource.obj" : $(SOURCE) $(DEP_CPP_RESOU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\spellfn.c
DEP_CPP_SPELL=\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\namedp.h"\
	".\db\spellfn.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\spellfn.obj" : $(SOURCE) $(DEP_CPP_SPELL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\db\cmd.c
DEP_CPP_CMD_C=\
	{$(INCLUDE)}"\typedef.h"\
	".\db\cmd.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\cmd.obj" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_word.c
DEP_CPP_OLC_W=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\db\word.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_word.obj" : $(SOURCE) $(DEP_CPP_OLC_W) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_area.c
DEP_CPP_OLC_A=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_area.obj" : $(SOURCE) $(DEP_CPP_OLC_A) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_clan.c
DEP_CPP_OLC_C=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_clan.obj" : $(SOURCE) $(DEP_CPP_OLC_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_help.c
DEP_CPP_OLC_H=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_help.obj" : $(SOURCE) $(DEP_CPP_OLC_H) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_lang.c
DEP_CPP_OLC_L=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_lang.obj" : $(SOURCE) $(DEP_CPP_OLC_L) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_mob.c
DEP_CPP_OLC_M=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_mob.obj" : $(SOURCE) $(DEP_CPP_OLC_M) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_mpcode.c
DEP_CPP_OLC_MP=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\olc_mpcode.obj" : $(SOURCE) $(DEP_CPP_OLC_MP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_msg.c
DEP_CPP_OLC_MS=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_msg.obj" : $(SOURCE) $(DEP_CPP_OLC_MS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_obj.c
DEP_CPP_OLC_O=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\olc_obj.obj" : $(SOURCE) $(DEP_CPP_OLC_O) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_room.c
DEP_CPP_OLC_R=\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	{$(INCLUDE)}"\Sys\Types.h"\
	

"$(INTDIR)\olc_room.obj" : $(SOURCE) $(DEP_CPP_OLC_R) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc_save.c
DEP_CPP_OLC_S=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\obj_prog.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\db\db.h"\
	{$(INCLUDE)}"\db\word.h"\
	{$(INCLUDE)}"\db\lang.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\olc_save.obj" : $(SOURCE) $(DEP_CPP_OLC_S) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\olc\olc.c
DEP_CPP_OLC_C90=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\merc.h"\
	{$(INCLUDE)}"\interp.h"\
	{$(INCLUDE)}"\olc\olc.h"\
	{$(INCLUDE)}"\typedef.h"\
	{$(INCLUDE)}"\const.h"\
	".\tables.h"\
	".\comm\comm.h"\
	".\db\gsn.h"\
	".\db\spellfn.h"\
	".\db\msg.h"\
	{$(INCLUDE)}"\namedp.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\mlstring.h"\
	{$(INCLUDE)}"\varr.h"\
	".\flag.h"\
	{$(INCLUDE)}"\log.h"\
	{$(INCLUDE)}"\util.h"\
	".\lookup.h"\
	".\recycle.h"\
	".\string_edit.h"\
	".\class.h"\
	".\clan.h"\
	".\skills.h"\
	".\religion.h"\
	

"$(INTDIR)\olc.obj" : $(SOURCE) $(DEP_CPP_OLC_C90) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\compat\winstuff.c
DEP_CPP_WINST=\
	{$(INCLUDE)}"\compat.h"\
	

"$(INTDIR)\winstuff.obj" : $(SOURCE) $(DEP_CPP_WINST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\compat\regex.c
DEP_CPP_REGEX=\
	{$(INCLUDE)}"\Sys\Types.h"\
	{$(INCLUDE)}"\buffer.h"\
	{$(INCLUDE)}"\regex.h"\
	
NODEP_CPP_REGEX=\
	".\compat\lisp.h"\
	".\compat\syntax.h"\
	".\compat\config.h"\
	

"$(INTDIR)\regex.obj" : $(SOURCE) $(DEP_CPP_REGEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\compat\strcasecmp.c
DEP_CPP_STRCA=\
	{$(INCLUDE)}"\compat.h"\
	

"$(INTDIR)\strcasecmp.obj" : $(SOURCE) $(DEP_CPP_STRCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
