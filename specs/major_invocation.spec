#SPEC
Name major_invocation~
Class major_school
Check
if (has_sp($n, "minor_invocation", $rm, $add)) {
	act_char("You already have invocation as your minor magic school", $n);
	return 1;
}

if (!has_sp($n, "class_wizard", $rm, $add)) {
	act_char("You are not even wizard, how can you have major school?", $n);
	return 1;
}

return 0;
~
End

#SKILL
Skill 'acid blast'
Level 35
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'chain lightning'
Level 31
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'cold shield'
Level 30
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'fire shield'
Level 30
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'fireball'
Level 35
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'iceball'
Level 34
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'shocking grasp'
Level 9
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'web'
Level 30
Rating 1
Min 1
Adept 75
Max 100
End

#$
