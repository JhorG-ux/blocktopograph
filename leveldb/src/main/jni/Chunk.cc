//
// Created by barco on 2018/3/23.
//


#include <leveldb/db.h>
#include "Chunk.h"
#include "mapkey.h"
#include "qstr.h"
#include <leveldbjni.h>
#include <stdlib.h>
#include <mapkey.h>
#include "subchunk.h"

#ifdef LOG_CHUNK_LOADSAVE
#define LOGE_LS(x, ...) LOGE(CAT("Chunk: ", x), ##__VA_ARGS__);
#else
#define LOGE_LS(x, ...)
#endif

#ifdef LOG_CHUNK_OPERATION
#define LOGE_OP(x, ...) LOGE(CAT("Chunk: ", x), ##__VA_ARGS__);
#else
#define LOGE_OP(x, ...)
#endif

//Init constants.

leveldb::ReadOptions Chunk::readOptions;

Chunk::Chunk(leveldb::DB *db, mapkey_t key)
    : key(key) {
    std::string str;
    memset(subchunks, 0, sizeof(subchunks));
    for (int i = 0; i < 16; i++) {
        loadSubChunk(db, i);
    }
}

void Chunk::loadSubChunk(leveldb::DB *db, unsigned char which) {
    LDBKEY_SUBCHUNK(this->key, which)
    leveldb::Slice slice(key, 0 == this->key.dimension ? 10 : 14);
    std::string val;
    bool hit = db->Get(readOptions, slice, &val).ok();
    if (hit) {//Found, detect version.
        switch (val[0]) {
            case 0://Aligned subchunk.
            case 2://All
            case 3://The
            case 4://Same
            case 5://Really
            case 6://Wierd
            case 7://Ahhh
                break;
            case 8://Current paletted format
                subchunks[which] = new SubChunk(val, true);
                break;
            case 1: //Previous paletted version
                subchunks[which] = new SubChunk(val, false);
                break;
            default:
                //Unsupported format.
                break;
        }
    }
}

uint16_t Chunk::getBlock(unsigned char x, unsigned char y, unsigned char z) {
    unsigned char sub = y >> 4;
    if (subchunks[sub] == nullptr)return 0;
    return subchunks[sub]->getBlock(x, static_cast<unsigned char>(y & 0xf), z);
}

Chunk::~Chunk() {
    for (int i = 0; i < 16; i++) {
        delete subchunks[i];
    }
}
