/*
 * Example of using commands - asynchronous input
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

/*
 * An example for directly using Comedi commands.  Comedi commands
 * are used for asynchronous acquisition, with the timing controlled
 * by on-board timers or external events.
 */

#define _GNU_SOURCE

#include "mmap.h"
#include<stdint.h>
#define DEFFREQ 20000000.0
#define DEFNCHAN 1
#define DEVICE "/dev/comedi0"
#define SUBDEV 0
#define OFFSET 946684800

// Differential input
#define AREF AREF_DIFF

// Time between data retreivals in microseconds
#define TIMESLICE 15

#pragma pack(push,2)

struct time_stamp {
//	int matt;
//	float matt2;
	uint32_t t_sec;
	uint32_t t_usec;
};
#pragma pack(pop)
unsigned int chanlist[256];

void strtary (int * array, char * string);
void *map;
static bool running = true;

int main(int argc, char *argv[])
{
	comedi_t *dev;
	comedi_cmd c,*cmd=&c;
//	char syscom[200];
	unsigned long int size;
	unsigned long long int islices = 0;
	unsigned short int *mtstore;
	int ret;
	int ranges[255];
	comedi_range *rangedef;
	struct parsed_options options;
	struct iovec monv[1];
	int monfile = 0;
	int bofft, slicesamp, slicesize; // = pow(2,18);
	int dt_usec, usec_elapsed;
	struct timeval now,then;
	FILE *ofile = NULL, *monstream;
	
	umask(0);
	signal(SIGINT,do_depart);

	if (geteuid() != 0) {
			fprintf(stderr,"Must be setuid root to renice self.\n");
			exit(0);
	}

	mmap_init_parsed_options(&options);
	mmap_parse_options(&options, argc, argv);

	/* We are important */
	ret = nice(-20);
	fprintf(stderr,"I've been niced to %i.\n",ret);
	if (ret != -20) fprintf(stderr,"  WARNING!! Data collection could not set nice=-20. Data loss is probable at high speed.");

	/*
	 * Set up the device
	 */

	dev = comedi_open(DEVICE);
	if(!dev){
		comedi_perror(DEVICE);
		exit(ACQERR_BADDEV);
	}

	strtary(ranges,options.ranges);

	for (int calchan = 0; calchan < options.n_chan; calchan++) {
		ret = comedi_apply_calibration(dev, SUBDEV, calchan, ranges[calchan], AREF, "/usr/comedi/config/comedi0.acal");
		rangedef = comedi_get_range(dev, SUBDEV, calchan, ranges[calchan]);
		if (ret < 0) printf("Calibration failure!  Chan: %i, Range: %i (%f to %f Volts)\n",calchan,ranges[calchan],rangedef->min,rangedef->max);
		else printf("Calibration success!  Chan: %i, Range: %i (%f to %f Volts)\n",calchan,ranges[calchan],rangedef->min,rangedef->max);
	}

	size = comedi_get_buffer_size(dev, SUBDEV);
	if (options.verbose) printf("Buffer size is %ld\n", size);

	map = mmap(NULL,size,PROT_READ,MAP_SHARED, comedi_fileno(dev), 0);
	printf("Map: %p\n", map);
	if( map == MAP_FAILED ){
		perror( "mmap" );
		exit(ACQERR_BADMEM);
	}

	//prepare_cmd_lib(dev, SUBDEV, options.n_scan, options.n_chan, 1e9 / options.freq, cmd);
	prepare_cmd(dev, options, cmd);

	printf("Testing command...");
	ret = comedi_command_test(dev, cmd);
	if (ret < 0) {
		comedi_perror("comedi_command_test");
		if(errno == EIO){
			fprintf(stderr,"Ummm... this subdevice doesn't support commands\n");
		}
		exit(ACQERR_BADCMD);
	}
	printf("%s...",cmdtest_messages[ret]);
	ret = comedi_command_test(dev, cmd);
	printf("%s...",cmdtest_messages[ret]);
	ret = comedi_command_test(dev, cmd);
	printf("%s...\n",cmdtest_messages[ret]);
	if (ret < 0) {
		fprintf(stderr,"Bad command, and unable to fix.\n");
		dump_cmd(stderr, cmd);
		exit(ACQERR_BADCMD);
	}
	dump_cmd(stdout,cmd);

	if (strcmp(options.outfile,"") != 0) {
		int ofd = open(options.outfile, O_CREAT|O_TRUNC|O_WRONLY|O_LARGEFILE, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		ofile = fdopen(ofd,"w");
	}
	
	ret = comedi_command(dev, cmd);
	if(ret < 0){
		comedi_perror("comedi_command");
		exit(ACQERR_BADCMD);
	}

	header.num_read = options.monct*options.n_chan;
	sprintf(header.site_id,"%s",site_str);
	header.num_channels=options.n_chan;
	header.channel_flags=0x0F;
	header.num_samples=options.monct;
	header.sample_frequency=options.freq;
	header.time_between_acquisitions=options.dt;
	header.byte_packing=0;
	header.code_version=current_code_version;
	monv[0].iov_len = options.monct*options.n_chan*2;
	gettimeofday(&then, NULL);
	dt_usec = options.dt * 1e6;

	if (options.dt != 0) {
		monfile = open(options.monfile,O_CREAT|O_TRUNC|O_WRONLY|O_LARGEFILE,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		monstream = fdopen(monfile, "w");

		if (options.montext) {
			fprintf(monstream, "## ##\nHF_High AGC_High HF_Low AGC_Low\n");
		}
	}

	slicesize = size/16;
	slicesamp = slicesize/2;

	printf("Slice size: %6.2f MB.\n",slicesize/1.0e6);

	int datarate = 2 * options.n_chan * options.freq;
	int datadur = slicesize/datarate;
	int datadur_usec = datadur * 10^3;

	int nsleeps = 0;
	int bcont;
	int usrvs = 0;
	printf("\nStarting acquisition...\n");

	 //Write the timestamp
	struct time_stamp current_time;
	struct timeval tv;
	
	memset(&current_time, 0, sizeof(struct time_stamp));	
	gettimeofday(&tv,NULL);
	current_time.t_sec = tv.tv_sec - OFFSET;
	current_time.t_usec = tv.tv_usec;
	fwrite(&current_time,1,sizeof(struct time_stamp),ofile);
	do {
		comedi_poll(dev, SUBDEV);
		bcont = comedi_get_buffer_contents(dev, SUBDEV);
		if (bcont >= slicesize) {
		printf("nsleeps: %d, bcont: %.3f MB, usrvs: %d\n", nsleeps, bcont/1e6, usrvs);fflush(stdout);
		nsleeps = 0;

			bofft = comedi_get_buffer_offset(dev, SUBDEV);

			if (strcmp(options.outfile,"") != 0) {

				ret = fwrite(map+bofft, 2, slicesamp, ofile);
			
				if (ret < slicesamp) {
					fprintf(stderr,"blerg!	Disk full?	Wrote %d samples.\n",ret);
					fflush(stderr);
					break;
				}
				islices++;
//				  printf("is: %llui, sm: %llui.\n",islices,options.slicemax);
				if (islices == options.slicemax) {
					printf("Reached %llu slices.\n",options.slicemax);
					running = false;
				}
			}

			// Check time and write monitor file if desired
			if (options.dt != 0) {
				gettimeofday(&now, NULL);

				usec_elapsed = (now.tv_sec - then.tv_sec) * 1e6 + (now.tv_usec - then.tv_usec);
				if (usec_elapsed >= dt_usec) {
					if (options.verbose) printf("Writing monitor file\n.");

					lseek(monfile, 0, SEEK_SET);
					monv[0].iov_base = map+bofft;
					header.start_time = time(NULL);
					header.start_timeval = now;
					write(monfile, &header, sizeof(header));
					writev(monfile, monv, 1);

					fflush(monstream);
					then = now;
				}
			}

			ret = comedi_mark_buffer_read(dev, SUBDEV, slicesize);
			if(ret < 0){
				comedi_perror("comedi_mark_buffer_read");
				break;
			}

		}
		else {
			nsleeps++;
			usrvs += usleep(1e4);
		}

	} while (running);

	if (dev != NULL){
		memset(&current_time, 0, sizeof(struct time_stamp));	
		gettimeofday(&tv,NULL);
		current_time.t_sec = tv.tv_sec - OFFSET;
		current_time.t_usec = tv.tv_usec;
		fwrite(&current_time,1,sizeof(struct time_stamp),ofile);
		comedi_close(dev);

	}
	if (strcmp(options.outfile,"") != 0) {
		if (ofile != NULL)
			fclose(ofile);
	}
	if (options.dt != 0) {
		if (monfile != 0)
			close(monfile);
			fclose(monstream);

	}

	printf("bye now.\n");

	return 0;
}

int prepare_cmd(comedi_t *dev, struct parsed_options opt, comedi_cmd *cmd)
{
	int i;
	int ranges[255];

	memset(cmd,0,sizeof(*cmd));

	cmd->subdev = SUBDEV;

	cmd->flags = 0;

	cmd->start_src = TRIG_NOW;
	cmd->start_arg = 0;

	cmd->scan_begin_src = TRIG_TIMER;
	cmd->scan_begin_arg = 1e9/opt.freq;

	cmd->convert_src = TRIG_NOW;
	cmd->convert_arg = 0;

	cmd->scan_end_src = TRIG_COUNT;
	cmd->scan_end_arg = opt.n_chan;

	cmd->stop_src = TRIG_NONE;
	cmd->stop_arg = 0;

	/* Convert range string to numbers, and commit to chanlist */
	strtary(ranges,opt.ranges);
	if (opt.verbose) printf("Current range tokens: %i %i %i %i.\n",ranges[0],ranges[1],ranges[2],ranges[3]);
	for (i = 0; i < opt.n_chan; i++) chanlist[i]=CR_PACK(i,ranges[i],AREF);

	cmd->chanlist = chanlist;
	cmd->chanlist_len = opt.n_chan;

	return 0;
}

void strtary (int * array, char * string)
{
  int i=0;

  while (*string)
	array[i++] = *(string++)-'0';
}

void do_depart(int signum) {
	running = false;
	fprintf(stderr,"\nStopping...");
}

/* Command-line option parsing */

int mmap_parse_options(struct parsed_options *options, int argc, char *argv[])
{
	int c;

	while (-1 != (c = getopt(argc, argv, "m:o:r:n:N:F:d:X:tvh"))) {
		switch (c) {
		case 'm':
			options->monfile = optarg;
			break;
		case 'o':
			options->outfile = optarg;
			break;
		case 'r':
			options->ranges = optarg;
			break;
		case 'n':
			options->n_chan = strtoul(optarg, NULL, 0);
			break;
		case 'N':
			options->monct = strtoul(optarg, NULL, 0);
			break;
		case 'F':
			options->freq = strtod(optarg, NULL);
			break;
		case 'd':
			options->dt = strtod(optarg, NULL);
			break;
		case 't':
			options->montext = true;
			break;
		case 'v':
			options->verbose = true;
			break;
		case 'X':
			options->slicemax = strtoull(optarg, NULL, 0);
			break;
		case 'h':
		default:
			printf("cmd Options:\n");
			printf("\t-m <file>\tMonitor File\n");
			printf("\t-o <file>\tOutput File\n");
			printf("\t-d <#>\t\tMonitor dt\n");
			printf("\t-t <#>\t\tASCII monitor file [false]\n");
			printf("\t-N <#>\t\tNumber of Monitor Samples [4096]\n");
			printf("\t-r <#>\t\tRange [1111 (all +-1V)]\n");
			printf("\t-n <#>\t\t# Channels [1]\n");
			printf("\t-X <#>\t\tNumber of Slices to acquire [0->inf]\n");
			printf("\t-F <#>\t\tFrequency [20MHz]\n");
			printf("\t-v\t\tBe Verbose\n");
			printf("\n");
			exit(1);
		}
	}
	if(optind < argc) {
		/* data value */
		options->value = strtod(argv[optind++], NULL);
	}

	return argc;
}

void mmap_init_parsed_options(struct parsed_options *options)
{
	memset(options, 0, sizeof(struct parsed_options));
	options->monfile = "./latest_acquisition.data";
	options->outfile = "";
	options->ranges = "1111";
	options->n_chan = DEFNCHAN;
	options->freq = DEFFREQ;
	options->verbose = false;
	options->value = 0;
	options->montext = false;
	options->monct = 4096;
	options->slicemax = 0;
	options->dt = 0;
}
