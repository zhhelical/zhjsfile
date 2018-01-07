#include <QtGui>
#include <QtCore>
#include "qwt_painter.h"
#include "ptsymbol.h"

PtSymbol::PtSymbol(const QString& fileName):
		QwtSymbol(QwtSymbol::Image, QBrush(), QPen(Qt::NoPen), QSize()),
	d_image_path(fileName)
{
	d_pixmap.load(fileName);
	QSize size(8, 8);
	setSize(size);
}

PtSymbol::PtSymbol(const QPixmap& pixmap, const QString& fileName):
		QwtSymbol(QwtSymbol::Image, QBrush(), QPen(Qt::NoPen), QSize()),
	d_image_path(fileName)
{
	d_pixmap = QPixmap(pixmap);
	QSize size(8, 8);
	setSize(size);
}

void PtSymbol::drawSymbol(QPainter *painter, const QPointF &pos) const
{
	drawSymbols( painter, &pos, 1 );
}
void PtSymbol::drawSymbols(QPainter *painter, const QPointF *points, int numPoints) const
{
	if ( numPoints <= 0 )
		return;

	painter->save();
	painter->setBrush( brush() );
	painter->setPen( pen() );
	const QSize sz = size();
	const int sw = sz.width();
	const int sh = sz.height();
	const int sw2 = sz.width() / 2;
	const int sh2 = sz.height() / 2;
	QPointF pos = points[0];
	QPointF real_pos(pos.x()-sw2, pos.y()-sh2);
	painter->drawPixmap(real_pos, d_pixmap.scaled(8, 8));
	painter->restore();
}

/*void PtSymbol::drawSymbol(QPainter *p, const QRect& r) const
{
	QwtPainter::drawPixmap(p, r, d_pixmap);
}*/
