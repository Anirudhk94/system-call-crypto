#!/bin/sh
# Testif encryption of two files using same password produce diffenent 
# encryption, as a result of different IVs.
# set -x
echo dummy test > in.test.$$
/bin/rm -f out.test.$$
separator="---------------------------------------------------------------"
echo $separator
echo $'Test basic [en|de]cryption functionality for a dummy string'
echo $'Testif encryption of two files using same password produce diffenent
 encryption, as a result of different IVs.'
echo $separator
../xcpenc -e in.test.$$ out.test.1 -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        exit $retval
else
        echo xcpenc encryption program succeeded
fi

../xcpenc -e in.test.$$ out.test.2 -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        exit $retval
else
        echo xcpenc encryption program succeeded
fi

# now verify that the two files (infile and result) are different
if cmp out.test.1 out.test.2 ; then
	echo "xcpenc: encrypted file contents match (Check if #EXTRA_CREDIT is defined)"
else
	echo "xcpenc: encrypted files are different"
	echo $separator
fi
# remove the generated files
/bin/rm -rf in.test.$$ out.test.1 out.test.2
