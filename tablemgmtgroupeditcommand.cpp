#include <QtCore>
#include <QtGui>
#include "tablemgmtgroupeditcommand.h"
#include "tablemanagement.h"
#include "tablesgroup.h"
#include "littlewidgetsview.h"
#include "plotwidget.h"
#include "dataselectmodel.h"

TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TableManagement * tree_guide, QStandardItem * info, const QString & order_text, TablesGroup * table_obj)
: QUndoCommand(), mid_item(0), child_item(0), data_model(0), sized_model(0)
{
	boss1 = tree_guide;
	parent_item = info;
	orderText = order_text;	
	boss2 = table_obj;
}

TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TableManagement * tree_guide, QStandardItem * info, const QString & order_text, const QList<QStandardItem *> & sel_dbs)
: QUndoCommand(), child_row(0), mid_item(0), child_item(0), data_model(0), sized_model(0), boss2(0)
{
	boss1 = tree_guide;
	parent_item = info;
	orderText = order_text;	
	undo_items = sel_dbs;
}

TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TablesGroup * table_obj, const QString & order_text, const QPair<QModelIndex, QString> & edited_strs)
: QUndoCommand(), parent_item(0), child_item(0), data_model(0), sized_model(0), boss1(0)
{
	boss2 = table_obj;
	orderText = order_text;
	newold_strs = edited_strs;
}

TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TablesGroup * table_obj, const QString & order_text, const QPair<QString, QModelIndexList> & framed_pair)
: QUndoCommand(), parent_item(0), child_item(0), data_model(0), sized_model(0), boss1(0)
{
	boss2 = table_obj;
	orderText = order_text;
	framed_list = framed_pair;	
}

TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TablesGroup * table_obj, const QString & order_text, const QMap<QModelIndex, QString> & selected_map)
: QUndoCommand(), parent_item(0), child_item(0), data_model(0), sized_model(0), boss1(0)
{
	boss2 = table_obj;
	orderText = order_text;
	contents_map = selected_map;
}


TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TablesGroup * table_obj, const QString & order_text, const QHash<QModelIndex, QPair<QString, QStandardItem *> > & selected_cells)
: QUndoCommand(), parent_item(0), child_item(0), data_model(0), sized_model(0), boss1(0)
{
	boss2 = table_obj;
	orderText = order_text;
	related_hash = selected_cells;
}

TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TablesGroup * table_obj, const QString & order_text, const QHash<QModelIndex, QPair<int, int> > & selected_cells)
: QUndoCommand(), parent_item(0), child_item(0), data_model(0), sized_model(0), boss1(0)
{
	boss2 = table_obj;
	orderText = order_text;
	cells_size = selected_cells;
}

TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TableManagement * tree_guide, TablesGroup * table_obj, QStandardItem * selected_item, QStandardItem * new_item, const QString & order_text)
: QUndoCommand(), data_model(0), sized_model(0), boss1(0)
{
	boss1 = tree_guide;
	boss2 = table_obj;
	parent_item = selected_item;
	child_item = new_item;
	orderText = order_text;
}

TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TableManagement * tree_guide, TablesGroup * table_obj, const QString & order_text, const QHash<QModelIndex, QPair<QMap<int, QVariant>, QStandardItem *> > & selected_cells)
: QUndoCommand(), parent_item(0), child_item(0), data_model(0), sized_model(0)
{
	boss1 = tree_guide;
	boss2 = table_obj;
	orderText = order_text;
	sel_cells = selected_cells;	
}

TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TableManagement * tree_guide, TablesGroup * table_obj, QStandardItem * info, QStandardItem * pos, QStandardItemModel * data_backup, QStandardItemModel * old_model, const QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & changing_hash, const QString & order_text)
: QUndoCommand(), sized_model(0)
{
	boss1 = tree_guide;
	boss2 = table_obj;
	parent_item = info;
	child_item = pos;
	data_model = data_backup;
	sized_model = old_model;
	j_ptr = QSharedPointer<QStandardItemModel>(sized_model, &QObject::deleteLater);
	before_hash = changing_hash;
	orderText = order_text;	
}

