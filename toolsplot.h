#ifndef TOOLSPLOT_H
#define TOOLSPLOT_H
#include <QtGui>
#include <QtCore>
#include "qwt_plot.h"
#include "qwt_plot_histogram.h"
#include "qwt_scale_draw.h"

class EditLongToolsViewer;
class PtSymbol;
class QwtPlotMarker;
class SpcNum;
class SpcDataBase;
class PlotMarksDraw: public QwtScaleDraw
{
public:
	PlotMarksDraw(const QString & lbl, SpcDataBase * db = 0);
	~PlotMarksDraw();

private:
	QwtText label(double value) const;
	QString aixs_lbl;
	SpcDataBase * db_source;
};

class Histogram : public QwtPlotHistogram
{
public:
	Histogram(const QString &, const QColor &);
	~Histogram();
	void setColor(const QColor &);
	void setValues(QStandardItemModel * model, const int *);
};

class HistogramPlot : public QwtPlot
{
	Q_OBJECT
public:
	HistogramPlot(const QString & project, QWidget * parent = 0, SpcNum * spc = 0, SpcDataBase * resource = 0);	
	~HistogramPlot();
	void setPlotShowingVars(const QwtText & title, double ratio);
	void createHistoForInitCpks(QStandardItemModel * dataes_source);
	void createHistoForItem(QStandardItem * item_info);
	void setPlotTitle(const QVariant & title);
	void toggleAreaLinesShowingState(bool init = false);
	bool chkAreaLinesShowingState();
	QStandardItemModel * currentCpkDataesModel();	

private Q_SLOTS:
	void showItem(QwtPlotItem *, bool on);	
	
private:
	void setSelectedDataes(int from, int to);
	void setCpkDataesFromDb();
	void showHistogramPlot();
	QwtScaleDiv XaxisScaleDiv() const;
	double unit_accuracy;// for engi
	QString current_proj;
	int *freqArray;	
	QStandardItemModel * dataes_model;	
	QStandardItemModel * view_model;
	QList<QwtPlotMarker *> marker_lines;	
	SpcNum * inspector;
	SpcDataBase * db_source;
}; 

class GaussPlot : public QwtPlot
{
	Q_OBJECT
public:
	GaussPlot(const QString & project, QWidget * parent = 0, SpcNum * spc = 0, SpcDataBase * resource = 0);	
	~GaussPlot();
	void setPlotShowingVars(const QwtText & title, double ratio);
	void createGaussForInitCpks(QStandardItemModel * dataes_source);
	void createPlotFromItem(QStandardItem * dataes_item);
	void setPlotTitle(const QVariant & title);
	void freshPerformLines();
	void freshCtrlLines();
	bool chkPerformLinesShowingState();
	bool chkCtrlLinesShowingState();
	QStandardItemModel * currentCpkDataesModel();

private slots:
	void setDataesMarkers();

private:
	void setSelectedDataes(int from, int to);
	void attachRegressorLine();
	void showGaussPlot();
	void setCpkDataesFromDb();
	void showRelatedYlines();
	void setYmarkerLines();
	PtSymbol * ptColorDef(int type);
	QwtScaleDiv XaxisScaleDiv() const;
	QwtScaleDiv YaxisScaleDiv() const;
	QString current_proj;
	QList<QPair<double, double> > y_markerpair;
	QList<QwtPlotMarker *> marker_lines;
	QList<QwtPlotMarker *> pt_markers;
	QStandardItemModel * dataes_model;	
	QStandardItemModel * view_model;
	SpcNum * inspector;
	SpcDataBase * db_source;
};

#endif
