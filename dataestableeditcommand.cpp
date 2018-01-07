#include <QtCore>
#include <QtGui>
#include "dataestableeditcommand.h"
#include "dataestableview.h"
#include "engitreeview.h"
#include "spcdatabase.h"

DataesTableEditCmd::DataesTableEditCmd(DataesTableView * obj, const QModelIndex & index, const QString & old_text, const QString & new_text)
: QUndoCommand()
{
	boss = obj;
	cur_index = index;
	oldText = old_text;
	newText = new_text;
}

DataesTableEditCmd::DataesTableEditCmd(DataesTableView * obj, QStandardItemModel * del_model, const QString & order_text, DataesTableView * boss_partner)
: QUndoCommand()
{
	boss = obj;
	unre_model = del_model;
	newText = order_text;	
	p_boss = boss_partner;	
}

DataesTableEditCmd::DataesTableEditCmd(DataesTableView * obj, QStandardItem * hint_item, const QString & order_text, DataesTableView * boss_partner)
: QUndoCommand()
{
 	boss = obj;
	if (boss->tableType() == 0)
	{
		parent_item = hint_item->parent()->parent()->parent();
		child_item = hint_item->parent()->parent();	
	}
	else
	{
		parent_item = hint_item->parent()->parent()->parent()->parent();
		child_item = hint_item->parent()->parent()->parent();	
	}	
	item_pos = child_item->row();
	unre_model = qobject_cast<QStandardItemModel *>(boss->model());
	newText = order_text;	 
	p_boss = boss_partner;
}

DataesTableEditCmd::~DataesTableEditCmd()
{}

void DataesTableEditCmd::redo()
{
	if (newText == QObject::tr("删除，，。"))
		redoModelsForBossZeroTwoFour();
	else
	{
		boss->model()->setData(cur_index, newText);
		if (boss->tableType() == 0)
			redoCellForBossTypeZero();
		else if (boss->tableType() == 2)
			redoCellForBossTypeTwo();
	}
}

void DataesTableEditCmd::undo()
{
 	if (newText == QObject::tr("删除，，。"))
		undoModelsForBossZeroTwoFour();
	else
	{
		boss->model()->setData(cur_index, oldText);
		if (boss->tableType() == 0)
			undoCellForBossTypeZero();
		else if (boss->tableType() == 2)
			undoCellForBossTypeTwo();
	}
}

void DataesTableEditCmd::undoCellForBossTypeZero()
{
	if (cur_index.row()==0 && cur_index.column()==0)
		return;
	else
		boss -> setNewNextIndex(cur_index);		
}

void DataesTableEditCmd::redoCellForBossTypeZero()
{
	if (cur_index.column() == boss->model()->columnCount()-1)
	{
		if (boss->model()->hasIndex(cur_index.row()+1, 0))
			boss -> setNewNextIndex(boss->model()->index(cur_index.row()+1, 0));
	}
	else
		boss -> setNewNextIndex(boss->model()->index(cur_index.row(), cur_index.column()+1));
}

void DataesTableEditCmd::undoCellForBossTypeTwo()
{
	if (cur_index.row() < boss->model()->rowCount()-3)
		boss -> setNewNextIndex(cur_index);		
}
		
void DataesTableEditCmd::redoCellForBossTypeTwo()
{
	if (cur_index.row() == boss->model()->rowCount()-3)
		boss -> setNewNextIndex(boss->model()->index(0, 0));
	else
		boss -> setNewNextIndex(boss->model()->index(cur_index.row()+1, cur_index.column()));	
}

void DataesTableEditCmd::redoModelsForBossZeroTwoFour()
{
	if (boss->improveTreePtr())
	{
		if (!boss->improveTreePtr()->deletionItemsList().contains(child_item))
			boss->improveTreePtr()->deletionItemsList() << child_item;		
		QString grp_key;
		if (boss->tableType() == 0)
			grp_key = p_boss->getInfoByModel(qobject_cast<QStandardItemModel *>(p_boss->model()), QObject::tr("时间"));
		else
			grp_key = unre_model->data(unre_model->index(0, 0)).toString();		
		QList<QPair<QString, QStandardItemModel *> > models_list = boss->modelHashs().value(grp_key);		
		if (models_list.size() == 1)
		{
			QStandardItemModel * c_model = boss->modelHashs().value(QObject::tr("草稿工程，，。")).at(0).second;
			c_model -> clear();
			boss -> copyModelDataesToOther(unre_model, c_model, false);		
			if (boss->tableType() == 4)
			{
				QStringList detail_list = boss->backDatabase()->curDBmanageTable().value(boss->projectInputor().second);		
				for (int i = 0; i < c_model->rowCount(); i++) 
				{
					if (i == 0)
						c_model -> setData(c_model->index(0, 0), grp_key);
					else if (i > 8)
					{
						if (c_model->data(c_model->index(i, 0)).isNull())
						{
							if (i == 9)
								c_model -> setData(c_model->index(9, 0), boss->projectInputor().first);
							else if (i == 10)
								c_model -> setData(c_model->index(10, 0), boss->projectInputor().second);
							else if (i == 11)
								c_model -> setData(c_model->index(11, 0), detail_list.at(4));
							else
								c_model -> setData(c_model->index(12, 0), detail_list.at(2));		
						}
					}
					else
						c_model->setData(c_model->index(i, 0), "");
				}
				boss->modelHashs()[QObject::tr("草稿工程，，。")][0].first = "";
			}
			else
				boss->modelHashs()[QObject::tr("草稿工程，，。")][0].first = grp_key;
			boss -> setModel(c_model);
			boss -> resetEditState(true);	
		}
		else
		{
			int cur_pos = 0;
			for (int i = 0; i < models_list.size(); i++)
			{
				if (models_list.at(i).second == boss->model())
				{
					cur_pos = i;
					break;
				}
			}
			if (cur_pos == models_list.size()-1)
				boss -> setModel(models_list.at(0).second);
			else
				boss -> setModel(models_list.at(cur_pos+1).second);
			boss -> resetEditState(false);						
		}
		boss -> dealModelsHashForUnreDo(unre_model, true, child_item);		
		if (p_boss->tableType() == 0)
		{
			QString boss_key = boss->modelConstructTime(boss->model()->data(boss->model()->index(0, 0)).toString(), qobject_cast<QStandardItemModel *>(boss->model()));
			p_boss -> setModel(p_boss->modelHashs().value(boss_key).at(0).second);
			p_boss -> changeViewStateForNewModel();
		}		
		parent_item -> takeRow(item_pos);	
	}
	else
		boss -> deleteTmpModels(unre_model, p_boss);
}
	
void DataesTableEditCmd::undoModelsForBossZeroTwoFour()
{
 	if (boss->improveTreePtr())
	{
		parent_item -> insertRow(item_pos, child_item);
		boss -> dealModelsHashForUnreDo(unre_model, false, child_item);
		boss -> setModel(unre_model);
		if (p_boss->tableType() == 0)
		{
			QString boss_key = boss->modelConstructTime(unre_model->data(unre_model->index(0, 0)).toString(), unre_model);
			p_boss -> setModel(p_boss->modelHashs().value(boss_key).at(0).second);
			p_boss -> changeViewStateForNewModel();
		}		
	}
	else
		boss -> cancelDeleteModelsAction(unre_model); 
}
