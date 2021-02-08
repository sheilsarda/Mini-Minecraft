#include "player.h"
#include <QString>
#include <iostream>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      flightMode(true), mcr_camera(m_camera)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain);
}

void Player::updateAcc(glm::vec3 v) {
    if (!flightMode) {
        v.y = 0;
        v = glm::normalize(v);
    }
    m_acceleration += v;
}

void Player::processInputs(InputBundle &inputs) {
    /*
     * Update the Player's velocity and acceleration
     * based on the state of the inputs.
     */

    // constant for accerelation
    float accScale = 1.f;

    if (inputs.fPressed)  {
        flightMode = !flightMode;
        std::cout << "flight mode: " << flightMode << "\n";
    }
    if (inputs.spacePressed && m_position.y < 129 &&
        mcr_terrain.getBlockAt(m_position.x, 128, m_position.z) == WATER) {
        m_acceleration += 0.1f * accScale * m_up;
    } else if (inputs.spacePressed && !flightMode) {
        m_acceleration.y = 100 * accScale;
    } else if (inputs.aPressed) {
        updateAcc(-accScale * m_right);
    } else if (inputs.dPressed) {
        updateAcc(accScale * m_right);
    } else if (inputs.sPressed) {
        updateAcc(-accScale * m_forward);
    } else if (inputs.wPressed) {
        updateAcc(accScale * m_forward);
    } else if (inputs.qPressed && flightMode) {
        m_acceleration += -accScale * m_up;
    } else if (inputs.ePressed && flightMode) {
        m_acceleration += accScale * m_up;
    } else {
        m_acceleration = glm::vec3(0, 0, 0);
    }

    // mouse rotation
    float rotateScale = 10.f;
    polarRotate(inputs.mouseX / rotateScale, inputs.mouseY / rotateScale);
    m_camera.polarRotate(inputs.mouseX / rotateScale, inputs.mouseY / rotateScale);
}

void Player::computePhysics(float dT, const Terrain &terrain) {
    /*
     * Update the Player's position based on
     * its acceleration and velocity, and also
     * perform collision detection.
     */

    float gravityScale = 9.81 * 5;
    m_velocity = m_acceleration * dT;
    glm::vec3 pos = m_velocity * dT;

    if (!flightMode) {
        glm::ivec3 currCell = glm::ivec3(glm::floor(m_position));
        try {
            //check if on the ground
            BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y - 1, currCell.z);
            if(cellType == EMPTY) {
                m_acceleration.y -= gravityScale;
                m_velocity = m_acceleration * dT;
                pos = m_velocity * dT;
            }
        } catch (const std::out_of_range& e) {
            //std::cout << "chunk out of range"<< "\n";
        }
        float minDist = FLT_MAX;
        for (float i = -0.5; i <= 0.5; i++) {
            for (int j = 0; j <= 2; j++) {
                for (float k = -0.5; k <= 0.5; k++) {
                    float outDist = 0.f;
                    glm::ivec3 outBlockHit(0, 0, 0);
                    glm::vec3 rayOrigin = glm::vec3(m_position.x + i, m_position.y + j, m_position.z + k);
                    if (gridMarch(rayOrigin, pos, terrain, &outDist, &outBlockHit)) {
                        minDist = glm::min(minDist, outDist);
                    }
                }
            }
        }

        if (minDist != FLT_MAX) {
            minDist = 0.999f * minDist;
            if (minDist < 1e-04) {
                minDist = 0;
            }
            pos = minDist * glm::normalize(pos);
        }
    }
    this->moveAlongVector(pos);
}

void Player::removeBlock() {
    float outDist = 0.f;
    glm::ivec3 outBlockHit(0, 0, 0);

    if (gridMarch(mcr_camera.mcr_position, 3.f * m_forward, mcr_terrain, &outDist, &outBlockHit)) {
        int xPos = outBlockHit.x % X_BOUND;
        int yPos = outBlockHit.y % Y_BOUND;
        int zPos = outBlockHit.z % Z_BOUND;
        mcr_terrain.getChunkAt(outBlockHit.x, outBlockHit.z)->setBlockAt(xPos, yPos, zPos, EMPTY);
    }
}

void Player::addBlock() {
    float outDist = 0.f;
    glm::ivec3 outBlockHit(0, 0, 0);
    glm::vec3 intersection(0, 0, 0);
    if (gridMarch(mcr_camera.mcr_position, 3.f * m_forward, mcr_terrain, &outDist, &outBlockHit, &intersection)) {
        if (intersection.x - outBlockHit.x == 1.f) {
            outBlockHit.x +=1;
        } else if (intersection.y - outBlockHit.y == 1.f) {
            outBlockHit.y +=1;
        } else if (intersection.z - outBlockHit.z == 1.f) {
            outBlockHit.z +=1;
        }
    } else {
        glm::vec3 pos = mcr_camera.mcr_position + 3.f * m_forward;
        outBlockHit = glm::ivec3(glm::floor(pos));
    }
    int xPos = outBlockHit.x % X_BOUND;
    int yPos = outBlockHit.y % Y_BOUND;
    int zPos = outBlockHit.z % Z_BOUND;
    if (mcr_terrain.getBlockAt(outBlockHit.x, outBlockHit.y, outBlockHit.z) == EMPTY) {
        mcr_terrain.getChunkAt(outBlockHit.x, outBlockHit.z)->setBlockAt(xPos, yPos, zPos, WATER);
    }

}

bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection,
                       const Terrain &terrain, float *outDist, glm::ivec3 *outBlockHit) {
    glm::vec3 intersection(0, 0, 0);
    return gridMarch(rayOrigin, rayDirection, terrain, outDist, outBlockHit, &intersection);
}

bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection,
                       const Terrain &terrain, float *outDist, glm::ivec3 *outBlockHit,
                       glm::vec3 *intersection) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        try {
            BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
            if(cellType != EMPTY) {
                *outBlockHit = currCell;
                if (cellType == WATER || cellType == LAVA) {
                    *outDist = 0.67 * glm::min(maxLen, curr_t);
                } else {
                    *outDist = glm::min(maxLen, curr_t);
                }
                *intersection = rayOrigin;
                return true;
            }
        } catch (const std::out_of_range& e) {
            //std::cout << "chunk out of range"<< "\n";
        }

    }
    *outDist = glm::min(maxLen, curr_t);
    return false;
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
