#include <QtGui>
#include <QtCore>
#include "qwt_plot_layout.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_curve.h"
#include "qwt_column_symbol.h"
#include "qwt_series_data.h"
#include "qwt_plot_canvas.h"
#include "qwt_text.h"
#include "qwt_text_label.h"
#include "toolsplot.h"
#include "ptsymbol.h"
#include "spcnum.h"
#include "spcdatabase.h"
#include "tablemanagement.h"
#include "dataselectmodel.h"
#include "editlongtoolsviewer.h"

PlotMarksDraw::PlotMarksDraw(const QString & lbl, SpcDataBase * db)
:QwtScaleDraw()
{
	setTickLength(QwtScaleDiv::MajorTick, 5);
	setLabelRotation(0);
	setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter );
	setSpacing(15);
	aixs_lbl = lbl;
	db_source = db;
}

PlotMarksDraw::~PlotMarksDraw()
{}

QwtText PlotMarksDraw::label(double value) const
{
	QString r_str;
	if (aixs_lbl == QObject::tr("概率纸纵坐标"))
	{
		if (value == 1.19)
			r_str = "0.01";
		else if (value == 1.81)
			r_str = "0.1";
		else if (value == 2.58)
			r_str = "1";
		else if (value == 3.26)
			r_str = "5";
		else if (value == 3.62)
			r_str = "10";
		else if (value == 4.06)
			r_str = "20";
		else if (value == 4.38)
			r_str = "30";
		else if (value == 4.65)
			r_str = "40";
		else if (value == 4.9)
			r_str = "50";
		else if (value == 5.15)
			r_str = "60";
		else if (value == 5.42)
			r_str = "70";
		else if (value == 5.74)
			r_str = "80";
		else if (value == 6.18)
			r_str = "90";
		else if (value == 6.54)
			r_str = "95";
		else if (value == 7.22)
			r_str = "99";
		else if (value == 7.99)
			r_str = "99.9";
		else if (value == 8.62)
			r_str = "99.99";
	}
	return r_str;
}

Histogram::Histogram(const QString &title, const QColor &symbolColor):
QwtPlotHistogram(title)
{
	setStyle(QwtPlotHistogram::Columns);
	setColor(symbolColor);
}

Histogram::~Histogram()
{}

void Histogram::setColor(const QColor &symbolColor)
{
	QColor color = symbolColor;
	color.setAlpha(180);
	setPen(QPen(Qt::black));
	setBrush(QBrush(color));
	QwtColumnSymbol *symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
	symbol->setFrameStyle(QwtColumnSymbol::Raised);
	symbol->setPalette(QPalette(color));
	setSymbol(symbol);
}

void Histogram::setValues(QStandardItemModel * model, const int *values)
{
	QVector<QwtIntervalSample> samples(model->rowCount()-1);
	for (int i = 1; i < model->rowCount(); i++)
	{
		QwtInterval interval(model->data(model->index(i, 1)).toDouble(), model->data(model->index(i, 1)).toDouble()+model->data(model->index(0, 1)).toDouble());
		interval.setBorderFlags(QwtInterval::ExcludeMaximum);   
		samples.push_back(QwtIntervalSample(values[i-1], interval));
	}
	setSamples(samples);
}

HistogramPlot::HistogramPlot(const QString & project, QWidget * parent, SpcNum * spc, SpcDataBase * resource)
: QwtPlot(parent), freqArray(0), dataes_model(0), view_model(0)
{
	current_proj = project;
	inspector = spc;
	db_source = resource;
	QwtText title(tr("直方图"));
	setTitle(title);
	setAxisTitle(QwtPlot::yLeft, tr("频数"));
	QPalette p(palette());
	p.setColor(QPalette::Background, QColor("#008B8B"));
	canvas()->setPalette(p);
}

HistogramPlot::~HistogramPlot()
{
	if (freqArray)
		delete []freqArray;
}

