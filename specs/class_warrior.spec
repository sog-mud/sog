#SPEC
Name class_warrior~
Class class
Check
act_char("Yo!", actor);

int spc_count = spclass_count(actor, "weapon", rm, add);
if ((actor->real_level < 15 && spc_count > 1)
||  (actor->real_level < 35 && spc_count > 2)) {
	act_char("You aren't expierenced enough to specialize in other type of weapon.", actor);
	return 1;
}

if (spc_count > 3) {
	act_char("You cannot gain new weapon specializations anymore.", actor);
	return 1;
}

return 0;
~
End

#SKILL
Skill 'area attack'
Level 41
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'axe'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'bash'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'bash door'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'berserk'
Level 12
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'blind fighting'
Level 13
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'bow'
Level 34
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'concentrate'
Level 30
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'counter'
Level 28
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'dagger'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'dirt kicking'
Level 5
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'disarm'
Level 12
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'dodge'
Level 10
Rating 1
Min 1
Adept 75
Max 140
End

#SKILL
Skill 'dual wield'
Level 18
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'enhanced damage'
Level 2
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'fast healing'
Level 5
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'fifth attack'
Level 38
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'flail'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'fourth attack'
Level 22
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'grip'
Level 23
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'haggle'
Level 14
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'hand to hand'
Level 4
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'kick'
Level 3
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'lore'
Level 17
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'mace'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'meditation'
Level 10
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'parry'
Level 1
Rating 1
Min 1
Adept 75
Max 140
End

#SKILL
Skill 'polearm'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'rescue'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'second attack'
Level 8
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'shield block'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'smithing'
Level 32
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'spear'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'staff'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'sword'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'third attack'
Level 17
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'throw weapon'
Level 27
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'trip'
Level 15
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'warcry'
Level 12
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'whip'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#$
