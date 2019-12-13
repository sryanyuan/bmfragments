#include "bmtilemap.h"
#include "streamreader.h"
#include "libfile.h"
#include <memory.h>
#include <stdio.h>
#include <map>
#include <list>

BMTileMap::BMTileMap() {
    tile_data_ = nullptr;
}

BMTileMap::~BMTileMap() {
    DestroyMap();
}

bool BMTileMap::CreateMap(int row, int col) {
    if (nullptr != tile_data_) {
        DestroyMap();
    }
    tile_data_ = new BMMapTile[row * col];
    memset(tile_data_, 0, sizeof(BMMapTile) * row * col);
    memset(&header_, 0, sizeof(header_));
    header_.bCol = col;
    header_.bRow = row;
    return true;
}

void BMTileMap::DestroyMap() {
    delete[] tile_data_;
    tile_data_ = nullptr;
}

bool BMTileMap::LoadMap(const char *filename) {
    FILE *pf = fopen(filename, "r");
    if (nullptr == pf) {
        return false;
    }

    if (0 != fseek(pf, 0, SEEK_END)) {
        fclose(pf);
        return false;
    }
    size_t file_size = ftell(pf);
    if (0 != fseek(pf, 0, SEEK_SET)) {
        fclose(pf);
        return false;
    }

    // Allocate bytes to read data
    uint8_t *map_data = new uint8_t[file_size];
    memset(map_data, 0, file_size);
    if (1 != fread(map_data, file_size, 1, pf)) {
        fclose(pf);
        delete []map_data;
        return false;
    }
    fclose(pf);

    bool res = false;
    if ((map_data[2] == 0x43) && (map_data[3] == 0x23)) {
        // Custom c# map
        res = loadMap100(map_data);
    } else if (map_data[0] == 0) {
        // Wemade mir3 map
        res = loadMap5(map_data);
    } else if (map_data[0] == 0x0f && map_data[5] == 0x53 && map_data[14] == 0x33) {
        // SD mir3 maps
        res = loadMap6(map_data);
    } else if (map_data[0] == 0x15 && map_data[4] == 0x32 && map_data[6] == 0x41 && map_data[19] == 0x31) {
        // Wemade antihack map (laby maps)
        res = loadMap4(map_data);
    } else if (map_data[0] == 0x10 && map_data[2] == 0x61 && map_data[7] == 0x31 && map_data[14] == 0x31) {
        // Wemades 2010 map format
        res = loadMap1(map_data);
    } else if (map_data[4] == 0x0f || (map_data[4] == 0x03 && map_data[18] == 0x0d && map_data[19] == 0x0a)) {
        // SD 2012 format and one of SD(Wemade) older formats share smae header info
        int w = map_data[0] + (map_data[1] << 8);
        int h = map_data[2] + (map_data[3] << 8);
        if (file_size > (52 + (w * h * 14))) {
            res = loadMap3(map_data);
        } else {
            res = loadMap2(map_data);
        }
    } else if (map_data[0] == 0x0d && map_data[1] == 0x4c && map_data[7] == 0x20 && map_data[11] == 0x6d) {
        res = loadMap7(map_data);
    }

    res = loadMap0(map_data);

    delete []map_data;

    return res;
}

