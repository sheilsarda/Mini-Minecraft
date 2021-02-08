#include "terrain.h"
#include "cube.h"
#include "chunk.h"
#include "noisefunctions.h"
#include <stdexcept>
#include <iostream>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context)
{}

Terrain::~Terrain() {

}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    /*
     * Map x and z to their nearest Chunk corner
     * By flooring x and z, then multiplying by 16,
     * we clamp (x, z) to its nearest Chunk-space corner,
     * then scale back to a world-space location.
     * Note that floor() lets us handle negative numbers
     * correctly, as floor(-1 / 16.f) gives us -1, as
     * opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
     */
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {

    if(hasChunkAt(x, z))
        return nullptr;

    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();


    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
    return cPtr;
}

/**
 * Draws each Chunk with the given ShaderProgram, remembering to set the
 * model matrix to the proper X and Z translation!
 */
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {

            if(hasChunkAt(x, z)) {

                const uPtr<Chunk> &chunk = getChunkAt(x, z);

                if(chunk->elemCount() < 0) chunk->create();

                shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, 0, z)));
                shaderProgram->drawChunk(*chunk);
                shaderProgram->drawTransChunk(*chunk);
            }
        }
    }
}



void Terrain::CreateTestScene()
{
    /*
     * Create the Chunks that will
     * store the blocks for our
     * initial world space
     */

     int xmin = 0;
     int xmax = 64;
     int zmin = 0;
     int zmax = 64;
    for(int x = xmin; x < xmax; x += X_BOUND) {
        for(int z = zmin; z < zmax; z += Z_BOUND) {
            instantiateChunkAt(x, z);
        }
    }
    /*
     * Tell our existing terrain set that
     * the "generated terrain zone" at (0,0)
     * now exists.
     */
    m_generatedTerrain.insert(toKey(0, 0));

    for(int x = xmin; x < xmax; x++) {
        for(int z = zmin; z < zmax; z++) {
            setBlockAt(x, 128, z, STONE);
        }
    }

    NoiseFunction *nf = new NoiseFunction();

    for(int x = xmin; x < xmax; x++) {
        for(int z = zmin; z < zmax; z++) {
            float g = nf->grasslandEffect(glm::vec2(abs(x)/(float)xmax, abs(z)/(float)zmax)) * 32 + 110;
            float m = nf->fractalPerlin(glm::vec2(abs(x)/(float)xmax, abs(z)/(float)zmax)) * 70 + 110;
            float t = abs(nf->perlinNoise(glm::vec2(abs(x)/(float)xmax, abs(z)/(float)zmax)));
            float s = glm::smoothstep(0.25f, 0.75f, 2 * t);
            float l = (1-s)*g+s*m;
            for(int y = 0; y < 128; ++y) {
                setBlockAt(x, y, z, STONE);
            }
            for(int y = 128; y < 256; ++y) {
                if (s > 0.9) {
                    if (y < m) {
                        setBlockAt(x, y, z, STONE);
                    } else if (y == ceil(m) && ceil(m) > 172) {
                        setBlockAt(x, y, z, SNOW);
                    }
                } else if (s == 0) {
                    if (y < g) {
                        setBlockAt(x, y, z, DIRT);
                    } else if (y == ceil(g) && y > 128) {
                        setBlockAt(x, y, z, GRASS);
                    } else if (ceil(g) <= 128) {
                        setBlockAt(x, 128, z, WATER);
                    }
                } else if (ceil(l) > 150) {
                    int r = rand() % 5;
                    if (y < l) {
                        if (r % 5 != 0 || y == ceil(l) || y < 150) {
                            setBlockAt(x, y, z, STONE);
                        } else {
                            setBlockAt(x, y, z, DIRT);
                        }
                    }
                } else {
                    if (y < l) {
                        setBlockAt(x, y, z, DIRT);
                    } else if (y == ceil(l) && y > 128) {
                        setBlockAt(x, y, z, GRASS);
                    } else if (ceil(l) <= 128) {
                        setBlockAt(x, 128, z, WATER);
                    }
                }
            }
        }
    }
    drawRiver(xmin, xmax, zmin, zmax);
    for(int x = xmin; x < xmax; x++) {
        for(int z = zmin; z < zmax; z++) {
            if (getBlockAt(x, 128, z) == WATER) {
                int dx = 0;
                int hxp = 128;
                while (x+dx < xmax && dx != 6) dx++;
                while (getBlockAt(x+dx-1, hxp, z) != GRASS && hxp != 256) hxp++;
                for (int i = 1; i < dx-1; i++) {
                    int h = 128;
                    while (getBlockAt(x+i, h, z) != GRASS && h != 256) h++;
                    if (hxp > 135) {
                        if (h != 256 && h > 128+2*i) {
                            setBlockAt(x+i, 128+2*i, z, GRASS);
                            for (int j = 129+2*i; j < 256; j++) {
                                setBlockAt(x+i, j, z, EMPTY);
                            }
                        }
                    } else {
                        if (h != 256 && h > 128+i) {
                            setBlockAt(x+i, 128+i, z, GRASS);
                            for (int j = 129+i; j < 256; j++) {
                                setBlockAt(x+i, j, z, EMPTY);
                            }
                        }
                    }
                }
                dx = 0;
                hxp = 128;
                while (x-dx > xmin && dx != 6) dx++;
                while (getBlockAt(x-dx+1, hxp, z) != GRASS && hxp != 256) hxp++;
                for (int i = 1; i < dx-1; i++) {
                    int h = 128;
                    while (getBlockAt(x-i, h, z) != GRASS && h != 256) h++;
                    if (hxp > 135) {
                        if (h != 256 && h > 128+2*i) {
                            setBlockAt(x-i, 128+2*i, z, GRASS);
                            for (int j = 129+2*i; j < 256; j++) {
                                setBlockAt(x-i, j, z, EMPTY);
                            }
                        }
                    } else {
                        if (h != 256 && h > 128+i) {
                            setBlockAt(x-i, 128+i, z, GRASS);
                            for (int j = 129+i; j < 256; j++) {
                                setBlockAt(x-i, j, z, EMPTY);
                            }
                        }
                    }
                }
                int dz = 0;
                int hzp = 128;
                while (z+dz < zmax && dz != 6) dz++;
                while (getBlockAt(x, hzp, z+dz-1) != GRASS && hzp != 256) hzp++;
                for (int i = 1; i < dz-1; i++) {
                    int h = 128;
                    while (getBlockAt(x, h, z+i) != GRASS && h != 256) h++;
                    if (hzp > 135) {
                        if (h != 256 && h > 128+2*i) {
                            setBlockAt(x, 128+2*i, z+i, GRASS);
                            for (int j = 129+2*i; j < 256; j++) {
                                setBlockAt(x, j, z+i, EMPTY);
                            }
                        }
                    } else {
                        if (h != 256 && h > 128+i) {
                            setBlockAt(x, 128+i, z+i, GRASS);
                            for (int j = 129+i; j < 256; j++) {
                                setBlockAt(x, j, z+i, EMPTY);
                            }
                        }
                    }
                }
                dz = 0;
                hzp = 128;
                while (z-dz > zmin && dz != 6) dz++;
                while (getBlockAt(x, hzp, z-dz+1) != GRASS && hzp != 256) hzp++;
                for (int i = 1; i < dz-1; i++) {
                    int h = 128;
                    while (getBlockAt(x, h, z-i) != GRASS && h != 256) h++;
                    if (hzp > 135) {
                        if (h != 256 && h > 128+2*i) {
                            setBlockAt(x, 128+2*i, z-i, GRASS);
                            for (int j = 129+2*i; j < 256; j++) {
                                setBlockAt(x, j, z-i, EMPTY);
                            }
                        }
                    } else {
                        if (h != 256 && h > 128+i) {
                            setBlockAt(x, 128+i, z-i, GRASS);
                            for (int j = 129+i; j < 256; j++) {
                                setBlockAt(x, j, z-i, EMPTY);
                            }
                        }
                    }
                }
            }
        }
    }
    int ymin = 154;
    int ymax = 166;
    for(int x = 8; x < 28; x++) {
        for(int z = 18; z < zmax; z++) {
            for (int y = ymin; y < ymax; y++) {
                float p = abs(nf->perlinNoise3D(glm::vec3(abs(x)/(float)xmax,
                y/float(ymax-ymin), abs(z)/(float)zmax)));
                if (p < 0.1) {
                    setBlockAt(x, y, z, EMPTY);
                }
            }
        }
    }
    for(int x = 10; x < 28; x++) {
        for(int z = 20; z < 30; z++) {
            int r1 = rand() % 8;
            int r2 = rand() % 3;
            if (r2 == 0) {
                for (int k = 0; k < r1; k++) {
                    setBlockAt(x, 166-k, z, OREA);
                }
            }

        }
    }
    for(int x = 8; x < 28; x++) {
        for (int y = 156; y < ymax; y++) {
            int r1 = rand() % 4;
            int r2 = rand() % 4;
            if (r1 == 0) {
                setBlockAt(x, y, 17, EMPTY);
                if (r2 == 0) {
                    setBlockAt(x, y, 16, OREB);
                } else if (r2 == 1) {
                    setBlockAt(x, y, 16, OREC);
                } else {
                    setBlockAt(x, y, 19, ORED);
                }
            } else if (r1 == 1) {
                setBlockAt(x, y, 18, STONE);
            } else if (r1 == 2) {
                setBlockAt(x, y, 18, STONE);
                if (r2 == 0) {
                    setBlockAt(x, y, 19, OREB);
                } else if (r2 == 1) {
                    setBlockAt(x, y, 19, OREC);
                } else {
                    setBlockAt(x, y, 19, ORED);
                }
            }
        }
    }
    for(int z = 18; z < 28; z++) {
        for (int y = ymin; y < 164; y++) {
            int r1 = rand() % 3;
            int r2 = rand() % 3;
            int r3 = rand() % 3;
            if (r1 == 0) {
                if (r2 == 0) {
                    setBlockAt(7, y, z, OREB);
                } else if (r2 == 1) {
                    setBlockAt(7, y, z, OREC);
                } else {
                    setBlockAt(7, y, z, ORED);
                }
            } else if (r1 == 1) {
                setBlockAt(7, y, z, EMPTY);
            }
            if (r3 == 1) {
                if (r2 == 0) {
                    setBlockAt(28, y, z, OREB);
                } else if (r2 == 1) {
                    setBlockAt(28, y, z, OREC);
                } else {
                    setBlockAt(28, y, z, ORED);
                }
            } else if (r3 == 2) {
                setBlockAt(28, y, z, EMPTY);
                if (r2 == 0) {
                    setBlockAt(29, y, z, OREB);
                } else if (r2 == 1) {
                    setBlockAt(29, y, z, OREC);
                } else {
                    setBlockAt(29, y, z, ORED);
                }
            } else {
                setBlockAt(28, y, z, EMPTY);
            }
        }
    }
    for(int x = 8; x < 28; x++) {
        for(int z = 18; z < 28; z++) {
            if (getBlockAt(x, 154, z) != STONE) {
                setBlockAt(x, 154, z, LAVA);
            }
        }
    }
    for(int x = 8; x < 28; x++) {
        for(int z = 28; z < 36; z++) {
            int r = rand() % 3;
            if (r == 0) {
                for (int k = 0; k < 4; k++) {
                    setBlockAt(x, 156+k, z, EMPTY);
                }
            }
        }
    }
}

