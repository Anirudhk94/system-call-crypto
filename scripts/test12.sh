#!/bin/sh
# Testing invalid flag combination
# file types :
#       1. Multiple atomic flags (eg. -e and -d) together
#       2. Missing Files
#       3. Flag with copy option
# set -x
LANG=C perl -e exit
separator="---------------------------------------------------------------"
echo 'Testing invalid flag combination file types :
       1. Multiple atomic flags (eg. -e and -d) together
       2. Missing Files
       3. Flag with copy option'
echo $separator
# ---------------
# Multiple Flags
# ---------------
echo "CASE 1 : Zero size file"
echo dummy test > in.test.$$
/bin/rm -f out.test.$$

../xcpenc -e -d in.test.$$ out.test.$$ -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        echo $separator
else
        echo xcpenc encryption program succeeded
fi

# ---------------
# Misssing Files
# ---------------
echo "CASE 2 : Missing Files"
/bin/rm -f out.test.$$

../xcpenc -e in.test.$$ -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        echo $separator
else
        echo xcpenc encryption program succeeded
fi

# -------------------
# Copy with password
# -------------------
echo "CASE 3 : Copy with password"
/bin/rm -f out.test.$$

../xcpenc -c in.test.$$ out.test.$$ -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        echo $separator
else
        echo xcpenc encryption program succeeded
fi

# remove the generated files
/bin/rm -rf in.test.$$ out.test.$$ result.test.$$
