#SPEC
Name major_summonning~
Class major_school
Check
if (has_sp($n, "minor_summoning", $rm, $add)) {
	act_char("You already have summoning as your minor magic school", $n);
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
Skill 'demon summon'
Level 30
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'find familiar'
Level 1
Rating 1
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'summon shadow'
Level 35
Rating 2
Min 1
Adept 75
Max 100
End

#$
