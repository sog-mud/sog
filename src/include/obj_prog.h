#ifndef _OBJ_PROG_H_
#define _OBJ_PROG_H_

/*
 * $Id: obj_prog.h,v 1.1 1998-06-28 04:47:16 fjoe Exp $
 */

void oprog_set(OBJ_INDEX_DATA*, const char*, const char*);
int oprog_call(int optype, OBJ_DATA *obj, CHAR_DATA *ch, void *arg);
int optype_lookup(const char *name);
char *oprog_name_lookup(OPROG_FUN* fn);

extern char* optype_table[];

#endif
