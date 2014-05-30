#!/usr/bin/env python
import sys, os, subprocess

RemoveList = open('/mnt/data/vlf_experiment/RemoveList.txt','r')
DataDir = '/mnt/data/vlf_experiment/data_files/'
for line in RemoveList:
	Filename = DataDir + line.strip()
	try:
		subprocess.call(['rm {0}'.format(Filename)],cwd=DataDir,shell=True)
	except:
		continue
