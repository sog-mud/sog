#SPEC
Name major_abjuration~
Class major_school
Check
if (!has_sp($n, "class_wizard", $rm, $add)) {
	act_char("You are not even wizard, how can you have major school?", $n);
	return 1;
}

if (has_sp($n, "minor_abjuration", $rm, $add)) {
	act_char("You have already chosen abjuration as your minor magic school", $n);
	return 1;
}

return 0;
~
End

#SKILL
Skill 'alarm'
Level 28
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'dispel magic'
Level 10
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'faerie fire'
Level 12
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'meld into stone'
Level 40
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'protection cold'
Level 21
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'protection from missiles'
Level 26
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'protection heat'
Level 23
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'protective shield'
Level 5
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'shield'
Level 15
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'stone skin'
Level 35
Rating 2
Min 1
Adept 75
Max 100
End

#$
