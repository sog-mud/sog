#!/bin/sh
for i in $*; do
	mv $i $i.orig &&
	sed 's/char_puts *( *"\(.*\)\\n" *, *\([a-zA-Z][a-zA-Z0-9]*\) *) *;/act_char("\1", \2);/' $i.orig > $i && diff -q $i $i.orig && rm $i.orig
done
