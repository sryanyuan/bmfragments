#ifndef _INC_QUICK_CRYPTO_
#define _INC_QUICK_CRYPTO_

#include <stdint.h>

class ByteBuffer;

class QuickCrypto
{
public:
    QuickCrypto();
    ~QuickCrypto();

public:
    void Init(uint16_t static_seed);

    int Encrypt(ByteBuffer &buffer);
    int Decrypt(ByteBuffer &buffer);

    static int SetChecksum(ByteBuffer &buffer);
    static uint16_t GetChecksum(ByteBuffer &buffer);
    static bool VerifyChecksum(ByteBuffer &buffer);

    private:
    int Encrypt(ByteBuffer &buffer, uint32_t keys[]);
    int Decrypt(ByteBuffer &buffer, uint32_t keys[]);
    void KeyNext();

private:
    uint32_t keys_[4];
};

#endif
