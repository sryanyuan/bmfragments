#include "QuickCrypto.h"
#include "bytebuffer.h"

#include <memory.h>

#define XXTEA_DELTA 0x9e3779b9
#define MX ((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (k[(p&3)^e] ^ z));

static void btea(uint32_t *v, int n, uint32_t const k[4])
{
    uint32_t y, z, sum;
    unsigned p, rounds, e;
    if (n > 1)
    { /* Coding Part */
        rounds = 6 + 52 / n;
        sum = 0;
        z = v[n - 1];
        do
        {
            sum += XXTEA_DELTA;
            e = (sum >> 2) & 3;
            for (p = 0; p < n - 1; p++)
                y = v[p + 1], z = v[p] += MX;
            y = v[0];
            z = v[n - 1] += MX;
        } while (--rounds);
    }
    else if (n < -1)
    { /* Decoding Part */
        n = -n;
        rounds = 6 + 52 / n;
        sum = rounds * XXTEA_DELTA;
        y = v[0];
        do
        {
            e = (sum >> 2) & 3;
            for (p = n - 1; p > 0; p--)
                z = v[p - 1], y = v[p] -= MX;
            z = v[n - 1];
            y = v[0] -= MX;
        } while ((sum -= XXTEA_DELTA) != 0);
    }
}

static const uint16_t crc16tab[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108,
    0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,
    0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b,
    0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee,
    0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
    0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d,
    0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5,
    0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
    0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4,
    0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
    0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13,
    0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
    0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e,
    0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1,
    0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,
    0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0,
    0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657,
    0x7676, 0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
    0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882,
    0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e,
    0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,
    0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d,
    0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
    0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

uint16_t crc16(const char *buf, int len) {
    int counter;
    uint16_t crc = 0;
    for (counter = 0; counter < len; counter++)
        crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ *buf++) & 0x00FF];
    return crc;
}

static uint32_t blank_fill = 0;

QuickCrypto::QuickCrypto() {
    memset(keys_, 0, sizeof(keys_));
}

QuickCrypto::~QuickCrypto() {

}

int QuickCrypto::Encrypt(ByteBuffer &buffer, uint32_t keys[]) {
    unsigned int cur_size = buffer.GetLength();
    if (cur_size <= 8) {
        // No payload
        return -1;
    }
    const unsigned char *data = buffer.GetBuffer();
    data += sizeof(uint32_t) * 2;
    cur_size -= sizeof(uint32_t) * 2;

    // Fill with blank bytes
    unsigned int fill_size = cur_size;
    unsigned int diff = fill_size % sizeof(int32_t);
    if (diff != 0) {
        fill_size += sizeof(int32_t) - diff;
        if (0 == buffer.Write(&blank_fill, fill_size - cur_size)) {
            return -1;
        }
    }
    // Append the original length information
    if (0 == buffer.Write(&cur_size, sizeof(unsigned int))) {
        return -1;
    }
    fill_size += sizeof(unsigned int);

    btea((uint32_t*)data, int(fill_size)/sizeof(uint32_t), keys);

    return 0;
}

int QuickCrypto::Decrypt(ByteBuffer &buffer, uint32_t keys[]) {
    unsigned int cur_size = buffer.GetLength();
    if (cur_size <= 4 + 4 + 4) {
        // No payload
        return -1;
    }
    if (cur_size % sizeof(uint32_t) != 0) {
        return -1;
    }

    const unsigned char *data = buffer.GetBuffer();
    data += sizeof(uint32_t) * 2;
    cur_size -= sizeof(uint32_t) * 2;

    btea((uint32_t*)data, -1 * int(cur_size)/sizeof(uint32_t), keys);

    // Read the original length
    unsigned int original_data_len = 0;
    memcpy(&original_data_len,
           buffer.GetBuffer() + buffer.GetLength() - 4,
           sizeof(unsigned int));
    if (original_data_len <= 0 || original_data_len > cur_size - 4) {
        return -1;
    }
    // Erase the original length
    buffer.Erase(sizeof(unsigned int));
    cur_size -= sizeof(unsigned int);

    // Erase the filled data
    if (original_data_len != cur_size) {
        buffer.Erase(cur_size - original_data_len);
    }
    
    return 0;
}

void QuickCrypto::Init(uint16_t static_key)
{
    uint16_t prev = static_key;
    int off = 0;
    while (off < sizeof(keys_) - 4)
    {
        uint16_t crc16_value = crc16((const char *)&prev, sizeof(prev));
        *(uint16_t *)((char *)(&keys_[0]) + off) = crc16_value;
        off += sizeof(uint16_t) / sizeof(uint8_t);
        prev = crc16_value;
    }
    keys_[3] = 0;
}

void QuickCrypto::KeyNext() {
    if (keys_[3] == UINT32_MAX) {
        keys_[3] = 0;
    }
    keys_[3]++;
}

int QuickCrypto::Encrypt(ByteBuffer &buffer) {
    int res = Encrypt(buffer, keys_);
    KeyNext();
    return res;
}

int QuickCrypto::Decrypt(ByteBuffer &buffer) {
    int res = Decrypt(buffer, keys_);
    KeyNext();
    return res;
}

int QuickCrypto::SetChecksum(ByteBuffer &buffer) {
    char *data = (char *)buffer.GetBuffer();
    unsigned int payload_len = buffer.GetLength();
    if (payload_len <= 8) {
        return -1;
    }
    payload_len -= 8;

    uint16_t val = crc16(data + 8, payload_len);
    uint32_t dwval = *(uint32_t*)(data + 4);
    dwval |= (val << 16);
    *(uint32_t*)(data + 4) = dwval;
    return 0;
}

uint16_t QuickCrypto::GetChecksum(ByteBuffer &buffer) {
    char *data = (char *)buffer.GetBuffer();
    unsigned int payload_len = buffer.GetLength();
    if (payload_len <= 8) {
        return 0;
    }

    uint32_t dwval = *(uint32_t*)(data + 4);
    return dwval >> 16;
}

bool QuickCrypto::VerifyChecksum(ByteBuffer &buffer) {
    uint16_t checksum = GetChecksum(buffer);
    if (0 == checksum) {
        return false;
    }

    uint16_t cur_checksum = crc16((const char*)buffer.GetBuffer() + 8, buffer.GetLength() - 8);
    return cur_checksum == checksum;
}
