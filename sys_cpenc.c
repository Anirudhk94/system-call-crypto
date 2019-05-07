#include <linux/linkage.h>
#include <linux/moduleloader.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/crypto.h>
#include <crypto/hash.h>
#include <crypto/skcipher.h>
#include <linux/scatterlist.h>
#include <linux/kernel.h>
#include "cpenc.h"

asmlinkage extern long (*sysptr)(void *arg);
#define PREAMBLE_LENGTH 16
#define MAX_FILE_NAME 254
#define LEN_TEMP_EXT 6
#define EXTRA_CREDIT
#define IV_LENGTH 16

/**
 * get_md5_digest() - generate md5 hash for a string
 * @keybuf	: buffer holding the input string
 * @keylen	: length of keybuf in bytes
 * @md5_digest	: buffer to hold the resultant md5 hash
 *
 * Return: 0 if the cipher operation was successful;
 *		< 0 if an error occurred
 */
static int get_md5_digest(void *keybuf, size_t keylen, char *md5_digest)
{
	struct crypto_shash *md5_tfm;
	struct shash_desc desc;
	int err = 0;

	// allocating the transform object for md5
	md5_tfm = crypto_alloc_shash("md5", 0, 0);
	if (md5_tfm == NULL) {
		pr_info("ERROR: Transformation object allocation failed\n");
		err = -ENOMEM;
		goto out;
	}

	// describing the context of the hash operation
	desc.tfm = md5_tfm;
	desc.flags = 0;
	err = crypto_shash_digest(&desc, keybuf, keylen, md5_digest);
	if (err) {
		pr_info("ERROR: MD5 hash generation failed\n");
		err = -EINVAL;
		goto out;
	}
	// cleanup
out:
	if (md5_tfm && !IS_ERR(md5_tfm))
		kfree(md5_tfm);
	return err;
}

/**
 * rename() - rename a source file to target
 * @src		: file structure of the source
 * @tar		: file structure of the target
 *
 * Return: 0 if the rename was successful;
 *		< 0 if an error occurred
 */
int rename(struct file *src, struct file *tar)
{
	int err = 0;
	struct dentry *rename = NULL;
	struct dentry *src_dentry, *tar_dentry;
	struct inode *src_inode, *tar_inode;

	src_dentry = src->f_path.dentry;
	src_inode = src_dentry->d_parent->d_inode;
	tar_dentry = tar->f_path.dentry;
	tar_inode = tar_dentry->d_parent->d_inode;

	rename = lock_rename(src_dentry->d_parent, tar_dentry->d_parent);
	// as src and target files recide in the same dir
	if (rename) {
		pr_info("ERROR: failed to acquire lock\n");
		err = -EPERM;
		goto out;
	}
	vfs_rename(src_inode, src_dentry, tar_inode, tar_dentry, NULL, 0);
	goto out;
out:
	unlock_rename(src_dentry->d_parent, tar_dentry->d_parent);
	return	err;
}

/**
 * preamble_handler() - handles the header for [en|de]cryption
 * @infile	: file structure of the infile
 * @outfile	: file structure of the outfile
 * @keybuf	: buffer holding the preamble value
 * @keylen	: length of the keybuf in bytes
 * @flag	: flag specifying the type of operation
 *
 * Return: 0 if the preamble operation was successful;
 *		< 0 if an error occurred
 */
