#include <QtCore>
#include <QtGui>
#include "relatedtreecommand.h"
#include "relatedtreeview.h"
#include "nestedmodel.h"
#include "editlongtoolsviewer.h"
#include "spcdatabase.h"

RelatedTreeCommand::RelatedTreeCommand(RelatedTreeView * tree_obj, QStandardItem * selected_item, const QString & order_text, const QString & old_text)
: QUndoCommand(), old_pos(0), mid_pos(0), mid_item(0), child_item(0)
{
	boss = tree_obj;
	parent_item = selected_item;
	if (!old_text.isEmpty())
		old_Text = old_text;
	if (boss->treeType()==1 && order_text.contains(","))
	{
		QStringList hints = order_text.split(",");
		if (hints[1] == QObject::tr("新部门"))
			new_Text = QObject::tr("在此填写新部门名称");
		else  
			new_Text = hints[1];
		act_Text = hints[0];
	}
	else if (boss->treeType()==2 && (selected_item->text()==QObject::tr("获授权限") || selected_item->text()==QObject::tr("授予权限")))
	{
		new_Text = order_text;
		act_Text = selected_item->text();
	}
	else
	{
		new_Text = selected_item->text();
		act_Text = order_text;	
	}
}

RelatedTreeCommand::~RelatedTreeCommand()
{}

void RelatedTreeCommand::redo()
{
	if (boss->qmlViewer())
		boss -> closeNestedQmlViewerOnCell(boss->qmlViewer());
	QModelIndexList sels = boss->selectionModel()->selectedIndexes();
	foreach (QModelIndex e_index, sels)
	{
		if (boss->indexWidget(e_index))
			boss->indexWidget(e_index)->deleteLater();
	}	
	NestedModel * tree_model = qobject_cast<NestedModel *>(boss->model());
	if (boss->treeType() == 0)
		redoForBossTypeZero(tree_model);
	else if (boss->treeType() == 1)
		redoForBossTypeOne(tree_model);
	else if (boss->treeType() == 2)
		redoForBossTypeTwo(tree_model);
	else if (boss->treeType()==4 || boss->treeType()==5)
		redoForBossFourFive(tree_model);
	else if (boss->treeType() == 6)
		redoForBossTypeSix(tree_model);
	else	
		redoForBossTypeSeven(tree_model);
}

void RelatedTreeCommand::undo()
{
	if (boss->qmlViewer())
		boss -> closeNestedQmlViewerOnCell(boss->qmlViewer());
	QModelIndexList sels = boss->selectionModel()->selectedIndexes();
	foreach (QModelIndex e_index, sels)
	{
		if (boss->indexWidget(e_index))
			boss->indexWidget(e_index)->deleteLater();
	}	
	NestedModel * tree_model = qobject_cast<NestedModel *>(boss->model());
	if (boss->treeType() == 0)
		undoForBossTypeZero(tree_model);
	else if (boss->treeType() == 1)
		undoForBossTypeOne(tree_model);
	else if (boss->treeType() == 2)
		undoForBossTypeTwo(tree_model);
	else if (boss->treeType() == 4 || boss->treeType() == 5)
		undoForBossFourFive(tree_model);
	else if (boss->treeType() == 6)
		undoForBossTypeSix(tree_model);
	else	
		undoForBossTypeSeven(tree_model);	
}
	
