#include "chunk.h"
#include <iostream>

Chunk::Chunk(OpenGLContext *context, float x, float z) :
    Drawable(context),
    m_blocks(),
    m_neighbors{{XPOS, nullptr},
                {XNEG, nullptr},
                {ZPOS, nullptr},
                {ZNEG, nullptr}},
    grassColor(glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f),
    dirtColor(glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f),
    stoneColor(glm::vec3(0.5f), 1.f),
    snowColor(glm::vec4(1.f)),
    worldX(x),
    worldZ(z){
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {

    if(x >= X_BOUND || y >= Y_BOUND || z >= Z_BOUND )
        return BlockType::EMPTY;

    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    if(x >= X_BOUND || x < 0)
        return BlockType::EMPTY;
    if(y >= Y_BOUND || y < 0)
        return BlockType::EMPTY;
    if(z >= Z_BOUND || z < 0)
        return BlockType::EMPTY;

    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};


void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

void updateChunkVBO(std::vector<GLuint> &idx, std::vector<glm::vec4> &PosNorCol,
                    std::vector<glm::vec2> &uvs, int &vertexCount, Direction dir, BlockType bType,
                    glm::vec4 blockPos, float animateable) {
    std::unordered_map<Direction, glm::vec2, EnumHash> faceUVs = blockFaceUVs.at(bType);
    BlockFace f = adjacentFaces.at(dir);
    glm::vec2 uv = faceUVs.at(dir);
    glm::vec4 col = glm::vec4(1, 0, 0, 1);
    glm::vec4 nor = glm::vec4(glm::vec4(f.directionVec, 1));
    for (int i = 0; i < 4; i++) {
        PosNorCol.push_back(f.vertices.at(i).pos + blockPos);
        PosNorCol.push_back(nor);
        PosNorCol.push_back(col);
        PosNorCol.push_back(glm::vec4(animateable));
        uvs.push_back(uv + f.vertices.at(i).uv);
    }
    idx.push_back(vertexCount);
    idx.push_back(vertexCount + 1);
    idx.push_back(vertexCount + 2);
    idx.push_back(vertexCount);
    idx.push_back(vertexCount + 2);
    idx.push_back(vertexCount + 3);
    vertexCount += 4;
}

bool isTransparent(BlockType bType) {
    return bType == WATER || bType == LAVA;
}

void Chunk::create(){

    std::vector<GLuint> idx;
    std::vector<glm::vec4> PosNorCol;
    std::vector<glm::vec2> uvs;
    int vertexCount = 0;

    // attributes for transparent blocks
    std::vector<GLuint> transIdx;
    std::vector<glm::vec4> transPosNorCol;
    std::vector<glm::vec2> transUVs;
    int transVertexCount = 0;

    // temp var which gets reassigned
    glm::vec4 p;

    for(int x = 0; x < X_BOUND; ++x){
        for(int y = 0; y < Y_BOUND; ++y){
            for(int z = 0; z < Z_BOUND; ++z){
                BlockType bt = getBlockAt(x, y, z);
                if (bt == BlockType::EMPTY) {
                    continue;
                }

                glm::vec4 blockPos(x, y, z, 0);
                BlockType xnegNeighbor = getBlockAt(x - 1, y, z);
                BlockType ynegNeighbor = getBlockAt(x, y - 1, z);
                BlockType znegNeighbor = getBlockAt(x, y, z - 1);

                if(x == 0 && m_neighbors[XNEG] != nullptr)
                    xnegNeighbor = m_neighbors[XNEG]->getBlockAt(X_BOUND - 1, y, z);

                if(y == 0 && m_neighbors[YNEG] != nullptr)
                    ynegNeighbor = m_neighbors[YNEG]->getBlockAt(x, Y_BOUND - 1, z);

                if(z == 0 && m_neighbors[ZNEG] != nullptr)
                    znegNeighbor = m_neighbors[ZNEG]->getBlockAt(x, y, Z_BOUND - 1);

                BlockType xposNeighbor = getBlockAt(x + 1, y, z);
                BlockType yposNeighbor = getBlockAt(x, y + 1, z);
                BlockType zposNeighbor = getBlockAt(x, y, z + 1);

                if(x == X_BOUND - 1 && m_neighbors[XPOS] != nullptr)
                    xposNeighbor = m_neighbors[XPOS]->getBlockAt(0, y, z);

                if(y == Y_BOUND - 1 && m_neighbors[YPOS] != nullptr)
                    yposNeighbor = m_neighbors[YPOS]->getBlockAt(x, 0, z);

                if(z == Z_BOUND - 1 && m_neighbors[ZPOS] != nullptr)
                    zposNeighbor = m_neighbors[ZPOS]->getBlockAt(x, y, 0);


                if(zposNeighbor == BlockType::EMPTY || (isTransparent(zposNeighbor) &&  zposNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, ZPOS, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, ZPOS, bt, blockPos, 0.f);
                    }
                }

                if(xposNeighbor == BlockType::EMPTY || (isTransparent(xposNeighbor) &&  xposNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, XPOS, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, XPOS, bt, blockPos, 0.f);
                    }
                }

                if(xnegNeighbor == BlockType::EMPTY || (isTransparent(xnegNeighbor) &&  xnegNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, XNEG, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, XNEG, bt, blockPos, 0.f);
                    }
                }

                if(znegNeighbor == BlockType::EMPTY || (isTransparent(znegNeighbor) &&  znegNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, ZNEG, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, ZNEG, bt, blockPos, 0.f);
                    }
                }

                if(yposNeighbor == BlockType::EMPTY || (isTransparent(yposNeighbor) &&  yposNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, YPOS, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, YPOS, bt, blockPos, 0.f);
                    }
                }

                if(ynegNeighbor == BlockType::EMPTY || (isTransparent(ynegNeighbor) &&  ynegNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, YNEG, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, YNEG, bt, blockPos, 0.f);
                    }
                }
            }
        }
    }

    m_count = idx.size();
    createCubeVBO(PosNorCol, idx, uvs, transPosNorCol, transIdx, transUVs);
}

