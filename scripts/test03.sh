#!/bin/sh
# Test basic [en|de]cryption functionality for a dummy string
# set -x
echo dummy test > in.test.$$
/bin/rm -f out.test.$$
separator="---------------------------------------------------------------"
echo $separator
echo $'Test basic [en|de]cryption functionality for a dummy string'
echo $separator
../xcpenc -e in.test.$$ out.test.$$ -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        exit $retval
else
        echo xcpenc encryption program succeeded
fi
../xcpenc -d out.test.$$ result.test.$$ -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        exit $retval
else
        echo xcpenc decryption program succeeded
fi
# now verify that the two files (infile and result) are the same
if cmp in.test.$$ result.test.$$ ; then
        echo "xcpenc: input and decrypted file contents are the same"
	echo $separator
else
        echo "xcpenc: input and decrypted files contents DIFFER"
        exit 1
fi
# remove the generated files
/bin/rm -rf in.test.$$ out.test.$$ result.test.$$