TableMgmtGroupEditCmd::TableMgmtGroupEditCmd(TableManagement * tree_guide, TablesGroup * table_obj, const QString & order_text, const QPair<QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> >, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > > & selected_pair)
: QUndoCommand(), parent_item(0), child_item(0), data_model(0), sized_model(0)
{
	boss1 = tree_guide;
	boss2 = table_obj;
	orderText = order_text;
	paste_pair = selected_pair;
	QHashIterator<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > new_hash(paste_pair.first);
	while (new_hash.hasNext())//maybe finished?
	{
		new_hash.next();
		if (new_hash.value().second && !pos_items.contains(new_hash.value().second))
		{
			QString h_pos(QString("%1").arg(new_hash.key().first.row()+1));
			QString v_pos(QString("%1").arg(new_hash.key().first.column()+1));
			QString pos_text = QObject::tr("视图位置：行")+h_pos+QObject::tr("，")+QObject::tr("列")+v_pos;
			QList<QStandardItem *> pos_list;
			int i = 0;
			while (new_hash.value().second->child(i))
			{
				if (new_hash.value().second->child(i)->text() == pos_text)
				{
					pos_list << new_hash.value().second->child(i);
					break;
				}
				i++;
			}
			pos_items.insert(new_hash.value().second, pos_list);
		}
	}
	QHashIterator<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > old_hash(paste_pair.second);
	while (old_hash.hasNext())
	{
		old_hash.next();
		if (old_hash.value().second && !pos_items.contains(old_hash.value().second))
		{
			QString h_pos(QString("%1").arg(old_hash.key().first.row()+1));
			QString v_pos(QString("%1").arg(old_hash.key().first.column()+1));
			QString pos_text = QObject::tr("视图位置：行")+h_pos+QObject::tr("，")+QObject::tr("列")+v_pos;
			QList<QStandardItem *> pos_list;
			int i = 0;
			while (old_hash.value().second->child(i))
			{
				if (old_hash.value().second->child(i)->text() == pos_text)
				{
					pos_list << old_hash.value().second->child(i);
					break;
				}
				i++;
			}
			pos_items.insert(old_hash.value().second, pos_list);
		}
	}
}

TableMgmtGroupEditCmd::~TableMgmtGroupEditCmd()
{
	if (data_model)
		delete data_model;
	QHashIterator<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > bi_hash(before_hash);
	while (bi_hash.hasNext())
	{
		bi_hash.next();
		if (bi_hash.value().second)
			delete bi_hash.value().second;
	}
	if (!j_ptr.isNull())
		j_ptr.clear();
}

