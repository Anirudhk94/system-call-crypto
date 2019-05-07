#!/bin/sh
# Count number of files in the folder befor and after syscall
# in case of password mismatch. Usually, the partial temp file 
# are not unlinked in case of failure to encrypt.
# In such cases the difference always needs to be exactly 0.
# set -x
echo dummy test > in.test.$$
/bin/rm -f out.test.$$
separator="---------------------------------------------------------------"
echo $separator
echo $'Count number of files in the folder befor 
	and after syscall in case of password mismatch'
echo $separator

../xcpenc -e in.test.$$ out.test.$$ -p passwordi123
retval=$?
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
        exit $retval
else
        echo xcpenc encryption program succeeded
fi

before=`find . -type f | wc -l`
../xcpenc -d out.test.$$ result.test.$$ -p password122
retval=$?
after=`find . -type f | wc -l`
if test $retval != 0 ; then
        echo xcpenc failed with error: $retval
else
        echo xcpenc decryption program succeeded
fi

diff=`expr $after - $before`
# now verify that the difference in no of files
if (( $diff == 0 )) ; then
        echo "xcpenc: temp file discarded"
        echo $separator
else
        echo "xcpenc: partial files not handled"
        exit 1
fi
#remove the generated files
/bin/rm -rf in.test.$$ out.test.$$
