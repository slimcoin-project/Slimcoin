/**
 * @file /cryptron/ecies.c
 *
 * @brief ECIES encryption/decryption functions.
 *
 * $Author: Ladar Levison $
 * $Website: http://lavabit.com $
 * $Date: 2010/08/06 06:02:03 $
 * $Revision: a51931d0f81f6abe29ca91470931d41a374508a7 $
 *
 * I hereby place the attached code in the public domain.
 * As such it comes without any warranty regarding its merchantability or
 * fitness for a particular purpose. Please use it at your own risk.
 *
 * Optimised by John Doering <ghostlander@slimcoin.org>
 *
 */

#include <string.h> 
#include "ecies.h"

static EC_KEY *ecies_key_create(const EC_KEY *user, char *error) {
    const EC_GROUP *group;
    EC_KEY *key = NULL;

    if(!(key = EC_KEY_new())) {
        sprintf(error, "EC_KEY_new() failed");
        return(NULL);
    }

    if(!(group = EC_KEY_get0_group(user))) {
        sprintf(error, "EC_KEY_get0_group() failed");
        EC_KEY_free(key);
        return(NULL);
    }

    if(EC_KEY_set_group(key, group) != 1) {
        sprintf(error, "EC_KEY_set_group() failed");
        EC_KEY_free(key);
        return(NULL);
    }

    if(EC_KEY_generate_key(key) != 1) {
        sprintf(error, "EC_KEY_generate_key() failed");
        EC_KEY_free(key);
        return(NULL);
    }

    return(key);
}

static EC_KEY *ecies_key_create_public_octets(EC_KEY *user, unsigned char *octets,
  size_t length, char *error) {
    EC_KEY *key = NULL;
    EC_POINT *point = NULL;
    const EC_GROUP *group = NULL;

    if(!(key = EC_KEY_new())) {
        sprintf(error, "EC_KEY_new() failed");
        return(NULL);
    }

    if(!(group = EC_KEY_get0_group(user))) {
        sprintf(error, "EC_KEY_get0_group() failed");
        EC_KEY_free(key);
        return(NULL);
    }

    if(EC_KEY_set_group(key, group) != 1) {
        sprintf(error, "EC_KEY_set_group() failed");
        EC_KEY_free(key);
        return(NULL);
    }

    if(!(point = EC_POINT_new(group))) {
        sprintf(error, "EC_POINT_new() failed");
        EC_KEY_free(key);
        return(NULL);
    }

    if(EC_POINT_oct2point(group, point, octets, length, NULL) != 1) {
        sprintf(error, "EC_POINT_oct2point() failed");
        EC_KEY_free(key);
        return(NULL);
    }

    if(EC_KEY_set_public_key(key, point) != 1) {
        sprintf(error, "EC_KEY_set_public_key() failed");
        EC_POINT_free(point);
        EC_KEY_free(key);
        return(NULL);
    }

    EC_POINT_free(point);

    if(EC_KEY_check_key(key) != 1) {
        sprintf(error, "EC_KEY_check_key() failed");
        EC_KEY_free(key);
        return(NULL);
    }

    return(key);
}

