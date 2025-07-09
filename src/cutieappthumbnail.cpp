#include "cutieappthumbnail.h"
#include <QOpenGLTexture>
#include <QOpenGLFunctions>

// Internal FBO Renderer
class AppThumbnailRenderer : public QQuickFramebufferObject::Renderer {
public:
    AppThumbnailRenderer(CutieShell* shell, ForeignToplevelHandleV1* toplevel)
        : m_shell(shell), m_toplevel(toplevel) {}

    void render() override {
        if (!m_shell || !m_toplevel)
            return;

        QOpenGLTexture* texture = m_shell->textureForToplevel(m_toplevel);
        if (!texture)
            return;

        texture->bind();

        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        f->glEnable(GL_TEXTURE_2D);
        f->glBegin(GL_QUADS);

        f->glTexCoord2f(0.0f, 1.0f); f->glVertex2f(-1.0f, -1.0f);
        f->glTexCoord2f(1.0f, 1.0f); f->glVertex2f( 1.0f, -1.0f);
        f->glTexCoord2f(1.0f, 0.0f); f->glVertex2f( 1.0f,  1.0f);
        f->glTexCoord2f(0.0f, 0.0f); f->glVertex2f(-1.0f,  1.0f);

        f->glEnd();

        texture->release();

        update(); // ensure continuous live update
    }

private:
    CutieShell* m_shell = nullptr;
    ForeignToplevelHandleV1* m_toplevel = nullptr;
};

// --------- CutieAppThumbnail implementation ----------

CutieAppThumbnail::CutieAppThumbnail(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
}

QQuickFramebufferObject::Renderer* CutieAppThumbnail::createRenderer() const {
    return new AppThumbnailRenderer(m_wlc, m_toplevel);
}

CutieShell* CutieAppThumbnail::wlc() {
    return m_wlc;
}

void CutieAppThumbnail::setWlc(CutieShell* wlc) {
    if (m_wlc == wlc)
        return;
    m_wlc = wlc;
    emit wlcChanged(m_wlc);
    update(); // ensure redraw
}

ForeignToplevelHandleV1* CutieAppThumbnail::toplevel() {
    return m_toplevel;
}

void CutieAppThumbnail::setToplevel(ForeignToplevelHandleV1* toplevel) {
    if (m_toplevel == toplevel)
        return;
    m_toplevel = toplevel;
    emit toplevelChanged(m_toplevel);
    update(); // ensure redraw
}
