#SPEC
Name weapon_sword~
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
Skill 'fence'
Level 15
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'feint'
Level 32
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'counter strike'
Level 23
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'cut'
Level 19
Rating 2
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
Max 110
End

#$