bool BMTileMap::loadMap5(uint8_t *data) {
    uint8_t flag = 0;
    int offset = 20;
    int16_t attribute = *(uint16_t*)(data + offset);
    offset += 2;
    header_.bCol = *(uint16_t*)(data + offset);
    offset += 2;
    header_.bRow = *(uint16_t*)(data + offset);
    offset += 2;
    offset = 28;
    if (!CreateMap(header_.bRow, header_.bCol)) {
        return false;
    }

    for (int x = 0; x < header_.bCol / 2; x++) {
        for (int y = 0; y < header_.bRow / 2; y++) {
            for (int i = 0; i < 4; i++) {
                tile_data_[((y * 2) + (i / 2)) * header_.bCol + (x * 2) + (i % 2)].wBkIndex = *(uint8_t*)(data + offset) != 255 ?
                 *(uint16_t*)(data + offset) + 200 : 0xffff;
                tile_data_[((y * 2) + (i / 2)) * header_.bCol + (x * 2) + (i % 2)].wBkImg = *(uint16_t*)(data + offset + 1) + 1;
            }
            offset += 3;
        }
    }

    offset = 28 + (3 * (header_.bCol / 2) + (header_.bCol % 2)) * (header_.bRow / 2);
    for (int x = 0; x < header_.bCol; x++) {
        for (int y = 0; y < header_.bRow; y++) {
            flag = *(uint8_t*)(data + offset++);
            tile_data_[y * header_.bCol + x].bMidAniFrame = *(uint8_t*)(data + offset) == 255 ? 0 : *(uint8_t*)(data + offset);
            tile_data_[y * header_.bCol + x].bMidAniFrame &= 0x8f;
            offset++;
            tile_data_[y * header_.bCol + x].bMidAniTick = 0;
            tile_data_[y * header_.bCol + x].bFrAniTick = 0;
            tile_data_[y * header_.bCol + x].wFrIndex = *(uint8_t*)(data + offset) != 255 ? *(uint8_t*)(data + offset) + 200 : 0xffff;
            offset++;
            tile_data_[y * header_.bCol + x].wMidIndex = *(uint8_t*)(data + offset) != 255 ? *(uint8_t*)(data + offset) + 200 : 0xffff;
            offset++;
            tile_data_[y * header_.bCol + x].wMidImg = *(uint16_t*)(data + offset) + 1;
            offset += 2;
            tile_data_[y * header_.bCol + x].wFrImg = *(uint16_t*)(data + offset) + 1;
            if (tile_data_[y * header_.bCol + x].wFrImg == 1 && tile_data_[y * header_.bCol + x].wFrIndex == 200) {
                tile_data_[y * header_.bCol + x].wFrIndex = 0xffff;
            }
            offset += 2;
            offset += 3;
            tile_data_[y * header_.bCol + x].bLight = *(uint8_t*)(data + offset) & 0x0f;
            offset += 2;
            if ((flag & 0x01) != 1) {
                tile_data_[y * header_.bCol + x].wBkImg |= 0x20000000;
            }
            if ((flag & 0x02) != 2) {
                tile_data_[y * header_.bCol + x].wFrImg = ((uint16_t)tile_data_[y * header_.bCol + x].wFrImg) | 0x8000;
            }

            if (tile_data_[y * header_.bCol + x].bLight >= 100 && tile_data_[y * header_.bCol + x].bLight <= 119) {
                // Fishing cell
            } else {
                tile_data_[y * header_.bCol + x].bLight *= 2;
            }
        }
    }

    return true;
}

bool BMTileMap::loadMap100(uint8_t *data) {
    int offset = 4;
    if ((data[0] != 1) || (data[1] != 0)) {
        return false;
    }
    header_.bCol = *(uint16_t*)(data + offset);
    offset += 2;
    header_.bRow = *(uint16_t*)(data + offset);
    offset += 2;
    if (!CreateMap(header_.bRow, header_.bCol)) {
        return false;
    }

    for (int x = 0; x < header_.bCol; x++) {
        for (int y = 0; y < header_.bRow; y++) {
            tile_data_[y * header_.bCol + x].wBkIndex = *(uint16_t*)(data + offset);
            offset += 2;
            tile_data_[y * header_.bCol + x].wBkImg = *(uint32_t*)(data + offset);
            offset += 4;
            tile_data_[y * header_.bCol + x].wMidIndex = *(uint16_t*)(data + offset);
            offset += 2;
            tile_data_[y * header_.bCol + x].wMidImg = *(uint16_t*)(data + offset);
            offset += 2;
            tile_data_[y * header_.bCol + x].wFrIndex = *(uint16_t*)(data + offset);
            offset += 2;
            tile_data_[y * header_.bCol + x].wFrImg = *(uint16_t*)(data + offset);
            offset += 2;

            tile_data_[y * header_.bCol + x].bDoorIndex = *(uint8_t*)(data + offset++) & 0x7f;
            tile_data_[y * header_.bCol + x].bDoorOffset = *(uint8_t*)(data + offset++);
            tile_data_[y * header_.bCol + x].bFrAniFrame = *(uint8_t*)(data + offset++);
            tile_data_[y * header_.bCol + x].bFrAniTick = *(uint8_t*)(data + offset++);
            tile_data_[y * header_.bCol + x].bMidAniFrame = *(uint8_t*)(data + offset++);
            tile_data_[y * header_.bCol + x].bMidAniTick = *(uint8_t*)(data + offset++);
            tile_data_[y * header_.bCol + x].wBkAniImg = *(uint16_t*)(data + offset);
            offset += 2;
            tile_data_[y * header_.bCol + x].wBkAniOffset = *(uint16_t*)(data + offset);
            offset += 2;
            tile_data_[y * header_.bCol + x].bBkAniFrames = *(uint8_t*)(data + offset++);
            tile_data_[y * header_.bCol + x].bLight = *(uint8_t*)(data + offset++);
        }
    }

    return true;
}