void Terrain::terrainUpdate(const glm::vec3 &playerPos){

    int x = static_cast<int>(glm::floor(playerPos.x));
    int z = static_cast<int>(glm::floor(playerPos.z));

    for(int i = -2; i < 3; ++i){
        for(int j = -2; j < 3; ++j){

            int xChunk = (static_cast<int>(glm::floor(x / 64.f)) + i) * 64;
            int zChunk = (static_cast<int>(glm::floor(z / 64.f)) + j) * 64;

            int64_t key = toKey(xChunk, zChunk);
            bool hasProcessedChunk =
                    (m_generatedTerrain.find(key) != m_generatedTerrain.end() ||
                     m_generatingTerrain.find(key) != m_generatingTerrain.end());

            if(!hasProcessedChunk){

                std::vector<Chunk *> add;

                // Link neighbors for all the chunks
                for(int a = 0; a < 4; ++a){
                    for(int b = 0; b < 4; ++b){
                        uPtr<Chunk> c = mkU<Chunk>(mp_context,
                                                   xChunk + X_BOUND*a,
                                                   zChunk + Z_BOUND*b);
                        Chunk *cPtr = c.get();
                        add.push_back(cPtr);
                        m_chunks[toKey(xChunk + a*X_BOUND,
                                       zChunk + b*Z_BOUND)] = move(c);

                        if(hasChunkAt(xChunk + a*X_BOUND,
                                      zChunk + (b+1)*Z_BOUND)){

                            uPtr<Chunk> &zpos = m_chunks[toKey(xChunk + a*X_BOUND,
                                                    zChunk + (b+1)*Z_BOUND)];
                            cPtr->linkNeighbor(zpos, Direction::ZPOS);
                        }

                        if(hasChunkAt(xChunk + a*X_BOUND,
                                      zChunk + (b-1)*Z_BOUND)){

                            uPtr<Chunk> &zneg = m_chunks[toKey(xChunk + a*X_BOUND,
                                                    zChunk + (b-1)*Z_BOUND)];
                            cPtr->linkNeighbor(zneg, Direction::ZNEG);
                        }

                        if(hasChunkAt(xChunk + (a+1)*X_BOUND,
                                      zChunk + b*Z_BOUND)){

                            uPtr<Chunk> &xpos = m_chunks[toKey(xChunk + (a+1)*X_BOUND,
                                                    zChunk + b*Z_BOUND)];
                            cPtr->linkNeighbor(xpos, Direction::XPOS);
                        }

                        if(hasChunkAt(xChunk + (a-1)*X_BOUND,
                                      zChunk + b*Z_BOUND)){

                            uPtr<Chunk> &xneg = m_chunks[toKey(xChunk + (a-1)*X_BOUND,
                                                    zChunk + b*Z_BOUND)];
                            cPtr->linkNeighbor(xneg, Direction::XNEG);
                        }
                    }
                }
                m_generatingTerrain.insert(toKey(xChunk, zChunk));

                // call thread to generate terrain
                getBiome(xChunk, zChunk);
                BlockTypeWorker *thread = new BlockTypeWorker(&chunks,
                                                              &chunkMutex,
                                                              add,
                                                              xChunk,
                                                              zChunk,
                                                              m_biomeMap);
                QThreadPool::globalInstance()->start(thread);
            }
        }
    }

    /*
     * Critical section 1
     * ------------------
     * Moving data from chunks to chunkData
     */
    chunkMutex.lock();
    while(!chunks.empty()){

        Chunk *cPtr = chunks.front();
        VBOWorker *vboWriter = new VBOWorker(&vboMutex,
                                             &chunkData,
                                             cPtr);
        QThreadPool::globalInstance()->start(vboWriter);
        chunks.erase(chunks.begin());
    }
    chunkMutex.unlock();

    /*
     * Critical section 2
     * ------------------
     * Writing VBO data
     */
    vboMutex.lock();
    while(!chunkData.empty()){
        uPtr<VBOData> &data = chunkData.front();

        data->cPtr->createCubeVBO(data->posNorCol, data->ix, data->uv,
                                  data->t_posNorCol, data->t_ix, data->t_uv);


        data->cPtr->setIndexCount((data->ix).size());

        /*
         * Check if m_generatedTerrain needs to be
         * updated based on m_generatedTerrainBuffer
         */

        int bufX = static_cast<int>(glm::floor(data->cPtr->getWorldSpaceX() / 16.f)) * 16;
        int bufZ = static_cast<int>(glm::floor(data->cPtr->getWorldSpaceZ() / 16.f)) * 16;//64.f)) * 64;

        int64_t bufferKey = toKey(bufX, bufZ);

        int blockCount = m_generatedTerrainBuffer[bufferKey];
        blockCount++;
        m_generatedTerrainBuffer[bufferKey] = blockCount;

        if(blockCount >= X_BOUND){
            m_generatingTerrain.erase(bufferKey);
            m_generatedTerrain.insert(bufferKey);
        }

        chunkData.erase(chunkData.begin());
    }
    vboMutex.unlock();

}