void HistogramPlot::setPlotShowingVars(const QwtText & title, double ratio)
{
	if (ratio < 0.5)
		ratio = 0.5;	
	if (!title.text().isEmpty())
	{
		const_cast<QwtText &>(title).setFont(QFont("宋体", 12*ratio));
		setTitle(title);
	}
	QwtText axis_text = axisTitle(QwtPlot::yLeft);
	axis_text.setFont(QFont("宋体", 12*ratio));
	setAxisTitle(QwtPlot::yLeft, axis_text);
	axisScaleDraw(QwtPlot::yLeft) -> setTickLength(QwtScaleDiv::MajorTick, 5*ratio);
	axisScaleDraw(QwtPlot::yLeft) -> setTickLength(QwtScaleDiv::MediumTick, 0);
	axisScaleDraw(QwtPlot::yLeft) -> setTickLength(QwtScaleDiv::MinorTick, 0);
	axisScaleDraw(QwtPlot::xBottom) -> setTickLength(QwtScaleDiv::MajorTick, 5*ratio);
	axisScaleDraw(QwtPlot::xBottom) -> setTickLength(QwtScaleDiv::MediumTick, 0);
	axisScaleDraw(QwtPlot::xBottom) -> setTickLength(QwtScaleDiv::MinorTick, 0);
	setAxisFont(QwtPlot::xBottom, QFont("宋体", 8*ratio));
	setAxisFont(QwtPlot::yLeft, QFont("宋体", 8*ratio));
}

void HistogramPlot::createHistoForInitCpks(QStandardItemModel * dataes_source)
{
	view_model = dataes_source;
	dataes_model = new QStandardItemModel(this);
	QMap<int, QList<double> > g_map;
	for (int i = 0; i < view_model->rowCount(); i++)
	{
		for (int j = 0; j < view_model->columnCount(); j++)
			g_map[i+1] << view_model->data(view_model->index(i, j)).toDouble();
	}
	inspector -> cpkHistogramTranslation(g_map, dataes_model);
	showHistogramPlot();	
}

void HistogramPlot::createHistoForItem(QStandardItem * item_info)
{
	if (item_info->text() == tr("直方图"))
	{
		setCpkDataesFromDb();
		return;
	}
	DataSelectModel * infoes_model = qobject_cast<DataSelectModel *>(item_info->model());
	TableManagement * infoes_view = infoes_model->nestedInTreeView();
	QString daily_table = current_proj.split(tr("，。，")).at(0);
	int from_group = 0, to_group = 0;
	QList<QStandardItem *> s_children;
	infoes_model -> findAllChildItems(item_info, s_children);
	QStandardItem * from = 0;
	QStandardItem * to = 0;
	for (int i = s_children.size()-1; i > 0; i--)
	{
		if (s_children.at(i)->text().contains(tr("到：")) && !to)
			to = s_children.at(i);				  
		if (s_children.at(i)->text().contains(tr("从：")) && !from)
			from = s_children.at(i);
		if (from && to)
			break;			  
	}
	if (from->text().contains(tr("时间")))
	{
		QDateTime from_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(from).at(0), Qt::ISODate);
		from_group = db_source->dataFromTableByTime(from_time, daily_table, "groupnum").toInt();  
	}
	else
		from_group = infoes_view->hashSavedRelatedItemInfo().value(from).at(0).toInt();
	if (to)
	{
		if (to->text().contains(tr("时间")))
		{
			QDateTime to_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(to).at(0), Qt::ISODate);
			to_group = db_source->dataFromTableByTime(to_time, daily_table, "groupnum").toInt();	  
		}
		else
			to_group = infoes_view->hashSavedRelatedItemInfo().value(to).at(0).toInt();
	}	
	else
		to_group = db_source->lastDataFromTable(daily_table, "groupnum").toInt();
	setSelectedDataes(from_group, to_group);
}

void HistogramPlot::setPlotTitle(const QVariant & title)
{
 	QwtText new_title(title.toString()); 
	if (!title.toString().isEmpty())
		setTitle(new_title);	
}

void HistogramPlot::toggleAreaLinesShowingState(bool init)
{
	if (init)
	{
		foreach (QwtPlotMarker * p_marker, marker_lines)
		{
			p_marker -> setLineStyle(QwtPlotMarker::HLine);
			p_marker -> setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));		  
			p_marker -> attach(this);
		}
	}
	else
	{
		foreach (QwtPlotMarker * p_marker, marker_lines)  
			p_marker -> setVisible(!p_marker->isVisible());
		replot();
	}
}

