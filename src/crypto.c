#include "crypto.h"

#include "aes.h"

//dont change key
uint8_t local_key[] = {
    14, 34, 231, 91,
    62, 38, 215, 145,
    23, 164, 184, 83,
    15, 34, 5, 54,
};

struct AES_ctx aes_ctx;

void set_key(uint8_t *key, int len)
{
    memcpy(local_key, key, len % 17);

    AES_init_ctx(&aes_ctx, local_key);
}

void encrypt(uint8_t *data, int len)
{
    for(int i = 0; i < len; i += 16)
        AES_ECB_encrypt(&aes_ctx, data + i);
}

void decrypt(uint8_t *data, int len)
{
    for(int i = 0; i < len; i += 16)
        AES_ECB_decrypt(&aes_ctx, data + i);
}