void TableMgmtGroupEditCmd::redo()
{
	if (boss1 && boss1->treeType() == 2)
		redoOnlyForTwoTypeBossOne();
	else
	{
		if (orderText == QObject::tr("清除选择"))
		{
			if (undo_items.isEmpty())
			{
				DataSelectModel * t_model = qobject_cast<DataSelectModel *>(parent_item->model());
				t_model -> findChildrenNotAll(parent_item, undo_items);
			}
			boss1 -> setExpanded(parent_item->index(), false);
			foreach (QStandardItem * item, undo_items)
			{
				parent_item -> takeRow(item->row());
				if (!boss1->keepDeletingItems().contains(item))
					boss1->keepDeletingItems() << item;
			}
		}
		else if (orderText == QObject::tr("重新选择"))
		{
			if (undo_items.isEmpty())
			{
				DataSelectModel * t_model = qobject_cast<DataSelectModel *>(parent_item->model());
				t_model -> findChildrenNotAll(parent_item->child(0), undo_items);
			}
			boss1 -> setExpanded(parent_item->child(0)->index(), false);
			foreach (QStandardItem * item, undo_items)
			{
				parent_item->child(0)->takeRow(item->row());
				if (!boss1->keepDeletingItems().contains(item))
					boss1->keepDeletingItems() << item;
			}
			boss1 -> removeItemChildrenInHash(parent_item, names_texts);
		}
		else if (orderText==QObject::tr("文字编辑"))//not unredo exact
			boss2 -> unredoForCellTextEdit(newold_strs, true);
		else if (orderText==QObject::tr("清空内容") || orderText==QObject::tr("清空格式"))
			boss2 -> unredoForClearContents(orderText, sel_cells, true);
		else if (orderText == QObject::tr("剪切"))
			boss2 -> unredoForCut(paste_pair.first, true);
		else if (orderText == QObject::tr("粘贴"))
		{
			QList<QPair<QModelIndex, QString> > pair_list1 = paste_pair.second.keys();
			QModelIndexList old_list;
			for (int i = 0; i < pair_list1.size(); i++)
				old_list << pair_list1.at(i).first;
			qSort(old_list);
			QList<QPair<QModelIndex, QString> > pair_list2 = paste_pair.first.keys();
			QModelIndexList new_list;
			for (int i = 0; i < pair_list2.size(); i++)
				new_list << pair_list2.at(i).first;
			qSort(new_list);
			int r_offset = old_list.at(0).row()-new_list.at(0).row();
			int c_offset = old_list.at(0).column()-new_list.at(0).column();
			QHashIterator<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > new_hash(paste_pair.first);
			while (new_hash.hasNext())
			{
				new_hash.next();
				if (new_hash.value().second)
				{
					QString key_hint = new_hash.key().second;
					QString h_pos(QString("%1").arg(new_hash.key().first.row()+r_offset+1));
					QString v_pos(QString("%1").arg(new_hash.key().first.column()+c_offset+1));
					QString pos_text = QObject::tr("视图位置：行")+h_pos+QObject::tr("，")+QObject::tr("列")+v_pos;
					if (key_hint.contains(";"))
					{
						QList<QStandardItem *> e_items = pos_items.value(new_hash.value().second);
						QStandardItem * pos_append = 0;
						bool found = false;
						foreach (QStandardItem * item, e_items)
						{
							if (item->text().contains(pos_text ))
							{
								pos_append = item;
								QStringList pos_list = item->text().split(QObject::tr("覆盖"));
								int r_append = pos_list[1].toInt();
								new_hash.value().second -> insertRow(r_append, pos_append);
								pos_append -> setText(pos_text);
								found = true;
								break;
							}
						}
						if (!found)
						{
							pos_append = new QStandardItem(pos_text);
							pos_items[new_hash.value().second] << pos_append;
							new_hash.value().second -> appendRow(pos_append);
						}
					}
					else
					{
						QString h_last(QString("%1").arg(new_hash.key().first.row()+1));
						QString v_last(QString("%1").arg(new_hash.key().first.column()+1));
						QString pos_last = QObject::tr("视图位置：行")+h_last+QObject::tr("，")+QObject::tr("列")+v_last;
						QStandardItem * plot_item = new_hash.value().second;
						int i = 0;
						while (plot_item->child(i))
						{
							if (plot_item->child(i)->text() == pos_last)
							{
								plot_item->child(i)->setText(pos_text);
								break;
							}
							i++;
						}
					}
				}
			}
			QHashIterator<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > old_hash(paste_pair.second);
			while (old_hash.hasNext())
			{
				old_hash.next();
				if (old_hash.value().second)
				{
					QString key_hint = old_hash.key().second;
					QString h_pos(QString("%1").arg(old_hash.key().first.row()+1));
					QString v_pos(QString("%1").arg(old_hash.key().first.column()+1));
					QString pos_text = QObject::tr("视图位置：行")+h_pos+QObject::tr("，")+QObject::tr("列")+v_pos;
					for (int i = 0; i < pos_items.value(old_hash.value().second).size(); i++)
					{
						if (pos_items.value(old_hash.value().second).at(i)->text() == pos_text)
						{
							int r_take = pos_items.value(old_hash.value().second).at(i)->row();
							pos_text += QObject::tr("覆盖")+QString("%1").arg(r_take);
							old_hash.value().second -> takeRow(r_take);
							pos_items[old_hash.value().second][i] -> setText(pos_text);
							break;
						}
					}
				}
			}		
			boss2 -> unredoForPaste(paste_pair, true);
		}
		else if (orderText==QObject::tr("左对齐") || orderText==QObject::tr("居中") || orderText==QObject::tr("右对齐"))
			boss2 -> unredoForCellsTextAlign(related_hash, true);
		else if (orderText==QObject::tr("合并单元") || orderText==QObject::tr("取消合并"))
			boss2 -> unredoForCellsMerge(related_hash, orderText, true);
		else if (orderText==QObject::tr("插入行") || orderText==QObject::tr("插入列"))
			boss2 -> unredoForInsertRowsCols(contents_map, orderText, true);
		else if (orderText==QObject::tr("删除行") || orderText==QObject::tr("删除列"))
			boss2 -> unredoForRemoveRowsCols(contents_map, orderText, true);
		else if (orderText.contains(QObject::tr("粗匣框线")) || orderText.contains(QObject::tr("外侧框线")) || orderText.contains(QObject::tr("所有框线")) || orderText.contains(QObject::tr("左框线")) || orderText.contains(QObject::tr("右框线")) || orderText.contains(QObject::tr("上框线")) || orderText.contains(QObject::tr("下框线")) || orderText.contains(QObject::tr("无框线")))
			boss2 -> unredoForCellsFrame(framed_list, true);	
		else if (orderText == QObject::tr("行高："))
			boss2 -> unredoSizeForCells(cells_size, false, true);
		else if (orderText == QObject::tr("列宽："))
			boss2 -> unredoSizeForCells(cells_size, true, true);
		else if (orderText == QObject::tr("文字"))
			boss2 -> unredoColorForCells(related_hash, true, true);
		else if (orderText == QObject::tr("表格"))
			boss2 -> unredoColorForCells(related_hash, false, true);
		else if (orderText == QObject::tr("添加数据"))
		{
			if (child_item && !boss1->keepDeletingItems().contains(child_item))
				boss1->keepDeletingItems() << child_item;				
			boss2 -> unredoForTreeDataesShow(parent_item, child_item, data_model, sized_model, before_hash, true);	
		}
		else if (orderText == QObject::tr("添加图"))
		{
			DataSelectModel * t_model = qobject_cast<DataSelectModel *>(boss1->model());
			QList<QStandardItem *> engi_line;
			t_model -> findItemsSingleLine(parent_item, engi_line, t_model->item(0)->text());
			if (engi_line.at(1)->text() == QObject::tr("其他图源"))
				boss1 -> unredoFolderPictureForTmtbl(parent_item, before_hash, true);	 
			else
				boss1 -> unredoGeneratePlotForTmtbl(parent_item, before_hash, true);	
		}
		else
		{
			parent_item -> appendRow(child_item);
			child_row = child_item->row();
			if (boss1->keepDeletingItems().contains(child_item))
				boss1->keepDeletingItems().removeOne(child_item);			
		}
	}
}