bool BMTileMap::loadMap6(uint8_t *data) {
    uint8_t flag = 0;
    int offset = 16;
    StreamReader reader(data);
    reader.Offset(offset);
    reader.Read(header_.bCol).Read(header_.bRow);
    if (!CreateMap(header_.bRow, header_.bCol)) {
        return false;
    }
    offset = 40;
    reader.Offset(40);
    uint8_t u8tmp = 0;
    uint16_t u16tmp = 0;

    for (int x = 0; x < header_.bCol; x++) {
        for (int y = 0; y < header_.bRow; y++) {
            reader.Read(flag);
            reader.Read(u8tmp);
            GetMapTile(x, y)->wBkIndex = uint16_t(u8tmp) != 255 ? uint16_t(u8tmp) + 300 : 0xffff;
            reader.Read(u8tmp);
            GetMapTile(x, y)->wMidIndex = uint16_t(u8tmp) != 255 ? uint16_t(u8tmp) + 300 : 0xffff;
            reader.Read(u8tmp);
            GetMapTile(x, y)->wFrIndex = uint16_t(u8tmp) != 255 ? uint16_t(u8tmp) + 300 : 0xffff;
            reader.Read(u16tmp);
            GetMapTile(x, y)->wBkImg = u16tmp + 1;
            reader.Read(u16tmp);
            GetMapTile(x, y)->wMidImg = u16tmp + 1;
            reader.Read(u16tmp);
            GetMapTile(x, y)->wFrImg = u16tmp + 1;

            if (GetMapTile(x, y)->wFrImg == 1 && GetMapTile(x, y)->wFrIndex == 200) {
                GetMapTile(x, y)->wFrIndex = 0xffff;
            }
            reader.Read(GetMapTile(x, y)->bMidAniFrame);
            reader.Read(GetMapTile(x, y)->bFrAniFrame);
            if (GetMapTile(x, y)->bFrAniFrame == 255) {
                GetMapTile(x, y)->bFrAniFrame = 0;
            }

            if (GetMapTile(x, y)->bFrAniFrame > 0x0f) {
                GetMapTile(x, y)->bFrAniFrame &= 0x0f;
            }

            GetMapTile(x, y)->bMidAniTick = 1;
            GetMapTile(x, y)->bFrAniTick = 1;
            reader.Read(GetMapTile(x, y)->bLight);
            GetMapTile(x, y)->bLight &= 0x0f;
            GetMapTile(x, y)->bLight *= 4;
            reader.Skip(7);

            if ((flag & 0x01) != 1) {
                GetMapTile(x, y)->wBkImg |= 0x20000000;
            }
            if ((flag & 0x02) != 2) {
                GetMapTile(x, y)->wFrImg = ((uint16_t)GetMapTile(x, y)->wFrImg) | 0x8000;
            }
        }
    }

    return true;
}

