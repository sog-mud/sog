/*
 * $Id: hunt.c,v 1.49 2003-05-08 14:00:07 fjoe Exp $
 */

/* Kak zovut sobaku Gejtsa?
			Sobaku Gejtsa zovut BILL! */
#if defined (WIN32)
#include "compat.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <merc.h>

#include <sog.h>

DECLARE_DO_FUN(do_hunt);
DECLARE_DO_FUN(do_find);

/***************************************************************************
 *  Original idea from SillyMUD v1.1b (C)1993.                             *
 *  Modified to merc2.1 by Rip.                                            *
 *  Modified by Turtle for Merc22 (07-Nov-94).                             *
 *  Adopted to ANATOLIA by Chronos.                                        *
 ***************************************************************************/

#define DEFAULT_DEPTH -40000

static void hunt_victim(CHAR_DATA *ch);
static int find_path(int in_room_vnum, int out_room_vnum,
		     CHAR_DATA *ch, int depth, int in_zone);
static char *find_way(CHAR_DATA *ch, ROOM_INDEX_DATA *rstart,
		      ROOM_INDEX_DATA *rend);

DO_FUN(do_hunt, ch, argument)
{
	char arg[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int direction, i;
	bool fArea, ok;
	int chance2;

	if (IS_NPC(ch) && ch->hunting) {
		hunt_victim(ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Whom are you trying to hunt?", ch);
		return;
	}

	fArea = !IS_IMMORTAL(ch);

	if ((chance2 = get_skill(ch, "world find"))) {
		if (number_percent() < chance2) {
			fArea = FALSE;
			check_improve(ch, "world find", FALSE, 1);
		} else
			check_improve(ch, "world find", TRUE, 1);
	}

	WAIT_STATE(ch, skill_beats("hunt"));

	if (fArea)
		victim = get_char_area(ch, arg);
	else
		victim = get_char_world(ch, arg);

	if (victim == NULL) {
		act_char("No-one around by that name.", ch);
		return;
	}

	if (ch->in_room == victim->in_room) {
		act("$N is here!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_NPC(ch)) {
		ch->hunting = victim;
		hunt_victim(ch);
		return;
	}

	/*
	 * Deduct some movement.
	 */
	if (!IS_IMMORTAL(ch)) {
		if (ch->endur > 2)
			ch->endur -= 3;
		else {
			act_char("You're too exhausted to hunt anyone!", ch);
			return;
		}
	}

	act("$n stares intently at the ground.", ch, NULL, NULL, TO_ROOM);

	direction = find_path(ch->in_room->vnum, victim->in_room->vnum,
			      ch, DEFAULT_DEPTH, fArea);
	if (direction < 0) {
		act("You couldn't find a path to $N from here.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (direction >= MAX_DIR) {
		act_char("Hmm... Something seems to be wrong.", ch);
		return;
	}

	/*
	 * Give a random direction if the player misses the die roll.
	 */
	if (IS_NPC (ch) && number_percent () > 75) {       /* NPC @ 25% */
		ok = FALSE;
		for(i = 0; i < 6; i++) {
			if (ch->in_room->exit[direction]) {
				ok = TRUE;
				break;
			}
		}

		if (ok)	{
			do {
				direction = number_door();
			}
			while ((ch->in_room->exit[direction] == NULL) ||
			       (ch->in_room->exit[direction]->to_room.r == NULL));
		} else {
			log(LOG_BUG, "do_hunt(player): no exits from room!");
			ch->hunting = NULL;
			act_char("Your room has not exits!!!!", ch);
			return;
		}
	}

	/*
	 * Display the results of the search.
	 */
	act_puts("$N is $t from here.",
		 ch, dir_name[direction], victim, TO_CHAR, POS_DEAD);
}

DO_FUN(do_find, ch, argument)
{
	char* path;
	ROOM_INDEX_DATA *location;

	if (argument[0] == '\0') {
		act_char("Ok. But what I should find?", ch);
		return;
	}

	if ((location = find_location(ch, argument)) == NULL) {
		act_char("No such location.", ch);
		return;
	}

	path = find_way(ch, ch->in_room, location);
	act_puts("$t.", ch, path, NULL,				// notrans
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
	log(LOG_INFO, "From %d to %d: %s.\n",
		   ch->in_room->vnum, location->vnum, path);
}

/*--------------------------------------------------------------------
 * static functions
 */

typedef struct room_q room_q;
struct room_q
{
	int	room_nr;
	room_q	*next_q;
};

typedef struct room_d room_d;
struct room_d
{
	int vnum;
	int exit;
};

static avltree_info_t c_info_room = {
	&avltree_ops, NULL, NULL,

	MT_PVOID, sizeof(room_d), ke_cmp_int,
};

static int
find_path(int in_room_vnum, int out_room_vnum,
	  CHAR_DATA *ch __attribute__((unused)), int depth, int in_zone)
{
	room_q *q_head, *q_tail, *tmp_q;
	avltree_t x_room;
	room_d *d_room;
	bool thru_doors;
	int count;
	ROOM_INDEX_DATA	*startp;

	if (depth < 0) {
		thru_doors = TRUE;
		depth = -depth;
	} else
		thru_doors = FALSE;

	startp = get_room_index(in_room_vnum);

	c_init(&x_room, &c_info_room);
	d_room = c_insert(&x_room, &in_room_vnum);
	d_room->vnum = in_room_vnum;
	d_room->exit = -1;

	/* initialize queue */
	q_head = (room_q *) malloc(sizeof(room_q));
	q_tail = q_head;
	q_tail->room_nr = in_room_vnum;
	q_tail->next_q = 0;

	count = 0;
	for (; q_head; tmp_q = q_head->next_q, free(q_head), q_head = tmp_q) {
		int i;
		ROOM_INDEX_DATA *herep;

		herep = get_room_index(q_head->room_nr);
		if (herep == NULL) {
			log(LOG_BUG, "Null herep in hunt.c, room #%d",
			    q_head->room_nr);
			continue;
		}

		if (in_zone && herep->area != startp->area)
			continue;

		/* for each room test all directions */
		for (i = 0; i < MAX_DIR; i++) {
			room_d *dp;
			int tmp_room;
			EXIT_DATA *pexit = herep->exit[i];

			if (pexit == NULL
			||  pexit->to_room.r == NULL
			||  (!thru_doors &&
			     !IS_SET(pexit->exit_info, EX_CLOSED)))
				continue;

			/* next room */
			tmp_room = pexit->to_room.r->vnum;
			if (tmp_room == out_room_vnum) {
				int rv;

				/* have reached our goal so free queue */
				tmp_room = q_head->room_nr;
				for(; q_head; q_head = tmp_q) {
					tmp_q = q_head->next_q;
					free(q_head);
				}

				/* return direction if first layer */
				dp = c_lookup(&x_room, &tmp_room);
				if (dp == NULL) {
					log(LOG_BUG, "key %d: not found",
					    tmp_room);
					c_destroy(&x_room);
					return -1;
				}

				rv = dp->exit == -1 ? i : dp->exit;
				c_destroy(&x_room);
				return rv;
			}

			/*
			 * shall we add room to queue?
			 * count determines total breadth and depth
			 */
			if (c_lookup(&x_room, &tmp_room) != NULL
			||  count++ >= depth)
				continue;

			/* mark room as visted and put on queue */
			tmp_q = (room_q *) malloc(sizeof(room_q));
			tmp_q->room_nr = tmp_room;
			tmp_q->next_q = NULL;
			q_tail->next_q = tmp_q;
			q_tail = tmp_q;

			/* ancestor for first layer is the direction */
			dp = c_lookup(&x_room, &q_head->room_nr);
			if (dp == NULL) {
				log(LOG_BUG, "vnum %d: ancestor not found",
				    q_head->room_nr);
				continue;
			}

			d_room = c_insert(&x_room, &tmp_room);
			d_room->vnum = tmp_room;
			d_room->exit = (dp->exit == -1 ? i : dp->exit);
		}
	}

	/* couldn't find path */
	c_destroy(&x_room);
	return -1;
}

static void
hunt_victim_attack(CHAR_DATA* ch)
{
	if (ch->hunting == NULL)
		return;

	if (ch->in_room == ch->hunting->in_room) {
		act("$n glares at $N and says, '{GYe shall DIE!{x'",
		    ch, NULL, ch->hunting, TO_NOTVICT);
		act("$n glares at you and says, '{GYe shall DIE!{x'",
		    ch, NULL, ch->hunting, TO_VICT);
		act("You glare at $N and say, '{GYe shall DIE!{x'",
		    ch, NULL, ch->hunting, TO_CHAR);
		multi_hit(ch, ch->hunting, NULL);
		ch->hunting = NULL;
	}  
}

/*
 * revised by chronos.
 */
static void
hunt_victim(CHAR_DATA *ch)
{
	int		dir;
	bool		found;
	CHAR_DATA	*tmp;

	/*
	 * Make sure the victim still exists.
	 */
	for (found = FALSE, tmp = char_list; tmp; tmp = tmp->next) {
		if (ch->hunting == tmp) {
			found = TRUE;
			break;
		}
	}

	if (!found || !can_see(ch, ch->hunting)) {
		if (get_char_area(ch, ch->hunting->name) != NULL) {
			char buf[MAX_INPUT_LENGTH];

			snprintf(buf, sizeof(buf), "portal %s",	// notrans
				 ch->hunting->name);
			dofun("cast", ch, buf);
			dofun("enter", ch, "portal");
			hunt_victim_attack(ch);
		} else {
			dofun("say", ch, "Ahhhh!  My prey is gone!!");
			ch->hunting = NULL;
		}
		return;
	}   /* end if !found or !can_see */

	dir = find_path(ch->in_room->vnum, ch->hunting->in_room->vnum,
			ch, DEFAULT_DEPTH, TRUE);

	if (dir < 0 || dir >= MAX_DIR) {
		if (get_char_area(ch, ch->hunting->name) != NULL
		&&  ch->level > 35) {
			char buf[MAX_INPUT_LENGTH];

			snprintf(buf, sizeof(buf), "portal %s",	// notrans
				 ch->hunting->name);
			dofun("cast", ch, buf);
			dofun("enter", ch, "portal");
			hunt_victim_attack(ch);
		} else {
			act_say(ch, "I have lost $N!", ch->hunting);
			ch->hunting = NULL;
		}
		return;
	} /* if dir < 0 or >= MAX_DIR */

	if (ch->in_room->exit[dir]
	&&  IS_SET(ch->in_room->exit[dir]->exit_info, EX_CLOSED)) {
		dofun("open", ch, dir_name[dir]);
		return;
	}

	if (!ch->in_room->exit[dir]) {
		log(LOG_BUG, "hunt through null door");
		ch->hunting = NULL;
		return;
	}
	move_char(ch, dir, 0);
	hunt_victim_attack(ch);
}

static char *
find_way(CHAR_DATA *ch, ROOM_INDEX_DATA *rstart, ROOM_INDEX_DATA *rend)
{
	int direction;
	static char buf[1024];
	EXIT_DATA *pExit;
	char buf2[2];

	snprintf(buf, sizeof(buf), "Bul: ");			// notrans
	while (1) {
		if ((rend == rstart))
			return buf;

		if ((direction = find_path(rstart->vnum, rend->vnum,
					   ch, DEFAULT_DEPTH, 0)) == -1) {
			strnzcat(buf, sizeof(buf), " BUGGY"); // notrans
			return buf;
		}

		if (direction < 0 || direction > 5) {
			strnzcat(buf, sizeof(buf), " VERY BUGGY"); // notrans
			return buf;
		}

		buf2[0] = dir_name[direction][0];
		buf2[1] = '\0';
		strnzcat(buf, sizeof(buf), buf2);

		/* find target room */
		pExit = rstart->exit[direction];
		if (!pExit)  {
			strnzcat(buf, sizeof(buf),
				 " VERY VERY BUGGY"); // notrans
			return buf;
		} else
			rstart = pExit->to_room.r;
	}
}
