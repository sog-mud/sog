#
# $Id: genconst.sed,v 1.2 2001-08-30 18:50:11 fjoe Exp $
#

s/^#define /"/
s/	/",/
s/^/	{ /
s/$/,	MT_INT },/
