#ifndef _INC_LIBFILE_
#define _INC_LIBFILE_

#include <stdint.h>
#include <stdio.h>

struct LibHead {
    uint32_t uVersion;
    uint16_t uImgCount;
};

struct LibImgInfo {
    uint32_t uIndex;
    uint32_t uLen;
    uint16_t uWidth;
    uint16_t uHeight;
    uint16_t uOffsetX;
    uint16_t uOffsetY;
    uint16_t uShadowX;
    uint16_t uShadowY;
    uint8_t uShadow;

    bool bHasMask;
    uint16_t uMaskWidth;
    uint16_t uMaskHeight;
    uint16_t uMaskX;
    uint16_t uMaskY;
    uint32_t uMaskLen;
};

class LibFile {
public:
    LibFile();
    ~LibFile();

    public:
    bool Load(const char *filename);
    void Unload();
    const char * GetImgData(int index, uint32_t &len, bool raw = false);
    const LibImgInfo *GetImgInfo(int index);

    protected:
    bool LoadImgInfo(int index);

    private:
    LibHead header_;
    LibImgInfo *img_infos_;
    FILE *pf_;
};

#endif
