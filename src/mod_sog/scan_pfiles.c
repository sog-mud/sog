/*-
 * Copyright (c) 2001 SoG Development Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: scan_pfiles.c,v 1.4 2001-08-20 16:47:45 fjoe Exp $
 */

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#if	defined (WIN32)
#	include <compat/compat.h>
#else
#	include <dirent.h>
#endif

#include <merc.h>

#include <sog.h>

/*
 * rip limited eq from containers
 */
static void
rip_limited_eq(CHAR_DATA *ch, OBJ_DATA *container)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	for (obj = container->contains; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;

		obj->pObjIndex->count++;

		/*
		 * extract_obj(xxx, XO_F_NORECURSE) will add objects to the
		 * beginning of the container->contains list --
		 * check nested containers first
		 */
		if (obj->pObjIndex->item_type == ITEM_CONTAINER)
			rip_limited_eq(ch, obj);

		if (obj->pObjIndex->limit < 0)
			continue;

		extract_obj(obj, XO_F_NORECURSE);
		log(LOG_INFO, "scan_pfiles: %s: %s (vnum %d)",
		    ch->name,
		    mlstr_mval(&obj->pObjIndex->short_descr),
		    obj->pObjIndex->vnum);
	}
}

/*
 * Count all objects in pfiles
 * Remove limited objects (with probability 1/10)
 * Update rating list
 */
void scan_pfiles()
{
	struct dirent *dp;
	DIR *dirp;
	bool eqcheck = dfexist(TMP_PATH, EQCHECK_FILE);
	bool eqcheck_save_all = dfexist(TMP_PATH, EQCHECK_SAVE_ALL_FILE);

	log(LOG_INFO, "scan_pfiles: start (eqcheck: %s, save all: %s)",
	    eqcheck ? "yes" : "no",				// notrans
	    eqcheck_save_all ? "yes" : "no");			// notrans

	if (eqcheck
	&&  dunlink(TMP_PATH, EQCHECK_FILE) < 0)
		log(LOG_INFO, "scan_pfiles: unable to deactivate 'eqcheck' (%s)", strerror(errno));

	if (eqcheck_save_all
	&&  dunlink(TMP_PATH, EQCHECK_SAVE_ALL_FILE) < 0)
		log(LOG_INFO, "scan_pfiles: unable to deactivate 'save all' (%s)", strerror(errno));

	if ((dirp = opendir(PLAYER_PATH)) == NULL) {
		log(LOG_ERROR, "scan_pfiles: unable to open player directory");
		exit(1);
	}

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		CHAR_DATA *ch;
		OBJ_DATA *obj, *obj_next;
		bool changed = FALSE;
		struct stat s;
		bool should_clear = FALSE;
		bool pet = FALSE;

#if defined (LINUX) || defined (WIN32)
		if (strlen(dp->d_name) < 3)
			continue;
#else
		if (dp->d_namlen < 3 || dp->d_type != DT_REG)
			continue;
#endif
		if (strchr(dp->d_name, '.')
		||  (ch = char_load(dp->d_name, LOAD_F_NOCREATE)) == NULL)
			continue;

		/* Remove limited eq from the pfile if it's two weeks old */
		if (dstat(PLAYER_PATH, dp->d_name, &s) < 0) {
			log(LOG_ERROR, "scan_pfiles: unable to stat %s.",
			    dp->d_name);
		} else {
			should_clear =
			    (current_time - s.st_mtime) > 60*60*24*14;
		}

		for (obj = ch->carrying; obj; obj = obj_next) {
			obj_next = obj->next_content;

			if (!obj_next && !pet && GET_PET(ch)) {
				obj_next = GET_PET(ch)->carrying;
				pet = TRUE;
			}

			obj->pObjIndex->count++;

			/*
			 * always rip limited eq from containers
			 */
			if (obj->pObjIndex->item_type == ITEM_CONTAINER)
				rip_limited_eq(ch, obj);

			/*
			 * skip not limited objects
			 * always clear if char is two weeks old
			 * otherwise clear if we are doing eqcheck with
			 * probability 6%
			 *
			 * !(should_clear || (eqcheck && number_percent() < 7))
			 * <=>
			 * !should_clear && !(eqcheck && number_percent() < 7)
			 * <=>
			 * !should_clear && (!eqcheck || number_percent() < 95)
			 */
			if (obj->pObjIndex->limit < 0
			||  (!should_clear &&
			     (!eqcheck || number_percent() < 95)))
				continue;

			changed = TRUE;
			log(LOG_INFO, "scan_pfiles: %s: %s (vnum %d)",
				   ch->name,
				   mlstr_mval(&obj->pObjIndex->short_descr),
				   obj->pObjIndex->vnum);
			extract_obj(obj, XO_F_NORECURSE);
		}

		if (!IS_IMMORTAL(ch))
			rating_add(ch);

		if (eqcheck_save_all
		||  changed
		||  PC(ch)->version < PFILE_VERSION)
			char_save(ch, SAVE_F_PSCAN);

		char_nuke(ch);
	}
	closedir(dirp);

	log(LOG_INFO, "scan_pfiles: end (eqcheck: %s, save all: %s)",
	    dfexist(TMP_PATH, EQCHECK_FILE) ?
		"yes" : "no",					// notrans
	    dfexist(TMP_PATH, EQCHECK_SAVE_ALL_FILE) ?
		"yes" : "no");					// notrans
}

