#SPEC
Name minor_necromancy~
Class minor_school
Check
if (has_sp(actor, "major_necromancy", rm, add)) {
	act_char("You already have necromancy as your major magic school", actor);
	return 1;
}

if (!has_sp(actor, "class_wizard", rm, add)) {
	act_char("You are not even wizard, how can you have magic school?", actor);
	return 1;
}

return 0;
~
End

#SKILL
Skill 'chill touch'
Level 2
Rating 2
Min 1
Adept 75
Max 100
End

#$
