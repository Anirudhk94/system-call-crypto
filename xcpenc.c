#include <asm/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <openssl/sha.h>
#include "cpenc.h"
#define MIN_PASS_LEN 6
#define MAX_FILE_NAME 254

#ifndef __NR_cpenc
#error cpenc system call not defined
#endif

void print_struct(struct cpenc *cp);

static char* remove_newline(char* key) {
	int i = 0, len = strlen(key);
	char *newstring;
	int curr = 0;
	newstring = (char *)malloc(len + 1);
	for(i = 0 ; i < len ; i++)
    	{
		if(key[i] == '\n') continue;
        	newstring[curr++] = key[i]; 
    	}
	newstring[curr] = '\0';
	return newstring;
}

int main(int argc,char *const argv[])
{
	int rc;
	char *optstring = "hp:edc";
	char *key;
	unsigned long pass_len = 0;
	char *INVOPT = "INVOPT:Only one of -e, -d or -c can be specified. \n"; 
	extern int optind;
	int opt, err = 0;	
	unsigned char hash[SHA_DIGEST_LENGTH];
	struct cpenc *cp = malloc(sizeof(struct cpenc));
	if(cp == NULL){
		printf("Memory allocation to cpenc failed\n");
		exit(-1);
	}
	cp->flags = -1;
	cp->keylen = 0;
	cp->keybuf = NULL;
	while((opt = getopt(argc, argv, optstring)) != -1) {
		switch(opt) {
			case 'e':
				if(cp->flags == -1) 
					cp->flags = 0x01;
				else {
					printf(INVOPT);
					err = -1;
					goto out;
				}
				break;

			case 'd':
				if(cp->flags == -1) 
					cp->flags = 0x02;
				else {
					printf(INVOPT);
					err = -1;
					goto out;
				}
				break;

			case 'c':
				if(cp->flags == -1) 
					cp->flags = 0x04;    
				else {
					printf(INVOPT);
					err = -1;
					goto out;
				} 
				break;

			case 'p':
				key = remove_newline(optarg);
				break;

			case 'h':
				printf( "useage: ./xcpenc [OPTION] ... [-p password] SOURCE DEST\n"
					"Copy, Encrypt or decrypt contents SOURCE to DEST.\n"
					"\n"
					"options:\n"
					"  -e           encrypt the contents of SOURCE file and\n"
					"                store it in DEST file\n"
					"  -d           decrypt the contents of SOURCE file and\n"
					"                store it in DEST file\n"
					"  -c           copy the contents of SOURCE file and\n"
					"                store it in DEST file\n"
					"  -p           password in case of encryption and\n"
					"                decryption\n"
					"\n"
					"	-h     display this help and exit\n"
					"\n"
					"Exit status:\n"
					" 0      if OK,\n"
					" else   if problems (e.g., cannot access infile)\n");
				goto out;

			case '?':
				printf("Unknown option : %c \n", opt);
				err = -1;
				goto out;
		}
	}

	if(cp->flags == -1){
		printf("INVOPT:Alteast one of -e, -d or -c must be specified\n");
		err = -EINVAL;
		goto out;
	}
	if((cp->flags &  0x01 || cp ->flags & 0x02) && key == NULL) {
		printf("INOPT:Password must be specified in case of [en|de]cryption \n");
		err = -EINVAL;
		goto out;
	}
	if(cp->flags & 0x04  && key != NULL) {
                printf("INOPT:Password must be specified only in case of [en|de]cryption \n");
                err = -EINVAL;
                goto out;
        }
	if(key != NULL && strlen(key) < MIN_PASS_LEN) {
		printf("Password too Short! Must be atleast 6 characters long\n");
		err = -EINVAL;
		goto out;
	} 
	if(optind + 2 != argc) {
		printf("INVOPT:Invalid file info \n");
		err = -EINVAL;
		goto out;	
	}
	if(strlen(argv[optind]) > MAX_FILE_NAME || strlen(argv[optind + 1]) > MAX_FILE_NAME) {
                printf("ERROR: File name too long!\n");
                err = -ENAMETOOLONG;
                goto out;
        }
	cp->infile = argv[optind];
	cp->outfile = argv[optind + 1];
	// print_struct(cp);
	if(access(cp->infile, R_OK) == -1) {
		printf("ERROR: file does not exist \n");
                err = -ENOENT;
                goto out;
	}
	if(key) {
		pass_len = strlen(key);	
		if(!SHA1((const unsigned char *)key, pass_len, hash)){
			printf("Hashing failed \n");
			err = -1;
			goto out;
		}
		cp->keybuf = hash;
		cp->keylen = SHA_DIGEST_LENGTH;
	} 

	//End of Mycode
	rc = syscall(__NR_cpenc, cp);
	if (rc == 0)
		printf("Success! syscall returned %d\n", rc);
	else {
		printf("%s\n",strerror(errno));
		err = errno;
	}
	goto out;
out:
	free(cp);
	exit(err);
}

void print_struct(struct cpenc *cp) {
	printf("Infile is : %s \n",cp->infile);
	printf("Outfile is : %s \n",cp->outfile);
	printf("Flag is set to : %d \n",cp->flags);
}
