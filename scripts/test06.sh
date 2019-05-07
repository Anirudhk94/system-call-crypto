#!/bin/sh
# Test for valid password lengths
# 	1. Password Length < 6	
#	2. Password Length > 6
# set -x
echo dummy test > in.test.$$
/bin/rm -f out.test.$$
separator="---------------------------------------------------------------"
echo $'Test for valid password lengths
	1. Password Length < 6
	2. Password Length > 6'

# --------------------
# Password Length < 6
# --------------------

echo $separator
echo 'CASE 1 : Password Length < 6'
../xcpenc -e in.test.$$ out.test.$$ -p pass
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc encryption program succeeded
fi

# --------------------
# Password Length > 6
# --------------------

echo $separator
echo 'CASE 2 : Password Length > 6'
../xcpenc -e in.test.$$ out.test.$$ -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc encryption program succeeded
fi
echo $separator
# remove the generated files
/bin/rm -rf in.test.$$ out.test.$$ result.test.$$
