#!/bin/sh
#
# NOTE: trailing slashes are significant
local_cvsroot=/usr/local/cvs/SoG-CVS/

#
# compression is turned on in ~/.ssh/config (see ssh_config)
rsync -e ssh -av --delete \
	--exclude '.#cvs.*' --exclude '*.core' --exclude 'core' --delete-excluded \
	cvs-sog:/usr/local/cvsroot/ ${local_cvsroot}
