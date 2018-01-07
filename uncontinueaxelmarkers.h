#ifndef UNCONTINUEAXELMARKERS_H
#define UNCONTINUEAXELMARKERS_H
#include <QtCore>
#include <QtGui>
#include "qwt_scale_draw.h"

class SpcDataBase;
class UncontinueAxelMarkers: public QwtScaleDraw
{
public:
	UncontinueAxelMarkers(QWidget * parent = 0);
	~UncontinueAxelMarkers();
	void setRealAxelMarkers(const QList<double> & maker_list);
	void resetXaxisTicksLbls(const QString & from_tbl, const QFont & font, double val, SpcDataBase * db, bool time_num);
	const QwtText & chkTickLabel(const QFont & font, double value);	
	const QList<double> & currentRealMarkers();
	QwtScaleDiv axisesScaleDiv() const;

private:
	QwtText label(double value) const;
	QList<double> tick_labels;
};

#endif
