#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "merc.h"
#include "rating.h"
#include "db.h"
#include "comm.h"

struct rating_data {
	char *name;
	int pc_killed;
};

#define RATING_TABLE_SIZE 20
struct rating_data rating_table[RATING_TABLE_SIZE];

/*
 * Updates player's rating.
 * Should be called every death.
 */
void rating_update(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (IS_NPC(ch) || IS_NPC(victim) || ch == victim || IS_IMMORTAL(ch))
		return;

	rating_add(ch->name, ++ch->pcdata->pc_killed);
}

/*
 * rating_add - add (or replace) `name/pc_killed' to (in) rating_table
 */
void rating_add(char* name, int pc_killed)
{
	int i;
	struct rating_data *p = rating_table;

	for (i = 0; i < RATING_TABLE_SIZE; i++) {
		if (str_cmp(name, rating_table[i].name) == 0) {
			rating_table[i].pc_killed = pc_killed;
			return;
		}
		if (rating_table[i].pc_killed < p->pc_killed)
			p = rating_table + i;
	}

	if (p->pc_killed < pc_killed) {
		if (p->name != NULL)
			free_string(p->name);
		p->name = str_dup(name);
		p->pc_killed = pc_killed;
	} 	
}

static
int
rating_data_cmp(const void *a, const void *b)
{
	return ((struct rating_data*) b)->pc_killed 
		- ((struct rating_data*) a)->pc_killed;
}

void do_rating(CHAR_DATA *ch, char *argument)
{
	int i;

	qsort(rating_table, RATING_TABLE_SIZE, sizeof(struct rating_data),
	      rating_data_cmp);

	send_to_char("Name                    | PC's killed\n\r", ch);
	send_to_char("------------------------+------------\n\r", ch);
	for (i = 0; i < RATING_TABLE_SIZE; i++) {
		if (rating_table[i].name == NULL)
			continue;
		char_printf(ch, "%-24s| %d\n\r",
			    rating_table[i].name, rating_table[i].pc_killed);
	}
	char_printf(ch, "\n\rYou have killed {R%d{x player(s).\n\r",
		    ch->pcdata->pc_killed);
}

