#!/bin/sh
# Test basic copy functionality for a dummy string
# 
# set -x
separator="---------------------------------------------------------------"
echo $separator
echo $'Test basic copy functionality for a dummy string'
echo $separator
echo dummy test > in.test.$$
/bin/rm -f out.test.$$
../xcpenc -c in.test.$$ out.test.$$
retval=$?
if test $retval != 0 ; then
	echo xcpenc failed with error: $retval
	exit $retval
else
	echo xcpenc program succeeded
fi
# now verify that the two files are the same
if cmp in.test.$$ out.test.$$ ; then
	echo "xcpenc: input and output files contents are the same"
	echo $separator
else
	echo "xcpenc: input and output files contents DIFFER"
	exit 1
fi
# remove the generated files
/bin/rm -rf in.test.$$ out.test.$$