void Chunk::createChunk(std::vector<glm::vec4> *PosNorColArr,
                        std::vector<GLuint> *idxArr,
                        std::vector<glm::vec2> *uvsArr,
                        std::vector<glm::vec4> *transPosNorColArr,
                        std::vector<GLuint> *transIdxArr,
                        std::vector<glm::vec2> *transUVsArr){

    std::vector<GLuint> idx;
    std::vector<glm::vec4> PosNorCol;
    std::vector<glm::vec2> uvs;
    int vertexCount = 0;

    // attributes for transparent blocks
    std::vector<GLuint> transIdx;
    std::vector<glm::vec4> transPosNorCol;
    std::vector<glm::vec2> transUVs;
    int transVertexCount = 0;

    // temp var which gets reassigned
    glm::vec4 p;

    for(int x = 0; x < X_BOUND; ++x){
        for(int y = 0; y < Y_BOUND; ++y){
            for(int z = 0; z < Z_BOUND; ++z){
                BlockType bt = getBlockAt(x, y, z);
                if (bt == BlockType::EMPTY) {
                    continue;
                }

                glm::vec4 blockPos(x, y, z, 0);
                BlockType xnegNeighbor = getBlockAt(x - 1, y, z);
                BlockType ynegNeighbor = getBlockAt(x, y - 1, z);
                BlockType znegNeighbor = getBlockAt(x, y, z - 1);

                if(x == 0 && m_neighbors[XNEG] != nullptr)
                    xnegNeighbor = m_neighbors[XNEG]->getBlockAt(X_BOUND - 1, y, z);

                if(y == 0 && m_neighbors[YNEG] != nullptr)
                    ynegNeighbor = m_neighbors[YNEG]->getBlockAt(x, Y_BOUND - 1, z);

                if(z == 0 && m_neighbors[ZNEG] != nullptr)
                    znegNeighbor = m_neighbors[ZNEG]->getBlockAt(x, y, Z_BOUND - 1);

                BlockType xposNeighbor = getBlockAt(x + 1, y, z);
                BlockType yposNeighbor = getBlockAt(x, y + 1, z);
                BlockType zposNeighbor = getBlockAt(x, y, z + 1);

                if(x == X_BOUND - 1 && m_neighbors[XPOS] != nullptr)
                    xposNeighbor = m_neighbors[XPOS]->getBlockAt(0, y, z);

                if(y == Y_BOUND - 1 && m_neighbors[YPOS] != nullptr)
                    yposNeighbor = m_neighbors[YPOS]->getBlockAt(x, 0, z);

                if(z == Z_BOUND - 1 && m_neighbors[ZPOS] != nullptr)
                    zposNeighbor = m_neighbors[ZPOS]->getBlockAt(x, y, 0);


                if(zposNeighbor == BlockType::EMPTY || (isTransparent(zposNeighbor) &&  zposNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, ZPOS, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, ZPOS, bt, blockPos, 0.f);
                    }
                }

                if(xposNeighbor == BlockType::EMPTY || (isTransparent(xposNeighbor) &&  xposNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, XPOS, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, XPOS, bt, blockPos, 0.f);
                    }
                }

                if(xnegNeighbor == BlockType::EMPTY || (isTransparent(xnegNeighbor) &&  xnegNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, XNEG, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, XNEG, bt, blockPos, 0.f);
                    }
                }

                if(znegNeighbor == BlockType::EMPTY || (isTransparent(znegNeighbor) &&  znegNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, ZNEG, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, ZNEG, bt, blockPos, 0.f);
                    }
                }

                if(yposNeighbor == BlockType::EMPTY || (isTransparent(yposNeighbor) &&  yposNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, YPOS, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, YPOS, bt, blockPos, 0.f);
                    }
                }

                if(ynegNeighbor == BlockType::EMPTY || (isTransparent(ynegNeighbor) &&  ynegNeighbor != bt)){
                    if (isTransparent(bt)) {
                        updateChunkVBO(transIdx, transPosNorCol, transUVs, transVertexCount, YNEG, bt, blockPos, 1.f);
                    } else {
                        updateChunkVBO(idx, PosNorCol, uvs, vertexCount, YNEG, bt, blockPos, 0.f);
                    }
                }
            }
        }
    }

    m_count = idx.size();

    *PosNorColArr = PosNorCol;
    *idxArr = idx;
    *uvsArr = uvs;
    *transPosNorColArr = transPosNorCol;
    *transIdxArr = transIdx;
    *transUVsArr = transUVs;

}


