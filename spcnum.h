#ifndef SPCNUM_H
#define SPCNUM_H
#include <QtGui>
#include <QtCore>

class QSqlTableModel;
class PlotWidget;
class DataesTableView;
class PlotWidget;
class SpcDataBase;
class SpcNum : public QObject
{
	Q_OBJECT
public:
	SpcNum(QObject * parent = 0);
	~SpcNum();
	void setDatabase(SpcDataBase * db);
	void setCalculatingPlots(PlotWidget * w_pt);
	void runSpcInitCal(DataesTableView * e_view, DataesTableView * g_view);
	void dailyDatesCal(DataesTableView * daily_view);
	void startPtRuningJudging(double val, int type);
	void setEngiDescriptions(const QPair<QString, QString> & paircontent);
	void setRelatedDataesByGroups(int from_num, int to_num, const QString & proj, QMap<int, QList<double> > & map_dataes);
	void cpkInitModelFromDb(int type, const QString & project, QStandardItemModel * cpk_model);
	void cpkHistogramTranslation(const QMap<int, QList<double> > & dataes_map, QStandardItemModel * histo_model);
	void cpkGaussTranslation(const QMap<int, QList<double> > & dataes_map, QStandardItemModel * gauss_model);
	void selectedDataesModel(int from, int to, int type, QStandardItemModel * dataes_model, const QString & engine);
	void dbSpcNamesTranslation(const QString & transed, QString & chinese);
	void dailyDataesTransFromDB(QVariant dataes, QList<double> & data_list);
	void randListGenerate(QList<int> & rand_list, const QList<int> & old_list = QList<int>());
	void compareTwoTimeLists(const QStringList & base_list, const QStringList & c_list, QString & compare_result);
	void insertListByTimeSequence(const QStringList & insing_list, QStringList & insed_list);
	void initNewFilePara();
	void sortItemSequenceByTime(const QList<QStandardItem *> & sort_items, QMap<int, QStandardItem *> & sorted_map);
	bool chkStrOnlyUseLetterOrNumber(const QString & str);
	bool checkPunctuationInString(const QString & str);
	bool inputNumbersJudging(const QString & judged);	
	int randNumberGenerate();	
	int findTimestrPosInStringlist(const QStringList & strs, const QString & base_str);
	double averageDataes(QList<double> & vals);
	double devCalFromDataes(QList<double> sdataes);
	double rngCalFromDataes(QList<double> rdataes);
	double getCpkTolAvr(const QString & cpk_tbl);
	double getAvrVal();
	double getDevVal();
	double getRngVal();
	double selectedGroupsMax(QList<double> & range);
	double curGroupMax();
	double selectedGroupsMin(QList<double> & range);
	double curGroupMin();
	double HistogramUnitSize(double rng, int groups);
	double HistogramUnitLowMark(double val);
	double normalProbabilityYaxleValue(QList<double> & xs, QList<double> & ys, double xaxel);
	double normalProbabilityB0Para(QList<double> & xs, QList<double> & ys);
	double normsinv(const double p);
	QString ensureCpkGrade(const double cpk);
	QList<double> ptsFrequency(QList<double> & pts);
	QList<double> mergeDataesFromMap(const QMap<int, QList<double> > & map);
	const QList<QPair<QString, QString> > & engiCarryings();
	const QList<QPair<double, double> > GaussPlotPair();
	PlotWidget * currentPlotWidget();

signals:
	void showCalResTable(QString & d_proj, const QList<double> & caleds);
	void resetHistogram(int from, int to);

protected:
	void threePtsJudging(double nextPt);
	void fivePtsJudging(double nextPt);
	void sixPtsJudging(double nextPt);
	void eightPtsJudging(double nextPt);
	void ninePtsJudging(double nextPt, int type);
	void fourteenPtsJudging(double nextPt, int type);
	void fifteenPtsJudging(double nextPt);	
	bool oneOutLimit(double judVal, int type);
	bool threeJudging();
	bool fiveJudging();
	bool sixJudging();
	bool eightJudging();
	bool nineJudging(QList<double> & judDataes, int type);
	bool fourteenJudging(QList<double> & judDataes);
	bool fifteenJudging();
	int calForHistogramGroups(int groupnum);
	int freqDisForHistogram(double up, QList<double> & range);
	int freqDisForGauss(double low, double up, QList<double> & range);
	double dataesTolavrCal(QMap<int, QList<double> > & tolDataes);
	double devFromAnyGroups(QMap<int, QList<double> > & keyDataes);
	double rngFromAnyGroups(QMap<int, QList<double> > & keyDataes);
	double calForAvrLowLimit(DataesTableView * g_view);
	double calForAvrUpLimit(QMap<int, QList<double> > & keyDataes, DataesTableView * g_view);
	double calForDevLowLimit(DataesTableView * g_view);
	double calForDevUpLimit(QMap<int, QList<double> > & keyDataes, DataesTableView * g_view);
	double calForRngLowLimit(DataesTableView * g_view);
	double calForRngUpLimit(QMap<int, QList<double> > & keyDataes, DataesTableView * g_view);
	double calForLinearRegressorLxx(QList<double> & xs);
	double calForLinearRegressorLyy(QList<double> & ys);
	double calForLinearRegressorLxy(QList<double> & xs, QList<double> & ys);
	double normalProbabilityLineSlope(QList<double> & xs, QList<double> & ys);
	double normFreqSum(double low, double up, QList<double> & range);	

private:
	void engiParaesCal(DataesTableView * key_view, QMap<int, QList<double> > & tolDataes, QList<double> & calings);
	void startAvrPtsJudge(double avr);
	void startDevPtsJudge(double dev);
	void startRngPtsJudge(double rng);
	void cpkDataesTransFromDB(const QString & pro_name, QMap<int, QList<double> > & stored_map);
	double maxFromList(QList<double> & list);
	double minFromList(QList<double> & list);
	double listSum(QList<double> & list);
	double listSquareSum(QList<double> & list);
	double normInvShiftedVal(double norminv);
	double unit_accuracy;//move to toolsplot
	double cpk_tolAvr;// maybe merge to num_avr in future
	double num_avr;
	double num_s;
	double num_r;
	QString caling_engi;// move to PlotWidget, this is for calculating judge
	QList<double> data;//cancel?
	QList<double> u_data;
	QMap<int, QList<double> > tol_dataes;// maybe need reserve not move to plotwidget
	QList<QPair<QString, QString> > product_pair;
	QList<QPair<QString, QString> > engi_carrying;
	PlotWidget * current_plots;
	SpcDataBase * spcdata_db;
};

#endif
