#ifndef _SKILLS_H_
#define _SKILLS_H_

/*
 * $Id: skills.h,v 1.1 1998-09-01 18:38:02 fjoe Exp $
 */

/*----------------------------------------------------------------------
 * skills stuff
 */

#define SKILL_CLAN	(A)
#define SKILL_RANGE	(B)

struct skill_data {
	char *		name;			/* skill name */
	SPELL_FUN *	spell_fun;		/* spell function */
	sflag_t		target;			/* legal target */
	sflag_t		minimum_position;	/* position for caster */
	int *		pgsn;			/* pointer to gsn */
	int		slot;			/* slot for #OBJOLD loading */
	int		min_mana;		/* min mana used */
	int		beats;			/* waiting time after use */
	char *		noun_damage;		/* damage message */
	char *		msg_off;		/* wear off message */
	char *		msg_obj;		/* wear off message for obj */
	int		flags;			/* skill flags */
	char *		restrict_race;		/* race restrictions */
	sflag_t		group;			/* skill group */
};

extern varr * skills;

#define SKILL(sn)		((SKILL_DATA*) VARR_GET(skills, sn))
#define skill_lookup(sn)	((SKILL_DATA*) varr_get(skills, sn))

char *	skill_name	(int sn);
int	sn_lookup	(const char *name);

/* lookup skill by name in skill list */
void *	skill_vlookup	(varr *v, const char *name);

int	slot_lookup	(int slot);

int	get_weapon_sn	(CHAR_DATA *ch, int type);
int	get_weapon_skill(CHAR_DATA *ch, int sn);

int	get_skill	(CHAR_DATA *ch, int sn);
void	set_skill	(CHAR_DATA *ch, int sn, int value);
int	skill_level	(CHAR_DATA *ch, int sn);

void	update_skills	(CHAR_DATA *ch);

int	mana_cost	(CHAR_DATA *ch, int sn);
void	say_spell	(CHAR_DATA *ch, int sn);

int	base_exp	(CHAR_DATA *ch);
int	exp_to_level	(CHAR_DATA *ch);
int	exp_for_level	(CHAR_DATA *ch, int level);
void	check_improve	(CHAR_DATA *ch, int sn, bool success, int multiplier);
int	group_lookup	(const char *name);

#endif
