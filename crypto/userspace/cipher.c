#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <linux/socket.h>

/* Some old versions of glibc doesn't have it set yet */
#ifndef AF_ALG
#define AF_ALG 38
#endif
#ifndef SOL_ALG
#define SOL_ALG 279
#endif

#define AES_KEY_LEN 16
#define AES_BLOCK_LEN 16
#define AES_IV_LEN 16

/* crypto key used in the encryption process */
__u8 key[AES_KEY_LEN] = {
	0x06, 0xa9, 0x21, 0x40, 0x36, 0xb8, 0xa1, 0x5b, 0x51, 0x2e, 0x03,
	0xd5, 0x34, 0x12, 0x00, 0x06
};

/* initial vector used in the encryption process */
__u8 ivbuf[AES_IV_LEN] = {
	0x3d, 0xaf, 0xba, 0x42, 0x9d, 0x9e, 0xb4, 0x30, 0xb4, 0x22, 0xda,
	0x80, 0x2c, 0x9f, 0xac, 0x41
};

int fd;

int encrypt(char *plaintext, size_t text_len, char *ciphertext)
{
	char ictext[AES_BLOCK_LEN];
	/* iovec is an special type of vector: a conjunt of segments that
	 * represents a single buffer spread throughtout the physical memory,
	 * in other words, a buffer big enough to not be held in contiguous
	 * memory in physical memory. This vector is used in scatter/gather
	 * I/O to speed up transitions through DMA. "msg_vec" holds our
	 * plaintext buffer. */
	struct iovec msg_vec;
	/* msghdr holds the main information sent to the socket, in this case,
	 * our "msg_vec". */
	struct msghdr msg = {};
	/* BUT, the crypto API requires some additional fields to be
	 * fulfilled, like the /operation/ we want to perform over our
	 * plaintext: ENCRYPT or DECRYPT? These additional data are passed
	 * using what is known as "control message" or "cmsg". */
	struct cmsghdr *cmsg;
	/* The buffer containing the whole control msg data must have the size
	 * of the respective data and be initialized with zeros. In this case,
	 * the buffer must have 4 bytes to hold the operation information plus
	 * 20 bytes for the IV structure (which holds the actual IV (16 bytes)
	 * and its len in int (4 bytes) */
	char cbuf[CMSG_SPACE(4) + CMSG_SPACE(4 + AES_IV_LEN)] = {0};
	/* Finally, we have the specific IV structure */
	struct af_alg_iv *iv;
	int err;

	/* Start populating the msghdr */
	msg.msg_control = cbuf;
	msg.msg_controllen = sizeof(cbuf);

	/* and initialize our scatter/gather I/O buffer */
	msg_vec.iov_base = plaintext;
	msg_vec.iov_len = text_len;
	msg.msg_iov = &msg_vec;
	msg.msg_iovlen = 1;

	/* Allocate the first control chunk */
	cmsg = CMSG_FIRSTHDR(&msg);
	if (!cmsg) {
		fprintf(stderr, "failed to get cmsg\n");
		return -EFAULT;
	}
	/* and set its fields accordingly to crypto API */
	cmsg->cmsg_level = SOL_ALG;
	cmsg->cmsg_type = ALG_SET_OP;
	cmsg->cmsg_len = CMSG_LEN(sizeof(__u32));
	/* 'data' field will contain the operation ID */
	*(int *)CMSG_DATA(cmsg) = ALG_OP_ENCRYPT;

	/* Once the OPERATION is defined, allocate another control msg to
	 * handle IV setting */
	cmsg = CMSG_NXTHDR(&msg, cmsg);
	if (!cmsg) {
		fprintf(stderr, "failed to get cmsg\n");
		return -EFAULT;
	}
	cmsg->cmsg_level = SOL_ALG;
	cmsg->cmsg_type = ALG_SET_IV;
	cmsg->cmsg_len = CMSG_LEN(4 + AES_IV_LEN);
	/* 'data' field here will contain the IV bytes */
	iv = (struct af_alg_iv *)CMSG_DATA(cmsg);
	iv->ivlen = AES_IV_LEN;
	memcpy(iv->iv, ivbuf, AES_IV_LEN);

	err = sendmsg(fd, &msg, 0);
	if (err < 0) {
		perror("enc: failed to send msg\n");
		return err;
	}

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	msg.msg_iov = &msg_vec;
	msg.msg_iovlen = 1;

	err = recvmsg(fd, &msg, 0);
	if (err < 0) {
		perror("dec: failed to recv msg");
		return err;
	}

	memcpy(ciphertext, msg_vec.iov_base, msg_vec.iov_len);
	return 0;
}

