#
# $Id: genconst.sed,v 1.1 2001-06-18 15:05:35 fjoe Exp $
#

s/^#define /"/
s/	/",/
s/^/	{ /
s/$/},/