bool BMTileMap::loadMap4(uint8_t *data) {
    int offset = 31;
    uint16_t xor16 = 0;
    StreamReader reader(data);
    reader.Offset(offset);
    reader.Read(header_.bCol).Read(xor16).Read(header_.bRow);
   
    header_.bCol = header_.bCol ^ xor16;
    header_.bRow = header_.bRow ^ xor16;
    if (!CreateMap(header_.bRow, header_.bCol)) {
        return false;
    }

    offset = 64;
    reader.Offset(offset);
    uint8_t u8tmp = 0;
    uint16_t u16tmp = 0;

    for (int x = 0; x < header_.bCol; x++) {
        for (int y = 0; y < header_.bRow; y++) {
            BMMapTile *tile = GetMapTile(x, y);
            tile->wBkIndex = 0;
            tile->wMidIndex = 0;
            reader.Read(u16tmp);
            tile->wBkImg = u16tmp ^ xor16;
            reader.Read(u16tmp);
            tile->wMidImg = u16tmp ^ xor16;
            reader.Read(u16tmp);
            tile->wFrImg = u16tmp ^ xor16;
            reader.Read(tile->bDoorIndex);
            tile->bDoorIndex &= 0x7f;
            reader.Read(tile->bDoorOffset);
            reader.Read(tile->bFrAniFrame);
            reader.Read(tile->bFrAniTick);
            reader.Read(u8tmp);
            tile->wFrIndex = u8tmp + 2;
            reader.Read(tile->bLight);

            if ((tile->wBkImg & 0x8000) != 0) {
                tile->wBkImg = (tile->wBkImg & 0x7fff) | 0x20000000;
            }

            if (tile->bLight >= 100 && tile->bLight <= 119) {
                // Fish cell
            }
        }
    }

    return true;
}

bool BMTileMap::loadMap3(uint8_t *data) {
    int offset = 0;
    StreamReader reader(data);
    reader.Offset(offset);
    reader.Read(header_.bCol).Read(header_.bRow);
    if (!CreateMap(header_.bRow, header_.bCol)) {
        return false;
    }

    offset = 52;
    reader.Offset(52);

    uint16_t u16tmp = 0;
    uint8_t u8tmp = 0;

    for (int x = 0; x < header_.bCol; x++) {
        for (int y = 0; y < header_.bRow; y++) {
            BMMapTile *tile = GetMapTile(x, y);

            reader.Read(u16tmp);
            tile->wBkImg = u16tmp;
            reader.Read(u16tmp);
            tile->wMidImg = u16tmp;
            reader.Read(u16tmp);
            tile->wFrImg = u16tmp;

            reader.Read(tile->bDoorIndex);
            tile->bDoorIndex &= 0x7f;
            reader.Read(tile->bDoorOffset);
            reader.Read(tile->bFrAniFrame);
            reader.Read(tile->bFrAniTick);

            reader.Read(u8tmp);
            tile->wFrIndex = uint16_t(u8tmp) + 120;
            reader.Read(tile->bLight);

            reader.Read(u8tmp);
            tile->wBkIndex = uint16_t(u8tmp) + 100;

            reader.Read(u8tmp);
            tile->wMidIndex = uint16_t(u8tmp) + 110;

            reader.Read(tile->wBkAniImg);
            reader.Skip(5);
            reader.Read(tile->bBkAniFrames);
            reader.Read(tile->wBkAniOffset);
            reader.Skip(12);

            if ((tile->wBkImg & 0x8000) != 0) {
                tile->wBkImg = (tile->wBkImg & 0x7fff) | 0x20000000;
            }
        }
    }

    return true;
}

