/*
 * Copyright (c) 2020 Bruno Meneguele <bmeneguele@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 */

/* __init/exit, macros (MODULE_*) that initializes the module itself */
#include <linux/module.h>
/* Printing function definitions */
#include <linux/kernel.h>
/* Skcipher kernel crypto API */
#include <crypto/skcipher.h>
/* Scatterlist manipulation */
#include <linux/scatterlist.h>
/* Error macros */
#include <linux/err.h>

/* Printing helper functions */
#include "../utils.h"

void crypto_req_done(struct crypto_async_request *req, int err)
{
	struct crypto_wait *wait = req->data;

	if (err == -EINPROGRESS)
		return;

	wait->err = err;
	complete(&wait->completion);
}

static int __init using_crypto_init(void)
{
	int err;
	struct crypto_skcipher *tfm;
	struct skcipher_request *req;
	struct crypto_wait wait;
	struct scatterlist sg;
	char buffer[6];
	/* We're going to use a zerod 128 bits key */
	char key[16] = { 0 };
	/* Initialization Vector */
	char *iv;
	unsigned int ivsize;

	PR_DEBUG("initializing module\n");

	/* Check the existence of the cipher in the kernel (it might be a module
	 * and it isn't loaded. */
	if (!crypto_has_skcipher("cbc(aes)", 0, 0)) {
		PR_ERROR("skcipher not found\n");
		return -EINVAL;
	}

	/* Allocate asynchronous cipher handler.
	 *
	 * For generic implementation you can provide either the generic
	 * name "cbc(aes)" or the driver (specific) name "cbc-aes-aesni".
	 *
	 * Also, cypher type will be left 0 since we want the default
	 * handler, but for the mask flag we're going to set the one that
	 * allow us to use the asynchronous handler.
	 */
	tfm = crypto_alloc_skcipher("cbc(aes)", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		PR_ERROR("impossible to allocate skcipher\n");
		return PTR_ERR(tfm);
	}

	/* Default function to set the key for the symetric key cipher */
	err = crypto_skcipher_setkey(tfm, key, sizeof(key));
	if (err) {
		PR_ERROR("fail setting key for transformation: %d\n", err);
		goto error0;
	}
	print_hex_dump(KERN_DEBUG, "key: ", DUMP_PREFIX_NONE, 16, 1, key, 16,
		       false);

	/* Each crypto cipher has its own Initialization Vector (IV) size,
	 * because of that I first request the correct size for aes IV and
	 * then set it. Considering this is just an example I'll use as IV
	 * the content of a random memory space which I just allocated. */
	ivsize = crypto_skcipher_ivsize(tfm);
	iv = kmalloc(ivsize, GFP_KERNEL);
	if (!iv) {
		PR_ERROR("could not allocate iv vector\n");
		err = -ENOMEM;
		goto error0;
	}
	print_hex_dump(KERN_DEBUG, "iv: ", DUMP_PREFIX_NONE, 16, 1, iv, ivsize,
		       false);

	/* The word to be encrypted */
	memcpy(&buffer, "aloha\0", 6);
	/* TODO: explain scatter/gather lists, that has relation to DMA */
	sg_init_one(&sg, buffer, 6);

	/* Requests are objects that hold all information about a crypto
	 * operation, from the tfm itself to the buffers and IV that will be
	 * used in the enc/decryption operations. But it also holds information
	 * about asynchronous calls to the crypto engine. */
	req = skcipher_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		PR_ERROR("impossible to allocate skcipher request\n");
		err = -ENOMEM;
		goto error0;
	}
	crypto_init_wait(&wait);
	skcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_SLEEP,
				      crypto_req_done, &wait);
	skcipher_request_set_crypt(req, &sg, &sg, 6, iv);

	print_hex_dump(KERN_DEBUG, "orig text: ", DUMP_PREFIX_NONE, 16, 1,
		       buffer, 16, true);

	/* Encrypt operation against "buffer" content */
	err = crypto_skcipher_encrypt(req);
	if (err) {
		PR_ERROR("could not encrypt data\n");
		goto error1;
	}
	sg_copy_to_buffer(&sg, 1, &buffer, 6);
	print_hex_dump(KERN_DEBUG, "encr text: ", DUMP_PREFIX_NONE, 16, 1,
		       buffer, 16, true);

	/* Decrypt operation against the new buffer (scatterlist that holds the
	 * ciphered text). */
	err = crypto_skcipher_decrypt(req);
	if (err) {
		PR_ERROR("could not decrypt data\n");
		goto error1;
	}
	sg_copy_to_buffer(&sg, 1, &buffer, 6);
	print_hex_dump(KERN_DEBUG, "decr text: ", DUMP_PREFIX_NONE, 16, 1,
		       buffer, 16, true);
error1:
	skcipher_request_free(req);
error0:
	crypto_free_skcipher(tfm);
	return err;
}

static void __exit using_crypto_exit(void)
{
	PR_DEBUG("exiting module\n");
}

module_init(using_crypto_init);
module_exit(using_crypto_exit);

MODULE_AUTHOR("Bruno E. O. Meneguele <bmeneguele@gmail.com>");
MODULE_DESCRIPTION("Testing kernel crypto api");
MODULE_LICENSE("GPL");
