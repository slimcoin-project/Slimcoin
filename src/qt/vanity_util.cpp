/*
 * Vanitygen, vanity bitcoin address generator
 * Copyright (C) 2011 <samr7@cs.washington.edu>
 *
 * Vanitygen is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * Vanitygen is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Vanitygen.  If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(_WIN32)
#define _USE_MATH_DEFINES
#endif /* defined(_WIN32) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>

#include "vanity_pattern.h"
#include "vanity_util.h"

const char *vg_b58_alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

const signed char vg_b58_reverse_map[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1,
    -1,  9, 10, 11, 12, 13, 14, 15, 16, -1, 17, 18, 19, 20, 21, -1,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1,
    -1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46,
    47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

void
fdumphex(FILE *fp, const unsigned char *src, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++) {
        fprintf(fp, "%02x", src[i]);
    }
    printf("\n");
}

void
fdumpbn(FILE *fp, const BIGNUM *bn)
{
    char *buf;
    buf = BN_bn2hex(bn);
    fprintf(fp, "%s\n", buf ? buf : "0");
    if (buf)
        OPENSSL_free(buf);
}

void
dumphex(const unsigned char *src, size_t len)
{
    fdumphex(stdout, src, len);
}

void
dumpbn(const BIGNUM *bn)
{
    fdumpbn(stdout, bn);
}

/*
 * Key format encode/decode
 */

void
vg_b58_encode_check(void *buf, size_t len, char *result)
{
    unsigned char hash1[32];
    unsigned char hash2[32];

    int d, p;

    BN_CTX *bnctx;
    BIGNUM *bn, *bndiv, *bntmp;
    BIGNUM bna, bnb, bnbase, bnrem;
    unsigned char *binres;
    int brlen, zpfx;

    bnctx = BN_CTX_new();
    BN_init(&bna);
    BN_init(&bnb);
    BN_init(&bnbase);
    BN_init(&bnrem);
    BN_set_word(&bnbase, 58);

    bn = &bna;
    bndiv = &bnb;

    brlen = (2 * len) + 4;
    binres = (unsigned char*) malloc(brlen);
    memcpy(binres, buf, len);

    SHA256(binres, len, hash1);
    SHA256(hash1, sizeof(hash1), hash2);
    memcpy(&binres[len], hash2, 4);

    BN_bin2bn(binres, len + 4, bn);

    for (zpfx = 0; zpfx < (len + 4) && binres[zpfx] == 0; zpfx++);

    p = brlen;
    while (!BN_is_zero(bn)) {
        BN_div(bndiv, &bnrem, bn, &bnbase, bnctx);
        bntmp = bn;
        bn = bndiv;
        bndiv = bntmp;
        d = BN_get_word(&bnrem);
        binres[--p] = vg_b58_alphabet[d];
    }

    while (zpfx--) {
        binres[--p] = vg_b58_alphabet[0];
    }

    memcpy(result, &binres[p], brlen - p);
    result[brlen - p] = '\0';

    free(binres);
    BN_clear_free(&bna);
    BN_clear_free(&bnb);
    BN_clear_free(&bnbase);
    BN_clear_free(&bnrem);
    BN_CTX_free(bnctx);
}

#define skip_char(c) \
    (((c) == '\r') || ((c) == '\n') || ((c) == ' ') || ((c) == '\t'))

int
vg_b58_decode_check(const char *input, void *buf, size_t len)
{


    int i, l, c;
    unsigned char *xbuf = NULL;
    BIGNUM bn, bnw, bnbase;
    BN_CTX *bnctx;
    unsigned char hash1[32], hash2[32];
    int zpfx;
    int res = 0;

    BN_init(&bn);
    BN_init(&bnw);
    BN_init(&bnbase);
    BN_set_word(&bnbase, 58);
    bnctx = BN_CTX_new();

    /* Build a bignum from the encoded value */
    l = strlen(input);
    for (i = 0; i < l; i++) {
        if (skip_char(input[i]))
            continue;
        c = vg_b58_reverse_map[(int)input[i]];
        if (c < 0)
            goto out;
        BN_clear(&bnw);
        BN_set_word(&bnw, c);
        BN_mul(&bn, &bn, &bnbase, bnctx);
        BN_add(&bn, &bn, &bnw);
    }

    /* Copy the bignum to a byte buffer */
    for (i = 0, zpfx = 0; input[i]; i++) {
        if (skip_char(input[i]))
            continue;
        if (input[i] != vg_b58_alphabet[0])
            break;
        zpfx++;
    }
    c = BN_num_bytes(&bn);
    l = zpfx + c;
    if (l < 5)
        goto out;
    xbuf = (unsigned char *) malloc(l);
    if (!xbuf)
        goto out;
    if (zpfx)
        memset(xbuf, 0, zpfx);
    if (c)
        BN_bn2bin(&bn, xbuf + zpfx);

    /* Check the hash code */
    l -= 4;
    SHA256(xbuf, l, hash1);
    SHA256(hash1, sizeof(hash1), hash2);
    if (memcmp(hash2, xbuf + l, 4))
        goto out;

    /* Buffer verified */
    if (len) {
        if (len > l)
            len = l;
        memcpy(buf, xbuf, len);
    }
    res = l;

out:
    if (xbuf)
        free(xbuf);
    BN_clear_free(&bn);
    BN_clear_free(&bnw);
    BN_clear_free(&bnbase);
    BN_CTX_free(bnctx);
    return res;
}

