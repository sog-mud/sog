#SPEC
Name minor_alteration~
Class minor_school
Check
if (has_sp($n, "major_alteration", $rm, $add)) {
	act_char("You already have alteration as your major magic school", $n);
	return 1;
}

if (!has_sp($n, "class_wizard", $rm, $add)) {
	act_char("You are not even wizard, how can you have magic school?", $n);
	return 1;
}

return 0;
~
End

#SKILL
Skill 'enlarge'
Level 15
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'shrink'
Level 16
Rating 1
Min 1
Adept 75
Max 100
End

#$
