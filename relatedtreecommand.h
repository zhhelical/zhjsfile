#ifndef RELATEDTREECOMMAND_H
#define RELATEDTREECOMMAND_H
#include <QtCore>
#include <QtGui>

//class SpcDataBase;
class RelatedTreeView;
class NestedModel;
class RelatedTreeCommand : public QUndoCommand
{
public:
	RelatedTreeCommand(RelatedTreeView * tree_obj, QStandardItem * selected_item, const QString & order_text, const QString & old_text = QString());	
	~RelatedTreeCommand();
	void undo();
	void redo();
	QStandardItem * parentItem();
	QStandardItem * childItem();

private:
 	void redoForBossTypeZero(NestedModel * data_model);
	void redoForBossTypeOne(NestedModel * data_model);
	void redoForBossTypeTwo(NestedModel * data_model);
	void redoForBossFourFive(NestedModel * data_model);
	void redoForBossTypeSix(NestedModel * data_model);	 
	void redoForBossTypeSeven(NestedModel * data_model);
	void undoForBossTypeZero(NestedModel * data_model);
	void undoForBossTypeOne(NestedModel * data_model);
	void undoForBossTypeTwo(NestedModel * data_model);
	void undoForBossFourFive(NestedModel * data_model);
	void undoForBossTypeSix(NestedModel * data_model);
	void undoForBossTypeSeven(NestedModel * data_model);
	int old_pos;
	int mid_pos;
	QString old_Text;
	QString new_Text;
	QString act_Text;
	QStandardItem * parent_item;
	QStandardItem * mid_item;
	QStandardItem * child_item;
	QList<QStandardItem *> undo_items;
	QHash<QStandardItem *, QList<QStandardItem *> > unre_powers;
	QHash<QStandardItem *, QString> free_unredb;
	RelatedTreeView * boss;	
};

#endif
