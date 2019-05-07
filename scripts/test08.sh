#!/bin/sh
# Test encryption for a huge filename (greater than 255 bytes)
# set -x
echo dummy test > in.test.$$
/bin/rm -f out.test.$$
separator="---------------------------------------------------------------"
echo $'Test encryption for a huge filename (greater than 255 bytes)'
outfile=`perl -e 'print "a"x256'`

echo $separator
../xcpenc -e in.test.$$ $outfile -p "password123"
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc encryption program succeeded
fi

echo $separator
# remove the generated files
/bin/rm -rf in.test.$$
