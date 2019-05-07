#!/bin/sh
# Test [en|de]cryption functionality in case of password mismatch
# set -x
echo dummy test > in.test.$$
/bin/rm -f out.test.$$
separator="---------------------------------------------------------------"
echo $separator
echo $'Test [en|de]cryption functionality in case of password mismatch'
echo $separator

../xcpenc -e in.test.$$ out.test.$$ -p password123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        exit $retval
else
        echo xcpenc encryption program succeeded
fi

../xcpenc -d out.test.$$ result.test.$$ -p password12
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        exit $retval
else
        echo xcpenc encryption program succeeded
fi

# remove the generated files
/bin/rm -rf in.test.$$ out.test.$$ result.test.$$