static int preamble_handler(struct file *infile, struct file *outfile,
		void *keybuf, int keylen, int flag)
{
	int err = 0, bytes = 0;
	void *temp_buff;
	char *md5_digest = NULL;

	// buffer to store the hash value in case of [en|de]cryption
	md5_digest = kmalloc(PREAMBLE_LENGTH, GFP_KERNEL);
	if (!md5_digest) {
		err = -ENOMEM;
		goto out;
	}
	err = get_md5_digest(keybuf, keylen, md5_digest);
	if (err < 0) {
		pr_info("ERROR: md5 hash generation failed\n");
		err = -EINVAL;
		goto out;
	}
	pr_info("MD5 Digest value : %s", md5_digest);

	/**
	 * check if [en|de]cryption and branch accordingly
	 * Encryption : Add the preamble to the outfile
	 * Decryption : Compare the infile preamble with md5_digest
	 */
	if (flag & 0x01) {
		pr_info("Preamble Handler : encryption stage\n");
		bytes = vfs_write(outfile, md5_digest,
				PREAMBLE_LENGTH, &outfile->f_pos);
		if (bytes != PREAMBLE_LENGTH) {
			pr_info("ERROR: writing hash to outfile failed\n");
			err = -EIO;
			goto out;
		}
	} else if (flag & 0x02) {
		pr_info("Preamble Handler : decryption stage\n");
		temp_buff = kmalloc(PREAMBLE_LENGTH, GFP_KERNEL);
		if (temp_buff == NULL) {
			pr_info("ERROR: kmalloc for temp_buff failed\n");
			err = -ENOMEM;
			goto out;
		}
		bytes = vfs_read(infile, temp_buff,
				PREAMBLE_LENGTH, &infile->f_pos);
		if (bytes == -1) {
			pr_info("ERROR: reading hash from infile failed\n");
			err = -EIO;
			goto out;
		}
		if (memcmp(temp_buff, md5_digest, PREAMBLE_LENGTH) != 0) {
			pr_info("The password is incorrect! Please enter the right password\n");
			err = -EACCES;
			goto out;
		}
	}
	goto out;

	// cleanup
out:
	if (temp_buff && !IS_ERR(temp_buff))
		kfree(temp_buff);
	if (md5_digest && !IS_ERR(md5_digest))
		kfree(md5_digest);
	return err;
}

/**
 * encrypt_decrypt() - performs the  [en|de]cryption operations
 * @key		: buffer that holds the [en|de]cryption key
 * @keylen	: length of key in bytes
 * @src		: buffer that holdes input for [en|de]cryption
 * @srclen	: length of src in bytes
 * @flag	: flag specifying the type of operation
 *
 * Return: 0 if [en|de]cryption  was successful;
 *		< 0 if an error occurred
 */
static int encrypt_decrypt(const void *key, size_t keylen,
		const void *src, size_t srclen, char *iv, int flag)
{
	struct scatterlist sg;
	struct crypto_blkcipher *tfm = NULL;
	struct blkcipher_desc desc;
	int err = 0;

	// initialise the scattler list
	sg_init_one(&sg, (u8 *)src, srclen);

	// allocating the transform object for AES in CTR mode
	tfm = crypto_alloc_blkcipher("ctr(aes)", 0, 0);
	if (IS_ERR(tfm)) {
		pr_info("ERROR: Transformation object allocation failed\n");
		err = PTR_ERR(tfm);
		goto out;
	}

	// setting up the crypto context
	desc.tfm = tfm;
	desc.flags = 0;
	crypto_blkcipher_set_iv(tfm, iv, 16);
	err = crypto_blkcipher_setkey(tfm, key, keylen);
	if (err) {
		pr_info("ERROR: setting the crypto key failed\n");
		goto out;
	}

	// perform required operation based on flag
	if (flag & 0x01)
		err = crypto_blkcipher_encrypt(&desc, &sg, &sg, srclen);
	else if (flag & 0x02)
		err = crypto_blkcipher_decrypt(&desc, &sg, &sg, srclen);
	if (err) {
		err = -EIO;
		goto out;
	}
	goto out;

	// cleanup
out:
	if (tfm && !IS_ERR(tfm))
		crypto_free_blkcipher(tfm);
	return err;

}

/**
 * init_iv() - set the initial state of the initialization vector
 * @infile	: file structure of the infile
 * @outfile	: file structure of the outfile
 * @iv		: buffer to store the initialization vector
 * @page_no	: intial page number
 * @flag	: flag specifying the type of operation
 *
 * Return: 0 if iv init was successful;
 *		< 0 if an error occurred
 */
#ifdef EXTRA_CREDIT
static int init_iv(struct file *infile, struct file *outfile,
		   char *iv, u64 page_no, int flag)
{
	int err = 0;

	if (flag & 0x01) {
		memcpy(iv, &page_no, 8);
		memcpy(&iv[8], &(outfile->f_path.dentry->d_inode->i_ino), 8);
		err = vfs_write(outfile, iv, IV_LENGTH, &outfile->f_pos);
		if (err == -1) {
			pr_info("ERROR: writing hash to outfile failed\n");
			err = -EIO;
			goto out;
		}
	} else if (flag & 0x02) {
		err = vfs_read(infile, iv, IV_LENGTH, &infile->f_pos);
		if (err == -1) {
			pr_info("ERROR: reading hash from infile failed\n");
			err = -EIO;
			goto out;
		}
	}
	goto out;
out:
	return err;
}
#endif

