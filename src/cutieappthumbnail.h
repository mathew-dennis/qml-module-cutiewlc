#pragma once

#include <QQuickFramebufferObject>
#include <wayland-cutie-shell.h>
#include <qt6-foreign-toplevel-management.h>

class CutieAppThumbnail : public QQuickFramebufferObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(CutieShell *wlc READ wlc WRITE setWlc NOTIFY wlcChanged)
    Q_PROPERTY(ForeignToplevelHandleV1 *toplevel READ toplevel WRITE setToplevel NOTIFY toplevelChanged)

public:
    CutieAppThumbnail(QQuickItem *parent = nullptr);
    Renderer *createRenderer() const override; // 🆕 Needed for FBO

    CutieShell *wlc();
    void setWlc(CutieShell *wlc);

    ForeignToplevelHandleV1 *toplevel();
    void setToplevel(ForeignToplevelHandleV1 *toplevel);

signals:
    void wlcChanged(CutieShell *wlc);
    void toplevelChanged(ForeignToplevelHandleV1 *toplevel);

private:
    CutieShell *m_wlc = nullptr;
    ForeignToplevelHandleV1 *m_toplevel = nullptr;
};
