/**
 * @file /cryptron/secure.c
 *
 * @brief Functions for handling the secure data type.
 *
 * $Author: Ladar Levison $
 * $Website: http://lavabit.com $
 * $Date: 2010/08/05 11:43:50 $
 * $Revision: c363dfa193830feb5d014a7c6f0abf2d1365f668 $
 *
 * I hereby place the attached code in the public domain.
 * As such it comes without any warranty regarding its merchantability or
 * fitness for a particular purpose. Please use it at your own risk.
 *
 */

#include "ecies.h"

#define HEADSIZE (sizeof(secure_head_t))

size_t secure_key_length(const secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *) cryptex;
    return(head->length.key);
}

size_t secure_mac_length(const secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *) cryptex;
    return(head->length.mac);
}

size_t secure_body_length(const secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *) cryptex;
    return(head->length.body);
}

size_t secure_data_sum_length(const secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *) cryptex;
    return(head->length.key + head->length.mac + head->length.body);
}

size_t secure_total_length(const secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *) cryptex;
    return(HEADSIZE + (head->length.key + head->length.mac + head->length.body));
}

unsigned char *secure_key_data(const secure_t *cryptex) {
    return((unsigned char *) cryptex + HEADSIZE);
}

unsigned char *secure_mac_data(const secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *) cryptex;
    return((unsigned char *) cryptex + (HEADSIZE + head->length.key + head->length.body));
}

unsigned char *secure_body_data(const secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *) cryptex;
    return((unsigned char *) cryptex + (HEADSIZE + head->length.key));
}

secure_t *secure_alloc(size_t key, size_t mac, size_t body) {
    secure_t *cryptex = (secure_t *) malloc(HEADSIZE + key + mac + body);
    secure_head_t *head = (secure_head_t *) cryptex;
    head->length.key = key;
    head->length.mac = mac;
    head->length.body = body;
    return(cryptex);
}

void secure_free(secure_t *cryptex) {
    free(cryptex);
    return;
}