void Terrain::drawRiver(int xmin, int xmax, int zmin, int zmax) {
    River* river = new River();
    river->create();
    glm::vec3 currInfo = river->info.pop();
    while(river->info.size() > 0) {
        glm::vec3 nextInfo = river->info.pop();
        if(currInfo.z == nextInfo.z + 1) {
            int x1 = currInfo.x;
            int x2 = nextInfo.x;
            int z1 = currInfo.y;
            int z2 = nextInfo.y;
            float dist = sqrt((x1-x2)*(x1-x2)+(z1-z2)*(z1-z2));
            float dx = (x2-x1)/dist;
            float dz = (z2-z1)/dist;
            int w = 4-currInfo.z/2;
            for(int i = 1; i <= dist; i++) {
                if(x1+i*dx < xmax && x1+i*dx > xmin && z1+i*dz < zmax && z1+i*dz > zmin) {
                    for(int j = 0; j <= w; j++) {
                        if(x1+i*dx+j < xmax && x1+i*dx+j > xmin && z1+i*dz+j < zmax
                           && z1+i*dz+j > zmin) {
                            setBlockAt(x1+i*dx+j, 128, z1+i*dz, WATER);
                            setBlockAt(x1+i*dx, 128, z1+i*dz+j, WATER);
                            setBlockAt(x1+i*dx+j, 128, z1+i*dz+j, WATER);
                            for(int y = 129; y < 256; y++) {
                                setBlockAt(x1+i*dx+j, y, z1+i*dz, EMPTY);
                                setBlockAt(x1+i*dx, y, z1+i*dz+j, EMPTY);
                                setBlockAt(x1+i*dx+j, y, z1+i*dz+j, EMPTY);
                            }
                        }
                    }
                }
            }
        }
        currInfo = nextInfo;
    }
}


