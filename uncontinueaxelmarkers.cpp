#include <QtCore>
#include <QtGui>
#include <qwt_scale_draw.h>
#include "uncontinueaxelmarkers.h"
#include "spcdatabase.h"

UncontinueAxelMarkers::UncontinueAxelMarkers(QWidget * parent)
: QwtScaleDraw()
{
	setTickLength(QwtScaleDiv::MajorTick, 5);
	setTickLength(QwtScaleDiv::MinorTick, 0);
	setTickLength(QwtScaleDiv::MediumTick, 0);
}
	
UncontinueAxelMarkers::~UncontinueAxelMarkers()
{}

void UncontinueAxelMarkers::setRealAxelMarkers(const QList<double> & maker_list)
{
	tick_labels = maker_list;
}

void UncontinueAxelMarkers::resetXaxisTicksLbls(const QString & from_tbl, const QFont & font, double val, SpcDataBase * db, bool time_num)
{
 	QwtText & resert_str = const_cast<QwtText &>(tickLabel(font, val)); 
	if (time_num)
	{
		QString var_lbl("groupnum");
		QString var = QString("%1").arg(val);
		QStringList num_line;
		db -> getDataesFromTable(from_tbl, var_lbl, num_line, var);	
		resert_str = num_line.back();
	}
	else
		resert_str = QwtText(QString("%1").arg(val));
}

const QwtText & UncontinueAxelMarkers::chkTickLabel(const QFont & font, double value)
{
	return tickLabel(font, value);
}

QwtScaleDiv UncontinueAxelMarkers::axisesScaleDiv() const
{
	QList<double> ticks[QwtScaleDiv::NTickTypes];
	QList<double> &majorTicks = ticks[QwtScaleDiv::MajorTick];
	for (int i = 0; i < tick_labels.size(); i++)
		majorTicks += i;
	QwtScaleDiv scaleDiv(majorTicks.first(), majorTicks.last() , ticks);
	return scaleDiv;	
}

QwtText UncontinueAxelMarkers::label(double value) const
{
	QString temp;	
	temp = QString("%1").arg(tick_labels.at(value));
		temp = QString("%1").arg(tick_labels.at(value));  
	return temp;
}

