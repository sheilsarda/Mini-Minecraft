#include "noisefunctions.h"

NoiseFunction::NoiseFunction() { }

float NoiseFunction::random1(glm::vec2 p) {
    return glm::fract(sin(glm::dot(p, glm::vec2(127.1f, 311.7f))) * 43758.5453f);
}

glm::vec2 NoiseFunction::random2(glm::vec2 p) {
    float x = glm::fract(sin(glm::dot(p, glm::vec2(127.1f, 311.7f))) * 43758.5453f);
    float y = glm::fract(sin(glm::dot(p, glm::vec2(269.5f, 183.3f))) * 43758.5453f);
    return glm::vec2(x, y);
}

glm::vec3 NoiseFunction::random3(glm::vec3 p) {
    float x = glm::fract(sin(glm::dot(p, glm::vec3(127.1f, 311.7f, 420.69f))) * 43758.5453f);
    float y = glm::fract(sin(glm::dot(p, glm::vec3(269.5f, 183.3f, 632.897f))) * 43758.5453f);
    float z = glm::fract(sin(glm::dot((p - glm::vec3(5.555, 10.95645, 70.266)),
    glm::vec3(765.54f, 631.2f, 109.21f))) * 43758.5453f);
    return glm::vec3(x, y, z);
}

float NoiseFunction::surflet(glm::vec2 p, glm::vec2 gridPoint) {
    glm::vec2 t2 = glm::abs(p - gridPoint);
    glm::vec2 t = glm::vec2(1.f) - 6.f * glm::vec2(pow(t2.x, 5.f), pow(t2.y, 5.f)) +
    15.f * glm::vec2(pow(t2.x, 4.f), pow(t2.y, 4.f)) -
    10.f * glm::vec2(pow(t2.x, 3.f), pow(t2.y, 3.f));
    glm::vec2 gradient = random2(gridPoint) * 2.f - glm::vec2(1,1);
    glm::vec2 diff = p - gridPoint;
    float height = glm::dot(diff, gradient);
    return height * t.x * t.y;
}


float NoiseFunction::perlinNoise(glm::vec2 p) {
    float surfletSum = 0.f;
    for (int dx = 0; dx <= 1; dx++) {
        for (int dy = 0; dy <= 1; dy++) {
            surfletSum += surflet(p, glm::vec2(floor(p.x), floor(p.y)) +
                                  glm::vec2(dx, dy));
        }
    }
    return surfletSum;
}

float NoiseFunction::fractalPerlin(glm::vec2 p) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += (1.f - abs(perlinNoise(p * freq))) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

float NoiseFunction::interpNoise(glm::vec2 p) {
    float intX = glm::floor(p.x);
    float fractX = glm::fract(p.x);
    float intY = glm::floor(p.y);
    float fractY = glm::fract(p.y);
    float v1 = random1(glm::vec2(intX, intY));
    float v2 = random1(glm::vec2(intX+1, intY));
    float v3 = random1(glm::vec2(intX, intY+1));
    float v4 = random1(glm::vec2(intX+1, intY+1));
    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);
    return glm::mix(i1, i2, fractY);
}

float NoiseFunction::fbm(glm::vec2 p) {
    float total = 0;
    float presistence = 0.5;
    int octaves = 8;
    for(int i = 0; i < octaves; i++) {
        float freq = pow(2.f, i);
        float amp = pow(presistence, i);
        total += interpNoise(p * freq) * amp;
    }
    return total;
}
float NoiseFunction::varonoiEffect(glm::vec2 p, float *cellHeight) {
    glm::vec2 pInt = glm::vec2(floor(p.x), floor(p.y));
    glm::vec2 pFract = glm::fract(p);
    float angle = perlinNoise(p * 2.f) * 3.14159f;
    pFract += glm::vec2(cos(angle), sin(angle)) * 0.25f;
    float minDist1 = 1.0;
    float minDist2 = 1.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y));
            glm::vec2 point = random2(pInt + neighbor);
            glm::vec2 diff = neighbor + point - pFract;
            float dist = diff.x * diff.x + diff.y * diff.y;
            if (dist < minDist1) {
                *cellHeight = random2(point).x;
                minDist2 = minDist1;
                minDist1 = dist;
            } else if (dist < minDist2) {
                minDist2 = dist;
            }
        }
    }
    *cellHeight = 0.5f * (*cellHeight) + 0.5f;
    float finalOutput = -1 * minDist1 + minDist2;
    return finalOutput;
}

float NoiseFunction::biomeHeight(glm::vec2 p) {
    float x = p.x / 64.f;
    float z = p.y / 64.f;
    float height = 0;
    float persistence = 0.45f;
    int octaves = 16;// 8;

    for(int o = 1; o <= octaves; ++o){
        float freq = pow(2.f, o);
        float amp = pow(persistence, o) * 50;
        height += interpNoise(glm::vec2(x * freq, z * freq)) * amp;
    }

    return height;
}


float NoiseFunction::grasslandEffect(glm::vec2 p) {
    float cellHeight = 1.f;
    float varonoi = varonoiEffect(p * 4.f, &cellHeight);
    varonoi = glm::max(0.f, varonoi - 0.1f);
    varonoi = glm::smoothstep(0.f, 1.f, varonoi);
    varonoi = cellHeight * varonoi;
    return fbm(p) * 0.67f + varonoi * 0.33f;
}