bool HistogramPlot::chkAreaLinesShowingState()
{
	return marker_lines.at(0)->isVisible();
}

QStandardItemModel * HistogramPlot::currentCpkDataesModel()
{
	return view_model;
}

void HistogramPlot::showItem(QwtPlotItem * item, bool on)
{
	item->setVisible(on);
}

void HistogramPlot::setSelectedDataes(int from, int to)
{
	dataes_model = new QStandardItemModel(this);
	QString daily_table = current_proj.split(tr("，。，")).at(0);	
	inspector -> selectedDataesModel(from, to, 8, dataes_model, daily_table);
	if (freqArray)
		delete []freqArray;
	freqArray = new int[dataes_model->rowCount()-1];
	for (int j = 1; j < dataes_model->rowCount(); j++)
		freqArray[j-1] = dataes_model->data(dataes_model->index(j, 2)).toInt();
//	plotLayout()->setAlignCanvasToScales(true);
	setAxisScaleDiv(QwtPlot::xBottom, XaxisScaleDiv());
	Histogram * data_histogram = new Histogram(QString(), Qt::darkGreen);
	data_histogram->setValues(dataes_model, freqArray);
	data_histogram->attach(this);
	replot();
}
	
void HistogramPlot::setCpkDataesFromDb()
{
	dataes_model = new QStandardItemModel(this);
	inspector -> cpkInitModelFromDb(7, current_proj, dataes_model);
	showHistogramPlot();
}

void HistogramPlot::showHistogramPlot()
{
	if (freqArray)
		delete []freqArray;
	freqArray = new int[dataes_model->rowCount()-1];
	for (int j = 1; j < dataes_model->rowCount(); j++)
	{
		freqArray[j-1] = dataes_model->data(dataes_model->index(j, 2)).toInt();
		QwtPlotMarker * top_marker = new QwtPlotMarker();
		top_marker -> setValue(0.0, freqArray[j-1]);
		marker_lines << top_marker;
	}
	setAxisScaleDiv(QwtPlot::xBottom, XaxisScaleDiv());
	Histogram * data_histogram = new Histogram(QString(), Qt::darkGreen);
	data_histogram -> setValues(dataes_model, freqArray);
	data_histogram -> attach(this);
	toggleAreaLinesShowingState(true);
	replot();  
}

QwtScaleDiv HistogramPlot::XaxisScaleDiv() const
{
	QList<double> ticks[QwtScaleDiv::NTickTypes];
	QList<double> &majorTicks = ticks[QwtScaleDiv::MajorTick];
	majorTicks += dataes_model->data(dataes_model->index(1, 1)).toDouble();
	for ( int i = 1; i < dataes_model->rowCount(); i++ )
		majorTicks += majorTicks.last() + dataes_model->data(dataes_model->index(0, 1)).toDouble();
	QwtScaleDiv scaleDiv(majorTicks.first(), majorTicks.last(), ticks);
	return scaleDiv;
}

GaussPlot::GaussPlot(const QString & project, QWidget * parent, SpcNum * spc, SpcDataBase * resource)
: QwtPlot(parent), dataes_model(0), view_model(0)
{
	current_proj = project;
	inspector = spc;
	db_source = resource;
	QwtText title(tr("概率纸"));
	setTitle(title);
	QPalette p(palette());
	p.setColor(QPalette::Background, QColor("#008B8B"));
	canvas()->setPalette(p);
	y_markerpair = inspector->GaussPlotPair();	
}

GaussPlot::~GaussPlot()
{}

