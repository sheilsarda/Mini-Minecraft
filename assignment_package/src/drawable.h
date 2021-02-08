#pragma once
#include <openglcontext.h>
#include <glm_includes.h>

//This defines a class which can be rendered by our shader program.
//Make any geometry a subclass of ShaderProgram::Drawable in order to render it with the ShaderProgram class.
class Drawable
{
protected:
    int m_count;     // The number of indices stored in bufIdx.
    GLuint m_bufIdx; // A Vertex Buffer Object that we will use to store triangle indices (GLuints)
    GLuint m_bufPos; // A Vertex Buffer Object that we will use to store mesh vertices (vec4s)
    GLuint m_bufNor; // A Vertex Buffer Object that we will use to store mesh normals (vec4s)
    GLuint m_bufCol; // Can be used to pass per-vertex color information to the shader, but is currently unused.
                   // Instead, we use a uniform vec4 in the shader to set an overall color for the geometry
    // Milestone 1
    GLuint m_bufPosNorCol;

    GLuint m_bufUV;

    // for transparent block
    GLuint m_transBufIdx;
    GLuint m_transBufPosNorCol;
    GLuint m_transBufUV;



    bool m_idxGenerated; // Set to TRUE by generateIdx(), returned by bindIdx().
    bool m_posGenerated;
    bool m_norGenerated;
    bool m_colGenerated;

    // Milestone 1
    bool m_posNorColGenerated;
    bool m_uvGenerated;

    bool m_transIdxGenerated;
    bool m_transPosNorColGenerated;
    bool m_transUVGenerated;


    OpenGLContext* mp_context; // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
                          // we need to pass our OpenGL context to the Drawable in order to call GL functions
                          // from within this class.


public:
    Drawable(OpenGLContext* mp_context);
    virtual ~Drawable();

    virtual void create() = 0; // To be implemented by subclasses. Populates the VBOs of the Drawable.
    void destroy(); // Frees the VBOs of the Drawable.

    // Getter functions for various GL data
    virtual GLenum drawMode();
    int elemCount();

    // Call these functions when you want to call glGenBuffers on the buffers stored in the Drawable
    // These will properly set the values of idxBound etc. which need to be checked in ShaderProgram::draw()
    void generateIdx();
    void generatePos();
    void generateNor();
    void generateCol();

    // Milestone 1
    void generatePosNorCol();

    // Milestone 2
    void generateUV();
    void generateTransIdx();
    void generateTransPosNorCol();
    void generateTransUV();

    bool bindIdx();
    bool bindPos();
    bool bindNor();
    bool bindCol();

    // Milestone 1
    bool bindPosNorCol();
    bool bindUV();

    bool bindTransIdx();
    bool bindTransPosNorCol();
    bool bindTransUV();
};
