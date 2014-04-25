#!/usr/bin/env python

import glob, sys, os, subprocess
print "Import successful"
DataDir = '/data/vlf_experiment/'

#infile = sys.argv[-1]

print "Starting"
#remote computer used for data storage
RemoteDir = 'aurora@157.132.24.200:/mnt/data/vlf_experiment/'
DataDir = '/data/vlf_experiment/'
#Find all data files
for infile in glob.glob(DataDir + '*.data'):
	print infile
	pathSplit = infile.split('/')
	filename = pathSplit[-1]
	print "filename = {0}".format(filename)
	#Open and write the definition file
	Def = DataDir + 'definition'
	FileDef = open(Def,'w')
	FileDef.write('y_data_num\t{0}\n'.format(8192))
	FileDef.write('y_data_min\t{0}\n'.format(0))
	FileDef.write('y_data_max\t{0}\n'.format(10000))
	FileDef.write('y_label_min\t{0}\n'.format(0))
	FileDef.write('y_label_max\t{0}\n'.format(10000))
	FileDef.write('y_label_increment\t{0}\n'.format(1000))
	FileDef.write('data_black_level\t{0}\n'.format(125))
	FileDef.write('data_white_level\t{0}\n'.format(85))
	FileDef.write('title\t{0}\n'.format("South Pole Station"))
	FileDef.write('subtitle\n')
	FileDef.write('y_axis_label\n')
	FileDef.write('x_axis_label\n')
	FileDef.write('use_magic_stamps\tno\n')
	FileDef.write('magic_char\t*\n')
	FileDef.close()
	

	#call the fortran program
	subprocess.call(['/data/vlf_experiment/vlf_decimate {0}'.format(infile)],shell=True,cwd=DataDir)

	HighResGray = DataDir + 'detail.graydata'
	SummaryGray = DataDir + 'summary.graydata'
#	FullData=DataDir + 'full_data'

	#Make the grayscale
	subprocess.call(['/usr/local/bin/gray -d {0} {1}'.format(Def, HighResGray)],shell=True,cwd=DataDir)
	subprocess.call(['mv {0} {1}'.format(DataDir + 'gray.ps',DataDir + 'ps_files/'+filename+'.highres.ps')],shell=True,cwd=DataDir)
	subprocess.call(['/usr/local/bin/gray -d {0} {1}'.format(Def, SummaryGray)],shell=True,cwd=DataDir)
	subprocess.call(['mv {0} {1}'.format(DataDir + 'gray.ps',DataDir + 'ps_files/'+filename+'.summary.ps')],shell=True,cwd=DataDir)

	#Rename Files
	subprocess.call(['mv {0} {1}'.format(Def,DataDir + 'def_files/'+infile+'.def')],shell=True,cwd=DataDir)
	subprocess.call(['mv {0} {1}'.format(SummaryGray,DataDir + 'graydata_files/'+filename+'.summary.graydata')],shell=True,cwd=DataDir)
	subprocess.call(['mv {0} {1}'.format(HighResGray,DataDir + 'graydata_files/'+filename+'.highres.graydata')],shell=True,cwd=DataDir)
	subprocess.call(['mv {0} {1}'.format(DataDir + 'DecimatedData',DataDir +'data_files/'+ filename + '.2.5MHz.data')],cwd=DataDir,shell=True)

	#Remove the large file
	subprocess.call(['rm {0}'.format(infile)],cwd=DataDir,shell=True)
	#copy the decimated file
	try:
		subprocess.check_call(['scp {0} {1}'.format(DataDir +'data_files/'+ filename + '.2.5MHz.data',RemoteDir)],cwd=DataDir,shell=True)
	except:
		break
	
	#Remove the decimated file if it has been copied successfully
	subprocess.call(['rm {0}'.format(DataDir +'data_files/'+ filename + '.2.5MHz.data')],cwd=DataDir,shell=True)
#sys.exit(0)
#	subprocess.call(['mv {0} {1}'.format(FullData,DataDir + 'data_files'+infile+'.1.2MHz.data')],shell=True,cwd=DataDir)