/**
 * file_read_write() - performs the read-write operation conditinally
 *			copying, encryption or decryption data
 * @infile      : file structure of the infile
 * @outfile     : file structure of the outfile
 * @data	: temporary buffer to hold
 * @size	: size of the input file
 * @keybuf      : buffer holding the [en|de]cryption (AES in this case)
 * @keylen      : length of keybuf in bytes
 * @flag        : flag specifying the type of operation
 *
 * Return: 0 if read-write  was successful;
 *		< 0 if an error occurred
 */
static int file_read_write(struct file *infile, struct file *outfile,
		void  *data, size_t size, void *keybuf,
		size_t keylen, int flag)
{
	int bytes = 0, i, no_pages, rem_bytes, err = 0;
	mm_segment_t oldfs;
#ifdef EXTRA_CREDIT
	char *iv = NULL;
	u64 page_no = 0;
#endif

	// Calculate number of bytes based on  [en|de]cryption
	if (flag & 0x02) {
		no_pages = (size - 2 * PREAMBLE_LENGTH)/PAGE_SIZE;
		rem_bytes = size - 2 * PREAMBLE_LENGTH - no_pages * PAGE_SIZE;
	} else {
		no_pages = size/PAGE_SIZE;
		rem_bytes = size - no_pages * PAGE_SIZE;
	}
	pr_info("Number of pages : %d\n", no_pages);

	// Setup the fs register inorder to avoid Kernel Address Translation
	oldfs = get_fs();
	set_fs(KERNEL_DS);

	// Evaluate preamble only when [en|de]crypting
	if (flag & 0x01 || flag & 0x02) {
		err = preamble_handler(infile, outfile, keybuf, keylen, flag);
		if (err) {
			pr_info("ERROR: Preamble handler failed\n");
			goto out;
		}
#ifdef EXTRA_CREDIT
		iv = kmalloc(PREAMBLE_LENGTH, GFP_KERNEL);
		if (iv == NULL) {
			err = -ENOMEM;
			goto out;
		}
		err = init_iv(infile, outfile, iv, page_no, flag);
		if (iv == NULL) {
			pr_info("ERROR: iv initialization failed\n");
			err = -ENOMEM;
			goto out;
		}
#endif
	}

	// Efficient mode of read-write - PAGE_SIZE read-write
	for (i = 0; i < no_pages ; i++) {
		bytes = vfs_read(infile, data, PAGE_SIZE, &infile->f_pos);
		if (bytes < 0) {
			err = -EIO;
			goto out;
		}
		if (!(flag & 0x04)) {
			err = encrypt_decrypt(keybuf, PREAMBLE_LENGTH,
					data, PAGE_SIZE, iv, flag);
#ifdef EXTRA_CREDIT
			page_no++;
			memcpy(iv, &page_no, 8);
#endif
		}
		if (err < 0)
			goto out;
		pr_info("Done [En/De]cryption!\n");
		bytes = vfs_write(outfile, data, PAGE_SIZE, &outfile->f_pos);
		if (bytes < 0) {
			err = -EIO;
			goto out;
		}
	}
	pr_info("Rem pages : %d\n", rem_bytes);
	bytes = vfs_read(infile, data, rem_bytes, &infile->f_pos);
	if (bytes < 0) {
		err = -EIO;
		goto out;
	}
	if (!(flag & 0x04))
		encrypt_decrypt(keybuf, PREAMBLE_LENGTH,
				data, rem_bytes, iv, flag);
	pr_info("Done [En/De]cryption!\n");
	bytes = vfs_write(outfile, data, rem_bytes, &outfile->f_pos);
	if (bytes < 0) {
		err = -EIO;
		goto out;
	}
	//Restore state of fs register post read-write
	goto out;
out:
	set_fs(oldfs);
#ifdef EXTRA_CREDIT
	kfree(iv);
#endif
	return err;
}

