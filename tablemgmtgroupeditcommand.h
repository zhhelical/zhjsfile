#ifndef TABLEMGMTGROUPEDITCOMMAND_H
#define TABLEMGMTGROUPEDITCOMMAND_H
#include <QtCore>
#include <QtGui>

class SpcDataBase;
class PlotWidget;
class TableManagement;
class TablesGroup;
class TableMgmtGroupEditCmd : public QUndoCommand
{
public:
	TableMgmtGroupEditCmd(TableManagement * tree_guide, QStandardItem * info, const QString & order_text, TablesGroup * table_obj);
	TableMgmtGroupEditCmd(TableManagement * tree_guide, QStandardItem * info, const QString & order_text, const QList<QStandardItem *> & sel_dbs);	
	TableMgmtGroupEditCmd(TablesGroup * table_obj, const QString & order_text, const QPair<QModelIndex, QString> & edited_strs);
	TableMgmtGroupEditCmd(TablesGroup * table_obj, const QString & order_text, const QPair<QString, QModelIndexList> & framed_pair);
	TableMgmtGroupEditCmd(TablesGroup * table_obj, const QString & order_text, const QMap<QModelIndex, QString> & selected_map);
	TableMgmtGroupEditCmd(TablesGroup * table_obj, const QString & order_text, const QHash<QModelIndex, QPair<QString, QStandardItem *> > & selected_cells);	
	TableMgmtGroupEditCmd(TablesGroup * table_obj, const QString & order_text, const QHash<QModelIndex, QPair<int, int> > & selected_cells);	
	TableMgmtGroupEditCmd(TableManagement * tree_guide, TablesGroup * table_obj, QStandardItem * selected_item, QStandardItem * new_item, const QString & order_text);
	TableMgmtGroupEditCmd(TableManagement * tree_guide, TablesGroup * table_obj, const QString & order_text, const QHash<QModelIndex, QPair<QMap<int, QVariant>, QStandardItem *> > & selected_cells);
	TableMgmtGroupEditCmd(TableManagement * tree_guide, TablesGroup * table_obj, QStandardItem * info, QStandardItem * pos, QStandardItemModel * data_backup, QStandardItemModel * old_model, const QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & changing_hash, const QString & order_text);
	TableMgmtGroupEditCmd(TableManagement * tree_guide, TablesGroup * table_obj, const QString & order_text, const QPair<QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> >, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > > & selected_pair);
	~TableMgmtGroupEditCmd();
	void undo();
	void redo();
	const QString & orderInfo();

private:
	void redoOnlyForTwoTypeBossOne();
	void undoOnlyForTwoTypeBossOne();	
	int child_row;
	QStandardItem * parent_item;
	QStandardItem * mid_item;	
	QStandardItem * child_item;
	QStandardItemModel * data_model;
	QStandardItemModel * sized_model;
	QSharedPointer<QStandardItemModel> j_ptr;
	QString orderText;
	QPair<QModelIndex, QString> newold_strs;
	QPair<QString, QModelIndexList> framed_list;
	QList<QStandardItem *> undo_items;
	QMap<QModelIndex, QString> contents_map;
	QHash<QStandardItem *, QStringList> names_texts;
	QHash<QStandardItem *, QList<QStandardItem *> > pos_items;
	QHash<QModelIndex, QPair<int, int> > cells_size;
	QHash<QModelIndex, QPair<QString, QStandardItem *> > related_hash;
	QHash<QModelIndex, QPair<QMap<int, QVariant>, QStandardItem *> > sel_cells;
	QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > before_hash;
	QPair<QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> >, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > > paste_pair;
	TableManagement * boss1;	
	TablesGroup * boss2;
};

#endif
