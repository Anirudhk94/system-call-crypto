#!/bin/sh
# Test cases in encryption when directory is provided for
#       1. Infile
#       2. Outfile
# set -x

/bin/mkdir infile 
/bin/mkdir outfile
/bin/rm -f out.test.$$
echo dummy test > in.test.$$

separator="---------------------------------------------------------------"
echo $'Test cases in encryption when directory is provided for
        1. Infile
        2. Outfile'

# --------
#  Infile
# --------

echo $separator
echo 'CASE 1 : Infile'
../xcpenc -e infile out.test.$$ -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc encryption program succeeded
fi

# ---------
#  Outfile
# ---------

echo $separator
echo 'CASE 2 : Outfile'
../xcpenc -e in.test.$$ outfile -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc encryption program succeeded
fi
echo $separator
# remove the generated files
/bin/rm -rf in.test.$$ out.test.$$
/bin/rmdir infile outfile
