#SPEC
Name minor_divination~
Class minor_school
Check
if (!has_sp($n, "class_wizard", $rm, $add)) {
	act_char("You are not even wizard, how can you have magic school?", $n);
	return 1;
}

return 0;
~
End

#SKILL
Skill 'detect charm'
Level 6
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'identify'
Level 10
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'locate object'
Level 15
Rating 2
Min 1
Adept 75
Max 100
End

#$