void
vg_encode_address(const EC_POINT *ppoint, const EC_GROUP *pgroup,
                  int addrtype, char *result)
{
    unsigned char eckey_buf[128], *pend;
    unsigned char binres[21] = {0,};
    unsigned char hash1[32];

    pend = eckey_buf;

    EC_POINT_point2oct(pgroup,
                       ppoint,
                       POINT_CONVERSION_UNCOMPRESSED,
                       eckey_buf,
                       sizeof(eckey_buf),
                       NULL);
    pend = eckey_buf + 0x41;//0x21;//
    binres[0] = addrtype;
    SHA256(eckey_buf, pend - eckey_buf, hash1);
    RIPEMD160(hash1, sizeof(hash1), &binres[1]);

    vg_b58_encode_check(binres, sizeof(binres), result);
}

void
vg_encode_address_compressed(const EC_POINT *ppoint, const EC_GROUP *pgroup,
                             int addrtype, char *result)
{

    unsigned char eckey_buf[128], *pend;
    unsigned char binres[21] = {0,};
    unsigned char hash1[32];

    pend = eckey_buf;

    EC_POINT_point2oct(pgroup,
                       ppoint,
                       POINT_CONVERSION_COMPRESSED,
                       eckey_buf,
                       sizeof(eckey_buf),
                       NULL);
    pend = eckey_buf + 0x21;//0x41;//
    binres[0] = addrtype;
    SHA256(eckey_buf, pend - eckey_buf, hash1);
    RIPEMD160(hash1, sizeof(hash1), &binres[1]);

    vg_b58_encode_check(binres, sizeof(binres), result);
}

void
vg_encode_script_address(const EC_POINT *ppoint, const EC_GROUP *pgroup,
                         int addrtype, char *result)
{
    unsigned char script_buf[69];
    unsigned char *eckey_buf = script_buf + 2;
    unsigned char binres[21] = {0,};
    unsigned char hash1[32];

    script_buf[ 0] = 0x51;  // OP_1
    script_buf[ 1] = 0x41;  // pubkey length
    // gap for pubkey
    script_buf[67] = 0x51;  // OP_1
    script_buf[68] = 0xae;  // OP_CHECKMULTISIG

    EC_POINT_point2oct(pgroup,
                       ppoint,
                       POINT_CONVERSION_UNCOMPRESSED,
                       eckey_buf,
                       65,
                       NULL);
    binres[0] = addrtype;
    SHA256(script_buf, 69, hash1);
    RIPEMD160(hash1, sizeof(hash1), &binres[1]);

    vg_b58_encode_check(binres, sizeof(binres), result);
}

void
vg_encode_privkey(const EC_KEY *pkey, int addrtype, char *result)
{


    unsigned char eckey_buf[128];
    const BIGNUM *bn;
    int nbytes;

    bn = EC_KEY_get0_private_key(pkey);

    eckey_buf[0] = addrtype;
    nbytes = BN_num_bytes(bn);
    assert(nbytes <= 32);
    if (nbytes < 32)
        memset(eckey_buf + 1, 0, 32 - nbytes);
    BN_bn2bin(bn, &eckey_buf[33 - nbytes]);

    vg_b58_encode_check(eckey_buf, 33, result);
}

