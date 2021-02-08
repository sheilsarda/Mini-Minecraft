#pragma once
#include <iostream>
#include <QStack>
#include <QHash>
#include "glm_includes.h"
#include <math.h>
#define PI 3.14159265

struct Turtle {
    glm::vec2 pos;
    int dir;
    int depth;
    Turtle() : pos(glm::vec2(12, 48)), dir(90), depth(0) {}
};

class River {
public:
    River();
    Turtle* turtle;
    QString axiom;
    QStack<glm::vec3> info;
    QHash<QChar, QString> strMap;
    void create();
};
