#!/usr/bin/env python

import subprocess
RemoteDir = 'aurora@157.132.24.200:/mnt/data/vlf_experiment/'
workDir = '/media/Xi_backup/sp/vlf_experiment'
subprocess.call(['scp RemoveList.txt {0}'.format(RemoteDir)],shell=True,cwd=workDir)
subprocess.call(['scp ChopList.txt {0}'.format(RemoteDir)],shell=True,cwd=workDir)
