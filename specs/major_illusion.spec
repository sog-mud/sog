#SPEC
Name major_illusion~
Class major_school
Check
if (has_sp(actor, "minor_illusion", rm, add)) {
	act_char("You already have illusion as your minor magic school", actor);
	return 1;
}

if (!has_sp(actor, "class_wizard", rm, add)) {
	act_char("You are not even wizard, how can you have major school?", actor);
	return 1;
}

return 0;
~
End

#SKILL
Skill 'blindness'
Level 10
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'blur'
Level 20
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'deafen'
Level 20
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'doppelganger'
Level 30
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'fear'
Level 20
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'hallucination'
Level 40
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'improved invis'
Level 25
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'invisibility'
Level 3
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'mass invis'
Level 10
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'mirror'
Level 20
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'misleading'
Level 20
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'phantasmal force'
Level 25
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'shadow magic'
Level 38
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'simulacrum'
Level 43
Rating 1
Min 1
Adept 75
Max 100
End

#$
