#ifndef NOISEFUNCTIONS_H
#define NOISEFUNCTIONS_H

#include "glm_includes.h"
#include <stdexcept>
#include "chunk.h"
#include "river.h"
#include "terrain.h"

class NoiseFunction {

public:

    NoiseFunction();

    float random1(glm::vec2 p);
    glm::vec2 random2(glm::vec2 p);
    glm::vec3 random3(glm::vec3 p);
    float surflet(glm::vec2 p, glm::vec2 gridPoint);
    float interpNoise(glm::vec2 p);
    float fbm(glm::vec2 p);
    float varonoiEffect(glm::vec2 p, float *cellHeight);


    float fractalPerlin(glm::vec2 p);
    float grasslandEffect(glm::vec2 p);
    float perlinNoise(glm::vec2 p);
    float surflet3D(glm::vec3 p, glm::vec3 gridPoint);
    float perlinNoise3D(glm::vec3 p);

    void drawRiver(Chunk *cPtr);

    float biomeHeight(glm::vec2 p);
    BlockType biomeBlock(glm::vec3 p, Biomes b, glm::vec2 noise, std::unordered_map<int64_t, std::pair<glm::vec2, Biomes>> biomeMap);


};
#endif // NOISEFUNCTIONS_H
