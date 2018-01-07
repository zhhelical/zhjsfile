#ifndef PLOT_H
#define PLOT_H
#include <QtGui>
#include "qwt_plot.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_curve.h"

class PtSymbol;
class SpcNum;
class SpcDataBase;
class Plot : public QwtPlot
{
	Q_OBJECT
public:
	Plot(const QwtText &title, SpcDataBase * db, QWidget * parent = 0);
	~Plot();
	void setPlotShowingVars(const QwtText & title, double ratio);
	void resetForMultiPlots(QString & setstr);
	void setCurrentPtNumber(int number);
	void plotXrangeForSingleMode(QPair<int, int> & range);
	void setNewValueForCtrlLine(double up, double low);
	void appendNewMarker();
	void deleteLastPoint();
	void clearCurrentShowing();
	void setPlotDeployment(const QString & engi, QStandardItem * deploy_item);
	void setPlotTitle(const QVariant & title);
	void toggleAreaLinesShowingState();
	void toggleCtrlLinesShowingState();
	void showHideDotsLines();
	void toggleXaxisLabels();
	bool chkAreaLinesShowingState();
	bool chkCtrlLinesShowingState();
	bool chkDotsLinesShowingState();
	bool chkXaxisTimeNumShow();
	double relatedUpCtrlLimit();
	double relatedLowCtrlLimit();
	QwtPlotMarker * ctrlLineShowPtr();
	QwtPlotMarker * areaLineShowPtr();
	QwtPlotCurve * dotsLineShowPtr();
	const QString & relatedPtJudgedState(int num);
	QList<double> & ptJudgeStack(int type);
	QMap<int, QString> & relatedPtJudgedState();

signals:
	void startRelatedDailyRunning(Plot * sending, QList<double> & val);

public slots:
	void setCanvasColor(const QString & color);

private:
	void initCtrlLine();
	void clearAllMemoryPtsDataes();
	void fileCtrLineShow(double up, double low);
	void emptyFileCtrLineShow();
	void continuedPtsShow(bool multi);
	void continuedLineShow();
	void uncontinuedLineShow(const QList<QwtPlotMarker *> & plot_markers);
	void initJudgLists(const QString & description);
	void initRelatedDataState(const QString & description);
	void redefPtsColorForNewAvrPt();
	void redefPtsColorForNewDevPt();
	void redefPtsColorForNewRngPt();
	void relistPtsValueStacks(double new_val);
	PtSymbol * ptColorDef(int type);
//	QwtScaleDraw defineXaxisScaleDraw(const QList<double> & markers) const;
	int all_pts;
	double b_left;
	double b_right;
	QString plot_title;
	QString dest_table;	
	QwtPlotMarker * l_upCtrl;
	QwtPlotMarker * l_upMiddleCtrl1; // here syn promised  = 0
	QwtPlotMarker * l_upMiddleCtrl2;
	QwtPlotMarker * l_middleCtrl;
	QwtPlotMarker * l_underMidCtrl1;
	QwtPlotMarker * l_underMidCtrl2;
	QwtPlotMarker * l_underMidText;
	QwtPlotMarker * l_underCtrl;
	QwtPlotCurve * line;	
	QList<double> three_avrJud;
	QList<double> five_avrJud;
	QList<double> six_avrJud;
	QList<double> eight_devJud;
	QList<double> nine_avrJud;
	QList<double> nine_devJud;
	QList<double> fourteen_avrJud;
	QList<double> fourteen_devJud;
	QList<double> fifteen_devJud;
	QList<QPair<int, QDateTime> > xaxel_pairs;
	QList<QwtPlotMarker *> pt_markers;
	QMap<int, QString> relatedDataState;
	QList<double> judging_list;
	QList<double> judged_list;
	SpcDataBase * db_source;
};

#endif