bool BMTileMap::loadMap2(uint8_t *data) {
    int offset = 0;
    StreamReader reader(data);
    reader.Offset(offset);
    reader.Read(header_.bCol).Read(header_.bRow);
    if (!CreateMap(header_.bRow, header_.bCol)) {
        return false;
    }

    offset = 52;
    reader.Offset(52);

    uint16_t u16tmp = 0;
    uint8_t u8tmp = 0;

    for (int x = 0; x < header_.bCol; x++) {
        for (int y = 0; y < header_.bRow; y++) {
            BMMapTile *tile = GetMapTile(x, y);

            reader.Read(u16tmp);
            tile->wBkImg = u16tmp;
            reader.Read(u16tmp);
            tile->wMidImg = u16tmp;
            reader.Read(u16tmp);
            tile->wFrImg = u16tmp;

            reader.Read(tile->bDoorIndex);
            tile->bDoorIndex &= 0x7f;
            reader.Read(tile->bDoorOffset);
            reader.Read(tile->bFrAniFrame);
            reader.Read(tile->bFrAniTick);

            reader.Read(u8tmp);
            tile->wFrIndex = uint16_t(u8tmp) + 120;
            reader.Read(tile->bLight);

            reader.Read(u8tmp);
            tile->wBkIndex = uint16_t(u8tmp) + 100;

            reader.Read(u8tmp);
            tile->wMidIndex = uint16_t(u8tmp) + 110;

            if ((tile->wBkImg & 0x8000) != 0) {
                tile->wBkImg = (tile->wBkImg & 0x7fff) | 0x20000000;
            }
        }
    }

    return true;
}

bool BMTileMap::loadMap1(uint8_t *data) {
    int offset = 21;
    uint16_t xor16 = 0;
    StreamReader reader(data);
    reader.Offset(offset);
    reader.Read(header_.bCol).Read(xor16).Read(header_.bRow);
   
    header_.bCol = header_.bCol ^ xor16;
    header_.bRow = header_.bRow ^ xor16;
    if (!CreateMap(header_.bRow, header_.bCol)) {
        return false;
    }

    offset = 54;
    reader.Offset(offset);
    uint8_t u8tmp = 0;
    uint16_t u16tmp = 0;
    uint32_t u32tmp = 0;

    for (int x = 0; x < header_.bCol; x++) {
        for (int y = 0; y < header_.bRow; y++) {
            BMMapTile *tile = GetMapTile(x, y);

            tile->wBkIndex = 0;
            reader.Read(tile->wBkImg);
            tile->wBkImg ^= 0xaa38aa38;
            tile->wMidIndex = 1;
            reader.Read(u16tmp);
            tile->wMidImg = u16tmp ^ xor16;
            reader.Read(u16tmp);
            tile->wFrImg = u16tmp ^ xor16;
            reader.Read(tile->bDoorIndex);
            tile->bDoorIndex &= 0x7f;
            reader.Read(tile->bDoorOffset);
            reader.Read(tile->bFrAniFrame);
            reader.Read(tile->bFrAniTick);
            reader.Skip(3);
        }
    }

    return true;
}

bool BMTileMap::loadMap7(uint8_t *data) {
    int offset = 21;
    StreamReader reader(data);
    reader.Offset(offset);
    reader.Read(header_.bCol);
    reader.Skip(2);
    reader.Read(header_.bRow);

    if (!CreateMap(header_.bRow, header_.bCol)) {
        return false;
    }

    offset = 54;
    reader.Offset(offset);
    uint8_t u8tmp = 0;
    uint16_t u16tmp = 0;

    for (int x = 0; x < header_.bCol; x++) {
        for (int y = 0; y < header_.bRow; y++) {
            BMMapTile *tile = GetMapTile(x, y);

            reader.Read(tile->wBkImg);
            tile->wMidIndex = 1;
            reader.Read(u16tmp);
            tile->wMidImg = u16tmp;
            reader.Read(u16tmp);
            tile->wFrImg = u16tmp;
            reader.Read(tile->bDoorIndex);
            tile->bDoorIndex &= 0x7f;
            reader.Read(tile->bDoorOffset);
            reader.Read(tile->bFrAniFrame);
            reader.Read(tile->bFrAniTick);
            reader.Read(u8tmp);
            tile->wFrIndex = u8tmp + 2;
            reader.Skip(2);

            if ((tile->wBkImg & 0x8000) != 0) {
                tile->wBkImg = (tile->wBkImg & 0x7fff) | 0x20000000;
            }
        }
    }

    return true;
}