Biomes Terrain::randomBiomeType(){
    float r = (rand() % 5) / 4.f;
    if(r < 0.25)
        return Biomes::DESERT;
     if(r < 0.5)
         return Biomes::TUNDRA;
     if(r < 0.75)
         return Biomes::GRASSLAND;
     else
         return Biomes::MOUNTAIN;
}

glm::vec2 biomeWorley(int x, int z){
    glm::vec2 x_coord(((rand() % 100) / 99.f),
                           ((rand() % 100) / 99.f));
    glm::vec2 z_coord(((rand() % 100) / 99.f),
                           ((rand() % 100) / 99.f));
    float randx = glm::fract(sin(glm::dot(x_coord, glm::vec2(12.9898, 4.1414))) * 43758.5453);
    float randz = glm::fract(sin(glm::dot(z_coord, glm::vec2(12.9898, 4.1414))) * 43758.5453);

    return glm::vec2((x + randx), (z + randz));
}

std::pair<glm::vec2, Biomes> Terrain::getBiome(int x, int z){
    int xFloor = X_BOUND * static_cast<int>(glm::floor((float) x / X_BOUND));
    int zFloor = Z_BOUND * static_cast<int>(glm::floor((float) z / Z_BOUND));
    int64_t key = toKey(xFloor, zFloor);
    if(m_biomeMap.find(key) != m_biomeMap.end()){
        std::pair<glm::vec2, Biomes> p = m_biomeMap[key];

        /*
         * generate biome on neighbors
         * (necessary for interpolating block types)
         */
        for(int i = -1; i <= 1; i++)
            for(int j = -1; j <= 1; ++j)
                if(i != 0 && j != 0){
                    int64_t key = toKey(xFloor + i * X_BOUND, zFloor + j * Z_BOUND);
                    Biomes b = randomBiomeType();
                    glm::vec2 worley = biomeWorley(xFloor + i * X_BOUND, zFloor + j * Z_BOUND);
                    std::pair<glm::vec2, Biomes> value(worley, b);
                    m_biomeMap[key] = value;
                }


        return p;
    }

    Biomes b = randomBiomeType();
    glm::vec2 worley = biomeWorley(xFloor, zFloor);
    std::pair<glm::vec2, Biomes> value(worley, b);
    m_biomeMap[key] = value;

    // generate biome on neighbors
    for(int i = -1; i <= 1; i++)
        for(int j = -1; j <= 1; ++j)
            if(i != 0 && j != 0){
                key = toKey(xFloor + i * X_BOUND, zFloor + j * Z_BOUND);
                Biomes b = randomBiomeType();
                glm::vec2 worley = biomeWorley(xFloor + i * X_BOUND, zFloor + j * Z_BOUND);
                std::pair<glm::vec2, Biomes> toInsert(worley, b);
                m_biomeMap[key] = toInsert;
            }

    return value;
}