void TableMgmtGroupEditCmd::undo()
{
 	if (boss1 && boss1->treeType() == 2)
		undoOnlyForTwoTypeBossOne();
	else
	{ 
		if (orderText == QObject::tr("清除选择"))
		{
			foreach (QStandardItem * item, undo_items)
			{
				parent_item->appendRow(item);
				if (boss1->keepDeletingItems().contains(item))
					boss1->keepDeletingItems().removeOne(item);				
			}
			boss1 -> setExpanded(parent_item->index(), true);
		}
		else if (orderText == QObject::tr("重新选择"))
		{
			foreach (QStandardItem * item, undo_items)
			{
				parent_item->child(0)->appendRow(item);
				if (boss1->keepDeletingItems().contains(item))
					boss1->keepDeletingItems().removeOne(item);				
			}
			boss1 -> setExpanded(parent_item->child(0)->index(), true);
			boss1 -> reinsertItemChildrenInHash(names_texts);
		}
		else if (orderText==QObject::tr("文字编辑"))
			boss2 -> unredoForCellTextEdit(newold_strs, false);
		else if (orderText==QObject::tr("清空内容") || orderText==QObject::tr("清空格式"))
			boss2 -> unredoForClearContents(orderText, sel_cells, false);
		else if (orderText == QObject::tr("剪切"))
			boss2 -> unredoForCut(paste_pair.first, false);
		else if (orderText == QObject::tr("粘贴"))
		{
			QList<QPair<QModelIndex, QString> > pair_list1 = paste_pair.second.keys();
			QModelIndexList old_list;
			for (int i = 0; i < pair_list1.size(); i++)
				old_list << pair_list1.at(i).first;
			qSort(old_list);
			QList<QPair<QModelIndex, QString> > pair_list2 = paste_pair.first.keys();
			QModelIndexList new_list;
			for (int i = 0; i < pair_list2.size(); i++)
				new_list << pair_list2.at(i).first;
			qSort(new_list);
			int r_offset = old_list.at(0).row()-new_list.at(0).row();
			int c_offset = old_list.at(0).column()-new_list.at(0).column();
			QHashIterator<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > new_hash(paste_pair.first);
			while (new_hash.hasNext())
			{
				new_hash.next();
				if (new_hash.value().second)
				{
					QString key_hint = new_hash.key().second;
					QString h_pos(QString("%1").arg(new_hash.key().first.row()+r_offset+1));
					QString v_pos(QString("%1").arg(new_hash.key().first.column()+c_offset+1));
					QString pos_text = QObject::tr("视图位置：行")+h_pos+QObject::tr("，")+QObject::tr("列")+v_pos;
					if (key_hint.contains(";"))
					{					
						for (int i = 0; i < pos_items.value(new_hash.value().second).size(); i++)
						{
							if (pos_items.value(new_hash.value().second).at(i)->text() == pos_text)
							{
								int r_take = pos_items.value(new_hash.value().second).at(i)->row();
								pos_text += QObject::tr("覆盖")+QString("%1").arg(r_take);
								new_hash.value().second -> takeRow(r_take);
								pos_items[new_hash.value().second][i] -> setText(pos_text);
								break;
							}
						}
					}
					else
					{
						QString h_old(QString("%1").arg(new_hash.key().first.row()+1));
						QString v_old(QString("%1").arg(new_hash.key().first.column()+1));
						QString old_text = QObject::tr("视图位置：行")+h_old+QObject::tr("，")+QObject::tr("列")+v_old;
						QStandardItem * plot_item = new_hash.value().second;
						int i = 0;
						while (plot_item->child(i))
						{
							if (plot_item->child(i)->text() == pos_text)
							{
								plot_item->child(i)->setText(old_text);
								break;
							}
							i++;
						}
					}
				}
			}
			QHashIterator<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > old_hash(paste_pair.second);
			while (old_hash.hasNext())
			{
				old_hash.next();
				if (old_hash.value().second)
				{
					QString key_hint = old_hash.key().second;
					QString h_pos(QString("%1").arg(old_hash.key().first.row()+1));
					QString v_pos(QString("%1").arg(old_hash.key().first.column()+1));
					QString pos_text = QObject::tr("视图位置：行")+h_pos+QObject::tr("，")+QObject::tr("列")+v_pos;
					for (int i = 0; i < pos_items.value(old_hash.value().second).size(); i++)
					{
						if (pos_items.value(old_hash.value().second).at(i)->text().contains(pos_text))
						{
							QStringList pos_list = pos_items.value(old_hash.value().second).at(i)->text().split(QObject::tr("覆盖"));
							int r_append = pos_list[1].toInt();
							new_hash.value().second -> insertRow(r_append, pos_items.value(old_hash.value().second).at(i));
							pos_items[old_hash.value().second][i] -> setText(pos_list[0]);
							break;
						}
					}
				}
			}
			boss2 -> unredoForPaste(paste_pair, false);
		}
		else if (orderText==QObject::tr("左对齐") || orderText==QObject::tr("居中") || orderText==QObject::tr("右对齐"))
			boss2 -> unredoForCellsTextAlign(related_hash, false);
		else if (orderText==QObject::tr("合并单元") || orderText==QObject::tr("取消合并"))
			boss2 -> unredoForCellsMerge(related_hash, orderText, false);
		else if (orderText==QObject::tr("插入行") || orderText==QObject::tr("插入列"))
			boss2 -> unredoForInsertRowsCols(contents_map, orderText, false);
		else if (orderText==QObject::tr("删除行") || orderText==QObject::tr("删除列"))
			boss2 -> unredoForRemoveRowsCols(contents_map, orderText, false);
		else if (orderText.contains(QObject::tr("粗匣框线")) || orderText.contains(QObject::tr("外侧框线")) || orderText.contains(QObject::tr("所有框线")) || orderText.contains(QObject::tr("左框线")) || orderText.contains(QObject::tr("右框线")) || orderText.contains(QObject::tr("上框线")) || orderText.contains(QObject::tr("下框线")) || orderText.contains(QObject::tr("无框线")))
			boss2 -> unredoForCellsFrame(framed_list, false);
		else if (orderText == QObject::tr("行高："))
			boss2 -> unredoSizeForCells(cells_size, false, true);
		else if (orderText == QObject::tr("行高："))
			boss2 -> unredoSizeForCells(cells_size, false, false);
		else if (orderText == QObject::tr("列宽："))
			boss2 -> unredoSizeForCells(cells_size, true, false);
		else if (orderText == QObject::tr("文字"))
			boss2 -> unredoColorForCells(related_hash, true, false);
		else if (orderText == QObject::tr("表格"))
			boss2 -> unredoColorForCells(related_hash, false, false);
		else if (orderText == QObject::tr("添加数据"))
			boss2 -> unredoForTreeDataesShow(parent_item, child_item, data_model, sized_model, before_hash, false);	
		else if (orderText == QObject::tr("添加图"))
		{
			DataSelectModel * t_model = qobject_cast<DataSelectModel *>(boss1->model());
			QList<QStandardItem *> engi_line;
			t_model -> findItemsSingleLine(parent_item, engi_line, t_model->item(0)->text());
			if (engi_line.at(1)->text() == QObject::tr("其他图源"))
				boss1 -> unredoFolderPictureForTmtbl(parent_item, before_hash, false);	 
			else
				boss1 -> unredoGeneratePlotForTmtbl(parent_item, before_hash, false);	
		}		
		else
		{
			undo_items = parent_item->takeRow(child_row);
			if (!boss1->keepDeletingItems().contains(child_item))
				boss1->keepDeletingItems() << child_item;			
			undo_items.push_front(parent_item);				
			if (!parent_item->hasChildren())
				boss1 -> setExpanded(parent_item->index(), false);	
		}	
	}
}

