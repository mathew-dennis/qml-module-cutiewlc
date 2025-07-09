#include "cutieappthumbnail.h"
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QDebug>

// Renderer using modern OpenGL (GLSL shaders + VBO/VAO)
class AppThumbnailRenderer : public QQuickFramebufferObject::Renderer,
                             protected QOpenGLFunctions
{
public:
    AppThumbnailRenderer(CutieShell* shell, ForeignToplevelHandleV1* toplevel)
        : m_shell(shell), m_toplevel(toplevel)
    {
        initializeOpenGL();
    }

    ~AppThumbnailRenderer() override {
        if (m_program)
            delete m_program;
    }

    void render() override {
        if (!m_shell || !m_toplevel)
            return;

        QOpenGLTexture* texture = m_shell->textureForToplevel(m_toplevel);
        if (!texture)
            return;

        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

        f->glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
        f->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_program->bind();
        texture->bind(0);
        m_program->setUniformValue("u_texture", 0);

        m_vao.bind();
        f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_vao.release();

        texture->release();
        m_program->release();

        update(); // schedule next render
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override {
        m_viewportSize = size;
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setTextureTarget(GL_TEXTURE_2D);
        format.setInternalTextureFormat(GL_RGBA);
        return new QOpenGLFramebufferObject(size, format);
    }

private:
    void initializeOpenGL() {
        initializeOpenGLFunctions(); // initialize QOpenGLFunctions

        // Shader source code (OpenGL 3.3)
        static const char *vertexShaderSrc =
            R"(#version 330
               in vec2 position;
               in vec2 texCoord;
               out vec2 v_texCoord;
               void main() {
                   v_texCoord = texCoord;
                   gl_Position = vec4(position, 0.0, 1.0);
               })";

        static const char *fragmentShaderSrc =
            R"(#version 330
               uniform sampler2D u_texture;
               in vec2 v_texCoord;
               out vec4 fragColor;
               void main() {
                   fragColor = texture(u_texture, v_texCoord);
               })";

        // Compile shader
        m_program = new QOpenGLShaderProgram();
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSrc))
            qWarning() << "Vertex shader error:" << m_program->log();
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSrc))
            qWarning() << "Fragment shader error:" << m_program->log();
        if (!m_program->link())
            qWarning() << "Shader link error:" << m_program->log();

        // Vertex data: 4 vertices (x, y, u, v)
        GLfloat vertices[] = {
            -1.0f, -1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 1.0f,
            -1.0f,  1.0f,  0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 0.0f
        };

        m_vbo.create();
        m_vbo.bind();
        m_vbo.allocate(vertices, sizeof(vertices));

        m_vao.create();
        m_vao.bind();

        int posLoc = m_program->attributeLocation("position");
        int texLoc = m_program->attributeLocation("texCoord");

        m_program->enableAttributeArray(posLoc);
        m_program->setAttributeBuffer(posLoc, GL_FLOAT, 0, 2, 4 * sizeof(GLfloat));

        m_program->enableAttributeArray(texLoc);
        m_program->setAttributeBuffer(texLoc, GL_FLOAT, 2 * sizeof(GLfloat), 2, 4 * sizeof(GLfloat));

        m_vao.release();
        m_vbo.release();
    }

    CutieShell* m_shell = nullptr;
    ForeignToplevelHandleV1* m_toplevel = nullptr;

    QOpenGLShaderProgram* m_program = nullptr;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QSize m_viewportSize;
};