secure_t *ecies_encrypt(const ecies_ctx_t *ctx, const unsigned char *data, size_t length,
  char *error) {
    unsigned int output_length;
    unsigned int status = 1;

    if(!ctx || !data || !length) {
        sprintf(error, "Input NULL pointer detected");
        return(NULL);
    }

    secure_t *cryptex = NULL;

    const size_t block_length = EVP_CIPHER_block_size(ctx->cipher);

    if(!block_length || (block_length > EVP_MAX_BLOCK_LENGTH)) {
        sprintf(error, "Invalid block size");
        return(NULL);
    }

    if(!(cryptex = secure_alloc(ctx->stored_key_length, EVP_MD_size(ctx->md),
      length + (length % block_length ? (block_length - (length % block_length)) : 0)))) {
        sprintf(error, "Unable to allocate a buffer for encrypted data");
        return(NULL);
    }

    /* Stage 1: envelope key creation */

    const size_t key_len = EVP_CIPHER_key_length(ctx->cipher) + EVP_MD_size(ctx->md);
    unsigned char envelope_key[key_len];

    const size_t ecdh_key_len = (EC_GROUP_get_degree(EC_KEY_get0_group(ctx->user_key)) + 7) / 8;
    unsigned char ecdh_temp_key[ecdh_key_len];

    EC_KEY *ephemeral = NULL;

    if(!(ephemeral = ecies_key_create(ctx->user_key, error))) {
        /* Error message has been set already */
        status = 0;
    } else {
        if(ECDH_compute_key(ecdh_temp_key, ecdh_key_len,
          EC_KEY_get0_public_key(ctx->user_key), ephemeral, NULL) != (int)ecdh_key_len) {
            sprintf(error, "ECDH_compute_key() failed");
            status = 0;
        } else {
            if(!ECDH_KDF_X9_62(envelope_key, key_len, ecdh_temp_key, ecdh_key_len,
              0, 0, ctx->kdf_md)) {
                sprintf(error, "ECDH_KDF_X9_62() failed to stretch the key");
                status = 0;
            } else {
                /* Verify the public key portion of the ephemeral key */
                output_length = EC_POINT_point2oct(EC_KEY_get0_group(ephemeral),
                  EC_KEY_get0_public_key(ephemeral), POINT_CONVERSION_COMPRESSED,
                  secure_key_data(cryptex), ctx->stored_key_length, NULL);
                if(output_length != ctx->stored_key_length) {
                    sprintf(error, "Incorrect length of the public portion of the envelope key");
                    status = 0;
                }
            }
        }
    }

    if(ephemeral) EC_KEY_free(ephemeral);

    if(!status) return(NULL);

    /* Stage 2: symmetric cipher context encryption */

    size_t encrypted_length;

    unsigned char iv[EVP_MAX_IV_LENGTH];
    unsigned char *body = secure_body_data(cryptex);

    EVP_CIPHER_CTX *cipher;

    memset(iv, 0, EVP_MAX_IV_LENGTH);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
      cipher = EVP_CIPHER_CTX_new();
#else
      cipher = malloc(sizeof(*cipher));
#endif
      if (!cipher) {
        fprintf(stderr, "Out of memory: EVP_CIPHER_CTX!\n");
        return(NULL);
      }
    EVP_CIPHER_CTX_init(cipher);

    if(EVP_EncryptInit_ex(cipher, ctx->cipher, NULL, envelope_key, iv) != 1) {
        sprintf(error, "Initial context encryption failed");
        status = 0;
    } else {
        if(EVP_EncryptUpdate(cipher, body, (int *) &output_length, data, length) != 1) {
            sprintf(error, "Context encryption failed");
            status = 0;
        } else {
            encrypted_length = output_length;
            if(EVP_EncryptFinal_ex(cipher, body + encrypted_length, (int *) &output_length) != 1) {
                sprintf(error, "Final context encryption failed");
                status = 0;
            } else {
                encrypted_length += output_length;
                if(secure_body_length(cryptex) < encrypted_length) {
                    sprintf(error, "Cipher context overflow");
                    status = 0;
                }
            }
        }
    }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
      EVP_CIPHER_CTX_reset(cipher);
#else
      EVP_CIPHER_CTX_cleanup(cipher);
#endif

    if(!status) {
        secure_free(cryptex);
        return(NULL);
    }

    /* Stage 3: HMAC generation */

    HMAC_CTX *hmac;

    const size_t key_offset = EVP_CIPHER_key_length(ctx->cipher);
    const size_t key_length = EVP_MD_size(ctx->md);
    const size_t mac_length = secure_mac_length(cryptex);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
      hmac = HMAC_CTX_new();
#else
      hmac = malloc(sizeof(*hmac));
#endif
      if (!hmac) {
        fprintf(stderr, "Out of memory: HMAC CTX!\n");
        exit(1);
      }
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
      HMAC_CTX_reset(hmac);
#else
      HMAC_CTX_init(hmac);
#endif

    /* Generate an authenticated hash which can be used to validate the 
     * data during decryption */

    if((HMAC_Init_ex(hmac, envelope_key + key_offset, key_length, ctx->md, NULL) != 1) ||
      (HMAC_Update(hmac, secure_body_data(cryptex), secure_body_length(cryptex)) != 1) ||
      (HMAC_Final(hmac, secure_mac_data(cryptex), &output_length) != 1)) {
        sprintf(error, "Unable to generate a HMAC tag");
        status = 0;
    } else {
        if(output_length != mac_length) {
            sprintf(error, "HMAC tag verification failed");
            status = 0;
        }
    }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    HMAC_CTX_free(hmac);
#else
    HMAC_CTX_cleanup(hmac);
    free(hmac);
#endif

    if(!status) {
        secure_free(cryptex);
        return(NULL);
    }

    memset(envelope_key, 0, key_len);
    memset(ecdh_temp_key, 0, ecdh_key_len);

    return(cryptex);
}

