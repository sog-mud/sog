#SPEC
Name major_necromancy~
Class major_school
Check
if (has_sp(actor, "minor_necromancy", rm, add)) {
	act_char("You already have necromancy as your minor magic school", actor);
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
Skill 'animate dead'
Level 20
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'bone dragon'
Level 40
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'chill touch'
Level 2
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'control undead'
Level 23
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'corruption'
Level 33
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'death wave'
Level 40
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'detect undead'
Level 9
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'disruption'
Level 26
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'draining touch'
Level 13
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'energy drain'
Level 20
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'hand of undead'
Level 24
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'mana drain'
Level 30
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'power word kill'
Level 38
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'protection negative'
Level 25
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'summon shadow'
Level 16
Rating 1
Min 1
Adept 75
Max 100
End

#$