void GaussPlot::setPlotShowingVars(const QwtText & title, double ratio)
{
	if (ratio < 0.5)
		ratio = 0.5;	
	if (!title.text().isEmpty())
	{
		const_cast<QwtText &>(title).setFont(QFont("宋体", 12*ratio));
		setTitle(title);
	}
	axisScaleDraw(QwtPlot::yLeft) -> setTickLength(QwtScaleDiv::MajorTick, 5*ratio);
	axisScaleDraw(QwtPlot::yLeft) -> setTickLength(QwtScaleDiv::MediumTick, 0);
	axisScaleDraw(QwtPlot::yLeft) -> setTickLength(QwtScaleDiv::MinorTick, 0);
	axisScaleDraw(QwtPlot::xBottom) -> setTickLength(QwtScaleDiv::MajorTick, 5*ratio);
	axisScaleDraw(QwtPlot::xBottom) -> setTickLength(QwtScaleDiv::MediumTick, 0);
	axisScaleDraw(QwtPlot::xBottom) -> setTickLength(QwtScaleDiv::MinorTick, 0);
	setAxisFont(QwtPlot::xBottom, QFont("宋体", 8*ratio));
	setAxisFont(QwtPlot::yLeft, QFont("宋体", 8*ratio));
	if (ratio > 1.0)
		axisScaleDraw(QwtPlot::yLeft) -> setSpacing(15);
	else
		axisScaleDraw(QwtPlot::yLeft) -> setSpacing(15+15*ratio);
}

void GaussPlot::createGaussForInitCpks(QStandardItemModel * dataes_source)
{
	view_model = dataes_source;
	dataes_model = new QStandardItemModel(this);
	QMap<int, QList<double> > g_map;
	for (int i = 0; i < view_model->rowCount(); i++)
	{
		for (int j = 0; j < view_model->columnCount(); j++)
			g_map[i+1] << view_model->data(view_model->index(i, j)).toDouble();
	}
	inspector -> cpkGaussTranslation(g_map, dataes_model);
	showGaussPlot();	
}

void GaussPlot::createPlotFromItem(QStandardItem * dataes_item)
{
 	if (dataes_item->text() == tr("正态概率纸"))
	{
		setCpkDataesFromDb();
		return;
	} 
	DataSelectModel * infoes_model = qobject_cast<DataSelectModel *>(dataes_item->model());
	TableManagement * infoes_view = infoes_model->nestedInTreeView();
	QString daily_table = current_proj.split(tr("，。，")).at(0);
	int from_group = 0, to_group = 0;
	QList<QStandardItem *> s_children;
	infoes_model -> findAllChildItems(dataes_item, s_children);
	QStandardItem * from = 0;
	QStandardItem * to = 0;
	for (int i = s_children.size()-1; i > 0; i--)
	{
		if (s_children.at(i)->text().contains(tr("到：")) && !to)
			to = s_children.at(i);				  
		if (s_children.at(i)->text().contains(tr("从：")) && !from)
			from = s_children.at(i);
		if (from && to)
			break;			  
	}
	if (from->text().contains(tr("时间")))
	{
		QDateTime from_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(from).at(0), Qt::ISODate);
		from_group = db_source->dataFromTableByTime(from_time, daily_table, "groupnum").toInt();  
	}
	else
		from_group = infoes_view->hashSavedRelatedItemInfo().value(from).at(0).toInt();
	if (to)
	{
		if (to->text().contains(tr("时间")))
		{
			QDateTime to_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(to).at(0), Qt::ISODate);
			to_group = db_source->dataFromTableByTime(to_time, daily_table, "groupnum").toInt();	  
		}
		else
			to_group = infoes_view->hashSavedRelatedItemInfo().value(to).at(0).toInt();
	}
	setSelectedDataes(from_group, to_group);		
}

void GaussPlot::setPlotTitle(const QVariant & title)
{
 	QwtText new_title(title.toString()); 
	if (!title.toString().isEmpty())
		setTitle(new_title);
}

void GaussPlot::freshPerformLines()
{
	foreach (QwtPlotMarker * p_marker, marker_lines)
	{
		if (p_marker->yValue()==1.19 || p_marker->yValue()==1.81 || p_marker->yValue()==2.58 || p_marker->yValue()==3.26 || p_marker->yValue()==3.62 || p_marker->yValue()==4.06 || p_marker->yValue()==4.38 || p_marker->yValue()==4.65 || p_marker->yValue()==4.9 || p_marker->yValue()==5.15 || p_marker->yValue()==5.42 || p_marker->yValue()==5.74 || p_marker->yValue()==6.18 || p_marker->yValue()==6.54 || p_marker->yValue()==7.22 || p_marker->yValue()== 7.99 || p_marker->yValue()==8.62)
			p_marker -> setVisible(!p_marker->isVisible());
	}
	replot();	  
}