void RelatedTreeCommand::redoForBossTypeZero(NestedModel * data_model)
{
	if (act_Text == QObject::tr("删除所有"))
	{
		QHash<QStandardItem *, QPair<QStandardItem *, QString> > origin_hash = data_model->originHashItems();
		int i = 0;
		while (parent_item->child(i))
		{
			QList<QStandardItem *> i_children;
			data_model -> findAllChildItems(parent_item->child(i), i_children);
			foreach (QStandardItem * c_item, i_children)
			{
				if (origin_hash.contains(c_item))
				{
					QStandardItem * c_proj = data_model->destParentItem(c_item);
					c_item -> setText(c_item->text()+QObject::tr("，，。")+c_proj->parent()->text());					
				}
			}
			undo_items << parent_item->takeRow(i);
			i++;
		}
	}
	else if (act_Text == QObject::tr("删除此授权代表"))
	{
		if (!child_item)
		{
			child_item = parent_item;		  
			old_pos = parent_item->row();		  
			mid_item = parent_item->parent();
			parent_item = 0;
		}
		QStandardItem * c_proj = data_model->destParentItem(child_item);
		mid_item->takeRow(old_pos);
		child_item -> setText(child_item->text()+QObject::tr("，，。")+c_proj->parent()->text());
		if (!mid_item->hasChildren())
		{
			parent_item = mid_item->parent();
			mid_pos = mid_item->row();
			parent_item -> takeRow(mid_pos);
		}
	}
	else
	{
		QStringList sel_agent = old_Text.split(",");	
		QList<QStandardItem *> cur_dprts;
		data_model -> findChildrenNotAll(parent_item, cur_dprts);		
		if (!child_item)
		{
			child_item = new QStandardItem(sel_agent.at(0));
			QMap<QString, QStringList> m_map = boss->databaseSource()->curDBmanageTable();
			QString depart_name = m_map.value(sel_agent.at(1)).at(2);
			bool found = false;
			foreach (QStandardItem * d_item, cur_dprts)
			{
				if (d_item->text() == depart_name)
				{
					mid_item = d_item;
					found = true;
					break;
				}
			}
			if (!found)
				mid_item = new QStandardItem(depart_name);
			mid_item -> appendRow(child_item);
			if (!cur_dprts.contains(mid_item))
				parent_item -> appendRow(mid_item);			  
		}
		else
		{
			if (!cur_dprts.contains(mid_item))
				parent_item -> insertRow(mid_pos, mid_item);
			mid_item -> insertRow(old_pos, child_item);
		}
		data_model -> tmpSavedItemUnshowInfo(child_item, sel_agent.at(1), true);
	}	
}

void RelatedTreeCommand::redoForBossTypeOne(NestedModel * data_model)
{
	if (act_Text == QObject::tr("修改标题"))
	{
		QObject::disconnect(data_model, SIGNAL(itemChanged(QStandardItem *)), boss, SLOT(activedItemChanged(QStandardItem *)));	  
		if (parent_item->text() != new_Text)
			parent_item -> setText(new_Text);
		QStandardItem * p_test = parent_item->parent();
		if (p_test->row() < 4)
		{
			if (data_model->fillStateCheckForNewComer())
			{
				if (unre_powers.isEmpty())
				{
					data_model -> freshModel();
					if (boss->itemTextsHash().contains(data_model->item(0)->child(0)->child(0)) || (boss->itemTextsHash().contains(data_model->item(0)->child(1)->child(0)) && boss->itemTextsHash()[data_model->item(0)->child(1)->child(0)][0]==QObject::tr("数据库里已存在")))
					{
						QList<QStandardItem *> two_list;
						two_list << data_model->item(0)->child(2)->child(0);
						unre_powers.insert(data_model->item(0)->child(2), two_list);
						QList<QStandardItem *> three_list;
						three_list << data_model->item(0)->child(3)->child(0);
						unre_powers.insert(data_model->item(0)->child(3), three_list);
						boss->itemTextsHash().remove(data_model->item(0)->child(1)->child(0));
						if (boss->itemTextsHash().contains(data_model->item(0)->child(0)->child(0)))
							return;
					}
					if (data_model->item(0)->child(4)->hasChildren())
					{
						QList<QStandardItem *> four_list;
						data_model -> findChildrenNotAll(data_model->item(0)->child(4), four_list);
						unre_powers.insert(data_model->item(0)->child(4), four_list);						
					}
					if (data_model->item(0)->child(5)->hasChildren())
					{
						QList<QStandardItem *> five_list;
						data_model -> findChildrenNotAll(data_model->item(0)->child(5), five_list);
						unre_powers.insert(data_model->item(0)->child(5), five_list);						
					}
					if (data_model->item(1)->hasChildren())
					{
						QList<QStandardItem *> back_list;
						data_model -> findChildrenNotAll(data_model->item(1), back_list);
						unre_powers.insert(data_model->item(1), back_list);
					}					
				}
				else
				{
					QHashIterator<QStandardItem *, QList<QStandardItem *> > i_hash(unre_powers);
					while (i_hash.hasNext())
					{
						i_hash.next();
						i_hash.key() -> appendRows(i_hash.value());
					}
				}
			}
		}
	}
	else if (act_Text==QObject::tr("撤销选择") || act_Text==QObject::tr("选择加入"))
	{
		if (!child_item)
		{
			old_pos = parent_item->row();
			child_item = parent_item;
			parent_item = parent_item->parent();
			if (act_Text==QObject::tr("撤销选择"))
				mid_item = data_model->item(1);
			else		  
				mid_item = data_model->item(0)->child(data_model->item(0)->rowCount()-1);
		}
		if (mid_item == data_model->item(1))
			boss->itemTextsHash()[child_item][1] = QObject::tr("选择加入");
		else
			boss->itemTextsHash()[child_item][1] = QObject::tr("撤销选择");			  		
		mid_item -> appendRow(parent_item->takeRow(old_pos));
	}
}

