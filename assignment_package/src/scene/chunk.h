#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"

#include "drawable.h"

#include <array>
#include <unordered_map>
#include <cstddef>

#define X_BOUND 16
#define Y_BOUND 256
#define Z_BOUND 16

// for chunk create function helpers
//#define CUB_IDX_COUNT 36;
//#define CUB_VERT_COUNT 24;

//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, SNOW, SAND, WATER, LAVA, OREA, OREB, OREC, ORED
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

enum Biomes : unsigned char
{
    DESERT, TUNDRA, GRASSLAND, MOUNTAIN
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

    // Colors for milestone 1
    glm::vec4 grassColor;
    glm::vec4 dirtColor;
    glm::vec4 stoneColor;
    glm::vec4 snowColor;

    // Milestone 2 changes
    float worldX;
    float worldZ;

public:

    Chunk(OpenGLContext *context, float x, float z);

    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;

    BlockType getBlockAt(int x, int y, int z) const;

    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);

    void create() override;
    void createChunk(std::vector<glm::vec4> *PosNorColArr,
                     std::vector<GLuint> *idxArr,
                     std::vector<glm::vec2> *uvsArr,
                     std::vector<glm::vec4> *transPosNorColArr,
                     std::vector<GLuint> *transIdxArr,
                     std::vector<glm::vec2> *transUVsArr);

    /*
     * Want to know for each block, what is around it, and that determines what
     * you render.
     *
     * Loop through X, Y, Z and search through block
     */
    void createCubeVBO(const std::vector<glm::vec4> &PosNorCol,
                       const std::vector<GLuint> &idx,
                       const std::vector<glm::vec2> &uvs,
                       const std::vector<glm::vec4> &transPosNorCol,
                       const std::vector<GLuint> &transIdx,
                       const std::vector<glm::vec2> &transUVs);

    int getWorldSpaceX();
    int getWorldSpaceZ();

    void setIndexCount(int ix_count);

};


struct VertexData{
    glm::vec4 pos;
    glm::vec2 uv;

    VertexData(glm::vec4 p, glm::vec2 u)
        : pos(p), uv(u)
    {}
};

struct BlockFace {
    Direction direction;
    glm::ivec3 directionVec;
    std::array<VertexData, 4> vertices;
    BlockFace(Direction dir, glm::ivec3 dirV, const VertexData &a, const VertexData &b, const VertexData &c, const VertexData &d)
        : direction(dir), directionVec(dirV), vertices{a, b, c, d}
    {}
};

const static std::array<BlockFace, 6> adjacentFaces{
    // +X
    BlockFace(XPOS, glm::ivec3(1, 0, 0), VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(0, 0)),
                                         VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(0.0625, 0)),
                                         VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(0.0625, 0.0625)),
                                         VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(0, 0.0625))),

    // -X
    BlockFace(XNEG, glm::ivec3(-1, 0, 0), VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)),
                                          VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0.0625, 0)),
                                          VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0.0625, 0.0625)),
                                          VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0, 0.0625))),

    // +Y
    BlockFace(YPOS, glm::ivec3(0, 1, 0), VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0, 0)),
                                         VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(0.0625, 0)),
                                         VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(0.0625, 0.0625)),
                                         VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0, 0.0625))),
    // -Y
    BlockFace(YNEG, glm::ivec3(0, -1, 0), VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)),
                                          VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(0.0625, 0)),
                                          VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(0.0625, 0.0625)),
                                          VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0, 0.0625))),

    // +Z
    BlockFace(ZPOS, glm::ivec3(0, 0, 1), VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0, 0)),
                                         VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(0.0625, 0)),
                                         VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(0.0625, 0.0625)),
                                         VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0, 0.0625))),
    // -Z
    BlockFace(ZNEG, glm::ivec3(0, 0, -1), VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(0, 0)),
                                          VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0.0625, 0)),
                                          VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0.0625, 0.0625)),
                                          VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(0, 0.0625)))
};

