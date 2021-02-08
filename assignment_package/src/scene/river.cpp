#include "river.h"

River::River() : turtle(new Turtle), axiom("[-FX]+FX") {}

void River::create() {
    QStack<glm::vec4> tempInfo;
    QString str = "";
    strMap.insert('X', axiom);
    for (int i = 0; i < 3; i++) {
        for(QChar c : axiom) {
            QString tempStr = "";
            if(c == 'X') tempStr = strMap.find('X').value();
            else tempStr = c;
            str = str + tempStr;
        }
    }
    for(QChar c : str) {
        if(c == '[') {
            tempInfo.push(glm::vec4(turtle->pos.x, turtle->pos.y, turtle->dir,
                                    turtle->depth));
            info.push(glm::vec3(turtle->pos.x, turtle->pos.y, turtle->depth));
        } else if( c == ']') {
            glm::vec4 temp = tempInfo.pop();
            turtle->pos = glm::vec2(temp.x, temp.y);
            turtle->dir = temp.z;
            turtle->depth = temp.w;
            info.push(glm::vec3(turtle->pos.x, turtle->pos.y, turtle->depth));
        } else if(c == 'F') {
            turtle->pos.x += 10 * cos(turtle->dir * PI / 180.0);
            turtle->pos.y += 10 * sin(turtle->dir * PI / 180.0);
            turtle->depth += 1;
            info.push(glm::vec3(turtle->pos.x, turtle->pos.y, turtle->depth));
        } else if (c == '-') {
            int r = rand() % 10; // random direction
            if (r != 0) turtle->dir += (r + 45); // randomly create a new branch
        } else if (c == '+') {
            int r = rand() % 10;
            if (r != 0) turtle->dir += (r - 45);
        }
    }
}
