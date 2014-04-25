#!/bin/sh

OUTDIR=/data/vlf_experiment
OUTPRE=""
OUTPOST="-SPS"

TSTAMP=`date +%Y%m%d-%H%M%S`

OUTNAME=$OUTPRE$TSTAMP$OUTPOST.data
OUTFULL=$OUTDIR/$OUTNAME
LOCKFULL=$OUTDIR/.ggsedatalock~$OUTNAME

# Lock data file
echo $$ > $LOCKFULL

echo "Writing data to $OUTFULL."
#/usr/local/bin/acq_c -d 2 -X 3295899 -n 1 -F 20000000 -r 1 -o $OUTFULL -m /tmp/rtd/latest_acquisition.data
#/usr/local/bin/acq_c -d 2 -X 1647950 -n 1 -F 20000000 -r 1 -o $OUTFULL -m /tmp/rtd/latest_acquisition.data
#/usr/local/bin/acq_c -d 2 -X 549317 -n 1 -F 20000000 -r 1 -o $OUTFULL -m /tmp/rtd/latest_acquisition.data
#/usr/local/bin/acq_c -d 2 -X 2746584 -n 1 -F 20000000 -r 1 -o $OUTFULL -m /tmp/rtd/latest_acquisition.data
#/usr/local/bin/acq_c -d 2 -X 3295899 -n 1 -F 20000000 -r 1 -o $OUTFULL -m /tmp/rtd/latest_acquisition.data
#/usr/local/bin/acq_c -d 2 -X 823975 -n 1 -F 20000000 -r 1 -o $OUTFULL -m /tmp/rtd/latest_acquisition.data
# write 'header'
#/usr/ggse/bin/addhead.py $OUTFULL

/usr/src/acq_c_2.0/acq_c -X 216000 n 1 -F 20000000 -o $OUTFULL
# Remove lock
rm $LOCKFULL


