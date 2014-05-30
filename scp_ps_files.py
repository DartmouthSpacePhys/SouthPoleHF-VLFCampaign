#!/usr/bin/env python

import sys, subprocess, glob

DataDir = '/mnt/data/vlf_experiment/ps_files/'
TransferredDir = '/mnt/data/vlf_experiment/ps_files/transferred/'
RemoteDir = 'canopus@aristotle.dartmouth.edu:/media/Xi_backup/sp/vlf_experiment/'

#Iterate over a list of files in DataDir that end in ps
for PsFile in glob.glob(DataDir + '*.pdf'):
	#try to scp the file
	ret = subprocess.check_call(['scp {0} {1}'.format(PsFile,RemoteDir)],cwd=DataDir,shell=True)

	#If we're successful, move the file to the transferred directory
	if (ret == 0):
		subprocess.call(['mv {0} {1}'.format(PsFile,TransferredDir)],shell=True,cwd=DataDir)
	else:
		continue
		