void GaussPlot::freshCtrlLines()
{
 	foreach (QwtPlotMarker * p_marker, marker_lines)
	{
		if (p_marker->yValue()==1.19 || p_marker->yValue()==1.81 || p_marker->yValue()==2.58 || p_marker->yValue()==3.26 || p_marker->yValue()==3.62 || p_marker->yValue()==4.06 || p_marker->yValue()==4.38 || p_marker->yValue()==4.65 || p_marker->yValue()==4.9 || p_marker->yValue()==5.15 || p_marker->yValue()==5.42 || p_marker->yValue()==5.74 || p_marker->yValue()==6.18 || p_marker->yValue()==6.54 || p_marker->yValue()==7.22 || p_marker->yValue()== 7.99 || p_marker->yValue()==8.62)
			continue;
		p_marker -> setVisible(!p_marker->isVisible());
	}
	replot(); 
}
	
bool GaussPlot::chkPerformLinesShowingState()
{
	bool pf_return = false;
	foreach (QwtPlotMarker * p_marker, marker_lines)
	{
		if (p_marker->yValue()==1.19)
		{
			pf_return = p_marker->isVisible();
			break;
		}
	}
	return pf_return;
}
	
bool GaussPlot::chkCtrlLinesShowingState()
{
	bool ctrl_return = false;
  	foreach (QwtPlotMarker * p_marker, marker_lines)
	{
		if (p_marker->yValue()!=1.19 && p_marker->yValue()!=1.81 && p_marker->yValue()!=2.58 && p_marker->yValue()!=3.26 && p_marker->yValue()!=3.62 && p_marker->yValue()!=4.06 && p_marker->yValue()!=4.38 && p_marker->yValue()!=4.65 && p_marker->yValue()!=4.9 && p_marker->yValue()!=5.15 && p_marker->yValue()!=5.42 && p_marker->yValue()!=5.74 && p_marker->yValue()!=6.18 && p_marker->yValue()!=6.54 && p_marker->yValue()!=7.22 && p_marker->yValue()!= 7.99 && p_marker->yValue()!=8.62)
		{
			ctrl_return = p_marker->isVisible();
			break;
		}
	} 
	return ctrl_return;
}

QStandardItemModel * GaussPlot::currentCpkDataesModel()
{
	return view_model;
}

void GaussPlot::setDataesMarkers()
{
	pt_markers.clear();
	for (int j = 1; j < dataes_model->rowCount(); j++)
	{
		QwtPlotMarker * pt_marker = new QwtPlotMarker();
		pt_marker -> setValue(dataes_model->data(dataes_model->index(j, 0)).toDouble(), inspector->normsinv(dataes_model->data(dataes_model->index(j, 2)).toDouble()));
		pt_marker -> setSymbol(ptColorDef(1));
		pt_marker -> attach(this);
		pt_markers << pt_marker;
	}
}

void GaussPlot::setSelectedDataes(int from, int to)
{
	dataes_model = new QStandardItemModel(this);
	QString daily_table = current_proj.split(tr("，。，")).at(0);
	inspector -> selectedDataesModel(from, to, 9, dataes_model, daily_table);
	showGaussPlot();
}

void GaussPlot::attachRegressorLine()
{
	QwtPlotCurve * regressor_line = new QwtPlotCurve();
	QList<double> xaxel_vals;
	QList<double> yaxel_vals;
	for (int i = 1; i < dataes_model->rowCount(); i++)
	{
		xaxel_vals << dataes_model->data(dataes_model->index(i, 0)).toDouble();
		yaxel_vals << inspector->normsinv(dataes_model->data(dataes_model->index(i, 2)).toDouble());
	}
	QPointF pt1(dataes_model->data(dataes_model->index(1, 0)).toDouble(), inspector->normalProbabilityYaxleValue(xaxel_vals, yaxel_vals, dataes_model->data(dataes_model->index(1, 0)).toDouble()));
	QPointF pt2(dataes_model->data(dataes_model->index(dataes_model->rowCount()-1, 0)).toDouble(), inspector->normalProbabilityYaxleValue(xaxel_vals, yaxel_vals, dataes_model->data(dataes_model->index(dataes_model->rowCount()-1, 0)).toDouble()));
	QVector<QPointF> pts;
	pts << pt1 << pt2;		
	regressor_line -> setSamples(pts);
	regressor_line -> attach(this);
}

