#include "cutieappthumbnail.h"
#include <QTimer>
#include <QDebug>

CutieAppThumbnail::CutieAppThumbnail(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

CutieShell *CutieAppThumbnail::wlc()
{
    return m_wlc;
}

void CutieAppThumbnail::setWlc(CutieShell *wlc)
{
    if (m_wlc == wlc)
        return;
    m_wlc = wlc;
    connect(m_wlc, &CutieShell::thumbnailDamage, this,
            &CutieAppThumbnail::onThumbnailDamage);
    emit wlcChanged(m_wlc);
}

ForeignToplevelHandleV1 *CutieAppThumbnail::toplevel()
{
    return m_toplevel;
}

void CutieAppThumbnail::setToplevel(ForeignToplevelHandleV1 *toplevel)
{
    if (m_toplevel == toplevel)
        return;
    m_toplevel = toplevel;
    emit toplevelChanged(m_toplevel);

    if (m_wlc && m_toplevel && !copying && !m_frozen) {
        copying = true;
        QTimer::singleShot(50, this, &CutieAppThumbnail::newFrame);
    }
}

void CutieAppThumbnail::paint(QPainter *painter)
{
    painter->drawImage(QRectF(QPointF(), size()), m_image);
}

void CutieAppThumbnail::newFrame()
{
    if (!m_wlc || !m_toplevel || m_frozen)
        return;

    m_frame = m_wlc->getThumbnail(m_toplevel);
    connect(m_frame, &ScreencopyFrameV1::ready, this,
            &CutieAppThumbnail::onReady);
    connect(m_frame, &ScreencopyFrameV1::failed, this,
            &CutieAppThumbnail::onFailed);
}

void CutieAppThumbnail::onReady(QImage image)
{
	if (m_frozen) {
        copying = false;
                 // <-- FIX
        return;
    }

    m_image = image;
    update(QRect(QPoint(), textureSize()));
    copying = false;
                 // <-- FIX
}

void CutieAppThumbnail::onFailed()
{
    copying = false;
                 // <-- FIX
}

void CutieAppThumbnail::onThumbnailDamage(void *object)
{
    if (!m_toplevel || m_frozen)
        return;

    ForeignToplevelHandleV1 *handle =
        m_toplevel->manager()->handleFromObject(object);
    if (handle == m_toplevel && !copying) {
        copying = true;
        QTimer::singleShot(50, this, &CutieAppThumbnail::newFrame);
    }
}

// --- Freeze / Resume ---
void CutieAppThumbnail::freeze()
{
	if (m_frozen)
        return;

	QTimer::singleShot(500, this, [this]() {
        // Only freeze if we weren't resumed during the 500ms delay.
        // This is a crucial check for this simplified approach!
        if (!m_frozen) {
            m_frozen = true;
            qDebug() << "module - CutieAppThumbnail - frozen (after 0.5s delay)";
        }
    });
	
}

void CutieAppThumbnail::resume()
{
    if (!m_frozen)
        return;

    m_frozen = false;
	qDebug() << "module - CutieAppThumbnail - resumed";

    if (m_toplevel && !copying)
        QTimer::singleShot(50, this, &CutieAppThumbnail::newFrame);
}
