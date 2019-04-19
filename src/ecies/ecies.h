/**
 * @file /cryptron/ecies.h
 *
 * @brief ECIES module functions.
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
 */

#ifndef LAVABIT_ECIES_H
#define LAVABIT_ECIES_H

#if (__cplusplus)
extern "C" {
#endif

#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>

#include <openssl/opensslv.h>

#if (OPENSSL_VERSION_NUMBER >= 0x10002000L)
#include <openssl/ecdh.h>  /* for ECDH_KDF_X9_62() */
#endif

typedef struct {
    struct {
        size_t key;
        size_t mac;
        size_t body;
    } length;
} secure_head_t;

typedef struct {
    const EVP_CIPHER *cipher;
    const EVP_MD *md;
    const EVP_MD *kdf_md;
    size_t stored_key_length;
    const EC_KEY *user_key;
} ecies_ctx_t;

typedef unsigned char *secure_t;

secure_t *ecies_encrypt(const ecies_ctx_t *ctx, const unsigned char *data, size_t length, char *error);
unsigned char *ecies_decrypt(const ecies_ctx_t *ctx, const secure_t *cryptex, size_t *length, char *error);

unsigned char *secure_key_data(const secure_t *cryptex);
unsigned char *secure_mac_data(const secure_t *cryptex);
unsigned char *secure_body_data(const secure_t *cryptex);
size_t secure_key_length(const secure_t *cryptex);
size_t secure_mac_length(const secure_t *cryptex);
size_t secure_body_length(const secure_t *cryptex);
size_t secure_data_sum_length(const secure_t *cryptex);
size_t secure_total_length(const secure_t *cryptex);

secure_t *secure_alloc(size_t key, size_t mac, size_t body);
void secure_free(secure_t *cryptex);

#if (__cplusplus)
}
#endif

#endif /* LAVABIT_ECIES_H */