void GaussPlot::showGaussPlot()
{
	setYmarkerLines();
	showRelatedYlines();
	setAxisScaleDiv(QwtPlot::xBottom, XaxisScaleDiv());
	setAxisScaleDiv(QwtPlot::yLeft, YaxisScaleDiv());	
	setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
	setAxisScaleDraw(QwtPlot::yLeft, new PlotMarksDraw(tr("概率纸纵坐标")));
	axisScaleDraw(QwtPlot::yLeft) -> setSpacing(30);
	attachRegressorLine();
	setDataesMarkers();	
	replot();	
}

void GaussPlot::setCpkDataesFromDb()
{
	dataes_model = new QStandardItemModel(this);
	inspector -> cpkInitModelFromDb(8, current_proj, dataes_model);
	showGaussPlot();
}

void GaussPlot::showRelatedYlines()
{
	foreach (QwtPlotMarker * p_marker, marker_lines)
		p_marker -> attach(this);		
}

void GaussPlot::setYmarkerLines()
{
	marker_lines.clear();
	int n = y_markerpair.size();
	for (int i = 0; i < n; i++)
	{
		QwtPlotMarker * marker = new QwtPlotMarker();
		marker -> setValue(0.0, y_markerpair.at(i).first);
		marker -> setLineStyle(QwtPlotMarker::HLine);
		marker -> setLinePen(QPen(Qt::yellow, 0, Qt::DashDotLine));
		marker_lines << marker;
	}
}

PtSymbol * GaussPlot::ptColorDef(int type)
{
	if (type == 0)
	{
		PtSymbol * mid_sym = new PtSymbol(":/images/mid.png");
		return mid_sym;
	}
	else if (type == 1)
	{
		PtSymbol * red_sym = new PtSymbol(":/images/red.png");
		return red_sym;		
	}
	else
	{
		PtSymbol * green_sym = new PtSymbol(":/images/green.png");
		return green_sym;
	}	
}

QwtScaleDiv GaussPlot::XaxisScaleDiv() const
{
	QList<double> ticks[QwtScaleDiv::NTickTypes];
	QList<double> &majorTicks = ticks[QwtScaleDiv::MajorTick];
	majorTicks += dataes_model->data(dataes_model->index(1, 0)).toDouble();
	for (int i = 0; i < dataes_model->rowCount(); i++)
		majorTicks += majorTicks.last() + dataes_model->data(dataes_model->index(2, 0)).toDouble()-dataes_model->data(dataes_model->index(1, 0)).toDouble();
	QwtScaleDiv scaleDiv(majorTicks.first(), majorTicks.last(), ticks);
	return scaleDiv;
}

QwtScaleDiv GaussPlot::YaxisScaleDiv() const
{
	QList<double> ticks[QwtScaleDiv::NTickTypes];
	QList<double> &majorTicks = ticks[QwtScaleDiv::MajorTick];
	for ( int i = 0; i < y_markerpair.size(); i++ )
	{
		if (y_markerpair.at(i).second==0.01 || y_markerpair.at(i).second==0.1 || y_markerpair.at(i).second==1 || y_markerpair.at(i).second==5 || y_markerpair.at(i).second==10 || y_markerpair.at(i).second==20 || y_markerpair.at(i).second==30 || y_markerpair.at(i).second==40 || y_markerpair.at(i).second==50 || y_markerpair.at(i).second==60 || y_markerpair.at(i).second==70 || y_markerpair.at(i).second==80 || y_markerpair.at(i).second==90 || y_markerpair.at(i).second==95 || y_markerpair.at(i).second==99 ||y_markerpair.at(i).second==99.9 || y_markerpair.at(i).second==99.99)
			majorTicks += y_markerpair.at(i).first;
		else
			continue;
	}
	QwtScaleDiv scaleDiv(majorTicks.first(), majorTicks.last(), ticks);
	return scaleDiv;	
}
