#ifndef VBOWORKER_H
#define VBOWORKER_H

#include <QRunnable>
#include "chunk.h"
#include <QMutex>

class VBOData {
public:
    std::vector<GLuint> ix;
    std::vector<glm::vec4> posNorCol;
    std::vector<glm::vec2> uv;
    std::vector<GLuint> t_ix;
    std::vector<glm::vec4> t_posNorCol;
    std::vector<glm::vec2> t_uv;
    Chunk *cPtr;

    VBOData(Chunk *cPtr);
};


class VBOWorker : public QRunnable
{
private:
    QMutex *mutex;
    std::vector<uPtr<VBOData>> *vboData;
    uPtr<VBOData> vbo;
    Chunk *cPtr;

public:
    VBOWorker(QMutex *mutex,
              std::vector<uPtr<VBOData>> *vboData,
              Chunk *cPtr);

    void run() override;
};



#endif // VBOWORKER_H