bool BMTileMap::loadMap0(uint8_t *data) {
    int offset = 0;
    StreamReader reader(data);
    reader.Offset(offset);
    reader.Read(header_.bCol);
    reader.Read(header_.bRow);

    if (!CreateMap(header_.bRow, header_.bCol)) {
        return false;
    }

    offset = 52;
    reader.Offset(offset);
    uint8_t u8tmp = 0;
    uint16_t u16tmp = 0;

    for (int x = 0; x < header_.bCol; x++) {
        for (int y = 0; y < header_.bRow; y++) {
            BMMapTile *tile = GetMapTile(x, y);

            reader.Read(tile->wBkImg);
            tile->wMidIndex = 1;
            reader.Read(u16tmp);
            tile->wMidImg = u16tmp;
            reader.Read(u16tmp);
            tile->wFrImg = u16tmp;
            reader.Read(tile->bDoorIndex);
            tile->bDoorIndex &= 0x7f;
            reader.Read(tile->bDoorOffset);
            reader.Read(tile->bFrAniFrame);
            reader.Read(tile->bFrAniTick);
            reader.Read(u8tmp);
            tile->wFrIndex = u8tmp + 2;
            reader.Read(tile->bLight);

            if ((tile->wBkImg & 0x8000) != 0) {
                tile->wBkImg = (tile->wBkImg & 0x7fff) | 0x20000000;
            }
        }
    }

    return true;
}

struct ImgPair {
    const LibImgInfo *info;
    LibFile *file;

    ImgPair(LibFile *f, const LibImgInfo *i) {
        info = i;
        file = f;
    }
};

