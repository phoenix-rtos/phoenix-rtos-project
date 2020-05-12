#!/bin/ash

UPDIR="$1"
UPDEV="$2"
ROOTDEV="$3"

SIGN=$UPDIR/sign.bin
PART=$UPDIR/update.img.gz
PLO=$UPDIR/arm-plo.img
UPDEVS="/dev/mmcblk0p1 /dev/mmcblk0p2"


EXIT_CODE_OTHER=1
EXIT_CODE_BAD_IMG=12
EXIT_CODE_UNSUPPORTED_IMG=13

TEMP=/var/tmp/tmp.bin

WRITE_RETRY=3

log()
{
	echo "phoenix_update : $@" 1>&2
	logger -t phoenix_update $@
}

be2le64()
{
		echo "${1:14:2}${1:12:2}${1:10:2}${1:8:2}${1:6:2}${1:4:2}${1:2:2}${1:0:2}"
}

log "performing update with PLO"

# TODO: check if fw image is correct for this device type
# on error, return EXIT_CODE_UNSUPPORTED_IMG
log "Reading current system sequence number"

dd if=$ROOTDEV bs=512 count=1 skip=3 of=$UPDIR/cursign.bin

CURSEQNUM=`dd if=$UPDIR/cursign.bin bs=8 count=1 skip=1 | hexdump -v -e '/1 "%02X"'`
rm $UPDIR/cursign.bin
CURSEQNUM=`be2le64 "$CURSEQNUM"`
CURSEQNUM=`echo $CURSEQNUM | sed "s/^0*\([1-9a-fA-F]\)/\1/;s/^0*$/0/"`
CURSEQNUM=`printf "%d" 0x$CURSEQNUM`


log "Clearing old signature"
dd of=$UPDEV if=/dev/zero bs=512 seek=3 count=3 conv=notrunc
RES=$?
if [ ! $RES -eq 0 ]; then
	log "Failed to dd into $UPDEV"
	exit $EXIT_CODE_OTHER
fi


log "Parsing kernel signature"
SNAME=`dd if=$SIGN bs=1 count=8 skip=0`
SOFFSET=`dd if=$SIGN bs=1 count=8 skip=24 | hexdump -v -e '/1 "%02X"'`
SOFFSET=`be2le64 "$SOFFSET"`
SOFFSET=`printf "%d" "0x$SOFFSET"`
SSEQNUM=$(( $CURSEQNUM + 1 ))
SSIGNTYPE=`dd if=$SIGN bs=1 count=8 skip=32`

SNAME=`echo $SNAME`
SVERSION=`echo $SVERSION`

SNAME=`echo $SNAME`
SVERSION=`echo $SVERSION`

SSIGNTYPE=`echo $SSIGNTYPE`

log "New system: $SNAME seqnum $SSEQNUM"
log "Signature type: $SSIGNTYPE"
log "Kernel offset: $SOFFSET"

log "Patching new kernel signature with seqence number"
SSEQNUM=`printf "%016X" "$SSEQNUM"`

echo `be2le64 "$SSEQNUM"` | xxd -r -p | dd of=$SIGN bs=8 count=1 conv=notrunc seek=1

SUM_A=`cat $UPDIR/partinfo`
PSIZE=`echo $SUM_A | cut -d' ' -f2`
SUM_A=`echo $SUM_A | cut -d' ' -f1`

if [ ! $(( (PSIZE/8192)*8192 )) -eq $PSIZE ]; then
	log "Improper partition image size - not a multiplicity of 512"
	exit $EXIT_CODE_OTHER
else
	log "Partition image size correct"
fi

PSIZE=$(( PSIZE / 8192 ))
for i in 1 2 3;
do
	#WARNING - total size of image in bytes has to be multiplicity of 8192 in order to make update working
	log "Trial $i: Writing partition image onto $UPDEV"
	gunzip -c $PART | dd of=$UPDEV ibs=8191 obs=8192 conv=notrunc seek=0
	RES=$?
	if [ ! $RES -eq 0 ]; then
		log "Failed to save new image onto $UPDEV"
		rm -rf $UPDIR/*
		exit $EXIT_CODE_OTHER
	fi
	log "Write done"

	rm $PART

	log "Verifying if image was written properly"


	SUM_B=`dd if=$UPDEV bs=8192 count=$PSIZE | md5sum`
	SUM_B=`echo $SUM_B | cut -d' ' -f1`

	if [ "$SUM_A" == "$SUM_B" ]; then
		log "Partition image write succeeded"
		break
	fi

done

if [ ! "$SUM_A" == "$SUM_B" ]; then
		log "Partition image write failed"
		rm -rf $UPDIR/*
		exit $EXIT_CODE_OTHER
fi

SUM_A=`md5sum < $SIGN`
for i in 1 2 3;
do
log "Trial $i: Writing kernel signature"
dd if=$SIGN of=$UPDEV bs=512 seek=3 conv=notrunc count=1
log "Verifying written signature"
dd of=$TEMP if=$UPDEV bs=512 skip=3 count=1
SUM_B=`md5sum < $TEMP`
rm $TEMP

if [ "$SUM_A" == "$SUM_B" ]; then
	log "kernel signature update succeeded"
	break
fi
done

if [ ! "$SUM_A" == "$SUM_B" ]; then
og "Kernel signature update failed"
	rm -rf $UPDIR/*
	exit $EXIT_CODE_OTHER
fi

log "Partition image update successfull, checking PLO."

if [ ! -f $PLO ]; then
	log "ARM plo image does not exist. Reverting update."
	rm -rf $UPDIR/*
	dd if=/dev/zero of=$UPDEV bs=512 count=4 conv=notrunc
	exit $EXIT_CODE_OTHER
fi
SUM_A=`md5sum < $PLO | cut -d' ' -f1`

for i in 1 2 3;
do
log "Trial $i: Writing new plo onto flash"

dd if=$PLO of=/dev/flash0 bs=8192 conv=notrunc
PLO_SIZE=`wc -c < $PLO`
PLO_BLOCKS=$(( PLO_SIZE/8192 ))
PLO_REM=$(( PLO_SIZE - (PLO_BLOCKS * 8192) ))

log "Plo update verification"

dd if=/dev/flash0 of=$UPDIR/plotmp.img bs=8192 count=$PLO_BLOCKS

if [ ! "$PLO_REM" -eq "0" ]; then
	dd if=/dev/flash0 of=$UPDIR/plotmp.img bs=1 skip=$(( PLO_BLOCKS * 8192 )) seek=$(( PLO_BLOCKS * 8192 )) count=$PLO_REM  conv=notrunc
fi

if [ ! "`wc -c < $UPDIR/plotmp.img`" -eq "$PLO_SIZE" ]; then
	log 'Error in reading plo - bad read size'
	dd if=/dev/zero of=$UPDEV bs=512 count=4 conv=notrunc
	exit $EXIT_CODE_OTHER
fi

SUM_B=`md5sum < $UPDIR/plotmp.img | cut -d' ' -f1`

if [ "$SUM_A" == "$SUM_B" ]; then
	log "PLO update succeeded"
	break
fi

done

if [ ! "$SUM_A" == "$SUM_B" ]; then
	log "PLO update failed"
	exit $EXIT_CODE_OTHER
fi

rm -rf $UPDIR/*
log "Update done."
exit 0
