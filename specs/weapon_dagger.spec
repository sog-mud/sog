#SPEC
Name weapon_dagger~
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
Skill 'twist'
Level 20
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'gash'
Level 33
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'close contact'
Level 30
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'downstrike'
Level 17
Rating 2
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
Max 110
End

#$