int decrypt(char *plaintext, size_t text_len, char *ciphertext)
{
	struct iovec msg_vec;
	struct msghdr msg = {};
	struct cmsghdr *cmsg;
	char cbuf[CMSG_SPACE(4) + CMSG_SPACE(4 + AES_IV_LEN)] = {0};
	struct af_alg_iv *iv;
	int i, err = 0;

	msg.msg_control = cbuf;
	msg.msg_controllen = sizeof(cbuf);

	msg_vec.iov_base = ciphertext;
	msg_vec.iov_len = AES_BLOCK_LEN;
	msg.msg_iov = &msg_vec;
	msg.msg_iovlen = 1;

	cmsg = CMSG_FIRSTHDR(&msg);
	if (!cmsg) {
		fprintf(stderr, "failed to get cmsg\n");
		return -EFAULT;
	}
	cmsg->cmsg_level = SOL_ALG;
	cmsg->cmsg_type = ALG_SET_OP;
	cmsg->cmsg_len = CMSG_LEN(4);
	*(int *)CMSG_DATA(cmsg) = ALG_OP_DECRYPT;

	cmsg = CMSG_NXTHDR(&msg, cmsg);
	if (!cmsg) {
		fprintf(stderr, "failed to get next cmsg\n");
		return -EFAULT;
	}
	cmsg->cmsg_level = SOL_ALG;
	cmsg->cmsg_type = ALG_SET_IV;
	cmsg->cmsg_len = CMSG_LEN(4 + AES_IV_LEN);
	iv = (struct af_alg_iv *)CMSG_DATA(cmsg);
	iv->ivlen = AES_IV_LEN;
	memcpy(iv->iv, ivbuf, AES_IV_LEN);

	err = sendmsg(fd, &msg, 0);
	if (err < 0) {
		perror("dec: failed to send msg");
		return -errno;
	}

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	msg.msg_iov = &msg_vec;
	msg.msg_iovlen = 1;

	err = recvmsg(fd, &msg, 0);
	if (err < 0) {
		perror("dec: failed to recv msg");
		return err;
	}

	memcpy(plaintext, msg_vec.iov_base, msg_vec.iov_len);
	return 0;
}

int main(int argc, char *argv[])
{
	int err, i, text_len;
	/* Socket related vars */
	int sock_fd;
	/* Crypto related buffers */
	/* text to be encrypted */
	char *plaintext;
	/* encrypted data */
	char ciphertext[AES_BLOCK_LEN];
	/* Different from what we use in normal TCP/IP socket programming,
	 * that fills a sockaddr_in structure, here we work over a
	 * sockaddr_alg one */
	struct sockaddr_alg sa_alg = {
		.salg_family = AF_ALG,
		.salg_type = "skcipher",
		.salg_name = "ctr(aes)"
	};

	/* Get input from user */
	if (argc > 1) {
		plaintext = argv[1];
	} else {
		plaintext = strndup("Hello World", 11);
		if (!plaintext) {
			fprintf(stderr, "not enough memory\n");
			return -ENOMEM;
		}
	}
	text_len = strlen(plaintext);

	/* AF_ALG is the address family we use to interact with Kernel
	 * Crypto API. SOCK_SEQPACKET is used because we always know the
	 * maximum size of our data (no fragmentation) and we care about
	 * getting things in order in case there are consecutive calls */
	sock_fd = socket(AF_ALG, SOCK_SEQPACKET, 0);
	if (sock_fd < 0) {
		perror("failed to allocate socket\n");
		return -1;
	}

	err = bind(sock_fd, (struct sockaddr *)&sa_alg, sizeof(sa_alg));
	if (err) {
		perror("failed to bind socket, alg may not be supported\n");
		return -EAFNOSUPPORT;
	}

	/* We should, of course, have a random generated key, which could've
	 * been initialized through the same AF_ALG, but for the sake of
	 * the example we're going to use a zeroed key */
	err = setsockopt(sock_fd, SOL_ALG, ALG_SET_KEY, key, AES_KEY_LEN);
	if (err < 0) {
		perror("failed to set crypto key\n");
		return -1;
	}

	/* Once it's "configured", we tell the kernel to get ready for
	 * receiving some requests */
	fd = accept(sock_fd, NULL, 0);
	if (fd < 0) {
		perror("failed to open connection for the socket\n");
		return -EBADF;
	}

	err = encrypt(plaintext, text_len, ciphertext);
	if (err)
		return err;

	/* Print digest to output */
	for (i = 0; i < AES_BLOCK_LEN; i++)
		printf("%02x", (unsigned char)ciphertext[i]);
	printf("\n");

	memset(plaintext, 0, text_len);
	err = decrypt(plaintext, text_len, ciphertext);
	if (err)
		return err;

	for (i = 0; i < text_len; i++)
		printf("%c", (unsigned char)plaintext[i]);
	printf("\n");

	close(fd);
	close(sock_fd);

	return 0;
}
