#ifndef BLOCKTYPEWORKER_H
#define BLOCKTYPEWORKER_H

#pragma once

#include <QRunnable>
#include <QMutex>
#include "chunk.h"
#include "noisefunctions.h"
#include "river.h"

class BlockTypeWorker : public QRunnable
{

private:
    std::vector<Chunk*> *chunks;
    QMutex *mutex;
    std::vector<Chunk*> add;
    int x;
    int z;
    int seed;
    std::unordered_map<int64_t, std::pair<glm::vec2, Biomes>> biomeMap;

public:
    BlockTypeWorker(std::vector<Chunk*> *chunks,
                    QMutex *mutex,
                    std::vector<Chunk*> add,
                    int x,
                    int z,
                    std::unordered_map<int64_t, std::pair<glm::vec2, Biomes>> biomeMap);
    void run() override;
    Chunk* createBlockData(Chunk *cPtr);

};

#endif // BLOCKTYPEWORKER_H
