#ifndef TABLEMGMTPLOTEDITCOMMAND_H
#define TABLEMGMTPLOTEDITCOMMAND_H
#include <QtCore>
#include <QtGui>

class SpcDataBase;
class PlotWidget;
class MatrixWidget;
class TableManagement;
class LittleWidgetsView;
class TableMgmtPlotEditCmd : public QUndoCommand
{
public:
	TableMgmtPlotEditCmd(TableManagement * tree_obj, QStandardItem * selected_item, const QString & order_text, LittleWidgetsView * qml_obj);
	TableMgmtPlotEditCmd(TableManagement * tree_obj, QStandardItem * selected_item, QStandardItem * new_item, const QString & order_text, LittleWidgetsView * qml_obj, PlotWidget * show_plot = 0, PlotWidget * covered_plot = 0);	
	TableMgmtPlotEditCmd(const QString & order_text, TableManagement * tree_obj, LittleWidgetsView * qml_obj, int old_rc, int new_rc, bool row_col, const QHash<QPair<int, int>, int> & hide_widgets, const QList<PlotWidget *> & hide_plots);
	~TableMgmtPlotEditCmd();
	void undo();
	void redo();

private:
	bool rowcol_change;
	int child_row;
	int rc_old;
	int rc_new;
	QString orderText;
	QStandardItem * add_pos;
	QStandardItem * parent_item;
	QStandardItem * child_item;
	PlotWidget * show_ptr;
	PlotWidget * covered_ptr;
	QList<QStandardItem *> undo_items;
	QList<PlotWidget *> undo_plots;
	QHash<QPair<int, int>, int> undo_mwps;	
	QHash<QStandardItem *, QStringList> names_texts;
	QHash<QStandardItem *, QPair<QStandardItem *, int> > inmodel_oldpos;
	TableManagement * boss1;	
	LittleWidgetsView * boss2;
};

#endif
