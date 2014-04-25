#!/usr/bin/env python

import glob, sys, os, subprocess

#INPUTS
#./process_decimated_file.py {0} {1}
#{0}: Put 0 for single file, anything else for multiple files
#{1}: if {0} = 0, put the filename, otherwise put nothing

DataDir = '/mnt/data/vlf_experiment/'
print "Script started!"
singleFile= sys.argv[-2]
if (singleFile==0):
	fileList = sys.argv[-1]
else:
	fileList = glob.glob(DataDir + '*.data')

for infile in fileList:
	print infile
	#Open and write the definition file
	Def = DataDir + 'definition'
	FileDef = open(Def,'w')
	FileDef.write('y_data_num\t{0}\n'.format(8192))
	FileDef.write('y_data_min\t{0}\n'.format(0))
	FileDef.write('y_data_max\t{0}\n'.format(1250))
	FileDef.write('y_label_min\t{0}\n'.format(0))
	FileDef.write('y_label_max\t{0}\n'.format(1250))
	FileDef.write('y_label_increment\t{0}\n'.format(250))
	FileDef.write('data_black_level\t{0}\n'.format(125))
	FileDef.write('data_white_level\t{0}\n'.format(85))
	FileDef.write('title\t{0}\n'.format("South Pole Station"))
	FileDef.write('subtitle\n')
	FileDef.write('y_axis_label\n')
	FileDef.write('x_axis_label\n')
	FileDef.write('use_magic_stamps\tno\n')
	FileDef.write('magic_char\t*\n')
	FileDef.close()
	#call te c  program
	subprocess.call(['/mnt/data/vlf_experiment/vlf_fft {0}'.format(infile)],shell=True,cwd=DataDir)
	HighResGray = DataDir + 'detail.graydata'
	
	#Make the grayscale
	subprocess.call(['/usr/local/bin/gray -d {0} {1}'.format(Def, HighResGray)],shell=True,cwd=DataDir)
	subprocess.call(['mv {0} {1}'.format(DataDir + 'gray.ps',infile+'.ps')],shell=True,cwd=DataDir)
	subprocess.call(['mv {0} {1}'.format(infile + '.ps',DataDir + 'ps_files/')],shell=True,cwd=DataDir)
	#Rename Files
	#subprocess.call(['mv {0} {1}'.format(Def,DataDir + 'def_files/'+infile+'.def')],shell=True,cwd=DataDir)
	subprocess.call(['mv {0} {1}'.format(HighResGray,infile+'.graydata')],shell=True,cwd=DataDir)
	subprocess.call(['mv {0} {1}'.format(infile+'.graydata',DataDir + 'graydata_files/')],shell=True,cwd=DataDir)
	subprocess.call(['mv {0} {1}'.format(infile, DataDir + 'data_files')],shell=True,cwd=DataDir)
