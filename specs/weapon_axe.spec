#SPEC
Name weapon_axe~
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
Skill 'axe'
Level 1
Rating 1
Min 1
Adept 75
Max 110
End

#SKILL
Skill 'cleave'
Level 35
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'shield cleave'
Level 26
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'weapon cleave'
Level 24
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'whirl'
Level 23
Rating 1
Min 1
Adept 75
Max 100
End

#$
