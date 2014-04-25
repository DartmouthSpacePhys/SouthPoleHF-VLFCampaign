South Pole HF/VLF Collaboration Notes
=================================
Matt Broughton
25 April 2014

Description of Experiment
---------------------------
Both Dartmouth and Stanford recorded continuous waveform data starting in mid-April 2014 from 22:30-04:30 UT at South Pole Station. The signal from one of the Imager antennas at South Pole Station passed through a standard preamp and the HF-2 receiver. The signal was then digitized at 20 MHz by a Dartmouth Measurement Computing A/D. All of the data acquisition was done on cirdan (IP: 157.132.41.80).

Description of data processing
---------------------------------
 Data were recorded in the directory /data/vlf_experiment. The filenames had the following format YYYYMMDD-HHMMSS-SPS.data. Where the date and time correspond to when the data recording started. This file was ~900 gigabytes. Therefore, each day a program filtered and decimated the data to a 2.5 MHz sampling rate. Summary and detail spectrograms from 0-10 MHz were created and stored each day. The decimated file was then transferred to narya (IP 157.132.24.200). Data were placed on an external drive in the following folder: /mnt/data/vlf_experiment/. Note that /mnt/data/ is the mount point of the drive. Each day, programs on narya would take that data file, create a spectrogram from 0-1.25 MHz and copy that spectrogram to aristotle in /media/Xi_backup/sp/vlf_experiment. Narya also contains programs to extract interesting pieces from the 2.5 MHz data file and to delete files.
 
Description of Data Management
----------------------------------
Spectrograms on aristotle were visually inspected by Howard Kim. Interesting intervals were placed in the file ChopList.txt in the following format
<data filename><tab><start time (YYYY-MM-DDTHH:MM:SS)><tab><end time (YYYY-MM-DDTHH:MM:SS)>

For example, if Howard wanted to extract all data from 14 April, 23:50 to 15 April, 0:35, he would have something like this
201404414-223000-SPS.data.2.5MHz.data 2014-04-14T23:50:00  2014-04-15T00:35:00

To remove files, add the filename to RemoveList.txt.

A cronjob on aristotle runs the script copyRemoveAndChopLists.py, which copies ChopList.txt and RemoveList.txxt to narya each day for files to be chopped or deleted.

A Note on Time
-------------------------
Time was given by the local NTP server at South Pole Station. Both the 20 MHz and 2.5 MHz files have 8 byte timestamps at the start and end. The first four bytes are the integer seconds since 1 Jan 2000 00:00:00 UT. To convert this to time since the Unix epoch, add 946684800. The second four bytes are the integer microseconds.

When data are extracted from the 2.5 MHz files, the resultant file also has 8 byte timestamps at the start and end. The timestamp at the start of the extracted data is dead reckoned from its parent file. The timestamp at the end of the file is dead reckoned from the end timestamp of its parent file. See the program chop_vlf.py for details.

Program Descriptions
------------------------
### Programs on cirdan
acq_c_2.0: C code that does the data acqusition and places timestamps at the start and end of acquisition
stdc1c.sh: Shell script called by cron at 22:30 UT each day to run the C acquisition program

vlf_decimate.c: C code to filter and decimate the 20 MHz file to 2.5 MHz. Compile this code with the following command:
gcc vlf_decimate.c  -Wall -lm -pthread -lfftw3_threads -std=gnu99  -Wall -O2 -pipe -o vlf_decimate
Note that you need the threaded version of fftw installed for this program to compile.

process_vlf_experiment.py: Python script that calls vlf_decimate.c, calls gray to make 0-10 spectrograms, copies the 2.5 MHz file to narya, and removes the 20 MHz file once the 2.5 MHz file has been created.

### Programs on narya
vlf_fft.c: C code that Fourier transforms the 2.5 MHz file and creates a graydata file. Compile this code with the following command
gcc vlf_fft.c -Wall -lm -pthread -lfftw3_threads -std=gnu99  -Wall -O2 -pipe -o vlf_fft

process_decimated_file.py: Python script that calls vlf_fft, calls gray to make a detailed spcectrogram, copies the detailed spectrogram to aristotle, moves the data file to /mnt/data/vlf_experiment/data_files, moves the postscript file to /mnt/data/vlf_experimemt/graydata_files, and moves the postscript file to /mnt/data/vlf_experiment/ps_files.

chop_vlf.py: Script that extracts pieces from the large 2.5 MHz file. The input file is ChopList.txt. Once the data are extracted to a new file, it makes a spectrogram of the piece that has been chopped out.

scp_ps_files.py: Script that copies postscript files to aristotle each day.

rm_files.py: Script that removes files from /mnt/data/vlf_experiment/data_files. It takes the file RemoveList.txt as its input.
 


