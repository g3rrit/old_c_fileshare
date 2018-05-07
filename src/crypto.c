#include "crypto.h"

void encrypt(uint8_t *data, int len, uint8_t *key)
{
    for(int i = 0; i < len; i++)
        data[i] ^= key[i % KEY_SIZE];

    /*
    for(int i = 0; i < len; i++)
    {
        uint8_t temp = data[i];
        int pos = (i + key[i % KEY_SIZE]) % len;
        data[i] = data[pos];
        data[pos] = temp;
    }
    */
}

void decrypt(uint8_t *data, int len, uint8_t *key)
{
    for(int i = 0; i < len; i++)
        data[i] ^= key[i % KEY_SIZE];

    /*
    for(int i = len - 1; i >= 0; i--)
    {
        uint8_t temp = data[i];
        int pos = (i + key[i % KEY_SIZE]) % len;
        data[i] = data[pos];
        data[pos] = temp;
    }
    */
}