unsigned char *ecies_decrypt(const ecies_ctx_t *ctx, const secure_t *cryptex, size_t *length,
  char *error) {
    unsigned int output_length;
    unsigned int status = 1;

    if(!ctx || !cryptex || !length || !error) {
        sprintf(error, "Input NULL pointer detected");
        return(NULL);
    }

    /* Stage 1: envelope key recovery */

    const size_t key_len = EVP_CIPHER_key_length(ctx->cipher) + EVP_MD_size(ctx->md);
    unsigned char envelope_key[key_len];

    const size_t ecdh_key_len = (EC_GROUP_get_degree(EC_KEY_get0_group(ctx->user_key)) + 7) / 8;
    unsigned char ecdh_temp_key[ecdh_key_len];

    EC_KEY *ephemeral = NULL, *user_copy = NULL;

    if(!(user_copy = EC_KEY_new())) {
        sprintf(error, "Failed to create a user key instance");
        status = 0;
    } else {
        if(!EC_KEY_copy(user_copy, ctx->user_key)) {
            sprintf(error, "Failed to copy user key");
            status = 0;
        } else {
            if(!(ephemeral = ecies_key_create_public_octets(user_copy, secure_key_data(cryptex),
              secure_key_length(cryptex), error))) {
                /* Error message has been set already */
                status = 0;
            } else {
                if(ECDH_compute_key(ecdh_temp_key, ecdh_key_len, EC_KEY_get0_public_key(ephemeral),
                  user_copy, NULL) != (int)ecdh_key_len) {
                    sprintf(error, "ECDH_compute_key() failed");
                    status = 0;
                } else {
                    if(!ECDH_KDF_X9_62(envelope_key, key_len, ecdh_temp_key, ecdh_key_len,
                      0, 0, ctx->kdf_md)) {
                        sprintf(error, "ECDH_KDF_X9_62() failed to stretch the key");
                        status = 0;
                    }
                }
            }
        }
    }

    if(!status) {
        if(ephemeral) EC_KEY_free(ephemeral);
        if(user_copy) EC_KEY_free(user_copy);
        return(NULL);
    }

    /* Stage 2: HMAC verification */

    unsigned char md[EVP_MAX_MD_SIZE];
    HMAC_CTX *hmac;

    const size_t key_offset = EVP_CIPHER_key_length(ctx->cipher);
    const size_t key_length = EVP_MD_size(ctx->md);
    const size_t mac_length = secure_mac_length(cryptex);

    /* Use the authenticated hash of the ciphered data to ensure it was not 
     * modified after being encrypted */

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
      hmac = HMAC_CTX_new();
#else
      hmac = malloc(sizeof(*hmac));
#endif
      if (!hmac) {
        fprintf(stderr, "Out of memory: HMAC CTX!\n");
        exit(1);
      }
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
      HMAC_CTX_reset(hmac);
#else
      HMAC_CTX_init(hmac);
#endif

    if((HMAC_Init_ex(hmac, envelope_key + key_offset, key_length, ctx->md, NULL) != 1) ||
      (HMAC_Update(hmac, secure_body_data(cryptex), secure_body_length(cryptex)) != 1) ||
      (HMAC_Final(hmac, md, &output_length) != 1)) {
        sprintf(error, "Unable to generate a HMAC tag");
        status = 0;
    } else {
        if((output_length != mac_length) ||
          (memcmp(md, secure_mac_data(cryptex), mac_length) != 0)) {
             sprintf(error, "HMAC tag verification failed");
             status = 0;
        }
    }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    HMAC_CTX_free(hmac);
#else
    HMAC_CTX_cleanup(hmac);
    free(hmac);
#endif

    if(!status) return(NULL);

    /* Stage 3: symmetric cipher context decryption */

    size_t decrypted_length;

    unsigned char iv[EVP_MAX_IV_LENGTH];
    unsigned char *output;
    EVP_CIPHER_CTX *cipher;

    const size_t body_length = secure_body_length(cryptex);

    if(!(output = (unsigned char *) malloc(body_length + 1))) {
        sprintf(error, "Unable to allocate a buffer for decrypted data");
        return(NULL);
    }

    memset(iv, 0, EVP_MAX_IV_LENGTH);
    memset(output, 0, body_length + 1);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
      cipher = EVP_CIPHER_CTX_new();
#else
      cipher = malloc(sizeof(*cipher));
#endif
    EVP_CIPHER_CTX_init(cipher);

    if(EVP_DecryptInit_ex(cipher, ctx->cipher, NULL, envelope_key, iv) != 1) {
        sprintf(error, "Initial context decryption failed");
        status = 0;
    } else {
        if(EVP_DecryptUpdate(cipher, output, (int *) &output_length,
          secure_body_data(cryptex), body_length) != 1) {
            sprintf(error, "Context decryption failed");
            status = 0;
        } else {
            decrypted_length = output_length;
            if(EVP_DecryptFinal_ex(cipher, output + decrypted_length,
              (int *) &output_length) != 1) {
                sprintf(error, "Final context decryption failed");
                status = 0;
            } else {
                decrypted_length += output_length;
            }
        }
    }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
      EVP_CIPHER_CTX_reset(cipher);
#else
      EVP_CIPHER_CTX_cleanup(cipher);
#endif

    if(!status) {
        free(output);
        return(NULL);
    }

    *length = decrypted_length;

    memset(envelope_key, 0, key_len);
    memset(ecdh_temp_key, 0, ecdh_key_len);

    return(output);
}
