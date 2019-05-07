#!/bin/sh
# Test for password with new line character
# set -x
echo dummy test > in.test.$$
/bin/rm -f out.test.$$
separator="---------------------------------------------------------------"
echo $'Test for password with new line character'

echo $separator
../xcpenc -e in.test.$$ out.test.$$ -p "pass
word123"
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc encryption program succeeded
fi

../xcpenc -d out.test.$$ result.test.$$ -p "pass
word123"
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
else
        echo "xcpenc: input and decrypted files contents DIFFER"
        exit 1
fi
echo $separator
# remove the generated files
/bin/rm -rf in.test.$$ out.test.$$ result.test.$$
