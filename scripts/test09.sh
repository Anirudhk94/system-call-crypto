#!/bin/sh
# Test cases for same infile and outfile
#       1. Encryption
#       2. Decryption
#	3. Copy
# set -x
echo dummy test > in.test.same
/bin/rm -f out.test.same
separator="---------------------------------------------------------------"
echo $'Test for same infile and outfile
        1. Encryption
        2. Decryption
	3. Copy'

# ------------
#  Encryption
# ------------

echo $separator
echo 'CASE 1 : Encryption'
../xcpenc -e in.test.same in.test.same -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc encryption program succeeded
fi

# ------------
#  Decryption
# ------------

echo $separator
echo 'CASE 2 : Decryption'
../xcpenc -e in.test.same out.test.same -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc encryption program succeeded
fi
../xcpenc -d out.test.same out.test.same -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc decryption program succeeded
fi

# ---------
#  Copying
# ---------

echo $separator
echo 'CASE 3 : Copying'
../xcpenc -e in.test.same in.test.same -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc encryption program succeeded
fi
echo $separator
# remove the generated files
/bin/rm -rf in.test.same out.test.same
