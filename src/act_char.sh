#!/bin/sh
for i in $*; do
	cp $i $i.old && \
	([ -f $i.orig ] || mv $i $i.orig) && \
	sed \
		-e 's/char_puts *( *"\(.*\)\\n" *, *\([a-zA-Z_][a-zA-Z0-9]*\) *) *;/act_char("\1", \2);/' \
		-e 's/char_printf *( *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, *"\(.*\)%s\(.*\)\\n", *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *) *;/XXX act_puts("\2$t\3", \1, \4, NULL, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);/' \
		-e 's/char_printf *(ch, *"\(.*\)%s\(.*\)\\n", flag_string(\(.*\)));/XXX act_puts("\1$t\2", ch, flag_string(\3), NULL, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);/' \
		-e 's/char_printf *(ch, *"\(.*\)%d\(.*\)\\n", *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *) *;/XXX act_puts("\1$j\2", ch, (const void *) \3, NULL, TO_CHAR, POS_DEAD);/' \
		-e 's/char_printf *(ch, *"\(.*\)%s\(.*\)%s\(.*\)\\n", *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *) *;/XXX act_puts("\1$t\2$T\3", ch, \4, \5, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);/' \
		-e 's/char_printf *(ch, *"\(.*\)%s\(.*\)%d\(.*\)\\n", *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *) *;/XXX act_puts("\1$T\2$j\3", ch, (const void *) \5, \4, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);/' \
		-e 's/char_printf *(ch, *"\(.*\)%s\(.*\)%d\(.*\)\\n", *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, varr_index(\(.*\))) *;/XXX act_puts("\1$T\2$j\3", ch, (const void *) varr_index(\5), \4, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);/' \
		-e 's/char_printf *(ch, *"\(.*\)%d\(.*\)%s\(.*\)\\n", *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *) *;/XXX act_puts("\1$j\2$T\3", ch, (const void *) \4, \5, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);/' \
		-e 's/char_printf *(ch, *"\(.*\)%s\(.*\)%s\(.*\)%s\(.*\)\\n", *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *) *;/XXX act_puts3("\1$t\2$T\3$U\4", ch, \5, \6, \7, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);/' \
		-e 's/char_printf *(ch, *"\(.*\)%s\(.*\)%s\(.*\)%s\(.*\)\\n", *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, flag_string(\(.*\)) *) *;/XXX act_puts3("\1$t\2$T\3$U\4", ch, \5, \6, flag_string(\7), TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);/' \
		-e 's/char_printf *(ch, *"\(.*\)%s\(.*\)%s\(.*\)%s\(.*\)\\n", *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, flag_string(\(.*\)), *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *) *;/XXX act_puts3("\1$t\2$T\3$U\4", ch, \5, flag_string(\6), \7, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);/' \
		-e 's/char_printf *(ch, *"\(.*\)%d\(.*\)%s\(.*\)%s\(.*\)\\n", *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, *\([a-zA-Z_\*][a-zA-Z_0-9>.() \/\*-]*\) *, word_form(\(.*\)) *) *;/XXX act_puts3("\1$j\2$T\3$U\4", ch, (const void *) \5, \6, word_form(\7), TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);/' \
		$i.old > $i && diff -q $i $i.orig && rm $i.orig $i.old
done
