#!/bin/sh
# Test basic copy functionality for the following
# file types :
#	1. Zero size file
# 	2. Size less than PAGE_SIZE 
#	3. Size multiple of PAGE_SIZE
#	4. Large Files
# set -x
LANG=C perl -e exit
separator="---------------------------------------------------------------" 
echo $'Test basic copy functionality for the following file types :
1. Zero size file
2. Size less than PAGE_SIZE
3. Size multiple of PAGE_SIZE
4. Large Files'

# --------------
# Zero size file
# --------------
perl -e 'print "operatingsystems"x0' > in.test.$$
/bin/rm -f out.test.$$
echo $separator
echo "CASE 1 : Zero size file"
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


# ---------------------------------
#  Size less than PAGE_SIZE (4096)
# ---------------------------------

perl -e 'print "operatingsystems"x25' > in.test.$$
/bin/rm -f out.test.$$
echo "CASE 2 : Size less than PAGE_SIZE"
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


# ----------------------------
#  Size multiple of PAGE_SIZE 
# ----------------------------

perl -e 'print "operatingsystems"x256' > in.test.$$
/bin/rm -f out.test.$$
echo "CASE 3 : Size multiple of PAGE_SIZE"
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


# ---------------------------------
#  Large size files i.e. Order(Mb)
# ---------------------------------

perl -e 'print "operatingsystems"x212000' > in.test.$$
/bin/rm -f out.test.$$
echo "CASE 4 : Large Files"
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



