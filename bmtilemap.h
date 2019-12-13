#ifndef _INC_TILEMAP_
#define _INC_TILEMAP_

#include <stdint.h>

struct BMMapInfo {
    int nRow;
    int nCol;
    int nWidth;
    int nHeight;
};

struct BMMapHeader {
    // 48*32 Col Row
    uint16_t bCol;
    uint16_t bRow;
    char szTitle[16];
    uint8_t UpdateTime[8];
    uint8_t Reserve[24];
};

struct BMMapTile {
    // 96*64 image -> Index of Tiles, the highest bit is movable
    uint32_t wBkImg;
    uint16_t wBkIndex;
    // 48*32 small image -> Index of SmTiles
    uint32_t wMidImg;
    uint16_t wMidIndex;
    // Groud object, -> Index of Objects, the highest bit is movable
    uint32_t wFrImg;
    uint16_t wFrIndex;

    // Has a door or not, the highest bit represents has or not, the other are
    // door index
    uint8_t bDoorIndex;
    // Door open or close
    uint8_t bDoorOffset;

    // Animation
    uint8_t bFrAniFrame;
    // Animation tick
    uint8_t bFrAniTick;

    uint8_t bMidAniFrame;
    uint8_t bMidAniTick;

    uint16_t wBkAniImg;
    uint16_t wBkAniOffset;
    uint8_t bBkAniFrames;

    uint8_t bLight;
    uint8_t bUnknown;

    // Object index
    uint8_t bArea;
};

class BMTileMap {
  public:
    BMTileMap();
    ~BMTileMap();

  public:
    bool CreateMap(int row, int col);
    void DestroyMap();

    bool LoadMap(const char *filename);
    bool GetMapSnapShot(const char *filename, uint32_t **out, BMMapInfo *info);

    bool IsDataInside() { return nullptr != tile_data_; }

    int GetResIndex() { return *(int *)&header_.Reserve[0]; }
    void SetResIndex(int index) { *(int *)&header_.Reserve[0] = index; }

    const BMMapTile *GetTileData() { return tile_data_; }
    const BMMapHeader *GetMapHeader() { return &header_; }

    const BMMapTile *GetMapTile(int x, int y) const {
        if (x < 0 || x >= header_.bCol) {
            return nullptr;
        }
        if (y < 0 || y >= header_.bRow) {
            return nullptr;
        }
        return &tile_data_[y * header_.bCol + x];
    }
    BMMapTile *GetMapTile(int x, int y) {
        if (x < 0 || x >= header_.bCol) {
            return nullptr;
        }
        if (y < 0 || y >= header_.bRow) {
            return nullptr;
        }
        return &tile_data_[y * header_.bCol + x];
    }

public:
	static bool MergeOne(const char *mapname, const char *destmapname, int resid, const char *datadir);

  private:
    bool loadMap100(uint8_t *data);
    bool loadMap5(uint8_t *data);
    bool loadMap6(uint8_t *data);
    bool loadMap4(uint8_t *data);
    bool loadMap1(uint8_t *data);
    bool loadMap3(uint8_t *data);
    bool loadMap2(uint8_t *data);
    bool loadMap7(uint8_t *data);
    bool loadMap0(uint8_t *data);

  private:
    BMMapTile *tile_data_;
    BMMapHeader header_;
};

#endif
