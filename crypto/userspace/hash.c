#include <stdio.h>
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

#define SHA256_DIG_LEN 32

int main(int argc, char *argv[])
{
	char *plaintext;
	int sock_fd, fd, text_len;
	unsigned char digest[SHA256_DIG_LEN];
	int err, i;

	/* Different from what we use in normal TCP/IP socket programming,
	 * that fills a sockaddr_in structure, here we work over a
	 * sockaddr_alg one */
	struct sockaddr_alg sa_alg = {
		.salg_family = AF_ALG,
		.salg_type = "hash",
		.salg_name = "sha256"
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

	/* Once it's "configured", we tell the kernel to get ready for
	 * receiving some requests */
	fd = accept(sock_fd, NULL, 0);
	if (fd < 0) {
		perror("failed to open connection for the socket\n");
		return -EBADF;
	}

	/* In hash cases, we don't really need to inform anything else, we
	 * can start sending data to the fd and read back from it to get our
	 * digest. OTOH, when working with ciphers, we need to perform some
	 * operations via setsockopt() interface, using the specifics
	 * options, like ALG_SET_KEY */
	text_len = strlen(plaintext);
	err = write(fd, plaintext, text_len);
	if (err != text_len) {
		perror("something went wrong while writing data to fd\n");
		return -1;
	}
	read(fd, digest, SHA256_DIG_LEN);

	close(fd);
	close(sock_fd);

	/* Print digest to output */
	for (i = 0; i < SHA256_DIG_LEN; i++)
		printf("%02x", digest[i]);
	printf("\n");

	return 0;
}
