#SPEC
Name major_divination~
Class major_school
End

#R
deny has_spec has_spec("minor_divination")~
deny has_spec !has_spec("class_wizard")~
end

#SKILL
Skill 'improved detect'
Level 35
Rating 2
Min 1
Adept 75
Max 100
End

#$
