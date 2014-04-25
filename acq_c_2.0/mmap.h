/*
 * Example of using commands - asynchronous input
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000,2001 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

#define ACQERR_BADINPUT 1       // User provided bad options
#define ACQERR_BADDEV 2         // Bad (sub?)device
#define ACQERR_BADCMD 3         // Error in generated comedi command
#define ACQERR_BADMEM 4         // Error in memory allocation
#define ACQERR_BADOUT 5

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

/* Stuff from examples.h */
extern comedi_t *device;

struct parsed_options {
	char *monfile;
	char *outfile;
	char *ranges;
	bool verbose;
	int n_chan;
	bool montext;
	int monct;
	double freq;
	double value;
    unsigned long long slicemax;
	float dt;
};

/* define the header structure for monitor file */
static struct header_info
        {
        char site_id[12];
        int num_channels;
        char channel_flags;
        unsigned int num_samples;
        unsigned int num_read;
        float sample_frequency;
        float time_between_acquisitions;
        int byte_packing;
        time_t start_time;
        struct timeval start_timeval;
        float code_version;
        } header;

extern char *cmd_src(int src,char *buf);
extern void dump_cmd(FILE *file,comedi_cmd *cmd);
/* some helper functions used primarily for counter demos */
extern int arm(comedi_t *device, unsigned subdevice, lsampl_t source);
extern int reset_counter(comedi_t *device, unsigned subdevice);
extern int set_counter_mode(comedi_t *device, unsigned subdevice, lsampl_t mode_bits);
extern int set_clock_source(comedi_t *device, unsigned subdevice, lsampl_t clock, lsampl_t period_ns);
extern int set_gate_source(comedi_t *device, unsigned subdevice, lsampl_t gate_index, lsampl_t gate_source);
extern int comedi_internal_trigger(comedi_t *dev, unsigned int subd, unsigned int trignum);

/* mmap.c header */
#define site_str "Dartmouth"
#define current_code_version 0.9

void do_depart(int signum);
int prepare_cmd(comedi_t *dev, struct parsed_options opt, comedi_cmd *cmd);

void mmap_init_parsed_options(struct parsed_options *options);
int mmap_parse_options(struct parsed_options *options, int argc, char *argv[]);

char *cmdtest_messages[]={
    "success",
    "invalid source",
    "source conflict",
    "invalid argument",
    "argument conflict",
    "invalid chanlist",
};
