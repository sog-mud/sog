#SPEC
Name minor_abjuration~
Class minor_school
Check
if (has_sp(actor, "major_abjuration", rm, add)) {
	act_char("You already have abjuration as your major magic school", actor);
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
Skill 'shield'
Level 15
Rating 2
Min 1
Adept 75
Max 100
End

#$
