#ifndef _RESOURCE_H_
#define _RESOURCE_H_

char* msg(int resnum, int lang);

#define LANG_ENG 0
#define LANG_RUS 1

#define MAX_STRING 1024

#define INFO_NOTHING 			1
#define EQ_USED_AS_LIGHT		2
#define EQ_WORN_ON_FINGER_1		3
#define EQ_WORN_ON_FINGER_2		4
#define EQ_WORN_AROUND_NECK_1		5
#define EQ_WORN_AROUND_NECK_2		6
#define EQ_WORN_ON_TORSO		7
#define EQ_WORN_ON_HEAD			8
#define EQ_WORN_ON_LEGS			9
#define EQ_WORN_ON_FEET			10
#define EQ_WORN_ON_HANDS		11
#define EQ_WORN_ON_ARMS			12
#define EQ_WORN_AS_SHIELD		13
#define EQ_WORN_ABOUT_BODY		14
#define EQ_WORN_ABOUT_WAIST		15
#define EQ_WORN_AROUND_WRIST_1		16
#define EQ_WORN_AROUND_WRIST_2		18
#define EQ_WIELDED			19
#define EQ_HELD				20
#define EQ_FLOATING_NEARBY		21
#define EQ_SCRATCHED_TATTOO		22
#define EQ_DUAL_WIELDED			23

#define INFO_SLEEPING_AT		24
#define INFO_SLEEPING_ON		25
#define INFO_SLEEPING_IN		26
#define INFO_SLEEPING			27

#endif