bool BMTileMap::MergeOne(const char *mapname, const char *destmapname, int resid, const char *datadir) {
    char buf[1024];
    // Load amp
    BMTileMap map;
    if (!map.LoadMap(mapname)) {
        return false;
    }

    std::list<ImgPair> allImgs;

    std::map<uint64_t, int> objectMap;
    int nResIndex = 0;

    std::map<uint64_t, LibFile*> resFiles;

    // Initialize map libs
    sprintf(buf, "%s\\Map\\WemadeMir2\\Tiles.Lib", datadir);
    resFiles[0] = new LibFile();
    resFiles[0]->Load(buf);
    sprintf(buf, "%s\\Map\\WemadeMir2\\Smtiles.Lib", datadir);
    resFiles[1] = new LibFile();
    resFiles[1]->Load(buf);
    sprintf(buf, "%s\\Map\\WemadeMir2\\Objects.Lib", datadir);
    resFiles[2] = new LibFile();
    resFiles[2]->Load(buf);
    for (int i = 2; i < 24; i++) {
        sprintf(buf, "%s\\Map\\WemadeMir2\\Objects%d.Lib", datadir, i);
        resFiles[i + 1] = new LibFile();
        resFiles[i + 1]->Load(buf);
    }

    sprintf(buf, "%s\\Map\\ShandaMir2\\Tiles.Lib", datadir);
    resFiles[100] = new LibFile();
    resFiles[100]->Load(buf);
    for (int i = 1; i < 10; i++) {
        sprintf(buf, "%s\\Map\\ShandaMir2\\Tiles%d.Lib", datadir, i + 1);
        resFiles[100 + i] = new LibFile();
        resFiles[100 + i]->Load(buf);
    }
    sprintf(buf, "%s\\Map\\ShandaMir2\\Smtiles.Lib", datadir);
    resFiles[110] = new LibFile();
    resFiles[110]->Load(buf);
    for (int i = 1; i < 10; i++) {
        sprintf(buf, "%s\\Map\\ShandaMir2\\Smtiles%d.Lib", datadir, i + 1);
        resFiles[110 + i] = new LibFile();
        resFiles[110 + i]->Load(buf);
    }
    sprintf(buf, "%s\\Map\\ShandaMir2\\Objects.Lib", datadir);
    resFiles[120] = new LibFile();
    resFiles[120]->Load(buf);
    for (int i = 1; i < 31; i++) {
        sprintf(buf, "%s\\Map\\ShandaMir2\\Objects%d.Lib", datadir, i + 1);
        resFiles[120 + i] = new LibFile();
        resFiles[120 + i]->Load(buf);
    }
    sprintf(buf, "%s\\Map\\ShandaMir2\\AniTiles1.Lib", datadir);
    resFiles[190] = new LibFile();
    resFiles[190]->Load(buf);

    const char * mapState[] = {"", "wood\\", "sand\\", "snow\\", "forest\\"};
    for (int i = 0; i < sizeof(mapState) / sizeof(mapState[0]); i++) {
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sTilesc.Lib", datadir, mapState[i]);
        resFiles[200 + (i * 15)] = new LibFile();
        resFiles[200 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sTiles30c.Lib", datadir, mapState[i]);
        resFiles[201 + (i * 15)] = new LibFile();
        resFiles[201 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sTiles5c.Lib", datadir, mapState[i]);
        resFiles[202 + (i * 15)] = new LibFile();
        resFiles[202 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sSmtilesc.Lib", datadir, mapState[i]);
        resFiles[203 + (i * 15)] = new LibFile();
        resFiles[203 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sHousesc.Lib", datadir, mapState[i]);
        resFiles[204 + (i * 15)] = new LibFile();
        resFiles[204 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sCliffsc.Lib", datadir, mapState[i]);
        resFiles[205 + (i * 15)] = new LibFile();
        resFiles[205 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sDungeonsc.Lib", datadir, mapState[i]);
        resFiles[206 + (i * 15)] = new LibFile();
        resFiles[206 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sInnersc.Lib", datadir, mapState[i]);
        resFiles[207 + (i * 15)] = new LibFile();
        resFiles[207 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sFurnituresc.Lib", datadir, mapState[i]);
        resFiles[208 + (i * 15)] = new LibFile();
        resFiles[208 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sWallsc.Lib", datadir, mapState[i]);
        resFiles[209 + (i * 15)] = new LibFile();
        resFiles[209 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%ssmObjectsc.Lib", datadir, mapState[i]);
        resFiles[210 + (i * 15)] = new LibFile();
        resFiles[210 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sAnimationsc.Lib", datadir, mapState[i]);
        resFiles[211 + (i * 15)] = new LibFile();
        resFiles[211 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sObject1c.Lib", datadir, mapState[i]);
        resFiles[212 + (i * 15)] = new LibFile();
        resFiles[212 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\%sObject2c.Lib", datadir, mapState[i]);
        resFiles[213 + (i * 15)] = new LibFile();
        resFiles[213 + (i * 15)]->Load(buf);
    }

    const char * smapState[] = {"", "wood", "sand", "snow", "forest"};
    for (int i = 0; i < sizeof(mapState) / sizeof(mapState[0]); i++) {
        sprintf(buf, "%s\\Map\\WemadeMir3\\Tilesc%s.Lib", datadir, smapState[i]);
        resFiles[300 + (i * 15)] = new LibFile();
        resFiles[300 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Tiles30c%s.Lib", datadir, smapState[i]);
        resFiles[301 + (i * 15)] = new LibFile();
        resFiles[301 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Tiles5c%s.Lib", datadir, smapState[i]);
        resFiles[302 + (i * 15)] = new LibFile();
        resFiles[302 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Smtilesc%s.Lib", datadir, smapState[i]);
        resFiles[303 + (i * 15)] = new LibFile();
        resFiles[303 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Housesc%s.Lib", datadir, smapState[i]);
        resFiles[304 + (i * 15)] = new LibFile();
        resFiles[304 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Cliffsc%s.Lib", datadir, smapState[i]);
        resFiles[305 + (i * 15)] = new LibFile();
        resFiles[305 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Dungeonsc%s.Lib", datadir, smapState[i]);
        resFiles[306 + (i * 15)] = new LibFile();
        resFiles[306 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Innersc%s.Lib", datadir, smapState[i]);
        resFiles[307 + (i * 15)] = new LibFile();
        resFiles[307 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Furnituresc%s.Lib", datadir, smapState[i]);
        resFiles[308 + (i * 15)] = new LibFile();
        resFiles[308 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Wallsc%s.Lib", datadir, smapState[i]);
        resFiles[309 + (i * 15)] = new LibFile();
        resFiles[309 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\smObjectsc%s.Lib", datadir, smapState[i]);
        resFiles[310 + (i * 15)] = new LibFile();
        resFiles[310 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Animationsc%s.Lib", datadir, smapState[i]);
        resFiles[311 + (i * 15)] = new LibFile();
        resFiles[311 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Object1c%s.Lib", datadir, smapState[i]);
        resFiles[312 + (i * 15)] = new LibFile();
        resFiles[312 + (i * 15)]->Load(buf);
        sprintf(buf, "%s\\Map\\WemadeMir3\\Object2c%s.Lib", datadir, smapState[i]);
        resFiles[313 + (i * 15)] = new LibFile();
        resFiles[313 + (i * 15)]->Load(buf);
    }

    // Scan for background
    for (int x = 0; x < map.GetMapHeader()->bCol; x++) {
        for (int y = 0; y < map.GetMapHeader()->bRow; y++) {
            BMMapTile *tile = map.GetMapTile(x, y);
            if ((tile->wBkImg & (~0x20000000)) == 0) {
                continue;
            }

            uint16_t bkIndex = tile->wBkIndex;
            uint32_t value = ((tile->wBkImg & (~0x20000000))) - 1;
            bool blocked = (tile->wBkImg & 0x20000000) != 0;

            auto it = objectMap.find(tile->wBkIndex << 32 | value);
            if (it != objectMap.end()) {
                // Same image exists
                tile->wBkIndex = 0;
                tile->wBkImg = it->second + 1;
                if (blocked) {
                    tile->wBkImg |= 0x20000000;
                }
                continue;
            }

            // Extract resource file
            auto res_it = resFiles.find(bkIndex);
            if (res_it == resFiles.end()) {
                return false;
            }
            const LibImgInfo *imgif = res_it->second->GetImgInfo(value);
            if (nullptr == imgif) {
                return false;
            }
            allImgs.push_back(ImgPair(res_it->second, imgif));

            int newval = nResIndex;
            nResIndex++;

            tile->wBkIndex = newval + 1;
            if (blocked) {
                tile->wBkImg |= 0x20000000;
            }

            objectMap.insert(std::make_pair(tile->wBkIndex << 32 | value, newval));
        }
    }

    // Scan for middle
    for (int x = 0; x < map.GetMapHeader()->bCol; x++) {
        for (int y = 0; y < map.GetMapHeader()->bRow; y++) {
            BMMapTile *tile = map.GetMapTile(x, y);
            if ((tile->wMidImg & (~0x8000)) == 0) {
                continue;
            }

            uint16_t bkIndex = tile->wMidIndex;
            uint32_t value = ((tile->wMidImg & (~0x8000))) - 1;
            bool blocked = (tile->wMidImg & 0x8000) != 0;

            auto it = objectMap.find(tile->wMidIndex << 32 | value);
            if (it != objectMap.end()) {
                // Same image exists
                tile->wMidIndex = 0;
                tile->wMidImg = it->second + 1;
                if (blocked) {
                    tile->wMidImg |= 0x8000;
                }
                continue;
            }

            // Extract resource file
            auto res_it = resFiles.find(bkIndex);
            if (res_it == resFiles.end()) {
                return false;
            }
            const LibImgInfo *imgif = res_it->second->GetImgInfo(value);
            if (nullptr == imgif) {
                return false;
            }
            allImgs.push_back(ImgPair(res_it->second, imgif));

            int newval = nResIndex;
            nResIndex++;

            tile->wMidIndex = newval + 1;
            if (blocked) {
                tile->wMidImg |= 0x8000;
            }

            objectMap.insert(std::make_pair(tile->wMidIndex << 32 | value, newval));
        }
    }
}