void RelatedTreeCommand::redoForBossTypeTwo(NestedModel * data_model)
{
	if (act_Text == QObject::tr("修改标题"))
	{
		QObject::disconnect(data_model, SIGNAL(itemChanged(QStandardItem *)), boss, SLOT(activedItemChanged(QStandardItem *)));	  
		if (parent_item->text() != new_Text)
			parent_item -> setText(new_Text);
		if (data_model->fillStateCheckForNewComer())
		{
			if (unre_powers.isEmpty())
			{
				data_model -> freshModel();
				QList<QStandardItem *> back_list;
				data_model -> findChildrenNotAll(data_model->item(1), back_list);
				unre_powers.insert(data_model->item(1), back_list);					
			}
			else
			{
				QHashIterator<QStandardItem *, QList<QStandardItem *> > i_hash(unre_powers);
				while (i_hash.hasNext())
				{
					i_hash.next();
					foreach (QStandardItem * c_item, i_hash.value())
						i_hash.key() -> appendRow(c_item);
				}
			}
		}
	}
	else if (act_Text==QObject::tr("获授权限") || act_Text==QObject::tr("授予权限"))
	{
		if (!child_item)
		{
			if (act_Text==QObject::tr("授予权限"))
			{
				child_item = new QStandardItem(new_Text);
				mid_pos = 1;
			}
			else
				child_item = parent_item->child(0);
		}
		if (child_item->text() != new_Text)
			child_item -> setText(new_Text);
		if (!parent_item->hasChildren())
			parent_item -> appendRow(child_item);
	}
	else if (act_Text == QObject::tr("删除加入者") || act_Text == QObject::tr("删除所有申请者"))
	{
		if (act_Text == QObject::tr("删除加入者"))	
			data_model->unshowHashItems()[parent_item] += ","+data_model->destParentItem(parent_item)->text();
		else
		{
			QList<QStandardItem *> all_list;
			data_model -> findChildrenNotAll(parent_item, all_list);
			foreach (QStandardItem * e_item, all_list)
				data_model->unshowHashItems()[e_item] += ","+parent_item->parent()->text();
		}	  
		if (undo_items.isEmpty())
		{
			if (act_Text == QObject::tr("删除加入者") && parent_item->parent()->rowCount()==1)
			{
				mid_item = parent_item->parent()->parent();
				old_pos = parent_item->parent()->row();
			}
			else
			{
				mid_item = parent_item->parent();
				old_pos = parent_item->row();
			}
			undo_items = mid_item->takeRow(old_pos);
		}
		else		
			mid_item->takeRow(old_pos);
	}	
}

void RelatedTreeCommand::redoForBossFourFive(NestedModel * data_model)
{
	if (act_Text == QObject::tr("添加子项"))
	{
		if (!child_item)
			child_item = new QStandardItem(QObject::tr("请修改为您想添加的"));
		parent_item -> appendRow(child_item);
	}
	else if (act_Text == QObject::tr("删除子项"))
		parent_item -> takeRow(child_item->row());	
	else if (act_Text == QObject::tr("添加兄弟项"))
	{
		if (!child_item)
			child_item = new QStandardItem(QObject::tr("请修改为您想添加的"));
		parent_item->parent()->insertRow(parent_item->row(), child_item);		
	}
	else if (act_Text == QObject::tr("修改标题"))
	{
		if (parent_item->text() != new_Text)
			parent_item -> setText(new_Text);
	}
	else if (act_Text == QObject::tr("删除此项"))
	{
		if (!child_item)
		{
			old_pos = parent_item->row();		  
			child_item = parent_item;
			parent_item = parent_item->parent();
		}
		parent_item -> takeRow(old_pos);	
	}	
	else if (act_Text==QObject::tr("删除所有") || act_Text==QObject::tr("删除其他子项"))
	{
		QList<QStandardItem *> all_children;
		data_model -> findChildrenNotAll(parent_item, all_children);
		if (act_Text == QObject::tr("删除其他子项"))
			all_children.pop_front();
		foreach (QStandardItem * item, all_children)
			undo_items << parent_item->takeRow(item->row());
	}  
}

