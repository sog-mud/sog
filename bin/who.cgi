#!/usr/local/bin/perl
#-
# Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $Id: who.cgi,v 1.2.4.1 2002-09-01 17:43:56 tatyana Exp $
#

$netcat = "/usr/local/bin/nc";
$port = 4001;

print << "EOF";
Content-type: text/HTML

<html>
<head><title>
List of players, who are online now in the Shades of Gray"
</title></head>
<body bgcolor=#000000><font size=4>
<font color=#C0C0C0>List of players, who are online now in the Muddy Realms:

<pre>
EOF

open(LIST, "echo html | $netcat localhost $port |") || die "Cannot connect to server.";

$max = <LIST>;
$found = 0;
while(<LIST>) {
	print;
	$found++;
}
printf "\n<font color=#C0C0C0>Players found: %d. Most so far today: %d.\n", $found, $max;

print << "EOF";
</pre>
</body>
</html>
EOF
