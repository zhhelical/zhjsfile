#ifndef EDITLONGTOOLSVIEWER_H
#define EDITLONGTOOLSVIEWER_H
#include <QtCore>
#include <QtDeclarative>

class DataesTableView;
class RelatedTreeView;
class TableManagement;
class LittleWidgetsView;
class TablesGroup;
class TableMgmtPlotEditCmd;
class TableMgmtGroupEditCmd;
class PlotWidget;
class MatrixWidget;
class EditLongToolsViewer : public QDeclarativeView
{
	Q_OBJECT

public:
	enum ScreenOrientation{ScreenOrientationLockPortrait, ScreenOrientationLockLandscape, ScreenOrientationAuto};
	enum CommandType{plot_com = 0, tree_com = 1};// add in future
	EditLongToolsViewer(QWidget *parent = 0);
	~EditLongToolsViewer();
	static EditLongToolsViewer * create();
	void setMainQmlFile(const QString &file, int q_type);
	void setNestedPosItem(QStandardItem * p_item);
	void addImportPath(const QString &path);
	// Note that this will only have an effect on Symbian and Fremantle.
	void setOrientation(ScreenOrientation orientation);
	void showExpanded();
	void setRelatedTreePt(int t_type, TableManagement * m_table);
	void setPlotsContainerPt(LittleWidgetsView * p_container);
	void setEditingTable(TablesGroup * editting);
	void setOtherEditingTable(QWidget * editting);
	void setNewStrList(const QStringList new_list);
//	void clearRelatedPt(int t_type);
	int qmlDelegateType();
	int currentStringPos();
	QStandardItem * currentNesttingItem();
	QUndoStack * commandStack();
	QHash<LittleWidgetsView *, QUndoStack *> & undoStackHash();

signals:
	void killMyself(QWidget * me);
	void sendStopInfoToMainWin(QString func);
	void clickedEditButton(QString menu, QString func);
	void newTitle(QVariant m_title);
	void relatedSelectedResource(QString detail, bool sel);
	void qmlRecSizeChanged(QVariant width, QVariant height);
	void ungrayForUndo(QVariant forundo);
	void ungrayForRedo(QVariant forredo);
	void sendSaveSigToWin(bool save_del);

public slots:
	void killSelf();
	void rectangleAnimating(const QVariant & rec);
	void setSelectedPos(int pos);
	void pushGroupEditCommand(TableMgmtGroupEditCmd * comm);	

private slots:
	void undoChanged(bool c_undo);
	void redoChanged(bool c_redo);
	void undoRedoActFromQml(QString func);
	void replyPlotPositionChanging(QStandardItem * old_pos, QStandardItem * new_pos, PlotWidget * mv_plot, PlotWidget * move_to);
	void unredoForMatrixChange(LittleWidgetsView * qml, int old_rc, int new_rc, bool row_col, const QString & undo_order, const QHash<QPair<int, int>, int> & hide_widgets, const QList<PlotWidget *> & hide_plots);
	void pushPlotEditCommand(TableMgmtPlotEditCmd * comm);

private:
	explicit EditLongToolsViewer(QDeclarativeView *view, QWidget *parent);
	class EditLongToolsViewerPrivate *d;
	void resetStateForViewsRedos(LittleWidgetsView * trans_view);
	void resetStateForTablesRedos(TablesGroup * t_trans);
	void sigSlotLinkForViewStack(LittleWidgetsView * view, QUndoStack * stack);
	void sigSlotLinkForTgrpStack(TablesGroup * grp, QUndoStack * stack);
	int qml_type;
	int str_pos;
	QHash<LittleWidgetsView *, QUndoStack *> views_undos;
	QHash<TablesGroup *, QUndoStack *> tables_undos;
	QWidget * nested_window; 
	QStandardItem * nested_item;	
	QUndoStack * m_commandStack;
	DataesTableView * d_table;
	RelatedTreeView * p_table;
	TableManagement * dataes_view;
	TableManagement * minfo_view;
	TableManagement * plots_tview;
	TableManagement * total_view;
	LittleWidgetsView * plots_view;
	TablesGroup * edit_table;
};

#endif 