void RelatedTreeCommand::redoForBossTypeSix(NestedModel * data_model)
{
	if (act_Text == QObject::tr("建立新项"))
	{
		data_model -> appendRow(parent_item);
		boss->itemTextsHash().remove(parent_item);
		if (data_model->resetDeleteUnredoItems().contains(parent_item))
			data_model->resetDeleteUnredoItems().removeOne(parent_item);
	}
	else if (act_Text == QObject::tr("删除子项"))
	{
		boss -> setExpanded(parent_item->index(), false);
		if (undo_items.isEmpty())
		{
			QList<QStandardItem *> all_children;
			data_model -> findChildrenNotAll(parent_item, all_children);
			foreach (QStandardItem * item, all_children)
				undo_items << parent_item->takeRow(item->row());	  
		}
		else
		{
			foreach (QStandardItem * item, undo_items)
				parent_item->takeRow(item->row());
		}
		foreach (QStandardItem * item, undo_items)
		{
			if (!data_model->resetDeleteUnredoItems().contains(item))
				data_model->resetDeleteUnredoItems() << item;
		}
	}
	else if (act_Text == QObject::tr("删除当前"))
	{
		if (!mid_item)
		{
			old_pos = parent_item->row();
			mid_item = parent_item->parent();
		}
		boss -> setExpanded(mid_item->index(), false);		
		mid_item -> takeRow(parent_item->row());
		boss -> setExpanded(mid_item->index(), true);
		QStandardItem * new_selected = data_model->itemFromIndex(boss->selectionModel()->selectedIndexes().at(0));
		boss -> resetSpecialClickedItem(new_selected);
		if (!data_model->resetDeleteUnredoItems().contains(parent_item))
			data_model->resetDeleteUnredoItems() << parent_item;		
	}	
	else if (act_Text == QObject::tr("添加兄弟项"))
	{
		if (!child_item)
			child_item = new QStandardItem(QObject::tr("请修改为您想添加的"));
		if (!parent_item->parent())
			data_model->insertRow(parent_item->row(), child_item);
		else
			parent_item->parent()->insertRow(parent_item->row(), child_item);
		if (data_model->resetDeleteUnredoItems().contains(child_item))
			data_model->resetDeleteUnredoItems().removeOne(child_item);		
	}
	else if (act_Text == QObject::tr("添加子项"))
	{
		if (!child_item)
			child_item = new QStandardItem(QObject::tr("请修改为您想添加的"));
		parent_item -> appendRow(child_item);
		if (data_model->resetDeleteUnredoItems().contains(child_item))
			data_model->resetDeleteUnredoItems().removeOne(child_item);			
	}
	else if (act_Text == QObject::tr("修改标题"))
	{
		if (parent_item->text() != new_Text)
			parent_item -> setText(new_Text);
	}
	else if (act_Text.contains(QObject::tr("工程表图选择")))
	{
		if (!child_item)
		{
			QStringList split_list = act_Text.split(QObject::tr("工程表图选择"));
			child_item = new QStandardItem(split_list.at(1));
		}
		parent_item -> appendRow(child_item);
		if (data_model->resetDeleteUnredoItems().contains(child_item))
			data_model->resetDeleteUnredoItems().removeOne(child_item);			
	}	
}