void
vg_encode_privkey_compressed(const EC_KEY *pkey, int addrtype, char *result)
{
    unsigned char eckey_buf[128];
    const BIGNUM *bn;
    int nbytes;

    bn = EC_KEY_get0_private_key(pkey);

    eckey_buf[0] = addrtype;
    nbytes = BN_num_bytes(bn);
    assert(nbytes <= 32);
    if (nbytes < 32)
        memset(eckey_buf + 1, 0, 32 - nbytes);
    BN_bn2bin(bn, &eckey_buf[33 - nbytes]);
    eckey_buf[nbytes+1] = 1;

    vg_b58_encode_check(eckey_buf, 34, result);
}

int
vg_set_privkey(const BIGNUM *bnpriv, EC_KEY *pkey)
{

    const EC_GROUP *pgroup;
    EC_POINT *ppnt;
    int res;

    pgroup = EC_KEY_get0_group(pkey);
    ppnt = EC_POINT_new(pgroup);

    res = (ppnt &&
           EC_KEY_set_private_key(pkey, bnpriv) &&
           EC_POINT_mul(pgroup, ppnt, bnpriv, NULL, NULL, NULL) &&
           EC_KEY_set_public_key(pkey, ppnt));

    if (ppnt)
        EC_POINT_free(ppnt);

    if (!res)
        return 0;

    assert(EC_KEY_check_key(pkey));
    return 1;
}

int
vg_decode_privkey(const char *b58encoded, EC_KEY *pkey, int *addrtype)
{
    BIGNUM bnpriv;
    unsigned char ecpriv[48];
    int res;

    res = vg_b58_decode_check(b58encoded, ecpriv, sizeof(ecpriv));
    if (res != 33)
        return 0;

    BN_init(&bnpriv);
    BN_bin2bn(ecpriv + 1, res - 1, &bnpriv);
    res = vg_set_privkey(&bnpriv, pkey);
    BN_clear_free(&bnpriv);
    *addrtype = ecpriv[0];
    return 1;
}

#if OPENSSL_VERSION_NUMBER < 0x10000000L
/* The generic PBKDF2 function first appeared in OpenSSL 1.0 */
/* ====================================================================
 * Copyright (c) 1999-2006 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
int
PKCS5_PBKDF2_HMAC(const char *pass, int passlen,
                  const unsigned char *salt, int saltlen, int iter,
                  const EVP_MD *digest,
                  int keylen, unsigned char *out)
{
    unsigned char digtmp[EVP_MAX_MD_SIZE], *p, itmp[4];
    int cplen, j, k, tkeylen, mdlen;
    unsigned long i = 1;
    HMAC_CTX hctx;

    mdlen = EVP_MD_size(digest);
    if (mdlen < 0)
        return 0;

    HMAC_CTX_init(&hctx);
    p = out;
    tkeylen = keylen;
    if(!pass)
        passlen = 0;
    else if(passlen == -1)
        passlen = strlen(pass);
    while(tkeylen)
    {
        if(tkeylen > mdlen)
            cplen = mdlen;
        else
            cplen = tkeylen;
        /* We are unlikely to ever use more than 256 blocks (5120 bits!)
         * but just in case...
         */
        itmp[0] = (unsigned char)((i >> 24) & 0xff);
        itmp[1] = (unsigned char)((i >> 16) & 0xff);
        itmp[2] = (unsigned char)((i >> 8) & 0xff);
        itmp[3] = (unsigned char)(i & 0xff);
        HMAC_Init_ex(&hctx, pass, passlen, digest, NULL);
        HMAC_Update(&hctx, salt, saltlen);
        HMAC_Update(&hctx, itmp, 4);
        HMAC_Final(&hctx, digtmp, NULL);
        memcpy(p, digtmp, cplen);
        for(j = 1; j < iter; j++)
        {
            HMAC(digest, pass, passlen,
                 digtmp, mdlen, digtmp, NULL);
            for(k = 0; k < cplen; k++)
                p[k] ^= digtmp[k];
        }
        tkeylen-= cplen;
        i++;
        p+= cplen;
    }
    HMAC_CTX_cleanup(&hctx);
    return 1;
}
#endif  /* OPENSSL_VERSION_NUMBER < 0x10000000L */


typedef struct {
    int mode;
    int iterations;
    const EVP_MD *(*pbkdf_hash_getter)(void);
    const EVP_CIPHER *(*cipher_getter)(void);
} vg_protkey_parameters_t;

static const vg_protkey_parameters_t protkey_parameters[] = {
    { 0, 4096,  EVP_sha256, EVP_aes_256_cbc },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 0, 0, NULL, NULL },
    { 1, 4096,  EVP_sha256, EVP_aes_256_cbc },
};
