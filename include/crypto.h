#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>

#define KEY_SIZE 128

//if you use a different key set it here
void set_key(uint8_t *key, int len);

//128 bit key
void encrypt(uint8_t *data, int len);

void decrypt(uint8_t *data, int len);

#endif
