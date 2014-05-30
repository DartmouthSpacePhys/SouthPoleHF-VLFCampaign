#!/usr/bin/env python
import sys, os, datetime, subprocess, calendar
from struct import unpack, pack
from math import floor, ceil
DataDir = '/mnt/data/vlf_experiment/data_files/'

processScript = '/mnt/data/vlf_experiment/process_decimated_file.py'
#DataDir = '/Users/mattbroughton/vlf_experiment/data_files/'

remoteDir = 'aurora@157.132.41.82:/daq/vlf_experiment/data_files/'

def ConvertTime(string):

	#input: string	#String meeds to be in the following format YYYY-MM-DDTHH:MM:SS.microseconds
	#returns: Datetime object
	#import datetime, from math import floor
	DatetimeSplit = string.split('T')
	DateSplit = DatetimeSplit[0].split('-')
	TimeSplit = DatetimeSplit[1].split(':')
	seconds = int(floor(float(TimeSplit[2])))
	microseconds = int(1.0e6 * (float(TimeSplit[2]) - floor(float(TimeSplit[2]))))
	Datetime = datetime.datetime(year = int(DateSplit[0]), month = int(DateSplit[1]), day = int(DateSplit[2]), hour = int(TimeSplit[0]), minute = int(TimeSplit[1]), second = seconds, microsecond = microseconds)
	return Datetime

ChopList = open('/mnt/data/vlf_experiment/ChopList.txt','r')
AllFiles = []
AllStart = []
AllEnd = []
#Read in all the chopping information
for line in ChopList:
	split = line.split()
	AllFiles.append(DataDir + split[0])
	AllStart.append(split[1])
	AllEnd.append(split[2])
print AllFiles
#For each file
for i,infile in enumerate(AllFiles):
	print infile
	TIME_OFFSET = 946684800
	SampleFreq=  2.5e6 


	#Get start and end times
	try:
		StartDatetime = ConvertTime(AllStart[i])
	except:
		continue
	EndDatetime = ConvertTime(AllEnd[i])

	#Get the start and end time into timestamp form
	StartTimeEpoch = calendar.timegm(StartDatetime.utctimetuple())
	StartTimeEpoch -= TIME_OFFSET
	EndTimeEpoch = calendar.timegm(EndDatetime.utctimetuple())
	EndTimeEpoch -= TIME_OFFSET
	StartTimeString = pack('2I', StartTimeEpoch, StartDatetime.microsecond)
	EndTimeString= pack('2I',EndTimeEpoch, EndDatetime.microsecond)
	#print StartTimeString
	#print EndTimeString

	#Open the file
	try:
		FileIn = open(infile,'r')
	except:
		continue
	FileOut = open(infile + '.%02i%02i%02i.data'%(StartDatetime.hour,StartDatetime.minute,StartDatetime.second), 'w')
	
	
	#Get the start time 
	FileTime = unpack('2I',FileIn.read(8))

	PartialDatetime = datetime.datetime.utcfromtimestamp(FileTime[0] + TIME_OFFSET)
	FileStartDatetime = datetime.datetime(year = PartialDatetime.year, month = PartialDatetime.month, day = PartialDatetime.day, hour = PartialDatetime.hour, minute = PartialDatetime.minute, second = PartialDatetime.second, microsecond = FileTime[1])

	#Get the end time
	FileIn.seek(-8, 2)
	print "byte of last record is at {0}".format(FileIn.tell())

	FileTime = unpack('2I',FileIn.read(8))
	print "FileTime = {0}".format(FileTime)
	PartialDatetime = datetime.datetime.utcfromtimestamp(FileTime[0] + TIME_OFFSET)
	FileEndDatetime = datetime.datetime(year = PartialDatetime.year, month = PartialDatetime.month, day = PartialDatetime.day, hour = PartialDatetime.hour, minute = PartialDatetime.minute, second = PartialDatetime.second, microsecond = FileTime[1])
	
	print "The first file time is {0}".format(FileStartDatetime)
	print "The last file time is {0}".format(FileEndDatetime)
	print "chop from {0} to\n{1}".format(StartDatetime,EndDatetime)
	#Dead reckon the last record to read based on the differnece between the 
	#last time desired and the end of the file. The -8 accounts for the timestamp
	#at the end of the file
	deltaEnd  = FileEndDatetime - EndDatetime
	print deltaEnd.seconds
	print deltaEnd.microseconds
	FileIn.seek(-8 + int(-1 * SampleFreq * (deltaEnd.seconds + deltaEnd.microseconds/1.0e6) * 2), 2)
	endPos = FileIn.tell()

	#Dead reckon the first record to read based on the time difference between
	#the first time desired and the start of the file
	StartDelta = StartDatetime - FileStartDatetime
	print StartDelta
	OffsetBytes = SampleFreq * (StartDelta.seconds + StartDelta.microseconds/1.0e6) * 2
	DataDelta = (StartDatetime - EndDatetime)

	print "OffsetBytes = {0}".format(OffsetBytes)
	#Skip to the start time. The +8 accounts for the 8 byte timestamp at the start of the file
	FileIn.seek(int(OffsetBytes + 8),0)

	print "endPos = {0}".format(endPos)
	#Read and write the data
#	for count in range(DataDelta.seconds):
#		data = FileIn.read(SampleFreq * 2)
	#FileOut.write(data)
	print "currently at {0}".format(FileIn.tell())
	#Read and write data until the end time
	while (FileIn.tell()<= endPos):
		data = FileIn.read(int(1e-4 * 2 * SampleFreq))
		FileOut.write(data)

	#Write the end timestamp
	FileOut.write(EndTimeString)

	#process the chopped file
	subprocess.call(['{0} 0 {1}'.format(processScript,infile + '.%02i%02i%02i.data'%(StartDatetime.hour,StartDatetime.minute,StartDatetime.second))],shell=True,cwd=DataDir)

	#Copy the chopped file to a remote direcotyr
	subprocess.call(['scp {0} {1}'.format(infile + '.%02i%02i%02i.data'%(StartDatetime.hour,StartDatetime.minute,StartDatetime.second),remoteDir)],shell=True,cwd=DataDir)

	#Move the chopped file to the data_files directory
#	subprocess.call(['mv {0} {1}'.format(infile + '.%02i%02i%02i.data'%(StartDatetime.hour,StartDatetime.minute,StartDatetime.second),DataDir + 'data_files')],shell=True,cwd=DataDir)
