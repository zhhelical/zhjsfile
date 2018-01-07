#ifndef LITTLEWIDGETITEM_H
#define LITTLEWIDGETITEM_H
#include <QtGui>
#include <QtCore>

class QDeclarativeItem;
class PlotWidget;
class MatrixWidget;
class SpcDataBase;
class SpcNum;
class LittleWidgetsView;
class TableManagement;
class QSqlTableModel;
class LittleWidgetItem : public QGraphicsProxyWidget
{
	Q_OBJECT
public:
	LittleWidgetItem(QDeclarativeItem * parent = 0);
	~LittleWidgetItem();
	void setWidgetLayoutPt(QGridLayout * w_layout, QWidget * container);
	void setDefaultWidget(const QPair<int, int> & pos, MatrixWidget * w_default);
	void zoomOutPlots();
	void zoomInPlots();
	void setCellSize(int n_width, int n_height);
	void findPlotsOnRowsColumns(int rows, int columns, QList<PlotWidget *> & push_plots, QHash<QPair<int, int>, int> & push_widgets, QString & over_limit);
	void unredoForPlotsMatrix(int old_rc, int new_rc, bool row_col, QHash<QPair<int, int>, int> & unre_widgets, QList<PlotWidget *> & unre_plots);
	void horverPlotsCounts(QPair<int, int> & row_col);
	void coverShowPlot(PlotWidget * showing, PlotWidget * covered);
	void setNewRect(const QRectF & rect);
	void strOrderForPlotFromUsr(const QString & order, const QString & detail);
	void newPlotwidgetToCoverOld(QStandardItem * cov_item, QPair<PlotWidget *, PlotWidget *> & new_old, QPair<int, int> & cover_pos, SpcNum * back_spc, SpcDataBase * base_db, LittleWidgetsView * p_view);
	void enterWhichWidget(const QPointF & enter_pos, QPair<int, int> & which);
	void unredoMatrixRowMerge(int col_from, int col_to, int row, bool un_redo, const QHash<QPair<int, int>, int> & unre_mws, const QList<PlotWidget *> & unre_plots);
	void unredoMatrixRowSplit(int col_from, int col_to, int row, bool un_redo);
	void unreClearGeneratedPlots(const QList<QStandardItem *> & c_plots, QList<PlotWidget *> & dest_plots, bool un_redo);
	void unreClearGeneratedPlot(QStandardItem * c_plot, PlotWidget * &dest_plot, bool un_redo);
	void checkSameColorInMatrix(QWidget * sender);
	void exportPng(LittleWidgetsView * view);
	void resetMatrixForSavePlots(TableManagement * tree_info, QSqlTableModel * plots_model, LittleWidgetsView * p_view, SpcNum * spc, SpcDataBase * db);
	void relatedTmpUnshowWidgetPos(PlotWidget * f_widget, QPair<int, int> & f_pos);
	void relatedPlotTitleEditting();
	void respondPlotShowingContent(const QString & s_order);
	bool findAdjacentBackPosPair(QWidget * front, QPair<int, int> & f_pos);
	bool findPlotsOnMergeRow(int * col_from, int col_to, int row, QHash<QPair<int, int>, int> & merge_matrixes, QList<PlotWidget *> & merge_plots);
	bool findPlotOnSplitRow(QStandardItem * span_info, QStandardItem * row_info);
	bool coveredPlotExisted(QPair<int, int> & cover_pos);	
	QStandardItem * destPlotDetailInformation(const QPair<int, int> & pos);
	MatrixWidget * generateDefault(const QRect & size, const QRect & origin_size = QRect());
	PlotWidget * newPlotwidgetPlaceMw(QStandardItem * with_item, QPair<int, int> & place_pos, SpcNum * back_spc, SpcDataBase * base_db, LittleWidgetsView * p_view);
	QPair<int, int> realRowsColsInGridLayout();	
	QList<PlotWidget *> & unshowPlotsMatrixWidgets();
	QMap<QPair<int, int>, MatrixWidget *> & allCurrentDefaults();
	QMap<QPair<int, int>, PlotWidget *> & allCurrentPlots();

public slots:
	void arrangePlotForBtnSig(QObject * deled);
	void findPlotToChange(int type, QStandardItem * find_plot);

signals:
	void finishedCellsArrangement();
	void matrixChanging();
	void changingPlotInfoes(PlotWidget * changing_info);
	void refreshPlotShowQmlBtns(QVariant modify, QVariant area, QVariant area_st, QVariant dl, QVariant dl_st, QVariant ctrl, QVariant ctrl_st, QVariant ds, QVariant ts, QVariant dtst);

private slots:
	void rearrangeForDelDefault(QObject * d_del);
	void receivePdfFileName(const QString & df_name);

private:
	void createCellsOnGridLayout(int create_from, int create_to, int create_num, bool h_v, QHash<QPair<int, int>, int> & red_widgets, QList<PlotWidget *> & red_plots);
	void deleteCellsOnGridLayout(int delete_from, int delete_to, int delete_num, bool h_v);
	void chkAndCleanNoWidgetItems(int row);
	const QPair<int, int> & findShowingPos(QWidget * finding);
	QWidget * findRowColFrontWidget(int row, int col);
	QString title;
	QRectF showing_rect;
	QGridLayout * cells_layout;
	QWidget * widget_container;
	QList<PlotWidget *> unshow_widgets;
	QMap<QPair<int, int>, MatrixWidget *> d_widgets;
	QMap<QPair<int, int>, PlotWidget *> plot_widgets;
};

#endif