void Chunk::createCubeVBO(const std::vector<glm::vec4> &PosNorCol,
                          const std::vector<GLuint> &idx,
                          const std::vector<glm::vec2> &uvs,
                          const std::vector<glm::vec4> &transPosNorCol,
                          const std::vector<GLuint> &transIdx,
                          const std::vector<glm::vec2> &transUVs){
    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                             idx.size() * sizeof(GLuint),
                             idx.data(),
                             GL_STATIC_DRAW);

    generatePosNorCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPosNorCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER,
                             PosNorCol.size() * sizeof(glm::vec4),
                             PosNorCol.data(),
                             GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER,
                             uvs.size() * sizeof(glm::vec2),
                             uvs.data(),
                             GL_STATIC_DRAW);

    generateTransIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_transBufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                             transIdx.size() * sizeof(GLuint),
                             transIdx.data(),
                             GL_STATIC_DRAW);

    generateTransPosNorCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_transBufPosNorCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER,
                             transPosNorCol.size() * sizeof(glm::vec4),
                             transPosNorCol.data(),
                             GL_STATIC_DRAW);

    generateTransUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_transBufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER,
                             transUVs.size() * sizeof(glm::vec2),
                             transUVs.data(),
                             GL_STATIC_DRAW);
}


int Chunk::getWorldSpaceX(){
    return static_cast<int>(glm::floor(worldX / 16.f)) * 16; // Check
}

int Chunk::getWorldSpaceZ(){
    return static_cast<int>(glm::floor(worldZ / 16.f)) * 16;
}

void Chunk::setIndexCount(int ix_count){
    this->m_count = ix_count;
}
