#ifndef PTSYMBOL_H
#define PTSYMBOL_H
//#include <QtCore>
//#include <QtGui>
#include <qwt_symbol.h>

class PtSymbol: public QwtSymbol
{
public:
	PtSymbol(const QString& fileName);
	PtSymbol(const QPixmap& pixmap, const QString& fileName = QString());

	virtual void drawSymbols( QPainter *, const QPointF *, int numPoints ) const;
	void drawSymbol( QPainter *, const QPointF & ) const;

	QPixmap pixmap(){return d_pixmap;};
	QString imagePath(){return d_image_path;};

private:
	QString d_image_path;
	QPixmap d_pixmap;
};

#endif
