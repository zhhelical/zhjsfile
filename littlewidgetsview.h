#ifndef LITTLEWIDGETSVIEW_H
#define LITTLEWIDGETSVIEW_H
#include <QtGui>
#include <QtCore>
#include <QtDeclarative>

class DataesTableView;
class TableManagement;
class RelatedTreeView;
class TablesGroup;
class PlotWidget;
class MatrixWidget;
class HistogramPlot;
class GaussPlot;
class LittleWidgetItem;
class TableManagement;
class HelpContents;
class SpcDataBase;
class QSqlTableModel;
class LittleWidgetsView : public QDeclarativeView
{
	Q_OBJECT

public:
	LittleWidgetsView(QWidget * parent = 0);
	~LittleWidgetsView();
	void setMainQmlFile(const QString &file);
	void initMarginForParent(const QRect & size);
	void initViewMatrix(int h_num, int v_num, QRect & size);
	void setViewForRelatedTable(QWidget * w_table);
	void setTblManager(TableManagement * tbl_mng);
	void setHelpPages(HelpContents * h_pages);
	void setManualNameFromDB(const QString & db_manual);//? why use it?
	void resetQmlFrozenState(bool new_state);
	void deletePlotsPicsInDbTbl(const QString & tbl_name, const QString & tbl_contents, SpcDataBase * db);
	bool frozenSate();
	QWidget * nestedContainer();
	LittleWidgetItem * qmlAgent();
	const QString & dbManualName();
	const QRect & initRect();	

signals:
	void initRectangle(QVariant width, QVariant height, QVariant real_width, QVariant real_height);
	void newRectangle(QVariant w, QVariant h);
	void setCellsNewSize(QVariant width, QVariant height);
	void freezingWidgetsPlots(QVariant freeze);
	void replySettedPlotInfoes(const QStringList & infoes);
	void changingPlot(int type, QStandardItem * which);
	void dragingPt(QVariant ptX, QVariant ptY, QVariant width, QVariant height);
	void dealSigFromAgentHidePlot(PlotWidget * d_p);
	void plotPositionChanging(QStandardItem * old_info, QStandardItem * new_info, PlotWidget * changing_plot, PlotWidget * dest_plot);
	void unredoOrderForMatrixChange(LittleWidgetsView * qml, int old_rc, int new_rc, bool row_col, const QString & undo_order, const QHash<QPair<int, int>, int> & hide_widgets, const QList<PlotWidget *> & hide_plots);
	void sendPlotShowToQmlBtns(QVariant modify, QVariant area, QVariant area_st, QVariant dl, QVariant dl_st, QVariant ctrl, QVariant ctrl_st, QVariant ds, QVariant ts, QVariant dtst);
	void sendNewFrozenState(bool freezing);

public slots:
	void resetViewWidth(const QVariant & size);  
	void relatedWorkFromQmlEditor(QString guide, QString action);
	void resizeAgentSize();
	void dealMimeForPlotMove(PlotWidget * moving, const QPoint & press_pos);	

protected:
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);

private:
	bool qml_frozen;
	int mcol_from;
	QString stored_manual;
	QRect view_window;
	QPoint topleft_moved;
	QWidget * big_container;
	QDrag * drag;
	QLabel * move_pos;
	TableManagement * tbl_guider;
	LittleWidgetItem * agent;
};

#endif
