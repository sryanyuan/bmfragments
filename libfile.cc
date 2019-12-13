#include "libfile.h"
#include "streamreader.h"
#include <memory.h>
#include <zlib.h>


// gzDecompress: do the decompressing
int gzDecompress(const char *src, int srcLen, const char *dst, int dstLen, int &decLen) {
	z_stream strm;
	strm.zalloc=NULL;
	strm.zfree=NULL;
	strm.opaque=NULL;
	 
	strm.avail_in = srcLen;
	strm.avail_out = dstLen;
	strm.next_in = (Bytef *)src;
	strm.next_out = (Bytef *)dst;
	 
	int err=-1, ret=-1;
	err = inflateInit2(&strm, MAX_WBITS+16);
    if (err != Z_OK) {
        inflateEnd(&strm);
        return err;
    }

    err = inflate(&strm, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&strm);
        return err;
    }

    decLen = strm.total_out;
    inflateEnd(&strm);
    return Z_OK;
}

LibFile::LibFile() {
    memset(&header_, 0, sizeof(header_));
    img_infos_ = nullptr;
    pf_ = nullptr;
}

LibFile::~LibFile() {
    Unload();
}

bool LibFile::Load(const char *filename) {
    pf_ = fopen(filename, "r");
    if (nullptr == pf_) {
        return false;
    }

    char buf[1024];
    if (1 != fread(buf, 4, 1, pf_)) {
        fclose(pf_);
        pf_ = nullptr;
        return false;
    }

    header_.uVersion = *(uint32_t*)buf;
    if (header_.uVersion != 2) {
        fclose(pf_);
        pf_ = nullptr;
        return false;
    }

    if (1 != fread(buf, 4, 1, pf_)) {
        fclose(pf_);
        pf_ = nullptr;
        return false;
    }

    header_.uImgCount = *(uint32_t*)buf;
    img_infos_ = new LibImgInfo[header_.uImgCount];
    memset(img_infos_, 0, sizeof(LibImgInfo) * header_.uImgCount);

    for (int i = 0; i < int(header_.uImgCount); i++) {
        if (1 != fread(buf, 4, 1, pf_)) {
            fclose(pf_);
            pf_ = nullptr;
            return false;
        }
        img_infos_[i].uIndex = *(uint32_t *)buf;
    }

    return true;
}

void LibFile::Unload() {
    if (nullptr != img_infos_) {
        delete [] img_infos_;
        img_infos_ = nullptr;
        return;
    }
    if (nullptr != pf_) {
        fclose(pf_);
        pf_ = nullptr;
    }
}

const LibImgInfo *LibFile::GetImgInfo(int index) {
    if (index < 0 || index >= header_.uImgCount) {
        return nullptr;
    }
    return &img_infos_[index];
}

bool LibFile::LoadImgInfo(int index) {
    if (index < 0 || index >= header_.uImgCount) {
        return false;
    }

    LibImgInfo *pinfo = &img_infos_[index];

    if (0 == pinfo->uLen) {
        // Read data
        if (0 != fseek(pf_, pinfo->uIndex, SEEK_SET)) {
            return false;
        }

        // Read meta
        char buf[1024];
        if (1 != fread(buf, 17, 1, pf_)) {
            return false;
        }
        StreamReader reader((uint8_t *)buf);
        reader.Read(pinfo->uWidth)
            .Read(pinfo->uHeight)
            .Read(pinfo->uOffsetX)
            .Read(pinfo->uOffsetY)
            .Read(pinfo->uShadowX)
            .Read(pinfo->uShadowY)
            .Read(pinfo->uShadow)
            .Read(pinfo->uLen);

        pinfo->bHasMask = ((pinfo->uShadow >> 7) == 1) ? true : false;
        if (pinfo->bHasMask) {
            if (0 != fseek(pf_, pinfo->uLen, SEEK_CUR)) {
                return false;
            }
            if (1 != fread(buf, 12, 1, pf_)) {
                return false;
            }
            reader.Offset(0);

            reader.Read(pinfo->uMaskWidth).
            Read(pinfo->uMaskHeight).
            Read(pinfo->uMaskX).
            Read(pinfo->uMaskY).
            Read(pinfo->uMaskLen);
        }
    }
    return true;
}

const char * LibFile::GetImgData(int index, uint32_t &len, bool raw) {
    if (!LoadImgInfo(index)) {
        return nullptr;
    }
    LibImgInfo *pinfo = &img_infos_[index];

    int nImgDataOff = pinfo->uIndex + 17;
    if (0 != fseek(pf_, nImgDataOff, SEEK_SET)) {
        return nullptr;
    }
    char *img_data = new char[pinfo->uLen];
    if (1 != fread(img_data, pinfo->uLen, 1, pf_)) {
        delete[] img_data;
        return nullptr;
    }

    // Decompress
    static const int s_pDebufLen = 4 * 1024 * 1024;
    static char * s_pDebuf = new char[s_pDebufLen];

    if (raw) {
        memcpy(s_pDebuf, img_data, pinfo->uLen);
        len = pinfo->uLen;
        delete[] img_data;
        return s_pDebuf;
    }

    int dec_len = 0;
    if (0 != gzDecompress(img_data, pinfo->uLen, s_pDebuf, s_pDebufLen, dec_len)) {
        delete[] img_data;
        return nullptr;
    }

    len = (uint32_t)dec_len;
    return s_pDebuf;
}
