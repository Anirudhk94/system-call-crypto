#!/bin/sh
# Test if the system call works for symbolic links
#	1. Encryption and decryption 
#	2. Copying
# set -x
separator="-----------------------------------------------------------------------"
echo $separator
echo $'Test if basic encryption, decryption and copy work for a symbolic link'
echo $separator
echo dummy test > in.test.$$
/bin/rm -f out.test.$$
/bin/ln -s in.test.$$ link.test.$$

# ----------------------------------------
#  Encrypting and Decrypting from symlink
# ----------------------------------------
echo 'CASE 1 : Encrypting and Decrypting from symlink'
../xcpenc -e link.test.$$ out.test.$$ -p password123
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


# ----------------------
#  Copying from symlink
# ----------------------
echo 'CASE 2 : Copying from symlink'
../xcpenc -c link.test.$$ out.test.$$
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
/bin/rm -rf in.test.$$ out.test.$$ result.test.$$
/bin/rm -rf link.test.$$