void RelatedTreeCommand::redoForBossTypeSeven(NestedModel * data_model)
{
	if (act_Text == QObject::tr("编辑数据库名"))
	{
		if (parent_item->text() != new_Text)
			parent_item -> setText(new_Text);
	}  
	else if (act_Text == QObject::tr("移出当前"))
	{
		if (!child_item)
			child_item = new QStandardItem(QObject::tr("在此创建数据库\n或在”选择数据库”\n选择"));
		data_model->item(1)->appendRow(data_model->item(0)->takeRow(0).at(0));
		data_model->item(0)->appendRow(child_item);
		QStringList new1_coms;			  
		new1_coms << QObject::tr("不操作") << QObject::tr("编辑数据库名");
		boss->itemTextsHash()[child_item] = new1_coms;
		QStringList new2_coms;			  
		new2_coms << QObject::tr("不操作") << QObject::tr("编辑数据库名") << QObject::tr("作为当前数据库");
		boss->itemTextsHash()[parent_item] = new2_coms;		
	}		
	else if (act_Text == QObject::tr("作为当前数据库"))
	{
		if (!child_item)
			child_item = data_model->item(0)->takeRow(0).at(0);
		else
			data_model->item(0)->takeRow(0);
		if (!old_pos)
			old_pos = parent_item->row();
		data_model->item(0)->appendRow(data_model->item(1)->takeRow(old_pos));
		QStringList new_coms;			  
		new_coms << QObject::tr("不操作") << QObject::tr("编辑数据库名") << QObject::tr("移出当前");				  
		boss->itemTextsHash()[parent_item] = new_coms;
		boss->itemTextsHash().remove(child_item);		
	}
	else if (act_Text == QObject::tr("交换数据库"))
	{
		if (!old_pos)
			old_pos = parent_item->row();
		if (undo_items.isEmpty())
			undo_items = data_model->item(0)->takeRow(0);
		data_model->item(0)->appendRow(data_model->item(1)->takeRow(old_pos));
		data_model->item(1)->insertRow(old_pos, undo_items[0]);
		QStringList new1_coms;			  
		new1_coms << QObject::tr("不操作") << QObject::tr("编辑数据库名") << QObject::tr("移出当前");
		QStringList new2_coms;			  
		new2_coms << QObject::tr("不操作") << QObject::tr("编辑数据库名") << QObject::tr("交换数据库") << QObject::tr("合并入当前数据库");	
		boss->itemTextsHash()[parent_item] = new1_coms;
		boss->itemTextsHash()[undo_items[0]] = new2_coms;
	}	
	else if (act_Text==QObject::tr("合并入当前数据库") || act_Text==QObject::tr("删除数据库"))
	{
		if (!old_pos)
			old_pos = parent_item->row();		
		if (!mid_item)
			mid_item = parent_item->parent();
		mid_item -> takeRow(old_pos);
		if (act_Text==QObject::tr("合并入当前数据库"))
		{
			QString merge_to = QObject::tr("合并入")+";"+QString::number((long)data_model->item(0)->child(0), 16);
			data_model->unshowHashItems().insert(parent_item, merge_to);
		}
		if (mid_item == data_model->item(0))
		{
			if (!child_item)
				child_item = new QStandardItem(QObject::tr("在此创建数据库\n或在”选择数据库”\n选择"));		  
			mid_item -> appendRow(child_item);
		}
		if (data_model->originHashItems().contains(parent_item) && act_Text==QObject::tr("删除数据库"))
			data_model->unshowHashItems().insert(parent_item, act_Text);	
	}
}

void RelatedTreeCommand::undoForBossTypeZero(NestedModel * data_model)
{
	if (act_Text == QObject::tr("删除所有"))
	{
		QHashIterator<QStandardItem *, QPair<QStandardItem *, QString> > it_hash(data_model->originHashItems());
		while (it_hash.hasNext())
		{
			it_hash.next();
			if (it_hash.key()->text().contains(QObject::tr("，，。")))
			{
				QStringList c_detail = it_hash.key()->text().split(QObject::tr("，，。"));
				it_hash.key() -> setText(c_detail.at(0));
			}
		}
		foreach (QStandardItem * item, undo_items)
			parent_item -> appendRow(item);
		undo_items.clear();
	}
	else if (act_Text == QObject::tr("删除此授权代表"))
	{
		if (parent_item)
		{
			parent_item -> insertRow(mid_pos, mid_item);
			parent_item = 0;
		}
		QStringList c_detail = child_item->text().split(QObject::tr("，，。"));
		child_item -> setText(c_detail.at(0));					
		mid_item -> insertRow(old_pos, child_item);
	}
	else
	{
		QStringList sel_agent = old_Text.split(",");	  
		data_model -> tmpSavedItemUnshowInfo(child_item, sel_agent.at(1), false);
		old_pos = child_item->row();
		mid_item -> takeRow(child_item->row());
		if (!mid_item->hasChildren())
		{
			mid_pos = mid_item->row();
			parent_item -> takeRow(mid_item->row());
		}
	}	  
}

