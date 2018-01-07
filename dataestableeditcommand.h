#ifndef DATAESTABLEEDITCOMMAND_H
#define DATAESTABLEEDITCOMMAND_H
#include <QUndoCommand>

class SpcDataBase;
class DataesTableView;
class DataesTableEditCmd : public QUndoCommand
{
public:
	DataesTableEditCmd(DataesTableView * obj, const QModelIndex & index, const QString & old_text, const QString & new_text);
	DataesTableEditCmd(DataesTableView * obj, QStandardItemModel * del_model, const QString & order_text, DataesTableView * boss_partner);
	DataesTableEditCmd(DataesTableView * obj, QStandardItem * hint_item, const QString & order_text, DataesTableView * boss_partner);
	~DataesTableEditCmd();
	void undo();
	void redo();

private:
	void redoCellForBossTypeZero();
	void undoCellForBossTypeZero();
	void redoCellForBossTypeTwo();
	void undoCellForBossTypeTwo();
	void redoModelsForBossZeroTwoFour();
	void undoModelsForBossZeroTwoFour();
	int item_pos;
	QString oldText;
	QString newText;
	QStandardItem * parent_item;
	QStandardItem * child_item;
	QModelIndex cur_index;
	QStandardItemModel * unre_model;	
	QList<QStandardItem *> undo_items;
	QHash<QString, QList<QPair<QString, QStandardItemModel *> > > bk_hash;
	DataesTableView * p_boss;
	DataesTableView * boss;	
};

#endif
