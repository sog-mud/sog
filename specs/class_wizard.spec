#SPEC
Name class_wizard~
Class class
Check
if (spclass_count(actor, "major_school", rm, add) > 1) {
	act_char("You have already chosen your major school", actor);
	return 1;
}

if ( spclass_count(actor, "minor_school", rm, add) > 2) {
	act_char("You have already chosen both your minor schools.", actor);
	return 1;
}

return 0;
~
End

#SKILL
Skill 'armor'
Level 13
Rating 1
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
Max 100
End

#SKILL
Skill 'detect invis'
Level 5
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'detect magic'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'dispel magic'
Level 25
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'faerie fire'
Level 15
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'faerie fog'
Level 27
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'fast healing'
Level 19
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'haggle'
Level 17
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'hand to hand'
Level 25
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'identify'
Level 12
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'infravision'
Level 10
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'magic missile'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'meditation'
Level 6
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'parry'
Level 15
Rating 1
Min 1
Adept 75
Max 90
End

#SKILL
Skill 'protective shield'
Level 10
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'scrolls'
Level 1
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'second attack'
Level 25
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'spell craft'
Level 10
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'staff'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'staves'
Level 1
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'teleport'
Level 20
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'trance'
Level 14
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'ventriloquate'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'wands'
Level 1
Rating 1
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
Max 100
End

#$
