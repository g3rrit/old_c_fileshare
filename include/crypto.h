#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>

#define KEY_SIZE 128

//128 bit key
void encrypt(uint8_t *data, int len, uint8_t *key);

void decrypt(uint8_t *data, int len, uint8_t *key);

#endif
