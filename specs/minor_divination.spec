#SPEC
Name minor_divination~
Class minor_school
End

#R
deny has_spec has_spec("major_divination")~
deny has_spec !has_spec("class_wizard")~
end

#SKILL
Skill 'locate object'
Level 15
Rating 2
Min 1
Adept 75
Max 100
End

#SKILL
Skill 'identify'
Level 10
Rating 1
Adept 75
Max 100
End

#$
