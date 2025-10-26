#include "cutieappthumbnail.h"

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
	if (m_wlc && m_toplevel && !copying) {
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
	m_frame = m_wlc->getThumbnail(m_toplevel);
	connect(m_frame, &ScreencopyFrameV1::ready, this,
		&CutieAppThumbnail::onReady);
	connect(m_frame, &ScreencopyFrameV1::failed, this,
		&CutieAppThumbnail::onFailed);
}

void CutieAppThumbnail::onReady(QImage image)
{
	m_image = image;
	update(QRect(QPoint(), textureSize()));
	copying = false;
}

void CutieAppThumbnail::onFailed()
{
	copying = false;
}

void CutieAppThumbnail::onThumbnailDamage(void *object)
{
	if (!m_toplevel)
		return;
	ForeignToplevelHandleV1 *handle =
		m_toplevel->manager()->handleFromObject(object);
	if (handle == m_toplevel && !copying) {
		copying = true;
		QTimer::singleShot(50, this, &CutieAppThumbnail::newFrame);
	}
}
