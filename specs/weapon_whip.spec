#SPEC
Name weapon_whip~
Class weapon
Check
if (!has_sp(actor, "class_warrior", rm, add)) {
	act_char("You aren't professional warrior.", actor);
	return 1;
}

return 0;
~
End

#SKILL
Skill 'weapon strip'
Level 25
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'entanglement'
Level 35
Rating 2
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
Max 110
End

#SKILL
Skill 'flail'
Level 1
Rating 1
Min 1
Adept 75
Max 110
End

#$

