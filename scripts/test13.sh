#!/bin/sh
# Count number of files in the folder befor and after syscall
# We can say that partial files are handled in case the 
# difference in number of files before and after the call is 
# less than 2.
# CASE1 : diff = 0, when outfile already exists
# CASE2 : diff = 1, when outfile is created
# set -x
echo dummy test > in.test.$$
/bin/rm -f out.test.$$
separator="---------------------------------------------------------------"
echo $separator
echo $'Count number of files in the folder befor and after syscall'
echo $separator

before=`find . -type f | wc -l`
../xcpenc -e in.test.$$ out.test.$$ -p passwordi123
retval=$?
after=`find . -type f | wc -l`
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        exit $retval
else
        echo xcpenc encryption program succeeded
fi

diff=`expr $after - $before`
# now verify that the difference in no of files
if (( $diff < 2 )) ; then
        echo "xcpenc: no extra files created as a result of syscall"
        echo $separator
else
        echo "xcpenc: partial files not handled"
        exit 1
fi
#remove the generated files
/bin/rm -rf in.test.$$ out.test.$$ 
