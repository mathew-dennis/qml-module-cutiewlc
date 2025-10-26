#pragma once

#include <QImage>
#include <QPainterPath>
#include <QQuickPaintedItem>
#include <wayland-cutie-shell.h>
#include <qt6-foreign-toplevel-management.h>
#include <qt6-screencopy.h>

class CutieAppThumbnail : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(CutieShell *wlc READ wlc WRITE setWlc NOTIFY wlcChanged)
	Q_PROPERTY(ForeignToplevelHandleV1 *toplevel READ toplevel WRITE
			   setToplevel NOTIFY toplevelChanged)
public:
    CutieAppThumbnail(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;

    CutieShell *wlc();
    void setWlc(CutieShell *wlc);

    ForeignToplevelHandleV1 *toplevel();
    void setToplevel(ForeignToplevelHandleV1 *toplevel);

    // Freeze / Resume
    Q_INVOKABLE void freeze();
    Q_INVOKABLE void resume();

signals:
    void wlcChanged(CutieShell *wlc);
    void toplevelChanged(ForeignToplevelHandleV1 *toplevel);

private slots:
    void onReady(QImage image);
    void onFailed();
    void onThumbnailDamage(void *object);
    void newFrame();

private:
    CutieShell *m_wlc = nullptr;
    ForeignToplevelHandleV1 *m_toplevel = nullptr;
    ScreencopyFrameV1 *m_frame = nullptr;
    QImage m_image;
    bool copying = false;
    bool m_frozen = false;   // freeze/resume flag
};