float NoiseFunction::surflet3D(glm::vec3 p, glm::vec3 gridPoint) {
    glm::vec3 t2 = glm::abs(p - gridPoint);
    glm::vec3 t = glm::vec3(1.f) - 6.f * glm::vec3(pow(t2.x, 5.f), pow(t2.y, 5.f),
    pow(t2.z, 5.f)) + 15.f * glm::vec3(pow(t2.x, 4.f), pow(t2.y, 4.f),
    pow(t2.z, 4.f)) - 10.f * glm::vec3(pow(t2.x, 3.f), pow(t2.y, 3.f),
    pow(t2.z, 3.f));
    glm::vec3 gradient = random3(gridPoint) * 2.f - glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 diff = p - gridPoint;
    float height = glm::dot(diff, gradient);
    return height * t.x * t.y * t.z;
}

float NoiseFunction::perlinNoise3D(glm::vec3 p) {
    float surfletSum = 0.f;
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            for(int dz = 0; dz <= 1; ++dz) {
                surfletSum += surflet3D(p, glm::vec3(floor(p.x), floor(p.y),
                floor(p.z)) + glm::vec3(dx, dy, dz));
            }
        }
    }
    return surfletSum;
}

void NoiseFunction::drawRiver(Chunk *cPtr){

    int x_min = 0;
    int x_max = X_BOUND;
    int z_min = 0;
    int z_max = Z_BOUND;

    uPtr<River> river = mkU<River>();
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
                if(x1+i*dx < x_max &&
                   x1+i*dx > x_min &&
                   z1+i*dz < z_max &&
                   z1+i*dz > z_min) {

                    int h = 128;

                    while (cPtr->getBlockAt(x1+i*dx, h, z1+i*dz) != EMPTY && h != 256)
                        h++;

                    for(int j = 0; j <= w; j++) {
                        if(x1+i*dx+j < x_max &&
                           x1+i*dx+j > x_min &&
                           z1+i*dz+j < z_max &&
                           z1+i*dz+j > z_min) {

                            cPtr->setBlockAt(x1+i*dx+j, 128, z1+i*dz, WATER);
                            cPtr->setBlockAt(x1+i*dx, 128, z1+i*dz+j, WATER);
                            cPtr->setBlockAt(x1+i*dx+j, 128, z1+i*dz+j, WATER);
                            for(int y = 129; y < 256; y++) {
                                cPtr->setBlockAt(x1+i*dx+j, y, z1+i*dz, EMPTY);
                                cPtr->setBlockAt(x1+i*dx, y, z1+i*dz+j, EMPTY);
                                cPtr->setBlockAt(x1+i*dx+j, y, z1+i*dz+j, EMPTY);
                            }
                        }
                    }
                }
            }
        }
        currInfo = nextInfo;
    }
}

BlockType NoiseFunction::biomeBlock(glm::vec3 p, Biomes b, glm::vec2 noise, std::unordered_map<int64_t, std::pair<glm::vec2, Biomes>> biomeMap){
    float primaryBiome = std::sqrt(pow(p.x - noise.x, 2) + pow(p.y - noise.y, 2));
    // iterate through biome neighbors
    int xFloor = X_BOUND * static_cast<int>(glm::floor((float) p.x / X_BOUND));
    int zFloor = Z_BOUND * static_cast<int>(glm::floor((float) p.y / Z_BOUND));

    for(int i = -1; i <= 1; ++i){
        for(int j = -1; j <= 1; ++j){
            if(i == 0 && j == 0) continue;
            int64_t key = toKey(xFloor + i * X_BOUND, zFloor + j * Z_BOUND);

            // want to compute distance of this block with neighbor
            std::pair<glm::vec2, Biomes> neighbor = biomeMap[key];
            glm::vec2 p_n = neighbor.first;
            Biomes b_n = neighbor.second;
            float secondaryBiome = std::sqrt(pow(p.x - p_n.x, 2) + pow(p.y - p_n.y, 2));
            if(secondaryBiome < 60.f){
                float dist = 20.f - (secondaryBiome - primaryBiome);
                float prob = glm::smoothstep(0.f, 64.f, dist);
                float r = (rand() % 100 / 99.f);
                if(r < prob) b = b_n;
            }

        }
    }


    float m = fractalPerlin(glm::vec2((xFloor % 64) /64.f, (zFloor % 64) /64.f)) * 70 + 110;
    float t = abs(perlinNoise(glm::vec2((xFloor % 64) /64.f, (zFloor % 64) /64.f)));
    float s = glm::smoothstep(0.25f, 0.75f, 2 * t);

    if(b == Biomes::GRASSLAND){
       if (s > 0.9) {
            if (p.z < m) return STONE;
            else if (p.z == ceil(m)) return SNOW;
        }
    }

    switch(b) {
    case DESERT:
        return SAND;
    case MOUNTAIN:
        return STONE;
    case GRASSLAND:
        return GRASS;
    case TUNDRA:
        return SNOW;
    }
}