const QString & TableMgmtGroupEditCmd::orderInfo()
{
	return orderText;
}

void TableMgmtGroupEditCmd::redoOnlyForTwoTypeBossOne()
{
	if (orderText==QObject::tr("单独备份") || orderText.contains(QObject::tr("备份到......")))
	{
		DataSelectModel * tree_model = qobject_cast<DataSelectModel *>(parent_item->model());	  
		if (!child_item)
		{
			child_item = new QStandardItem(parent_item->text());
			if (orderText == QObject::tr("单独备份"))
			{
				mid_item = new QStandardItem(QObject::tr("请修改为新的数据库名"));
				tree_model->item(2)->appendRow(mid_item);
			}
			else
			{
				QStringList info_list = orderText.split(QObject::tr("，"));
				QList<QStandardItem *> dbs_list;
				tree_model -> findChildrenNotAll(tree_model->item(3), dbs_list);
				foreach (QStandardItem * db_item, dbs_list)
				{
					if (db_item->text() == info_list[0])
					{
						mid_item = db_item;
						break;
					}
				}
			}
			mid_item -> appendRow(child_item);			
			child_row = child_item->row();
		}
		else
		{
			if (orderText == QObject::tr("单独备份"))
			{
				tree_model->item(2)->insertRow(child_row, mid_item);
				if (undo_items.contains(mid_item))
					undo_items.removeOne(mid_item);
			}
			else
			{
				mid_item -> insertRow(child_row, child_item);
				if (undo_items.contains(child_item))
					undo_items.removeOne(child_item);				
			}
		}
		boss1->backupManualFileItems()[child_item] << parent_item;
	}
	else if (orderText==QObject::tr("删除此工程") || orderText==QObject::tr("删除此自制表"))
	{
		if (!child_item)
		{
			child_item = new QStandardItem(QObject::tr("标记为删除"));
			child_row = 0;
			undo_items << child_item;
		}
		parent_item -> insertRow(child_row, child_item);		
	}
	else if (orderText==QObject::tr("取消备份"))
	{
		DataSelectModel * tree_model = qobject_cast<DataSelectModel *>(parent_item->model());
		if (!child_item)
		{
			child_item = parent_item;
			mid_item = parent_item->parent();			 
			parent_item = mid_item->parent();
			if (parent_item == tree_model->item(2))
				child_row = mid_item->row();
			else
				child_row = child_item->row();
		}
		if (parent_item == tree_model->item(2))
			parent_item -> takeRow(child_row);
		else
			mid_item -> takeRow(child_row);
	}
	else if (orderText.contains(QObject::tr("修改名称")) || orderText.contains(QObject::tr("修改标题")))
	{	  
		QStringList order_old = orderText.split(QObject::tr("，，。"));		
		if (order_old[0]==QObject::tr("修改名称"))
		{
			if (!boss1->unredoForRenameDbEngi(order_old[2], order_old[1]))
			{
				child_row += 1;
				if (child_row > 3)
				{
					child_row = 0;
					return;
				}
				else
				{
					if (boss1->selectionForDbBackupFault())
						redo();
					else
						undo();
				}
			}
			foreach (QStandardItem * p_item, undo_items)
			{
				if (p_item->text() != order_old[1])
					p_item -> setText(order_old[1]);
			}
		}		
		if (parent_item->text() != order_old[1])
			parent_item -> setText(order_old[1]);
	}	
}
	