void RelatedTreeCommand::undoForBossTypeOne(NestedModel * data_model)
{
	if (act_Text == QObject::tr("修改标题") )
	{
		QObject::disconnect(data_model, SIGNAL(itemChanged(QStandardItem *)), boss, SLOT(activedItemChanged(QStandardItem *)));	  
		if (parent_item->text() != old_Text)
			parent_item -> setText(old_Text);
		QHashIterator<QStandardItem *, QList<QStandardItem *> > i_hash(unre_powers);
		while (i_hash.hasNext())
		{
			i_hash.next();
			foreach (QStandardItem * item, i_hash.value())
				i_hash.key() -> takeRow(item->row());
		}
	}
	else if (act_Text==QObject::tr("撤销选择") || act_Text==QObject::tr("选择加入"))
	{
		if (parent_item == data_model->item(1))
			boss->itemTextsHash()[child_item][1] = QObject::tr("选择加入");	
		else
			boss->itemTextsHash()[child_item][1] = QObject::tr("撤销选择");		  
		parent_item -> insertRow(old_pos, mid_item->takeRow(child_item->row()));
	}	
}

void RelatedTreeCommand::undoForBossTypeTwo(NestedModel * data_model)
{
	if (act_Text == QObject::tr("修改标题"))
	{
		QObject::disconnect(data_model, SIGNAL(itemChanged(QStandardItem *)), boss, SLOT(activedItemChanged(QStandardItem *)));	  
		if (parent_item->text() != old_Text)
			parent_item -> setText(old_Text);
		QHashIterator<QStandardItem *, QList<QStandardItem *> > i_hash(unre_powers);
		while (i_hash.hasNext())
		{
			i_hash.next();
			foreach (QStandardItem * item, i_hash.value())
				i_hash.key() -> takeRow(item->row());
		}
	} 
	else if (act_Text==QObject::tr("获授权限") || act_Text==QObject::tr("授予权限"))
	{
		if (child_item->text() != old_Text)
			child_item -> setText(old_Text);
		if (mid_pos)
			parent_item -> takeRow(0);
	}
	else if (act_Text == QObject::tr("删除加入者") || act_Text == QObject::tr("删除所有申请者"))
	{
		mid_item -> insertRow(old_pos, undo_items);	  
		if (act_Text == QObject::tr("删除加入者"))	
		{
			QStringList old_content = data_model->unshowHashItems()[parent_item].split(",");
			data_model->unshowHashItems()[parent_item] = old_content[0];
		}
		else
		{
			QList<QStandardItem *> all_list;
			data_model -> findChildrenNotAll(parent_item, all_list);
			foreach (QStandardItem * e_item, all_list)
			{
				QStringList old_content = data_model->unshowHashItems()[e_item].split(",");
				data_model->unshowHashItems()[e_item] = old_content[0];
			}
		}	  
	}		
}

void RelatedTreeCommand::undoForBossFourFive(NestedModel * data_model)
{
	Q_UNUSED(data_model);
	if (act_Text == QObject::tr("添加子项"))
		parent_item -> takeRow(child_item->row());
	else if (act_Text == QObject::tr("删除子项"))
		parent_item -> appendRow(child_item);		
	else if (act_Text == QObject::tr("添加兄弟项"))
		parent_item->parent()->takeRow(parent_item->row()-1);
	else if (act_Text == QObject::tr("修改标题"))
	{
		if (parent_item->text() != old_Text)
			parent_item -> setText(old_Text);
	}
	else if (act_Text == QObject::tr("删除此项"))
		parent_item -> insertRow(old_pos, child_item);		
	else if (act_Text == QObject::tr("删除所有") || act_Text==QObject::tr("删除其他子项"))
	{
		foreach (QStandardItem * item, undo_items)
			parent_item -> appendRow(item);
		undo_items.clear();
	} 
}