const static std::unordered_map<BlockType, std::unordered_map<Direction, glm::vec2, EnumHash>, EnumHash> blockFaceUVs {
    {GRASS, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(3.f/16.f, 15.f/16.f)},
                                                               {XNEG, glm::vec2(3.f/16.f, 15.f/16.f)},
                                                               {YPOS, glm::vec2(8.f/16.f, 13.f/16.f)},
                                                               {YNEG, glm::vec2(2.f/16.f, 15.f/16.f)},
                                                               {ZPOS, glm::vec2(3.f/16.f, 15.f/16.f)},
                                                               {ZNEG, glm::vec2(3.f/16.f, 15.f/16.f)}}},
    {DIRT, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(2.f/16.f, 15.f/16.f)},
                                                              {XNEG, glm::vec2(2.f/16.f, 15.f/16.f)},
                                                              {YPOS, glm::vec2(2.f/16.f, 15.f/16.f)},
                                                              {YNEG, glm::vec2(2.f/16.f, 15.f/16.f)},
                                                              {ZPOS, glm::vec2(2.f/16.f, 15.f/16.f)},
                                                              {ZNEG, glm::vec2(2.f/16.f, 15.f/16.f)}}},
    {STONE, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(1.f/16.f, 15.f/16.f)},
                                                               {XNEG, glm::vec2(1.f/16.f, 15.f/16.f)},
                                                               {YPOS, glm::vec2(1.f/16.f, 15.f/16.f)},
                                                               {YNEG, glm::vec2(1.f/16.f, 15.f/16.f)},
                                                               {ZPOS, glm::vec2(1.f/16.f, 15.f/16.f)},
                                                               {ZNEG, glm::vec2(1.f/16.f, 15.f/16.f)}}},
    {SAND, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(2.f/16.f, 14.f/16.f)},
                                                              {XNEG, glm::vec2(2.f/16.f, 14.f/16.f)},
                                                              {YPOS, glm::vec2(2.f/16.f, 14.f/16.f)},
                                                              {YNEG, glm::vec2(2.f/16.f, 14.f/16.f)},
                                                              {ZPOS, glm::vec2(2.f/16.f, 14.f/16.f)},
                                                              {ZNEG, glm::vec2(2.f/16.f, 14.f/16.f)}}},
    {LAVA, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(15.f/16.f, 1.f/16.f)},
                                                              {XNEG, glm::vec2(15.f/16.f, 1.f/16.f)},
                                                              {YPOS, glm::vec2(15.f/16.f, 1.f/16.f)},
                                                              {YNEG, glm::vec2(15.f/16.f, 1.f/16.f)},
                                                              {ZPOS, glm::vec2(15.f/16.f, 1.f/16.f)},
                                                              {ZNEG, glm::vec2(15.f/16.f, 1.f/16.f)}}},
    {WATER, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(15.f/16.f, 3.f/16.f)},
                                                              {XNEG, glm::vec2(15.f/16.f, 3.f/16.f)},
                                                              {YPOS, glm::vec2(15.f/16.f, 3.f/16.f)},
                                                              {YNEG, glm::vec2(15.f/16.f, 3.f/16.f)},
                                                              {ZPOS, glm::vec2(15.f/16.f, 3.f/16.f)},
                                                              {ZNEG, glm::vec2(15.f/16.f, 3.f/16.f)}}},
    {SNOW, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(2.f/16.f, 11.f/16.f)},
                                                              {XNEG, glm::vec2(2.f/16.f, 11.f/16.f)},
                                                              {YPOS, glm::vec2(2.f/16.f, 11.f/16.f)},
                                                              {YNEG, glm::vec2(2.f/16.f, 11.f/16.f)},
                                                              {ZPOS, glm::vec2(2.f/16.f, 11.f/16.f)},
                                                              {ZNEG, glm::vec2(2.f/16.f, 11.f/16.f)}}},
    {OREA, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(2.f/16.f, 12.f/16.f)},
                                                              {XNEG, glm::vec2(2.f/16.f, 12.f/16.f)},
                                                              {YPOS, glm::vec2(2.f/16.f, 12.f/16.f)},
                                                              {YNEG, glm::vec2(2.f/16.f, 12.f/16.f)},
                                                              {ZPOS, glm::vec2(2.f/16.f, 12.f/16.f)},
                                                              {ZNEG, glm::vec2(2.f/16.f, 12.f/16.f)}}},
    {OREB, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(3.f/16.f, 12.f/16.f)},
                                                              {XNEG, glm::vec2(3.f/16.f, 12.f/16.f)},
                                                              {YPOS, glm::vec2(3.f/16.f, 12.f/16.f)},
                                                              {YNEG, glm::vec2(3.f/16.f, 12.f/16.f)},
                                                              {ZPOS, glm::vec2(3.f/16.f, 12.f/16.f)},
                                                              {ZNEG, glm::vec2(3.f/16.f, 12.f/16.f)}}},
    {OREC, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(0.f/16.f, 13.f/16.f)},
                                                              {XNEG, glm::vec2(0.f/16.f, 13.f/16.f)},
                                                              {YPOS, glm::vec2(0.f/16.f, 13.f/16.f)},
                                                              {YNEG, glm::vec2(0.f/16.f, 13.f/16.f)},
                                                              {ZPOS, glm::vec2(0.f/16.f, 13.f/16.f)},
                                                              {ZNEG, glm::vec2(0.f/16.f, 13.f/16.f)}}},
    {ORED, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(1.f/16.f, 13.f/16.f)},
                                                              {XNEG, glm::vec2(1.f/16.f, 13.f/16.f)},
                                                              {YPOS, glm::vec2(1.f/16.f, 13.f/16.f)},
                                                              {YNEG, glm::vec2(1.f/16.f, 13.f/16.f)},
                                                              {ZPOS, glm::vec2(1.f/16.f, 13.f/16.f)},
                                                              {ZNEG, glm::vec2(1.f/16.f, 13.f/16.f)}}}
};
