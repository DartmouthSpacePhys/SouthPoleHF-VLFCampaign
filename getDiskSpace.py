#!/usr/bin/env python
import sys, os, subprocess

filename = '/mnt/data/naryaFreeSpace.txt'
remoteDir = 'canopus@aristotle.dartmouth.edu:/media/Xi_backup/sp/vlf_experiment/'
#get the free disk space
subprocess.call(['df -h > {0}'.format(filename)],shell=True,cwd='/mnt/data')

#copy the file to narya
subprocess.call(['scp {0} {1}'.format(filename,remoteDir)],shell=True,cwd='/mnt/data')
