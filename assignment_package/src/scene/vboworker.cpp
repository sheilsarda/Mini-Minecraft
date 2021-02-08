#include "vboworker.h"

VBOData::VBOData(Chunk *cPtr) :
    ix(std::vector<GLuint>()),
    posNorCol(std::vector<glm::vec4>()),
    uv(),
    t_ix(std::vector<GLuint>()),
    t_posNorCol(std::vector<glm::vec4>()),
    t_uv(),
    cPtr(cPtr) { }


VBOWorker::VBOWorker(QMutex *mutex,
                     std::vector<uPtr<VBOData>> *vboData,
                     Chunk *cPtr) :
    mutex(mutex),
    vboData(vboData),
    vbo(mkU<VBOData>(cPtr)),
    cPtr(cPtr) { }

void VBOWorker::run(){
    /*
     * Create chunks and set
     * num_vertices in chunk to ix.size()
     */

    vbo->cPtr->createChunk( &(vbo->posNorCol),
                            &(vbo->ix),
                            &(vbo->uv),
                            &(vbo->t_posNorCol),
                            &(vbo->t_ix),
                            &(vbo->t_uv));

    // Critical section
    mutex->lock();
    vboData->push_back(std::move(vbo));
    mutex->unlock();
}