asmlinkage long cpenc(void *arg)
{
	int err = 0, restore = 0, out_present;
	void *temp_buff = NULL;
	struct cpenc *cp = NULL;
	struct filename *kpath_in = NULL, *kpath_out = NULL;
	struct file *inf = NULL, *outf = NULL, *outf_temp = NULL;
	struct kstat stat_infile, stat_outfile;
	void *keybuf = NULL;
	struct dentry *temp_dentry = NULL;
	struct inode *temp_inode = NULL;
	struct dentry *out_dentry = NULL;
	struct inode *out_inode = NULL;
	char *temp_file = NULL;

	/*
	 * The below block of code allocates memory in the kernel and
	 * copies the contents from the userland cpenc struct
	 */
	if (access_ok(VERIFY_READ, cp, sizeof(struct cpenc)) == 0) {
		pr_info("ERROR: Access to cp for sizeof cpecn denied\n");
		err = -EACCES;
		goto out;
	}
	cp = kmalloc(sizeof(struct cpenc), GFP_KERNEL);
	if (cp == NULL) {
		err = -ENOMEM;
		goto out;
	}
	if (copy_from_user(cp, arg, sizeof(struct cpenc))) {
		pr_info("ERROR: copying from userland failed\n");
		err = -EFAULT;
		goto out;
	}

	/*
	 * Validate the flag
	 * 1 - Encryption | 2 - Decryption | 3 - Simple Copy
	 * NOTE : Need to add more validations. Building happy path now.
	 */
	if (!(cp->flags & 0x01) && !(cp->flags & 0x02) && !(cp->flags & 0x04)) {
		pr_info("ERROR: Specified flag is incorrect\n");
		err = -EINVAL;
		goto out;
	}
	pr_info("OPTION : %d | cp->keylen : %d\n", cp->flags, cp->keylen);
	// Handle cases where password is sent with copy flag
	if (cp->flags & 0x04 && cp->keylen != 0) {
		pr_info("INVALID OPTION: Do not provide password for -c option\n");
		err = -EINVAL;
		goto out;
	}
	// Get the keybuf from the userland and check validity
	if (!(cp->flags & 0x04) && cp->keybuf == NULL) {
		pr_info("ERROR: length of hash key is 0\n");
		err = -EINVAL;
		goto out;
	}
	if (access_ok(VERIFY_READ, cp->keybuf, cp->keylen) == 0) {
		pr_info("ERROR: Access to keybuf for sizeof keylen denied\n");
		err = -EACCES;
		goto out;
	}
	keybuf = kmalloc(cp->keylen, GFP_KERNEL);
	if (keybuf == NULL) {
		err = -ENOMEM;
		goto out;
	}
	if (copy_from_user(keybuf, cp->keybuf, cp->keylen)) {
		pr_info("ERROR: Copying keybuf from user failed\n");
		err = -EINVAL;
		goto out;
	}
	pr_info("Values copied safely from userland to kernel\n");

	/*
	 * The below block of code opens the INFILE and validates the file
	 * NOTE : Need to add more validations. Building happy path now.
	 */
	kpath_in = getname(cp->infile);
	if (IS_ERR(kpath_in)) {
		pr_info("ERROR: getname() failed for infile\n");
		err = PTR_ERR(kpath_in);
		goto out;
	}
	if (strlen(kpath_in->name) > MAX_FILE_NAME) {
		err = -ENAMETOOLONG;
		goto out;
	}
	if (vfs_stat(cp->infile, &stat_infile)) {
		pr_info("ERROR: infile stat retrieval failed");
		goto out;
	}
	pr_info("The size of the file is : %lld\n", stat_infile.size);
	inf = filp_open(kpath_in->name, O_RDONLY, 0);
	if (IS_ERR(inf)) {
		pr_info("ERROR: filp_open() failed while opening infile\n");
		err = PTR_ERR(inf);
		goto out;
	}
	if (!S_ISREG(inf->f_path.dentry->d_inode->i_mode)) {
		pr_info("ERROR: infile is not a regular file\n");
		err = -EINVAL;
		goto out;
	}

	/**
	 * The below block of code opens the OUTFILE and validates the file
	 * NOTE : Need to add more validations. Building happy path now.
	 */
	kpath_out = getname(cp->outfile);
	if (IS_ERR(kpath_out)) {
		pr_info("ERROR: getname() failed for outfile\n");
		err = PTR_ERR(kpath_out);
		goto out;
	}
	if (strlen(kpath_out->name) > MAX_FILE_NAME - LEN_TEMP_EXT) {
		err = -ENAMETOOLONG;
		goto out;
	}
	err = vfs_stat(cp->outfile, &stat_outfile);
	if (err == 0)
		out_present = 1;
	else if (err == -ENOENT)
		out_present = 0;
	else {
		pr_info("ERROR: outfile stat retrieval failed\n");
		err = out_present;
		goto out;
	}
	outf = filp_open(kpath_out->name, O_WRONLY | O_CREAT, 0);
	if (IS_ERR(outf)) {
		pr_info("ERROR: filp_open failed while opening outfile\n");
		err = PTR_ERR(outf);
		goto out;
	}
	if (!S_ISREG(outf->f_path.dentry->d_inode->i_mode)) {
		err = -EINVAL;
		goto out;
	}

	/**
	 * Check for indentical files: Identical only when the files belong to
	 * same fs i.e. superblock and have same inode number
	 */
	if ((inf->f_path.dentry->d_inode->i_ino ==
				outf->f_path.dentry->d_inode->i_ino)
			&& (inf->f_path.dentry->d_inode->i_sb ==
				outf->f_path.dentry->d_inode->i_sb)) {
		pr_info("ERROR: identical infile and outfile. Provide diff files\n");
		err = -EINVAL;
		goto out;
	}

	/**
	 * temp_file to handle cases where the files
	 * are partially written. To restore the previous
	 * state of fs in case of errors/partial writes
	 */
	temp_file = kmalloc(strlen(kpath_out->name) + 6, GFP_KERNEL);
	memcpy(temp_file, kpath_out->name, strlen(kpath_out->name));
	memcpy(&temp_file[strlen(kpath_out->name)], ".temp\0", 6);

	outf_temp = filp_open(temp_file, O_WRONLY | O_CREAT, stat_infile.mode);
	if (IS_ERR(outf_temp)) {
		pr_info("ERROR: filp_open failed while opening outfile\n");
		err = PTR_ERR(outf_temp);
		goto out;
	}
	if (!S_ISREG(outf_temp->f_path.dentry->d_inode->i_mode)) {
		err = -EINVAL;
		goto out;
	}

	// Initialize the outf and outf_temp structures
	temp_dentry = outf_temp->f_path.dentry;
	temp_inode = temp_dentry->d_parent->d_inode;
	out_dentry = outf->f_path.dentry;
	out_inode = out_dentry->d_parent->d_inode;

	// Read-write the contents of infile and copy to outfile.
	temp_buff = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (temp_buff == NULL) {
		err = -ENOMEM;
		goto out;
	}
	err = file_read_write(inf, outf_temp, temp_buff, stat_infile.size,
			keybuf, cp->keylen, cp->flags);
	/* in case of read-write failure, restore the
	 * previous state of the system
	 **/
	if (err < 0) {
		pr_info("ERROR: issues with read-write opearation\n");
		restore = 1;
		goto out;
	}

	err = rename(outf_temp, outf);
	if (err < 0) {
		pr_info("ERROR: issues with rename opearation\n");
		goto out;
	}
	goto out;

	// Cleanup activity
out:
	if (cp && !IS_ERR(cp))
		kfree(cp);
	kfree(temp_file);
	if (temp_buff && !IS_ERR(temp_buff))
		kfree(temp_buff);
	if (restore == 1) {
		pr_info("Inside out and restore = 1\n");
		if (temp_dentry && temp_inode)
			vfs_unlink(temp_inode, temp_dentry, NULL);
		// if the outfile already exists in the system
		if (out_present == 0 && out_inode && out_dentry)
			vfs_unlink(out_inode, out_dentry, NULL);
	}
	if (kpath_in && !IS_ERR(kpath_out))
		putname(kpath_in);
	if (inf && !IS_ERR(inf))
		filp_close(inf, NULL);
	if (kpath_out && !IS_ERR(kpath_out))
		putname(kpath_out);
	if (outf && !IS_ERR(outf))
		filp_close(outf, NULL);
	if (outf_temp && !IS_ERR(outf_temp))
		filp_close(outf_temp, NULL);
	return err;
}

static int __init init_sys_cpenc(void)
{
	pr_info("installed new sys_cpenc module\n");
	if (sysptr == NULL)
		sysptr = cpenc;
	return 0;
}
static void  __exit exit_sys_cpenc(void)
{
	if (sysptr != NULL)
		sysptr = NULL;
	pr_info("removed sys_cpenc module\n");
}
module_init(init_sys_cpenc);
module_exit(exit_sys_cpenc);
MODULE_LICENSE("GPL");


/*
 * PENDING ITEMS :
 *	- Shell scripts for testing
 */
