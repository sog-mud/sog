#SPEC
Name major_charm~
Class major_school
Check
if (has_sp(actor, "minor_charm", rm, add)) {
	act_char("You already have enchantment as your minor magic school", actor);
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
Skill 'adamantite golem'
Level 44
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'attract other'
Level 25
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'charm person'
Level 35
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'enchant armor'
Level 19
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'enchant armor'
Level 33
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'enchant weapon'
Level 22
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'enchant weapon'
Level 17
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'fireproof'
Level 20
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'insanity'
Level 40
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'iron golem'
Level 35
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'mend'
Level 40
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'mysterious dream'
Level 38
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'recharge'
Level 25
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'sleep'
Level 12
Rating 1
Min 1
Adept 75
Max 100
End

#$