void TableMgmtGroupEditCmd::undoOnlyForTwoTypeBossOne()
{
	if (orderText==QObject::tr("单独备份") || orderText.contains(QObject::tr("备份到......")))
	{
		if (orderText==QObject::tr("单独备份"))
		{
			DataSelectModel * tree_model = qobject_cast<DataSelectModel *>(parent_item->model());	  
			tree_model->item(2)->takeRow(child_row);
			undo_items << mid_item;			
		}
		else
		{
			mid_item -> takeRow(child_row);
			undo_items << child_item;	
		}
		boss1->backupManualFileItems().remove(child_item);		
	}
	else if (orderText==QObject::tr("删除此工程") || orderText==QObject::tr("删除此自制表"))
		parent_item -> takeRow(child_row);
	else if (orderText==QObject::tr("取消备份"))
	{
		if (parent_item == parent_item->model()->item(2))
			parent_item -> insertRow(child_row, mid_item);
		else
			mid_item -> insertRow(child_row, child_item);
	}
	else if (orderText.contains(QObject::tr("修改名称")) || orderText.contains(QObject::tr("修改标题")))
	{
		QStringList order_old = orderText.split(QObject::tr("，，。"));
		if (order_old[0]==QObject::tr("修改名称"))
		{
			if (!boss1->unredoForRenameDbEngi(order_old[1], order_old[2]))//no finished for db corrupt
			{
				child_row += 1;
				if (child_row > 3)
				{
					child_row = 0;
					return;
				}
				else
				{
					if (boss1->selectionForDbBackupFault())
						undo();
					else
						redo();
				}
			}
			foreach (QStandardItem * p_item, undo_items)
			{
				if (p_item->text() != order_old[2])
					p_item -> setText(order_old[2]);
			}		  
		}		
		if (parent_item->text() != order_old[2])
			parent_item -> setText(order_old[2]);		
	}		
}
