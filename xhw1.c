#include <asm/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_cpenc
#error cpenc system call not defined
#endif

int main(int argc, const char *argv[])
{
	int rc;
	void *dummy = (void *) argv[1];

  	rc = syscall(__NR_cpenc, dummy);
	if (rc == 0)
		printf("syscall returned %d\n", rc);
	else
		printf("syscall returned %d (errno=%d)\n", rc, errno);

	exit(rc);
}
