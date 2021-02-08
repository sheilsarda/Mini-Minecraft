#pragma once

#include "blocktypeworker.h"

BlockTypeWorker::BlockTypeWorker(std::vector<Chunk*>* chunks,
                                 QMutex *mutex,
                                 std::vector<Chunk*> add,
                                 int x,
                                 int z,
                                 std::unordered_map<int64_t, std::pair<glm::vec2, Biomes>> biomeMap) :
    chunks(chunks), mutex(mutex), add(add),
    x(x), z(z), biomeMap(biomeMap) {}

Chunk* BlockTypeWorker::createBlockData(Chunk *cPtr){

    // Added these getters in case they are necessary for noise functions
    int chunkX = (int)cPtr->getWorldSpaceX();
    int chunkZ = (int)cPtr->getWorldSpaceZ();

    NoiseFunction* nf = new NoiseFunction();
    int xFloor = X_BOUND * static_cast<int>(glm::floor((float) x / X_BOUND));
    int zFloor = Z_BOUND * static_cast<int>(glm::floor((float) z / Z_BOUND));
    int64_t key = toKey(xFloor, zFloor);

    std::pair<glm::vec2, Biomes> p = biomeMap[key];


    for(int i = 0; i < X_BOUND; ++i) {
        for(int j = 0; j < Z_BOUND; ++j) {
            int x = chunkX + i;
            int z = chunkZ + j;

            float blockHeight = nf->biomeHeight(glm::vec2(x, z)) + 100;
            blockHeight = (blockHeight > Y_BOUND) ? Y_BOUND : blockHeight;

            Biomes b = p.second;
            glm::vec2 noise = p.first;
            BlockType block = nf->biomeBlock(glm::vec3(x, z, blockHeight), b, noise, biomeMap);

            // only top block should be grass, rest should be dirt
            for(int y = 0; y < blockHeight; ++y) {
                 BlockType toSet = (block == GRASS && y < blockHeight - 1) ? DIRT : block;
                cPtr->setBlockAt(i, y, j, toSet);
            }

        }
    }

    return cPtr;
}

void BlockTypeWorker::run(){
    for (Chunk *c : add) createBlockData(c);

    // Critical section
    mutex->lock();
    for(Chunk *c : add) chunks->push_back(c);
    mutex->unlock();
}
