#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_count(-1), m_bufIdx(), m_bufPos(), m_bufNor(), m_bufCol(), m_bufUV(), m_transBufIdx(), m_transBufPosNorCol(), m_transBufUV(),
      m_idxGenerated(false), m_posGenerated(false), m_norGenerated(false), m_colGenerated(false), m_uvGenerated(false),
      m_transIdxGenerated(false), m_transPosNorColGenerated(false), m_transUVGenerated(false),
      mp_context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    mp_context->glDeleteBuffers(1, &m_bufIdx);
    mp_context->glDeleteBuffers(1, &m_bufPos);
    mp_context->glDeleteBuffers(1, &m_bufNor);
    mp_context->glDeleteBuffers(1, &m_bufCol);
    mp_context->glDeleteBuffers(1, &m_bufPosNorCol);
    mp_context->glDeleteBuffers(1, &m_bufUV);
    mp_context->glDeleteBuffers(1, &m_transBufIdx);
    mp_context->glDeleteBuffers(1, &m_transBufPosNorCol);
    mp_context->glDeleteBuffers(1, &m_transBufUV);

    m_idxGenerated = m_posGenerated = m_norGenerated = m_colGenerated = m_uvGenerated = m_posNorColGenerated
            = m_transIdxGenerated = m_transPosNorColGenerated = m_transUVGenerated = false;
    m_count = -1;
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return m_count;
}

void Drawable::generateIdx()
{
    m_idxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx);
}

void Drawable::generatePos()
{
    m_posGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufPos);
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    mp_context->glGenBuffers(1, &m_bufNor);
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

// Milestone 1
void Drawable::generatePosNorCol()
{
    m_posNorColGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufPosNorCol);
}

void Drawable::generateUV() {
    m_uvGenerated = true;
    mp_context->glGenBuffers(1, &m_bufUV);
}

void Drawable::generateTransIdx() {
    m_transIdxGenerated = true;
    mp_context->glGenBuffers(1, &m_transBufIdx);
}

void Drawable::generateTransPosNorCol(){
    m_transPosNorColGenerated = true;
    mp_context->glGenBuffers(1, &m_transBufPosNorCol);
}

void Drawable::generateTransUV() {
    m_transUVGenerated = true;
    mp_context->glGenBuffers(1, &m_transBufUV);
}

bool Drawable::bindIdx()
{
    if(m_idxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    }
    return m_idxGenerated;
}

bool Drawable::bindPos()
{
    if(m_posGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindPosNorCol(){
    if(m_posNorColGenerated)
        mp_context->glBindBuffer(GL_ARRAY_BUFFER,
                                 m_bufPosNorCol);
    return m_posNorColGenerated;
}

bool Drawable::bindUV() {
    if (m_uvGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    }
    return m_uvGenerated;
}

bool Drawable::bindTransIdx() {
    if (m_transIdxGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_transBufIdx);
    }
    return m_transIdxGenerated;
}

bool Drawable::bindTransPosNorCol() {
    if (m_transPosNorColGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_transBufPosNorCol);
    }
    return m_transPosNorColGenerated;
}

bool Drawable::bindTransUV() {
    if (m_transUVGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_transBufUV);
    }
    return m_transUVGenerated;
}
