#SPEC
Name weapon_spear~
Class weapon
Check
if (!has_sp($n, "class_warrior", $rm, $add)) {
	act_char("You aren't professional warrior.", $n);
	return 1;
}

return 0;
~
End

#SKILL
Skill 'distance'
Level 20
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
Max 110
End

#SKILL
Skill 'polearm'
Level 1
Rating 1
Min 1
Adept 75
Max 110
End

#SKILL
Skill 'staff'
Level 1
Rating 1
Min 1
Adept 75
Max 110
End

#$
