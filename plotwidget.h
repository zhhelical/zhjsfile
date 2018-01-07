#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H
#include <QtGui>

class SpcNum;
class SpcDataBase;
class LittleWidgetItem;
class LittleWidgetsView;
class TablesGroup;
class DataesTableView;
class Plot;
class HistogramPlot;
class GaussPlot;
class PlotWidget : public QWidget
{
	Q_OBJECT
public:
	PlotWidget(SpcNum * val, SpcDataBase * db, QWidget * parent = 0);
	~PlotWidget();
	void setEnginame(const QString & name = QString());//for multi
	void removeCoverPad();
	void setEndRect(const QRect & end);
	void setCloseBtnPtr(QPushButton * close_btn);
	void updateCloseBtnPos();
	void respondMultiDailyNewData(DataesTableView * t_daily);
	void deleteNewDataOnMultiDaily();
	void respondChangingAct(int type);
	void setRelatedCtrlValue(const QList<double> & vals);
	void plotShowDetailChanging(const QString & style, const QString & detail);
	void resetRelatedPlotVars(double ratio);
	void resetEndRect(int w_size, int h_size);
	void generateSingleToolplot(QStandardItemModel * cpk_source, bool h_g);
	void generateSingle(QStandardItem * guide_item, QStandardItem * pos_info, const QRect & size, LittleWidgetsView * p_view, LittleWidgetItem * item_view, TablesGroup * group, const QRect & orig_size = QRect());
	void resetRelatedToolPlot(QStandardItemModel * guide_ptr);
	void actionForNestedorTitle();
	void nestedPlotContentAction(const QString & action);
	bool hasColorPad();
	bool validForPlotChanging();
	bool isDataesEmpty();
	bool engiCtrlType();
	int currentEngiSamples();
	int currentEngiGroups();
	double upAvrCtrlLimit();
	double lowAvrCtrlLimit();
	double upDevCtrlLimit();
	double lowDevCtrlLimit();
	double upRngCtrlLimit();
	double lowRngCtrlLimit();
	double engiDefinedUpCtrl();
	double engiDefinedLowCtrl();
	QWidget * coverWidgetPtr();
	const QString & currentEngi();
	const QString & avrDataCurState(int num);
	const QString & devDataCurState(int num);
	const QString & rngDataCurState(int num);
	QRect & endRect();
	QStandardItem * singlePlotInfo();
	QPushButton * exsitedBtn();
	QWidget * respondMatchPtr(const QString & ptr_type);
	QList<double> & inputtedDailyDataes();
	QList<double> & avrDevJudgeStack(bool avr_dev, int type);
	QList<HistogramPlot *> & currentHistograms();
	QList<GaussPlot *> & currentGausses();
	QMap<int, QString> & relatedJudgedStateMap(int type);
	QMap<int, QList<double> > & ownedEngiTolDataes();

signals:
	void canvasColorChanging(const QString & color);
	void plotMoving(PlotWidget * plot, const QPoint & pt);
	void enableQmlBtnsForShowContents(QVariant a_modify, QVariant a_area, QVariant a_ast, QVariant a_dl, QVariant a_dlst, QVariant a_ctrl, QVariant a_ctrlst, QVariant a_ds, QVariant a_ts, QVariant a_dtst);

public slots:
	void startRelatedDailyCaling(Plot * w_plot, QList<double> & vals);
	
private slots:
	void rectangleAnimating(const QVariant & v_rec);
	void clickedBtnFromSure();
	void clickedBtnFromCancel();
	void clearNestedAnimation();	

protected:
	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void paintEvent(QPaintEvent * event);

private:
	void initPlotWidgetForMulti();
	void initPlotWidgetForSingle(QStandardItem * s_item);
	void reInitForMultiPlots(QStringList & relist);
	void initEngiDailyPartDataesRecord(const QString & description);
	void chkPlotShowingState();
	void generateCoverWidget();
	void setClueText(int row, int col);			
	void openNestedEdittor(QWidget * nesting, int end_width, int end_height);
	void closeNestedViewer(QWidget * nested);
	bool focusing;
	int engi_samples;
	int engi_groups;
	int last_group;	
	double upper_engiLimit;
	double lower_engiLimit;
	double cpk;
	QList<double> tmp_data;
	QList<HistogramPlot *> test_histoes;
	QList<GaussPlot *> test_gausses;
	QMap<int, QList<double> > part_records;	
	QString curengi_name;
	QPoint mv_pt;
	QRect anim_endrect;
	QPushButton * func_btn;
	QVBoxLayout * vl_layout;
	Plot * single_plot;
	Plot * multi_plot;
	HistogramPlot * histo_plot;
	GaussPlot * gauss_plot;
	QStandardItem * item_info;
	QStandardItem * pos_item;
	QWidget * editor_viewer;
	QLineEdit * wl_editor;
	QPropertyAnimation * nested_animation;	
	LittleWidgetItem * qml_win;
	TablesGroup * nested_table;
	QWidget * color_pad;
	SpcNum * back_guide;
	SpcDataBase * db_source;
};

#endif
