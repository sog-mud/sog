CC=cl.exe

CFLAGS=/nologo /ML /W3 /G4e /O2g /D "WIN32" /YX /c
INCLUDES= -I . -I ..\msgdb -I .\COMM -I .\COMPAT -I .\DB -I .\OLC -I .\COMPAT\regex-win32

LINK32=link.exe

LFLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no\
 /machine:I386 /out:"sog.exe"

COMM_CFILES = comm\ban.c comm\charset.c comm\comm.c comm\comm_act.c\
		      comm\comm_colors.c comm\comm_info.c comm\resolver.c
COMM_OFILES = comm\ban.obj comm\charset.obj comm\comm.obj comm\comm_act.obj\
	    	  comm\comm_colors.obj comm\comm_info.obj comm\resolver.obj

DB_CFILES = db\cmd.c db\db.c db\db_area.c db\db_clan.c db\db_class.c\
		    db\db_skills.c db\db_lang.c db\db_race.c\
		    db\gsn.c db\lang.c db\msg.c db\resource.c db\spellfn.c db\word.c
DB_OFILES = db\cmd.obj db\db.obj db\db_area.obj db\db_clan.obj db\db_class.obj\
			db\db_skills.obj db\db_lang.obj db\db_race.obj db\gsn.obj\
			db\lang.obj db\msg.obj db\resource.obj db\spellfn.obj db\word.obj


OLC_CFILES = olc\olc.c olc\olc_area.c olc\olc_clan.c olc\olc_help.c\
		     olc\olc_lang.c olc\olc_mob.c olc\olc_mpcode.c olc\olc_msg.c\
		     olc\olc_room.c olc\olc_obj.c olc\olc_save.c olc\olc_word.c
OLC_OFILES = olc\olc.obj olc\olc_area.obj olc\olc_clan.obj olc\olc_help.obj\
		     olc\olc_lang.obj olc\olc_mob.obj olc\olc_mpcode.obj\
             olc\olc_msg.obj olc\olc_room.obj olc\olc_obj.obj olc\olc_save.obj\
             olc\olc_word.obj


COMPAT_CFILES = compat\winstuff.c compat\strsep.c compat\strcasecmp.c\
				compat\regex-win32\regex_regcomp.c\
				compat\regex-win32\regex_regerror.c\
				compat\regex-win32\regex_regexec.c\
				compat\regex-win32\regex_regfree.c
COMPAT_OFILES = compat\winstuff.obj compat\strsep.obj compat\strcasecmp.obj\
				compat\regex-win32\regex_regcomp.obj\
				compat\regex-win32\regex_regerror.obj\
				compat\regex-win32\regex_regexec.obj\
				compat\regex-win32\regex_regfree.obj

CFILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c auction.c\
         buffer.c clan.c class.c\
		 effects.c fight.c flag.c handler.c healer.c help.c hometown.c\
		 hunt.c interp.c log.c lookup.c magic.c magic2.c martial_art.c mem.c\
		 mlstring.c mob_cmds.c mob_prog.c namedp.c note.c obj_prog.c quest.c\
		 race.c raffects.c rating.c recycle.c religion.c repair.c\
		 save.c skills.c special.c str.c string_edit.c\
		 tables.c update.c util.c varr.c\
		 $(COMM_CFILES) $(DB_CFILES) $(OLC_CFILES) $(COMPAT_CFILES)

OFILES = act_comm.obj act_info.obj act_move.obj act_obj.obj act_wiz.obj\
     auction.obj buffer.obj clan.obj class.obj effects.obj fight.obj flag.obj\
     handler.obj healer.obj help.obj hometown.obj hunt.obj interp.obj log.obj\
     lookup.obj magic.obj magic2.obj martial_art.obj mem.obj mlstring.obj\
     mob_cmds.obj mob_prog.obj namedp.obj note.obj obj_prog.obj quest.obj\
	 race.obj raffects.obj rating.obj recycle.obj religion.obj repair.obj\
	 save.obj skills.obj special.obj str.obj string_edit.obj tables.obj\
     update.obj util.obj varr.obj\
	 $(COMM_OFILES) $(DB_OFILES) $(OLC_OFILES) $(COMPAT_OFILES)


ALL : "sog.exe"

CLEAN :
        -@erase "sog.exe"
        -@erase "*.obj"
        -@erase "COMM\*.obj"
        -@erase "COMPAT\*.obj"
        -@erase "COMPAT\regex-win32\*.obj"
        -@erase "DB\*.obj"
        -@erase "OLC\*.obj"

"sog.exe" : $(DEF_FILE) $(OFILES)
    $(LINK32) @<<
  $(LFLAGS) $(OFILES)
<<

.c.obj: 
	$(CC) $(CFLAGS) $(INCLUDES) /Fo%|pfF.obj $< 
