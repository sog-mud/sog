#SPEC
Name major_alteration~
Class major_school
Check
if (has_sp($n, "minor_alteration", $rm, $add)) {
	act_char("You already have alteration as your minor magic school", $n);
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
Skill 'colour spray'
Level 15
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'enlarge'
Level 14
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'giant strength'
Level 10
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'haste'
Level 18
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'infravision'
Level 5
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'knock'
Level 42
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'levitation'
Level 17
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'light'
Level 40
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'nexus'
Level 40
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'pass door'
Level 15
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'polymorph'
Level 38
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'portal'
Level 35
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'shrink'
Level 15
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'slow'
Level 20
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'water breathing'
Level 13
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'weaken'
Level 12
Rating 1
Min 1
Adept 75
Max 100
End

#$
