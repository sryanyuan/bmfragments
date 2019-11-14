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

    int EncryptXXtea(ByteBuffer &buffer);
    int DecryptXXtea(ByteBuffer &buffer);

    int EncryptTtea(ByteBuffer &buffer);
    int DecryptTtea(ByteBuffer &buffer);

    static int SetChecksum(ByteBuffer &buffer);
    static uint16_t GetChecksum(ByteBuffer &buffer, bool &ok);
    static bool VerifyChecksum(ByteBuffer &buffer);

private:
    int EncryptXXtea(ByteBuffer &buffer, uint32_t keys[]);
    int DecryptXXtea(ByteBuffer &buffer, uint32_t keys[]);
    int EncryptTtea(ByteBuffer &buffer, uint32_t keys[]);
    int DecryptTtea(ByteBuffer &buffer, uint32_t keys[]);
    void KeyNext();

private:
    uint32_t keys_[4];
};

#endif

/*
Erase of ByteBuffer:
void Erase(unsigned int _uSize) {
        if (_uSize > m_uDestPointer) {
            m_uDestPointer = _uSize;
        }
        m_uDestPointer -= _uSize;
        if (m_uCurPointer > m_uDestPointer) {
            m_uDestPointer = m_uCurPointer;
        }
    }
*/