void RelatedTreeCommand::undoForBossTypeSix(NestedModel * data_model)
{
	if (act_Text == QObject::tr("建立新项"))
	{
		data_model -> takeRow(0);
		if (!data_model->resetDeleteUnredoItems().contains(parent_item))
			data_model->resetDeleteUnredoItems() << parent_item;		
	}
	else if (act_Text == QObject::tr("删除子项"))
	{
		boss -> setExpanded(parent_item->index(), false);
		foreach (QStandardItem * item, undo_items)
			parent_item -> appendRow(item);
		boss -> setExpanded(parent_item->index(), true);
		foreach (QStandardItem * item, undo_items)
		{
			if (data_model->resetDeleteUnredoItems().contains(item))
				data_model->resetDeleteUnredoItems().removeOne(item);
		}
	}
	else if (act_Text == QObject::tr("删除当前"))
	{
		boss -> setExpanded(mid_item->index(), false);
		mid_item -> insertRow(old_pos, parent_item);
		boss -> setExpanded(mid_item->index(), true);
		if (data_model->resetDeleteUnredoItems().contains(parent_item))
			data_model->resetDeleteUnredoItems().removeOne(parent_item);
	}
	else if (act_Text == QObject::tr("添加兄弟项"))
	{
		if (!parent_item->parent())
			data_model -> takeRow(parent_item->row()-1);
		else
			parent_item->parent()->takeRow(parent_item->row()-1);
		if (!data_model->resetDeleteUnredoItems().contains(child_item))
			data_model->resetDeleteUnredoItems() << child_item;		
	}
	else if (act_Text == QObject::tr("添加子项"))
	{
		parent_item -> takeRow(child_item->row());
		if (!data_model->resetDeleteUnredoItems().contains(child_item))
			data_model->resetDeleteUnredoItems() << child_item;			
	}
	else if (act_Text == QObject::tr("修改标题"))
	{
		if (parent_item->text() != old_Text)
			parent_item -> setText(old_Text);
	}
	else if (act_Text.contains(QObject::tr("工程表图选择")))
	{
		parent_item -> takeRow(child_item->row());	 
		if (!data_model->resetDeleteUnredoItems().contains(child_item))
			data_model->resetDeleteUnredoItems() << child_item;		
	}
}

void RelatedTreeCommand::undoForBossTypeSeven(NestedModel * data_model)
{
	if (act_Text == QObject::tr("编辑数据库名"))
	{
		if (parent_item->text() != old_Text)
			parent_item -> setText(old_Text);
	}  
	else if (act_Text == QObject::tr("移出当前"))
	{
		data_model->item(0)->takeRow(0);
		data_model->item(0)->appendRow(data_model->item(1)->takeRow(parent_item->row()).at(0));		
		QStringList new_coms;			  
		new_coms << QObject::tr("不操作") << QObject::tr("编辑数据库名") << QObject::tr("移出当前");
		boss->itemTextsHash()[parent_item] = new_coms;
		boss->itemTextsHash().remove(child_item);
	}		
	else if (act_Text == QObject::tr("作为当前数据库"))
	{
		data_model->item(1)->insertRow(old_pos, data_model->item(0)->takeRow(0).at(0));
		data_model->item(0)->appendRow(child_item);
		QStringList new1_coms;			  
		new1_coms << QObject::tr("不操作") << QObject::tr("编辑数据库名") << QObject::tr("作为当前数据库");				  
		boss->itemTextsHash()[parent_item] = new1_coms;
		QStringList new2_coms;			  
		new2_coms << QObject::tr("不操作") << QObject::tr("编辑数据库名");		
		boss->itemTextsHash()[child_item] = new2_coms;		
	}
	else if (act_Text == QObject::tr("交换数据库"))
	{
		data_model->item(0)->appendRow(data_model->item(1)->takeRow(old_pos));
		data_model->item(1)->insertRow(old_pos, undo_items[0]);
		QStringList new1_coms;			  
		new1_coms << QObject::tr("不操作") << QObject::tr("编辑数据库名") << QObject::tr("移出当前");
		QStringList new2_coms;			  
		new2_coms << QObject::tr("不操作") << QObject::tr("编辑数据库名") << QObject::tr("交换数据库") << QObject::tr("合并入当前数据库");	
		boss->itemTextsHash()[undo_items[0]] = new1_coms;
		boss->itemTextsHash()[parent_item] = new2_coms;
	}	
	else if (act_Text==QObject::tr("合并入当前数据库") || act_Text==QObject::tr("删除数据库"))
	{
		if (child_item && child_item->text() == QObject::tr("在此创建数据库\n或在”选择数据库”\n选择"))
		{
			mid_item -> takeRow(0);
			mid_item -> appendRow(parent_item);
		}
		else
			mid_item -> insertRow(old_pos, parent_item);
		data_model->unshowHashItems().remove(parent_item);			
	}	
}
