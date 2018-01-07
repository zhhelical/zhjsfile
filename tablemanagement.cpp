#include <QtGui>
#include <QtCore>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlError>
#include "tablemanagement.h"
#include "dataselectmodel.h"
#include "tablesgroup.h"
#include "tablemgmtploteditcommand.h"
#include "tablemgmtgroupeditcommand.h"
#include "editlongtoolsviewer.h"
#include "littlewidgetsview.h"
#include "littlewidgetitem.h"
#include "plotwidget.h"
#include "dialog.h"
#include "spcnum.h"
#include "spcdatabase.h"
#include "mainwindow.h"

TableManagement::TableManagement(QWidget * parent)
:QTreeView(parent), editting(true), press_time(0), nested_animation(0), strs_viewer(0), tools_bar(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setAnimated(true);
	setDragDropMode(QAbstractItemView::NoDragDrop);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setStyleSheet(QString("font-family: %1; font-size:18px; border: 0px").arg(tr("宋体")));
	setHeaderHidden(true);
	connect(this, SIGNAL(itemEditting(const QModelIndex &)), this, SLOT(edit(const QModelIndex &)));
	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(cellClicked(const QModelIndex &)));
	connect(this, SIGNAL(expanded(const QModelIndex &)), this, SLOT(recalculateSize(const QModelIndex &)));
	connect(this, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(recalculateSize(const QModelIndex &)));
}

TableManagement::~TableManagement()
{
	if (nested_animation)
		delete nested_animation;
	if (press_time)
		delete press_time;
	if (!delete_items.isEmpty())
	{
		QList<QStandardItem *> no_parents;
		foreach (QStandardItem * item, delete_items)
		{
			if (!item->parent())
				no_parents << item;
		}
		foreach (QStandardItem * item, no_parents)
			delete item;
	}
}

void TableManagement::initManageTree(const QPair<QString, QString> & ctrl_name, SpcNum * spc_guider, SpcDataBase * dataes_source, int type, LittleWidgetsView * related_viewer, EditLongToolsViewer * guide_bar)
{
	t_type = type;
	back_calculator = spc_guider;
	source = dataes_source;
	partner = related_viewer;
	tools_bar = guide_bar;
	if (t_type == 2)
	{
		editting = false;
		m_commandStack = new QUndoStack(this);
		connect(m_commandStack, SIGNAL(canRedoChanged(bool)), this, SIGNAL(currentRedoToNone(bool)));
		connect(m_commandStack, SIGNAL(canUndoChanged(bool)), this, SIGNAL(currentUndoToNone(bool)));
	}
	else if (t_type>2 && t_type<6)
	{
		m_commandStack = 0;
		connect(this, SIGNAL(plotFeildToBeChanged(int, QStandardItem *)), partner, SIGNAL(changingPlot(int, QStandardItem *)));
		connect(partner, SIGNAL(dealSigFromAgentHidePlot(PlotWidget *)), this, SLOT(sendsigOrderFromPartner(PlotWidget *)));
	}
	DataSelectModel * roots_model = new DataSelectModel(this);
	roots_model -> initModel(ctrl_name, dataes_source);
	setModel(roots_model);
	if (t_type != 2)
	{
		QPair<QStandardItem *, LittleWidgetsView *> init_pair(roots_model->item(0), partner);
		bk_viewers.insert(0, init_pair);
		partner -> setTblManager(this);
		connect(partner, SIGNAL(sendNewFrozenState(bool)), this, SLOT(syncPartnersFrozenState(bool)));	  
		QString z_text = roots_model->item(0)->text();
		z_text += "\n"+tr("（编辑新文件中...）");
		roots_model->item(0)->setText(z_text);
	}
}

void TableManagement::setInitViewRect(const QRect & rect)
{
	if (origin_rect != rect)
		origin_rect = rect;	
}

void TableManagement::sendSignalToRelated(int type, QStandardItem * info)
{
	emit plotFeildToBeChanged(type, info);
}

void TableManagement::removeItemsInHash(const QList<QStandardItem *> & items)
{
	QList<QStandardItem *> removings;
	foreach (QStandardItem * item, items)
	{
		if (!removings.contains(item))
			removings << item;
	}	
	foreach (QStandardItem * item, removings)
	{
		if (names_list.contains(item))
			names_list.remove(item);
		if (texts_hash.contains(item))
			texts_hash.remove(item);
	}
	rearrangeItemsForDelete(removings);
	foreach (QStandardItem * item, removings)
	{
		if (!item->parent())
			delete item;
	}
}

void TableManagement::rearrangeItemsForDelete(QList<QStandardItem *> & items)
{
	foreach (QStandardItem * item, items)
	{
		int i = 0;
		while (item->child(i))
		{
			if (items.contains(item->child(i)))
				items.removeOne(item->child(i));
			i++;
		}
	}	
}

void TableManagement::unredoItemsForMatrixChange(const QList<PlotWidget *> & hide_plots, const QHash<QStandardItem *, QPair<QStandardItem *, int> > & on_old, bool re_un)
{
	QList<PlotWidget *> plots_list = hide_plots;
	QHashIterator<QStandardItem *, QPair<QStandardItem *, int> > items_action(on_old);
	while (items_action.hasNext())
	{
		items_action.next();
		setExpanded(items_action.value().first->index(), false);		
		if (re_un)
			items_action.value().first -> takeRow(items_action.value().second);
		else
			items_action.value().first -> insertRow(items_action.value().second, items_action.key());
		setExpanded(items_action.value().first->index(), true);		
	}
}

void TableManagement::itemsInfoesTransToDB(QStandardItem * parent_item, QString & trans_plots, QString & trans_hash, QString & trans_names)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QList<QStandardItem *> children;
	t_model -> findAllChildItems(parent_item, children);	
	children.push_front(parent_item);
	foreach (QStandardItem * item, children)
	{
		if (texts_hash.contains(item))
		{
			QStringList hash_values = texts_hash.value(item);
			trans_hash += QString::number((long)item, 16)+";"+hash_values.join("%")+tr("，");
		}
		if (names_list.contains(item))
		{
			QStringList hash_values = names_list.value(item);
			trans_names += QString::number((long)item, 16)+";"+hash_values.join("%")+tr("，");
		}
	}
	QString children_info;
	foreach (QStandardItem * item, children)
	{
		QString item_infoes;
		item_infoes = QString::number((long)item, 16)+tr("，，～")+item->text()+tr("，，～，");
		if (item->hasChildren())
		{
			int i = 0;
			while (item->child(i))
			{
				item_infoes += QString::number((long)item->child(i), 16)+tr("，！，")+item->child(i)->text()+tr("，，～");
				i++;
			}
		}
		children_info += item_infoes+tr("，，～；");
	}
	QStandardItem * p_parent = parent_item->parent();
	trans_plots = QString::number((long)p_parent, 16)+tr("，，～")+p_parent->text()+tr("，，～：")+children_info;
}

void TableManagement::feedbackForManualsName(QString & t_name)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	if (!selectedIndexes().isEmpty())
	{
		QStandardItem * selected_item = t_model->itemFromIndex(selectedIndexes().at(0));
		QList<QStandardItem *> f_line;
		t_model -> findItemsSingleLine(selected_item, f_line, t_model->item(1)->text());
		if (!f_line.isEmpty())
			manualDBnameByItem(f_line.at(1), t_name);
	}	
}

void TableManagement::actionForOtherDelManualWork(const QString & del_name)
{
	QString find_str = del_name+tr("，，数据库。");
	QStandardItem * match_item = lookingForMatchItem(0, find_str);
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QHash<QPair<QStandardItem *, QString>, QSqlTableModel *> bk_sqls = t_model->listForManualTableNames();
	QPair<QStandardItem *, QString> f_pair;
	QList<QPair<QStandardItem *, QString> > h_list = bk_sqls.keys();
	for (int i = 0; i < h_list.size(); i++)
	{
		if (h_list.at(i).first == match_item)
		{
			f_pair = h_list.at(i);
			break;
		}
	}
	QSqlTableModel * sql_model = bk_sqls.value(f_pair); 	
	TablesGroup * manual_table = qobject_cast<TablesGroup *>(partner->qmlAgent()->widget());	
	if (!manual_table)
	{
		QString all_contents = sql_model->data(sql_model->index(0, 1)).toString();
		partner -> deletePlotsPicsInDbTbl(del_name, all_contents, source);
	}
	else
	{ 
		QString all_contents = sql_model->data(sql_model->index(0, 0)).toString();
		manual_table -> deletePicturesInDbTbl(all_contents);	
	}
	QList<QPair<QStandardItem *, LittleWidgetsView *> > m_views = bk_viewers.values();	  
	for (int i = 0; i < m_views.size(); i++)
	{
		if (m_views.at(i).first == match_item)
		{
			if (m_views.at(i).second == partner)
			{
				LittleWidgetsView * to_win = partner;
				partner = m_views.at(i-1).second;
				TablesGroup * manual_table = qobject_cast<TablesGroup *>(partner->qmlAgent()->widget());	
				if (!manual_table)
				{
					treeProjsFresh(m_views.at(i-1).first, match_item);
					tools_bar -> setPlotsContainerPt(partner);
				}
				else
					tools_bar -> setEditingTable(manual_table);					
				QStringList z_list = t_model->item(0)->text().split("\n");
				QString new_text;
				if (m_views.at(i-1).first == t_model->item(0))
					new_text = z_list[0]+"\n"+tr("（编辑新文件中...）");
				else
					new_text = z_list[0]+"\n"+tr("（编辑")+m_views.at(i-1).first->text()+tr("中...）");
				t_model->item(0)->setText(new_text);				
				emit showSavedManualPlots(this, to_win, false);				
			}
			else
				m_views.at(i).second->deleteLater();
			delete tools_bar->undoStackHash().value(m_views.at(i).second);
			tools_bar->undoStackHash().remove(m_views.at(i).second);			
			bk_viewers.remove(i);	
			break;
		}		
	}
	t_model->item(1)->removeRow(match_item->row());	
	t_model -> removeSqltableInhash(del_name);
}

void TableManagement::removeItemChildrenInHash(QStandardItem * parent_item, QHash<QStandardItem *, QStringList> & bk_hashes)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QList<QStandardItem *> children_items;
	t_model -> findAllChildItems(parent_item->child(0), children_items);	
	QHashIterator<QStandardItem *, QStringList> t_hash(texts_hash);
	while (t_hash.hasNext())
	{
		t_hash.next();
		if (children_items.contains(t_hash.key()))
		{
			bk_hashes.insert(t_hash.key(), t_hash.value());
			texts_hash.remove(t_hash.key());
		}		
	}
	QHashIterator<QStandardItem *, QStringList> n_hash(names_list);
	while (n_hash.hasNext())
	{
		n_hash.next();
		if (children_items.contains(n_hash.key()))
		{
			bk_hashes.insert(n_hash.key(), n_hash.value());
			names_list.remove(n_hash.key());
		}		
	}
}

void TableManagement::reinsertItemChildrenInHash(const QHash<QStandardItem *, QStringList> & bk_hashes)
{
	QHashIterator<QStandardItem *, QStringList> i_bk(bk_hashes);
	while (i_bk.hasNext())
	{
		i_bk.next();
		QStandardItem * key_item = i_bk.key();
		if (key_item->parent()->text().contains(tr("人员")))
			names_list.insert(key_item, i_bk.value());
		else
			texts_hash.insert(key_item, i_bk.value());	
	}	
}

void TableManagement::setTypeTwoEditting(bool edit_define)
{
	editting = edit_define;
}

void TableManagement::unredoGeneratePlotForTmtbl(QStandardItem * plot, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & ur_hash, bool act)
{
	QPair<QModelIndex, QString> index_info = ur_hash.begin().key();
	QPair<QMap<int, QVariant>, QStandardItem *> pix_info = ur_hash.begin().value();
	QModelIndex cell_index = index_info.first;	
	TablesGroup * p_grp = qobject_cast<TablesGroup *>(partner->qmlAgent()->widget());	
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());	
	if (act)
	{
		QStandardItem * pix_detail = db_items.value(plot).at(0);		
		QList<QStandardItem *> is_line;
		t_model -> findItemsSingleLine(plot, is_line, t_model->item(0)->text());
		QString find_tbl;
		QHashIterator<QPair<QStandardItem *, QString>, QSqlTableModel *> f_hash(t_model->listForManualTableNames());
		while (f_hash.hasNext())
		{
			f_hash.next();
			if (is_line.contains(f_hash.key().first))
			{
				find_tbl = f_hash.key().second;
				break;
			}	
		}
		QMap<int, QVariant> cell_pix;
		QPixmap l_pix = source->foundPictureFromDb(find_tbl, pix_detail->text()).scaled(p_grp->visualRect(cell_index).size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);		
		cell_pix[1] = l_pix;		  
		p_grp->model()->setItemData(cell_index, cell_pix);
	}
	else
		p_grp->model()->setItemData(cell_index, pix_info.first);		
}

void TableManagement::unredoFolderPictureForTmtbl(QStandardItem * plot, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & ur_hash, bool act)
{
	QPair<QModelIndex, QString> index_info = ur_hash.begin().key();
	QPair<QMap<int, QVariant>, QStandardItem *> pix_info = ur_hash.begin().value();
	QModelIndex cell_index = index_info.first;	
	TablesGroup * p_grp = qobject_cast<TablesGroup *>(partner->qmlAgent()->widget());	
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	if (act)
	{
		QMap<int, QVariant> cell_pix;	
		cell_pix[1] = p_grp->loadPictureFromImageFolder(t_model, plot).scaled(p_grp->visualRect(cell_index).size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);		
		p_grp->model()->setItemData(cell_index, cell_pix);	  
	}
	else
		p_grp->model()->setItemData(cell_index, pix_info.first);
}

void TableManagement::mBackItemsTransFromDB(QStandardItem * c_item, const QString & items, const QHash<QString, QStringList> & db_hash, const QHash<QString, QStringList> & db_names)
{
	QList<QStandardItem *> head_children;
	db_items.insert(c_item, head_children);	
	QStringList items_expand = items.split(tr("，，&；"));
	foreach (QString item_str, items_expand)
	{
		if (item_str.isEmpty())
			break;
		QStringList ev_list = item_str.split(tr("，，～："));
		QString next_back = ev_list.back();
		QStringList old_nums = next_back.split(tr("，，～；"));
		if (item_str.contains(tr("测量基础数据")))
			old_nums.pop_front();	
		QStringList dest_nums = next_back.split(tr("，，～"));
		QStringList dest_list = ev_list[1].split(tr("，，～"));
		QHash<QString, QStandardItem *> old_base;		
		QStandardItem * type_item = new QStandardItem(ev_list[0]+tr("，，～")+dest_list[1]);
		db_items[c_item] << type_item;
		old_base.insert(type_item->text(), type_item);			
		foreach (QString t_str, old_nums)
		{
			if (!t_str.isEmpty())
			{
				QStringList ts_details = t_str.split(tr("，，～，"));
				QStringList tail_details = ts_details.at(0).split(tr("，，～"));
				if (!old_base.contains(tail_details.at(0)))
				{
					QStandardItem * tail_item = new QStandardItem(tail_details.at(1));
					if (db_hash.contains(tail_details.at(0)))
						texts_hash.insert(tail_item, db_hash.value(tail_details.at(0)));
					old_base.insert(tail_details.at(0), tail_item);	
					old_base.value(ev_list[0]+tr("，，～")+dest_list[1]) -> appendRow(tail_item);						
				}
				if (t_str.contains(tr("，！，")))	
				{
					QStringList chd_list = ts_details.at(1).split(tr("，，～"));
					chd_list.pop_back();
					foreach (QString str, chd_list)
					{
						QStringList chnm_list = str.split(tr("，！，"));
						QStandardItem * c_children = new QStandardItem(chnm_list.at(1));
						if (db_hash.contains(chnm_list.at(0)))
							texts_hash.insert(c_children, db_hash.value(chnm_list.at(0)));
						if (db_names.contains(chnm_list.at(0)))
							names_list.insert(c_children, db_names.value(chnm_list.at(0)));						
						old_base.insert(chnm_list.at(0), c_children);	
						old_base.value(tail_details.at(0)) -> appendRow(c_children);
					}
				}
			}
		}
	}
}

bool TableManagement::findMatchBranch(QStandardItem * dest_item, const QString & find_text)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QList<QStandardItem *> guid_list;
	t_model -> findAllChildItems(dest_item, guid_list);	
	foreach (QStandardItem * item, guid_list)
	{
		if (item->text().contains(find_text))
			return true;
	}
	return false;
}

bool TableManagement::considerSaveNewOrOldManuals(QString & save_res)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QStandardItem * cur_save = showingItemFind();
	if (cur_save == t_model->item(0))
		return false;
	QString passwd = t_model->savedUserName().second;
	QHash<QPair<QStandardItem *, QString>, QSqlTableModel *> cur_manuals = t_model->listForManualTableNames();
	QString rs_name;	
	QHashIterator<QPair<QStandardItem *, QString>, QSqlTableModel *> m_hash(cur_manuals);
	while (m_hash.hasNext())
	{
		m_hash.next();
		if (m_hash.key().first == cur_save)
		{
			rs_name = m_hash.key().second;
			break;
		}		
	}	
	if (!source->databaseResaveRelativeManuals(passwd, rs_name, this, partner))
	{
		save_res = tr("保存未成功");
		return false;
	}
	return true;
}

bool TableManagement::unredoForRenameDbEngi(const QString & oldname, const QString & newname)
{
	if (!source->renameProject(oldname, newname))
		return false;
	return true;
}

bool TableManagement::backupRelatedDbForItems(/*need add db error operation info string container*/)
{
	DataSelectModel * bk_model = qobject_cast<DataSelectModel *>(model());	
	QList<QStandardItem *> bk_list1;
	bk_model -> findChildrenNotAll(bk_model->item(2), bk_list1);
	QList<QStandardItem *> bk_list2;
	bk_model -> findChildrenNotAll(bk_model->item(3), bk_list2);
	bool found_items = false;
	foreach (QStandardItem * bk_item, bk_list2)
	{
		QList<QStandardItem *> bk_list3;
		bk_model -> findChildrenNotAll(bk_item, bk_list3);
		if (bk_list3.size() > 1)
			found_items = true;
	}
	if (bk_list1.isEmpty() && !found_items)
		return false;
 	MainWindow * mw = qobject_cast<MainWindow *>(source->parent()->parent());
	QProgressDialog save_following(mw, Qt::FramelessWindowHint);	  
	save_following.setWindowModality(Qt::WindowModal);
	save_following.setRange(0, bk_list1.size()+bk_list2.size());
	save_following.setLabel(0);
	save_following.setCancelButton(0);
	save_following.setGeometry(mw->width()/2-100, mw->height()/2-15, 200, 15);	
	save_following.setFixedSize(QSize(200, 15));
	save_following.setStyleSheet("color: white; background: rgb(50, 50, 50)"); 	
	save_following.show(); 	
	QDateTime c_time = QDateTime::currentDateTime();
	foreach (QStandardItem * bk_item, bk_list1)
	{	
		save_following.setValue(bk_list1.indexOf(bk_item)+1);
		if (!newDbGenerationForItem(bk_item, c_time))
			return false;
		c_time.addMSecs(1000);				
	}
	int end_process = save_following.value();
	foreach (QStandardItem * bk_item, bk_list2)
	{
		save_following.setValue(bk_list2.indexOf(bk_item)+end_process);  
		if (bk_item->child(1))
		{
			QString old_db = source->currentConnectionDb()->connectionName();
			QString db_name = "/home/dapang/workstation/spc-tablet/qsqlitebkdb/"+bk_item->text();			
			*source->currentConnectionDb() = QSqlDatabase::addDatabase("QSQLITE", bk_item->text());
			source->currentConnectionDb()->setDatabaseName(db_name);		  
			if (!source->currentConnectionDb()->open())
			{
					//warning dailog
				*source->currentConnectionDb() = QSqlDatabase::database(old_db);
				return false;					
			}		  
			QList<QStandardItem *> bk_list3;
			bk_model -> findChildrenNotAll(bk_item, bk_list3);
			*source->currentConnectionDb() = QSqlDatabase::database(old_db);
			foreach (QStandardItem * bk_proj, bk_list3)
			{
				if (bk_list3.indexOf(bk_proj) == 0)
					continue;
				QString test_result;
				if (!source->replaceProjBetweenDbsAction(bk_proj->text(), bk_item->text(), test_result))
				{
					QSqlDatabase::database(bk_item->text()).close();
					QSqlDatabase::removeDatabase(bk_item->text());
					return false;
				}
			}
			QSqlDatabase::database(bk_item->text()).close();
			QSqlDatabase::removeDatabase(bk_item->text());
		}				
	}
	save_following.setValue(bk_list1.size()+bk_list2.size());
	return true;	
}

bool TableManagement::selectionForDbBackupFault()
{
	QString error_text = "error:"+tr("数据库存储出现问题\n您可以按确定继续或\n按取消结束此次操作");
	DiaLog * bk_error = new DiaLog;
	MainWindow * mw = qobject_cast<MainWindow *>(source->parent()->parent());
	bk_error -> initWinDialog(mw, error_text, t_type);
	return bk_error->exec();
}

int TableManagement::relationshipBetweenItems(QStandardItem * one, QStandardItem * other)
{
	QList<QStandardItem *> match_list;
//	qobject_cast<DataSelectModel *>(model()) -> findIndexParentsChidren(one->index(), match_list);
	int one_pos = match_list.indexOf(one);
	int other_pos = match_list.indexOf(other);
	if (other_pos == -1)
		return 0;
	if (other_pos < one_pos)
		return 1;
	if (other_pos > one_pos)
		return 2;
	return -1;
}

int TableManagement::treeType()
{
	return t_type;
}

int TableManagement::biggerWidthBetweenTextList(const QFontMetrics & fm, const QStringList & list, const QString & str)// need str para?
{	
	int max_listwidth = 0;
	foreach (QString w_str, list)
	{	
		if (max_listwidth < fm.width(w_str))
			max_listwidth = fm.width(w_str);
	}		
	int str_width = 0;
	if (!str.isEmpty())
		str_width = fm.width(str);	
	if (max_listwidth < str_width)	
		max_listwidth = str_width;
	return max_listwidth;
}

QStandardItem * TableManagement::lookingForMatchItem(QStandardItem * cause_item, const QString & clue_str)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QList<QStandardItem *> guid_list;
	if (clue_str == tr("次序"))
	{
		t_model -> findAllChildItems(cause_item, guid_list);
		if (cause_item->parent()->text()==tr("均值图") || cause_item->parent()->text()==tr("西格玛图") || cause_item->parent()->text()==tr("极差图"))
		{
			bool sure_again = false;
			bool sure_next = false;
			int item_pos = 0;
			foreach (QStandardItem * item, guid_list) 
			{
				if (item->text()==tr("继续缩小选择范围") || item->text()==tr("再次缩小选择范围"))
				{
					QList<QStandardItem *> child_items;
					t_model -> findAllChildItems(item, child_items);
					if (item->text()==tr("继续缩小选择范围") && child_items.size()>1 && !sure_next)
					{
						item_pos = guid_list.indexOf(item);
						sure_next = true;
					}
					if (item->text()==tr("再次缩小选择范围") && child_items.size()>1 && !sure_again)
					{
						item_pos = guid_list.indexOf(item);
						sure_again = true;
					}				
				}			
			}
			if (item_pos)
				return guid_list[item_pos];
		}		
	}
	else if (clue_str == tr("链上人员"))
	{
		t_model -> findAllChildItems(cause_item, guid_list);
		if (cause_item->parent()->text()==tr("均值图") || cause_item->parent()->text()==tr("西格玛图") || cause_item->parent()->text()==tr("极差图"))
		{
			int item_pos = 0;
			foreach (QStandardItem * item, guid_list) 
			{
				if (item->text().contains(tr("人员")))
				{
					item_pos = guid_list.indexOf(item);
					break;				
				}			
			}
			if (item_pos)
				return guid_list[item_pos];
		}		
	}
	else if (clue_str.contains(tr("，，。")) && !clue_str.contains(tr("，，数据库。")))
	{
		QStringList exact_match = clue_str.split(tr("，，。"));
		QList<QStandardItem *> children;
		t_model -> findChildrenNotAll(cause_item, children);
		foreach (QStandardItem * item, children)
		{
			if (item->text() == exact_match[0])
				return item;
		}
	}
	else if (clue_str.contains(tr("，，数据库。")))
	{
		QStringList exact_match = clue_str.split(tr("，，数据库。"));	
		QHashIterator<QPair<QStandardItem *, QString>, QSqlTableModel *> m_hash(t_model->listForManualTableNames());
		while (m_hash.hasNext())
		{
			m_hash.next();
			if (m_hash.key().second == exact_match[0])
				return m_hash.key().first;		
		}
	}
	else if (clue_str == tr("按时间"))
	{
		QStringList f_split = cause_item->text().split(tr("，，～"));
		QStringList f_keys = f_split.at(0).split(tr("，"));
		QList<QStandardItem *> children;
		t_model -> findAllChildItems(t_model->item(0), children);
		foreach (QStandardItem * f_item, children)
		{
			if (f_item->text() == f_keys.at(0))
			{				
				QList<QStandardItem *> again_children;
				t_model -> findAllChildItems(f_item->parent()->parent(), again_children);
				foreach (QStandardItem * again_item, again_children)
				{
					if (again_item->parent()->text()==f_keys.at(1) && again_item->text() == f_split.at(1))
						return again_item;
				}
			}		
		}
	}	
	else
	{
		if (cause_item->text().contains(clue_str))
			return cause_item;
	}
	return 0;
}

DataSelectModel * TableManagement::currentModel()
{
	return qobject_cast<DataSelectModel *>(model());
}

QUndoStack * TableManagement::commandStack()
{
	return m_commandStack;
}

EditLongToolsViewer * TableManagement::delegateViewer()
{
	return strs_viewer;
}

LittleWidgetsView * TableManagement::currentPartner()
{
	return partner;
}

const QRect & TableManagement::originShowRect()
{
	return origin_rect;
}

QList<QStandardItem *> & TableManagement::keepDeletingItems()
{
	return delete_items;
}

QMap<int, QPair<QStandardItem *, LittleWidgetsView *> > & TableManagement::backupViewerHash()
{
	return bk_viewers;
}

QHash<QStandardItem *, QList<QStandardItem *> > & TableManagement::backupManualFileItems()
{
	return db_items;
}

const QHash<QStandardItem *, QStringList> & TableManagement::hashSavedRelatedItemInfo()
{
	return texts_hash;
}

void TableManagement::clearShowDataes()
{
	for (int i = 0; i < model()->rowCount(); i++)
	{
		if (model()->data(model()->index(i, 0)).toString().isEmpty())
			continue;
		model() -> setData(model()->index(i, 0), "");
	}
}

void TableManagement::relatedStyleTreeActions(QString style, bool action)
{
	if (style == tr("选取浏览") || style == tr("选取编辑") || style == tr("选取数据") || style == tr("选取综合") || style == tr("选取信息") || style == tr("编制新图"))
		editting = action;
	else if (style == tr("暂停选取") || style == tr("取消编制"))
		editting = !action;
}

void TableManagement::listenFreezingState(bool state)
{
	emit freezingNestCell(state);	
}

void TableManagement::resetDatelist(int year, int month)
{
	QStandardItem * hash_item = strs_viewer->currentNesttingItem();
	QDateTime base_date(QDate(year_list[year+2].toInt(), month_list[month+2].toInt(), 1));
	QStringList all_times = texts_hash.value(hash_item);
	all_times.pop_front();
	QStringList new_month, new_days;
	redefineDatesListForNewSelection(base_date.toString(Qt::ISODate), all_times, true, new_month);
	redefineDatesListForNewSelection(base_date.toString(Qt::ISODate), all_times, false, new_days);
	QStringList m_list, d_list;
	m_list << "" << "";
	d_list << "" << "";
	foreach (QString mon, new_month)
	{
		QStringList str = mon.split("-");
		if (m_list.back() != str[1])
			m_list.push_back(str[1]);
	}		
	foreach (QString day, new_days)
	{
		QStringList str = day.split("-");
		if (d_list.back() != str[2])
			d_list.push_back(str[2]);
	}
	m_list << "" << "";
	d_list << "" << "";
	QDeclarativeContext * ctxt = strs_viewer->rootContext();
	if (m_list != month_list)
	{
		month_list = m_list;	
		ctxt -> setContextProperty("MonthsList", QVariant::fromValue(month_list));
	}
	if (d_list != days_list)
	{
		days_list = d_list;	
		ctxt -> setContextProperty("DaysList", QVariant::fromValue(days_list));
	}
}

void TableManagement::endSelectionForDate(int year, int month, int day)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QStandardItem * hash_item = strs_viewer->currentNesttingItem();
	QString year_str, month_str, day_str;
	year_str = year_list[year+2];
	month_str = month_list[month+2];
	day_str = days_list[day+2];
	QDate hash_date(year_str.toInt(), month_str.toInt(), day_str.toInt());
	QStringList date_list;
	QStringList cur_times = texts_hash.value(hash_item);
	QString time_str;
	if (!hash_item -> hasChildren())
	{
		foreach (QString time_loop, cur_times)
		{
			QDateTime first_time = QDateTime::fromString(time_loop, Qt::ISODate);
			if (hash_date == first_time.date())
			{
				date_list << time_loop;
				break;
			}
		}
		time_str = tr("时间从：")+year_str+tr("年")+month_str+tr("月")+day_str+tr("日");
	}
	if (t_model->rowCount(hash_item->index())==1)
	{
		foreach (QString time_loop, cur_times)
		{
			QDateTime last_time = QDateTime::fromString(time_loop, Qt::ISODate);
			if (hash_date < last_time.date())
			{
				date_list << cur_times.at(cur_times.indexOf(time_loop)-1);
				break;
			}
		}
		if (date_list.isEmpty())	
			date_list << cur_times.at(cur_times.size()-1);
		time_str = tr("时间到：")+year_str+tr("年")+month_str+tr("月")+day_str+tr("日");
	}
	if (partner->nestedContainer())
		emit newPlotEditCommand(new TableMgmtPlotEditCmd(this, hash_item, new QStandardItem(time_str), time_str, partner));
	else
		qobject_cast<TablesGroup *>(partner->qmlAgent()->widget()) -> sendSigToToolbar(this, hash_item, new QStandardItem(time_str), time_str);
	if (hash_item->rowCount() == 1)
		texts_hash.insert(hash_item->child(0), date_list);
	else
	{
		texts_hash.insert(hash_item->child(1), date_list);
		if (!hash_item->text().contains(tr("最终")))
		{
			QList<QStandardItem *> fac_lines;
			DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
			t_model -> findItemsSingleLine(hash_item, fac_lines, tr("日常数据"));
			if (fac_lines.at(1)->text()==tr("直方图") || fac_lines.at(1)->text()==tr("正态概率纸"))		
				return;			  
			QString def_text;
			defineNextSelectString(hash_item, def_text);
			hash_item->child(1)->appendRow(new QStandardItem(def_text));
		}
	}
}

void TableManagement::viewerRealWidthReset(int real_val)
{
	strs_viewer -> setFixedWidth(real_val);
	QRect new_rect = visualRect(strs_viewer->currentNesttingItem()->index());
	if (new_rect.topLeft().x()+strs_viewer->width() > width())	
	{
		setFixedWidth(new_rect.topLeft().x()+strs_viewer->width());
		emit sizeChanged();
	}	
}

void TableManagement::endSelectionForNumber(QString data)
{
	QStandardItem * item = strs_viewer->currentNesttingItem();
	QString num_str;
	if (!item -> hasChildren())
		num_str = tr("组从：第")+data+tr("组");
	else
		num_str = tr("组到：第")+data+tr("组");
	if (partner->nestedContainer())
		emit newPlotEditCommand(new TableMgmtPlotEditCmd(this, item, new QStandardItem(num_str), num_str, partner));
	else
		qobject_cast<TablesGroup *>(partner->qmlAgent()->widget()) -> sendSigToToolbar(this, item, new QStandardItem(num_str), num_str);
	QStringList append_list;
	append_list << data;
	if (item->rowCount() == 1)
		texts_hash.insert(item->child(0), append_list);
	else
		texts_hash.insert(item->child(1), append_list);
	closeNestedWidgetInBranch(strs_viewer);	
}

void TableManagement::closeNestedWidget()
{
	closeNestedWidgetInBranch(strs_viewer);
}

void TableManagement::clearLongToolPt(QWidget * pt)
{
	Q_UNUSED(pt);
	strs_viewer = 0;
}

void TableManagement::activedItemChanged(QStandardItem * a_item)
{
	if ((edit_order==tr("修改标题") || edit_order==tr("修改名称")) && a_item->text()!=cell_oldText)
	{
		DataSelectModel * tree_model = qobject_cast<DataSelectModel *>(model());
		QList<QStandardItem *> dbs_list;
		tree_model -> findChildrenNotAll(tree_model->item(2), dbs_list);		
		if (edit_order==tr("修改标题"))
		{
			if (!a_item->text().contains(".db"))
				a_item -> setText(a_item->text()+".db");
			foreach (QStandardItem * db_item, dbs_list)
			{
				if (a_item == db_item)
					continue;
				if (a_item->text() == db_item->text())
				{
				//warning dialog for same name;
					a_item -> setText(cell_oldText);
					edit_order.clear();
					cell_oldText.clear();						
					return;
				}
			}
			edit_order += tr("，，。")+a_item->text()+tr("，，。")+cell_oldText;
			m_commandStack->push(new TableMgmtGroupEditCmd(this, a_item, edit_order, 0));			
		}
		else
		{
			QList<QStandardItem *> projs_list;
			tree_model -> findChildrenNotAll(tree_model->item(0), projs_list);	
			foreach (QStandardItem * proj_item, projs_list)
			{
				if (proj_item == a_item)
					continue;
				if (proj_item->text() == a_item->text())
				{
					//warning dialog for same proj name in this db
					a_item -> setText(cell_oldText);
					edit_order.clear();
					cell_oldText.clear();					
					return;
				}
			}
			QList<QStandardItem *> added_items;
			foreach (QStandardItem * db_item, dbs_list)
			{
				if (db_item->hasChildren())
				{
					QList<QStandardItem *> db_children;
					tree_model -> findChildrenNotAll(db_item, db_children);				
					foreach (QStandardItem * db_child, db_children)
					{
						QString old_db;
						if (db_child->text() == cell_oldText)
						{
							old_db = source->currentConnectionDb()->connectionName();
							QString test_name = "/home/dapang/workstation/spc-tablet/qsqlitebkdb"+db_item->text();			
							*source->currentConnectionDb() = QSqlDatabase::addDatabase("QSQLITE", db_item->text());

							source->currentConnectionDb()->setDatabaseName(test_name);
							if (!source->currentConnectionDb()->open()) 
							{
								//warning dialog for db open;
								edit_order.clear();
								cell_oldText.clear();								  
								return;
							}
							*source->currentConnectionDb() = QSqlDatabase::database(old_db);
							if (source->existedProjectNameIndb(a_item->text(), db_item->text()))
							{
								//warning dialog for same engi;
								QSqlDatabase::database(db_item->text()).close();
								QSqlDatabase::removeDatabase(db_item->text());
								a_item -> setText(cell_oldText);
								edit_order.clear();
								cell_oldText.clear();									
								return;
							}
							source->currentConnectionDb()->database(db_item->text()).close();							
							added_items << db_child;
						}
						if (!old_db.isEmpty())
							QSqlDatabase::removeDatabase(db_item->text());
					}
				}
			}
			edit_order += tr("，，。")+a_item->text()+tr("，，。")+cell_oldText;
			m_commandStack -> push(new TableMgmtGroupEditCmd(this, a_item, edit_order, added_items));						
		}
	}
	edit_order.clear();
	cell_oldText.clear();		
}

void TableManagement::cellClicked(const QModelIndex & d_index)//need promotion to as tree type
{
	if (!editting)
		return;
	int time_passed = 0;
	if (press_time)
	{
		time_passed = press_time->elapsed();
		delete press_time;
		press_time = 0;			
	}
	if (time_passed > 500)
	{
		DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
		QStandardItem * clicked_item = t_model->itemFromIndex(d_index);	  
		QString guide_str = clicked_item->text();
		if (clicked_item == t_model->item(0) && t_type != 2)
		{
			if (bk_viewers.size() == 1)
				return;
			QStringList show_list;
			show_list << tr("不选择");
			QStandardItem * showing_item = showingItemFind();
			QList<QPair<QStandardItem *, LittleWidgetsView *> > bk_items = bk_viewers.values();
			for (int i = 0; i < bk_items.size(); i++)
			{
				if (bk_items.at(i).first == showing_item)
					continue;
				QString f_strs;
				if (bk_items.at(i).first == t_model->item(0))
					f_strs = tr("编辑新文件");
				else
					f_strs = tr("编辑")+bk_items.at(i).first->text();
				show_list << f_strs;
			}
			texts_hash.insert(clicked_item, show_list);
			initDelegateStrViewer(clicked_item, show_list);		
		}
		else if (clicked_item->parent() == t_model->item(1) && t_type != 2)
		{
			QStringList manual_list;
			if (compareShowingClickingItem(clicked_item))
				manual_list << tr("不选择") << tr("关闭");
			else if (!containsPartnerGuideItem(clicked_item))
				manual_list << tr("不选择") << tr("打开");
			else
				manual_list << tr("不选择") << tr("编辑") << tr("关闭");
			if (!texts_hash.contains(clicked_item) || texts_hash.value(clicked_item)!=manual_list)
				texts_hash[clicked_item] = manual_list;
			initDelegateStrViewer(clicked_item, manual_list);
		}
		else if (t_type == 0)
		{
			if (clicked_item==t_model->item(0) || clicked_item->parent()==t_model->item(0) || clicked_item->parent()->parent()==t_model->item(0) || clicked_item->text().contains(tr("完成版本")) || clicked_item->text()==tr("结果集") || clicked_item->text()==tr("日常数据"))
				return;
			longPressForDataesModel(clicked_item, t_model);
		}
		else if (t_type == 1)
			longPressForInfoesModel(clicked_item);
		else if (t_type == 2)//no finished
			longPressForBackupModel(clicked_item, t_model);			
		else if (t_type<6 && t_type>2)
			longPressForPlotsModel(clicked_item);
		else if (t_type == 6)
			longPressForDataInfoModel(clicked_item, t_model);
		else
			longPressForTotalModel(clicked_item, t_model);
	}
}

void TableManagement::clearNestedAnimation()
{
	if (strs_viewer)
		strs_viewer -> setFixedWidth(nested_animation->endValue().toRect().width());
	nested_animation -> deleteLater();
	nested_animation = 0;
	update();
}

void TableManagement::replyForSelectedStrData(int data)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QStandardItem * c_item = strs_viewer->currentNesttingItem();
	QString selected_text;
	if (c_item->text()==tr("记录人员") || c_item->text()==tr("继续选择人员") || c_item->text()==tr("最终选择人员"))
		selected_text = names_list[c_item].at(data);
	else if (strs_viewer->qmlDelegateType() == 11)
		selected_text = strs_viewer->rootContext()->contextProperty("StringesList").toList().at(data).toString();
	else
		selected_text = texts_hash.value(c_item).at(data);
	if (selected_text == tr("不选择") || selected_text == tr("清除选择") || selected_text == tr("不操作"))
	{
		closeNestedWidgetInBranch(strs_viewer);
		if (selected_text == tr("清除选择"))
		{
			if (t_type == 6)
				tools_bar -> pushGroupEditCommand(new TableMgmtGroupEditCmd(this, c_item, selected_text, 0));
			else
				emit newPlotEditCommand(new TableMgmtPlotEditCmd(this, c_item, selected_text, partner));	
		}
		return;
	}
	bool t_found = false;
	if (selected_text == tr("打开"))
	{
		t_found = true;
		QString sel_name;
		QSqlTableModel * sel_model = 0;
		QList<QPair<QStandardItem *, QString> > sel_names = t_model->listForManualTableNames().keys();
		for (int i = 0; i < sel_names.size(); i++)
		{
			if (sel_names.at(i).first == c_item)
			{
				sel_name = sel_names.at(i).second;
				sel_model = t_model->listForManualTableNames().value(sel_names.at(i));
				break;
			}
		}
		if (!db_items.contains(c_item))
			actionForManualDataesShow(c_item, sel_name, sel_model);
		if (t_type>2 && t_type<6)
			treeProjsFresh(c_item, showingItemFind());
		QStringList r_list = t_model->item(0)->text().split("\n");
		QString new_text = r_list[0]+"\n"+tr("（编辑")+c_item->text()+tr("中...）");
		t_model->item(0) -> setText(new_text);
		LittleWidgetsView * v_partner = containsPartnerGuideItem(c_item);
		if (v_partner)
		{				
			emit showSavedManualPlots(this, v_partner, false);
			partner = v_partner;
			TablesGroup * manual_table = qobject_cast<TablesGroup *>(partner->qmlAgent()->widget());
			if (!manual_table)
				tools_bar -> setPlotsContainerPt(partner);
			else
				tools_bar -> setEditingTable(manual_table);	
		}
		else
			createPartnerForClickedItem(c_item, sel_model, sel_name);
		if (!c_item->hasChildren())
		{
			QStandardItem * open_item = new QStandardItem(tr("已打开"));
			c_item -> appendRow(open_item);
		}
	}
	else if ((selected_text.contains(tr("编辑")) && selected_text!=tr("编辑")))
	{
		t_found = true;
		bool found = false;
		QStringList r_list = selected_text.split(tr("编辑"));
		QList<QPair<QStandardItem *, LittleWidgetsView *> > sel_names = bk_viewers.values();			
		for (int i = 0; i < sel_names.size(); i++)
		{
			if (sel_names.at(i).first->text() == r_list[1])
			{
				if (t_type>2 && t_type<6)
					treeProjsFresh(sel_names.at(i).first, showingItemFind());				
				emit showSavedManualPlots(this, sel_names.at(i).second, false);
				partner = sel_names.at(i).second;
				QStringList z_list = t_model->item(0)->text().split("\n");
				QString new_text = z_list[0]+"\n"+tr("（编辑")+sel_names.at(i).first->text()+tr("中...）");
				t_model->item(0) -> setText(new_text);
				found = true;
				break;				
			}
		}
		if (!found)
		{
			if (t_type>2 && t_type<6)
				treeProjsFresh(t_model->item(0), showingItemFind());
			emit showSavedManualPlots(this, containsPartnerGuideItem(t_model->item(0)), false);
			partner = containsPartnerGuideItem(t_model->item(0));
			QStringList z_list = t_model->item(0)->text().split("\n");
			QString new_text = z_list[0]+"\n"+tr("（编辑新文件中...）");
			t_model->item(0) -> setText(new_text);			
		}
		TablesGroup * manual_table = qobject_cast<TablesGroup *>(partner->qmlAgent()->widget());
		if (!manual_table)
			tools_bar -> setPlotsContainerPt(partner);
		else
			tools_bar -> setEditingTable(manual_table);		
	}
	else if (selected_text == tr("编辑"))
	{
		t_found = true;
		if (t_type>2 && t_type<6)
		{
			treeProjsFresh(c_item, showingItemFind());
			tools_bar -> setPlotsContainerPt(containsPartnerGuideItem(c_item));
		}
		else
		{
			TablesGroup * manual_table = qobject_cast<TablesGroup *>(containsPartnerGuideItem(c_item)->qmlAgent()->widget());
			tools_bar -> setEditingTable(manual_table);
		}
		emit showSavedManualPlots(this, containsPartnerGuideItem(c_item), false);
		partner = containsPartnerGuideItem(c_item);				
		QStringList z_list = t_model->item(0)->text().split("\n");
		QString new_text = z_list[0]+"\n"+tr("（编辑")+c_item->text()+tr("中...）");
		t_model->item(0) -> setText(new_text);
	}
	else if (selected_text == tr("关闭"))
	{
		t_found = true;
		QList<QPair<QStandardItem *, LittleWidgetsView *> > key_items = bk_viewers.values();
		QPair<QStandardItem *, LittleWidgetsView *> iv_pair(c_item, containsPartnerGuideItem(c_item));		
		if (partner == containsPartnerGuideItem(c_item))
		{
			QStandardItem * last_item = 0;
			if (key_items.indexOf(iv_pair) == 0)
				last_item = key_items.at(key_items.size()-1).first;
			else
				last_item = key_items.at(key_items.indexOf(iv_pair)-1).first;
			if (t_type>2 && t_type<6)
				treeProjsFresh(last_item, c_item);
			emit showSavedManualPlots(this, containsPartnerGuideItem(last_item), false);
			partner = containsPartnerGuideItem(last_item);
			QStringList z_list = t_model->item(0)->text().split("\n");
			QString last_text;
			if (last_item == t_model->item(0))
				last_text = tr("（编辑新文件中...）");
			else
				last_text = tr("（编辑")+last_item->text()+tr("中...）");
			t_model->item(0)->setText(z_list.at(0)+"\n"+last_text);
		}
		else
			containsPartnerGuideItem(c_item) -> deleteLater();
		int r_key = key_items.indexOf(iv_pair);
		if (r_key == bk_viewers.size()-1)
			bk_viewers.remove(r_key);
		else
		{
			for (int i = r_key+1; i < bk_viewers.size(); i++)
			{
				QPair<QStandardItem *, LittleWidgetsView *> re_pair = bk_viewers.take(i);
				bk_viewers.insert(i-1, re_pair);
			}
		}
		c_item -> removeRow(0);
		foreach (QStandardItem * d_item, db_items.value(c_item))
			delete d_item;
		db_items.remove(c_item);
		texts_hash.remove(c_item);
	}	
	if (!t_found)
	{
		if (t_type == 0)
			replyQmlStrDataForDmTree(c_item, data, selected_text);	
		else if (t_type == 2)
		{
			QString viewer_left = tr("不返回");
			replyQmlStrDataForBkTree(c_item, t_model, selected_text, viewer_left);
			if (viewer_left == tr("返回"))
				return;
		}
		else if (t_type==3 || t_type==4 || t_type==5)
			replyQmlStrDataForPmTree(c_item, data, selected_text);
		else if (t_type==6 || t_type==7)
			replyQmlStrDataForTmTree(c_item, data, selected_text);
	}
	closeNestedWidgetInBranch(strs_viewer);
}

void TableManagement::recalculateSize(const QModelIndex & e_index)
{
	DataSelectModel * tree_model = qobject_cast<DataSelectModel *>(model());
	QStandardItem * last_item = tree_model->item(tree_model->rowCount()-1);
	QRect pos_rect;	
	QList<QStandardItem *> children_items;
	tree_model -> findAllChildItems(last_item, children_items);
	bool found_child = false;
	while (!children_items.isEmpty())
	{
		QStandardItem * c_tail = children_items.takeLast();
		if (visualRect(c_tail->index()).isValid())
		{
			found_child = true;
			pos_rect = visualRect(c_tail->index());
			break;
		}
	}
	if (!found_child)
		pos_rect = visualRect(last_item->index());
	int max_right = maxCurrentWidth();
	bool w_chging = false, h_chging = false;
	if (max_right > width())
		w_chging = true;
	int end_geometry = pos_rect.y()+pos_rect.height();
	if (isExpanded(e_index))
	{
		if (end_geometry>height() && end_geometry>origin_rect.height())			
			h_chging = true;
	}
	else
	{
		if (end_geometry < origin_rect.height())
		{
			end_geometry = origin_rect.height();
			h_chging = true;
		}
	}
	if (w_chging || h_chging)
	{
		if (w_chging)
			setFixedWidth(max_right);
		if (h_chging)
			setFixedHeight(end_geometry);
		emit sizeChanged();
	}
}

void TableManagement::sendsigOrderFromPartner(PlotWidget * hide)
{
	QString replace(tr("替换"));
	QStandardItem * plot_item = hide->singlePlotInfo();
	emit newPlotEditCommand(new TableMgmtPlotEditCmd(this, plot_item->parent(), plot_item, replace, partner, 0, hide));
}

void TableManagement::closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint)
{
	setEditTriggers(QAbstractItemView::NoEditTriggers);	
	QTreeView::closeEditor(editor, hint);
}

void TableManagement::syncPartnersFrozenState(bool state)
{
	if (bk_viewers.size() == 1)
		return;
	QMapIterator<int, QPair<QStandardItem *, LittleWidgetsView *> > i_map(bk_viewers);
	while (i_map.hasNext())
	{
		i_map.next();
		if (i_map.value().second == partner)
			continue;
		i_map.value().second->resetQmlFrozenState(state);
	}
}

void TableManagement::mousePressEvent(QMouseEvent * event)
{
	press_time = new QTime;
	press_time -> start();
	QTreeView::mousePressEvent(event);
}

void TableManagement::longPressForDataesModel(QStandardItem * click_item, DataSelectModel * dataes_model)
{
	QList<QStandardItem *> on_line;
	dataes_model -> findItemsSingleLine(click_item, on_line, tr("日常数据"));
	if (on_line.at(0)->text() == tr("日常数据"))
		longPressForDailyItems(click_item);
	else
		qobject_cast<TablesGroup *>(partner->qmlAgent()->widget())->judgeShowForTreeDataes(this, click_item);	
}

void TableManagement::longPressForInfoesModel(QStandardItem * click_item)
{
	qobject_cast<TablesGroup *>(partner->qmlAgent()->widget()) -> judgeShowForTreeDataes(this, click_item);	
}

void TableManagement::longPressForBackupModel(QStandardItem * click_item, DataSelectModel * bk_model)
{	
	QStringList command_list;  
	if (click_item->parent()==bk_model->item(0) || click_item->parent()==bk_model->item(1))
	{
		command_list << tr("不操作") << tr("修改名称") << tr("单独备份");
		QStringList others_dbs;
		bk_model -> findExistedDbsBackupTo(click_item, others_dbs);
		if (!others_dbs.isEmpty())
			command_list << tr("备份到......");
		if (click_item->parent() == bk_model->item(0))
		{
			if (click_item -> hasChildren())
			{
				QString test_text = click_item->child(0)->text();
				if (test_text != tr("标记为删除"))
					command_list << tr("删除此工程");
			}
		}
		else
		{
			if (click_item -> hasChildren())
			{
				QString test_text = click_item->child(0)->text();
				if (test_text != tr("标记为删除"))
					command_list << tr("删除此自制表");
			}
		}
	}
	else if (click_item->parent() == bk_model->item(2))
	{
		connect(bk_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(activedItemChanged(QStandardItem *)));
		cell_oldText = click_item->text();
		edit_order = tr("修改标题");
		emit itemEditting(click_item->index());		
		return;
	}
	else if (click_item->parent()->parent()==bk_model->item(2) || click_item->parent()->parent()==bk_model->item(3))
		command_list << tr("不操作")<< tr("取消备份");	
	else
		return;
	if (!texts_hash.contains(click_item))
		texts_hash.insert(click_item, command_list);
	else if ((click_item->parent()==bk_model->item(0) || click_item->parent()==bk_model->item(1)) && command_list!=texts_hash.value(click_item))
		texts_hash[click_item] = command_list;			
	initDelegateStrViewer(click_item);
}

void TableManagement::longPressForPlotsModel(QStandardItem * click_item)
{
	QStringList view_list; 
	if (click_item->text()==tr("均值图") || click_item->text()==tr("西格玛图") || click_item->text()==tr("极差图"))
	{
		if (!texts_hash.contains(click_item))
		{
			view_list << tr("不选择") << tr("按组序选择") << tr("按时间选择") << tr("按人员选择") << tr("清除选择");
			texts_hash.insert(click_item, view_list);
		}
		initDelegateStrViewer(click_item);
	}
	else if (click_item->text()==tr("直方图") || click_item->text()==tr("正态概率纸"))
	{
		if (!texts_hash.contains(click_item))
		{
			if (click_item->parent()->text() == tr("测量基础数据"))
				view_list << tr("不选择") << tr("生成图") << tr("清除选择") ;
			else
				view_list << tr("不选择") << tr("按组序选择") << tr("按时间选择") << tr("清除选择");
			texts_hash.insert(click_item, view_list);
		}
		initDelegateStrViewer(click_item);
	}
	else if (click_item->text().contains(tr("按组序选择")) || click_item->text().contains(tr("按时间选择")) || click_item->text().contains(tr("按人员选择")))
	{
		if (click_item->hasChildren() && !click_item->child(0)->hasChildren())
			return;			
		QStandardItem * p_human = lookingForMatchItem(click_item, tr("链上人员"));
		if (p_human && p_human->hasChildren())
		{
			int i = 0;
			while (p_human->child(i))
			{
				if (p_human->child(i)->foreground() != QBrush(Qt::gray))
					break;
				if (i == p_human->rowCount()-1)
					return;
				i++;
			}
		}
		if (!texts_hash.contains(click_item))
		{
			view_list << tr("不选择") << tr("重新选择") << tr("生成图");
			texts_hash.insert(click_item, view_list);
		}
		initDelegateStrViewer(click_item);
	}
	else
		threeFactoresProcess(click_item);		
}

void TableManagement::longPressForDataInfoModel(QStandardItem * click_item, DataSelectModel * di_model)
{
	QList<QStandardItem *> info_list;
	di_model -> findItemsSingleLine(click_item, info_list, tr("工程信息"));
	if (info_list.at(0)->text() == tr("工程信息"))
		longPressForInfoesModel(click_item);
	else
		longPressForDataesModel(click_item, di_model);
}

void TableManagement::longPressForTotalModel(QStandardItem * click_item, DataSelectModel * total_model)
{
	if (!texts_hash.contains(click_item))
	{
		QStringList view_list; 
		view_list << tr("不选择");
		QString f_lbl(total_model->item(0)->text());
		QList<QStandardItem *> tol_list;
		total_model -> findItemsSingleLine(click_item, tol_list, f_lbl);
		if (tol_list.size()>1 && tol_list.at(1)->text().contains(tr("图")))
		{
			if (click_item->parent()->text()==tr("均值图") || click_item->parent()->text()==tr("西格玛图") || click_item->parent()->text()==tr("极差图") || click_item->parent()->text()==tr("直方图") || click_item->parent()->text()==tr("正态概率纸") || click_item->parent()->text()==tr("测量基础数据"))
				view_list << tr("添加图");
			else if (tol_list.at(1)->text() == tr("其他图源"))
			{
				if (!click_item->hasChildren())
					view_list << tr("添加图");
				else
					return;
			}	
			else
				return;
		}
		else if (tol_list.size()>1 && (tol_list.at(1)->text().contains(tr("数据")) || tol_list.at(1)->text().contains(tr("信息"))))		  
			view_list << tr("添加表");
		else
			return;
		texts_hash.insert(click_item, view_list);
	}
	initDelegateStrViewer(click_item);
}

void TableManagement::longPressForDailyItems(QStandardItem * daily_item)
{
	QStringList view_list;  
	if (daily_item->text()==tr("周期测量数据") || daily_item->text()==tr("全部均值") || daily_item->text()==tr("受控均值") || daily_item->text()==tr("异常均值") || daily_item->text()==tr("全部离散值") || daily_item->text()==tr("受控离散值") || daily_item->text()==tr("异常离散值") || daily_item->text()==tr("全部极差值") || daily_item->text()==tr("受控极差值") || daily_item->text()==tr("异常极差值") || daily_item->text()==tr("测量者"))
	{
		if (daily_item->text().contains(tr("受控")) || daily_item->text().contains(tr("异常")))
		{
			DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
			QStandardItem * tkey_item = t_model->findTimeParentItem(daily_item);
			QString et_stamp;	
			if (t_type < 6 && t_type > 2)
				et_stamp = QString("%1").arg(QDateTime::fromString(tkey_item->child(0)->child(0)->text()).toTime_t());
			else
				et_stamp = QString("%1").arg(QDateTime::fromString(t_model->timesSavedHash().value(tkey_item->child(0)), Qt::ISODate).toTime_t());		
			QSqlQueryModel dest_indb(this);
			QString dest_table;
			if (daily_item->text().contains(tr("均值")))
				dest_table = "dailyavr";			
			else if  (daily_item->text().contains(tr("离散值")))
				dest_table = "dailydev";
			else
				dest_table = "dailyrng";
			QString daily_info;
			t_model -> findExistedRelatedTable(et_stamp, dest_table, source->allTables(), daily_info);
			QString normal_state;		
			if (daily_item->text().contains(tr("受控")))
				normal_state = QString("%")+"endnormal"+QString("%");
			else
				normal_state = QString("%")+"endunormal"+QString("%");		
			source -> dataesByRelatedStringFromDb(daily_info, &dest_indb, "state", normal_state);												
			if (!dest_indb.rowCount())
			{
				if (daily_item->text().contains(tr("受控")))
					daily_item -> appendRow(new QStandardItem(tr("无受控点")));
				else
					daily_item -> appendRow(new QStandardItem(tr("无异常点")));
				return;
			}
		}
		if (!texts_hash.contains(daily_item))
		{
			view_list << tr("不选择") << tr("按组序选择") << tr("按时间选择") << tr("按人员选择") << tr("清除选择");
			texts_hash.insert(daily_item, view_list);
		}
		initDelegateStrViewer(daily_item);
	}  
	else if (daily_item->text().contains(tr("按组序选择")) || daily_item->text().contains(tr("按时间选择")) || daily_item->text().contains(tr("按人员选择")))
	{
		if (!texts_hash.keys().contains(daily_item))
		{
			view_list << tr("不选择") << tr("浏览数据");
			texts_hash.insert(daily_item, view_list);
		}
		initDelegateStrViewer(daily_item);	  
	}
	else
		threeFactoresProcess(daily_item);	
}

void TableManagement::replyQmlStrDataForDmTree(QStandardItem * for_item, int str_pos, const QString & reply_str)
{
	if (reply_str == tr("重新选择"))//no finished
	{}
	else if (reply_str == tr("浏览数据"))
		qobject_cast<TablesGroup *>(partner->qmlAgent()->widget())->judgeShowForTreeDataes(this, for_item);
	else
		threeFactoresProcess(for_item, str_pos, reply_str);
}

void TableManagement::replyQmlStrDataForBkTree(QStandardItem * for_item, DataSelectModel * bk_reply, QString & reply_str, QString & is_return)
{
	if (reply_str==tr("修改名称"))
	{
		connect(bk_reply, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(activedItemChanged(QStandardItem *)));
		cell_oldText = for_item->text();
		edit_order = tr("修改名称");
		emit itemEditting(for_item->index());
		return;
	}
	else if (reply_str == tr("备份到......"))
	{
		edit_order = reply_str;
		QStringList new_strs;
		bk_reply -> findExistedDbsBackupTo(for_item, new_strs);
		new_strs.push_front(tr("不操作"));
		strs_viewer -> setNewStrList(new_strs);
		is_return = tr("返回");
		return;
	}
	else if (strs_viewer->qmlDelegateType() == 11)
	{
		QStandardItem * db_item = 0;
		QList<QStandardItem *> chk_dbs;
		bk_reply -> findChildrenNotAll(bk_reply->item(3), chk_dbs);
		foreach (QStandardItem * d_item, chk_dbs)
		{
			if (d_item->text() == reply_str)
			{
				db_item = d_item;
				break;
			}
		}
		QString old_db = source->currentConnectionDb()->connectionName();
		QString db_name = "/home/dapang/workstation/spc-tablet/qsqlitebkdb/"+reply_str;			
		*source->currentConnectionDb() = QSqlDatabase::addDatabase("QSQLITE", reply_str);
		source->currentConnectionDb()->setDatabaseName(db_name);
		if (!source->currentConnectionDb()->open())
		{
			QSqlDatabase::removeDatabase(reply_str);
			*source->currentConnectionDb() = QSqlDatabase::database(old_db);				
			return;					
		}
		*source->currentConnectionDb() = QSqlDatabase::database(old_db);
		MainWindow * mw = qobject_cast<MainWindow *>(source->parent()->parent());		
		QString checking;
		QHash<QString, QString> collide_proj;
		if (!source->canMergeProjBetweenDbs(for_item->text(), checking, reply_str))
		{
			if (checking.contains(tr("备份过")))
			{
				DiaLog * bkhint_dialog = new DiaLog;
				QString hint_lbl = tr("hint:工程")+for_item->text()+tr("\n在")+reply_str+tr("\n数据库内备份过");
				bkhint_dialog -> initWinDialog(mw, hint_lbl, 0);
				QSqlDatabase::database(reply_str).close();
				QSqlDatabase::removeDatabase(reply_str);
				return;		  
			}
			else if (checking.contains(tr("同名")))
				collide_proj[for_item->text()] = "";
		}
		QPair<QStandardItem *, QStandardItem *> proj_db(for_item, db_item);
		if (!bk_reply->bkdbUsrsInformation().contains(proj_db))
			bk_reply -> projBackupJudgeMngs(for_item, db_item);
		QStringList chk_mngs = bk_reply->bkdbUsrsInformation().value(proj_db);
		if (!collide_proj.isEmpty() || !chk_mngs.isEmpty())
		{
			QHash<QString, QString> collide_mngs;
			QSqlTableModel dbto_model(this, QSqlDatabase::database(reply_str));
			source -> varsModelFromTable("dbInformations", &dbto_model);	
			QString dbtotime = dbto_model.data(dbto_model.index(0, 2)).toString();
			foreach (QString mng, chk_mngs)
			{
				QString mc_pswd = dbtotime+tr("，，。")+source->curDBmanageTable().value(mng).at(0)+tr("，，。")+mng;
				source->tmpMngsHashForDbsConfliction().insert(mc_pswd, "");
			}	  
			DiaLog * collide_dialog = new DiaLog;
			collide_dialog -> showForCollisionDbsDataes(collide_proj, source->tmpMngsHashForDbsConfliction(), mw); 
			if (!collide_dialog->exec())
			{
				QSqlDatabase::database(reply_str).close();
				QSqlDatabase::removeDatabase(reply_str);
				return;
			}
			else
			{
				if (!collide_dialog->chkCollideDbsState())
					return;
			}
		}
		QSqlDatabase::database(reply_str).close();	
		QSqlDatabase::removeDatabase(reply_str);  
		reply_str += tr("，")+edit_order;	
	}	
	m_commandStack->push(new TableMgmtGroupEditCmd(this, for_item, reply_str, 0));	
}

void TableManagement::replyQmlStrDataForPmTree(QStandardItem * for_item, int str_pos, const QString & reply_str)
{
	if (reply_str==tr("按组序选择") || reply_str==tr("按时间选择") || reply_str==tr("按人员选择"))
	{
	  	QStandardItem * next_item = new QStandardItem(reply_str);
		int i = 0;
		QString end_num;
		while (for_item->child(i))
		{
			if (for_item->child(i)->text().contains(reply_str))
			{
				QStringList next_nums = for_item->child(i)->text().split(reply_str);
				end_num = next_nums[1];
			}
			i++;
		}
		if (end_num.isEmpty())
		{
			QString new_text = reply_str+"1";
			next_item -> setText(new_text);
		}
		else
		{
			int new_num = end_num.toInt()+1;
			QString new_text = reply_str+QString("%1").arg(new_num);
			next_item -> setText(new_text);
		}
		if (reply_str == tr("按组序选择"))
			next_item -> appendRow(new QStandardItem(tr("组范围")));
		else if (reply_str == tr("按时间选择"))
			next_item -> appendRow(new QStandardItem(tr("时间范围")));
		else
			next_item -> appendRow(new QStandardItem(tr("记录人员")));
		emit newPlotEditCommand(new TableMgmtPlotEditCmd(this, for_item, next_item, reply_str, partner));
	}
	else if (reply_str == tr("重新选择"))
		emit newPlotEditCommand(new TableMgmtPlotEditCmd(this, for_item, reply_str, partner));
	else if (reply_str == tr("生成图"))
	{
		int i = 0;
		while (for_item->child(i))
		{
			if (for_item->child(i)->text().contains(tr("视图位置")))
				return;					
			i++;
		}
		QPair<int, int> add_pos;
		QPair<PlotWidget *, PlotWidget *> show_cover(0, 0);
		if (!partner->qmlAgent()->coveredPlotExisted(add_pos))
		{
			if (partner->qmlAgent()->allCurrentDefaults().isEmpty())// add warn dialog here
				return;
			show_cover.first = partner->qmlAgent()->newPlotwidgetPlaceMw(for_item, add_pos, back_calculator, source, partner);
		}		
		else
			partner->qmlAgent()->newPlotwidgetToCoverOld(for_item, show_cover, add_pos, back_calculator, source, partner);	
		QString h_pos = QString("%1").arg(add_pos.first+1);
		QString v_pos = QString("%1").arg(add_pos.second+1);		
		QString pos_text = tr("视图位置：")+h_pos+tr("，")+v_pos;
		QStandardItem * add_child = new QStandardItem(pos_text);
		emit newPlotEditCommand(new TableMgmtPlotEditCmd(this, for_item, add_child, reply_str, partner, show_cover.first, show_cover.second));	
	}
	else
		threeFactoresProcess(for_item, str_pos, reply_str);
}

void TableManagement::replyQmlStrDataForTmTree(QStandardItem * for_item, int str_pos, const QString & reply_str)
{
	if (reply_str==tr("周期测量数据") || reply_str==tr("组均值") || reply_str==tr("组离散值") || reply_str==tr("组极差值") || reply_str==tr("测量者") || reply_str == tr("浏览数据"))
		replyQmlStrDataForDmTree(for_item, str_pos, reply_str);
	else if (reply_str==tr("按组序选择") || reply_str==tr("按时间选择") || reply_str==tr("按人员选择"))
		replyQmlStrDataForPmTree(for_item, str_pos, reply_str);
	else if (reply_str == tr("重新选择"))
		tools_bar -> pushGroupEditCommand(new TableMgmtGroupEditCmd(this, for_item, reply_str, 0));	
	else if (reply_str == tr("添加表"))
		qobject_cast<TablesGroup *>(partner->qmlAgent()->widget())->judgeShowForTreeDataes(this, for_item);	
	else if (reply_str == tr("添加图"))
		qobject_cast<TablesGroup *>(partner->qmlAgent()->widget()) -> judgeShowOrderForPlot(this, for_item, reply_str);
	else
		threeFactoresProcess(for_item, str_pos, reply_str);
}

void TableManagement::threeFactoresProcess(QStandardItem * fac_item, int data_pos, const QString & reply_str)
{
	if (!reply_str.isEmpty())
	{
		if (reply_str==tr("不继续选择"))
			return;
		QStandardItem * next_item = new QStandardItem(reply_str);	  
		if (reply_str==tr("按组序选择") || reply_str==tr("按时间选择") || reply_str==tr("按人员选择"))
		{
			int i = 0;
			QString end_num;
			while (fac_item->child(i))
			{
				if (fac_item->child(i)->text().contains(reply_str))
				{
					QStringList next_nums = fac_item->child(i)->text().split(reply_str);
					end_num = next_nums[1];
				}
				i++;
			}
			if (end_num.isEmpty())
			{
				QString new_text = reply_str+"1";
				next_item -> setText(new_text);
			}
			else
			{
				int new_num = end_num.toInt()+1;
				QString new_text = reply_str+QString("%1").arg(new_num);
				next_item -> setText(new_text);
			}
			if (reply_str == tr("按组序选择"))
				next_item -> appendRow(new QStandardItem(tr("组范围")));
			else if (reply_str == tr("按时间选择"))
				next_item -> appendRow(new QStandardItem(tr("时间范围")));
			else
				next_item -> appendRow(new QStandardItem(tr("记录人员")));
		}
		else if (reply_str==tr("按组序继续选择") || reply_str==tr("按时间继续选择") || reply_str==tr("按人员继续选择"))
		{
			if (reply_str == tr("按组序继续选择"))
				next_item -> setText(tr("继续组范围"));
			else if (reply_str == tr("按时间继续选择"))
				next_item -> setText(tr("继续时间范围"));
			else if (reply_str == tr("按人员继续选择"))
				next_item -> setText(tr("继续选择人员"));			
		}		
		else if (fac_item->text()==tr("记录人员") || fac_item->text()==tr("继续选择人员") || fac_item->text()==tr("最终选择人员"))
		{
			if (fac_item->text() == tr("记录人员"))
				next_item -> appendRow(new QStandardItem(tr("继续缩小选择范围")));
			if (fac_item->text() == tr("继续选择人员"))
				next_item -> appendRow(new QStandardItem(tr("再次缩小选择范围")));
			QStringList real_names;
			real_names << texts_hash.value(fac_item).at(data_pos-1);
			texts_hash.insert(next_item, real_names);
			names_list[fac_item].removeAt(data_pos);
			if (names_list[fac_item].size() == 1)
				names_list[fac_item].clear();
			if (!texts_hash[fac_item].isEmpty())
				texts_hash[fac_item].removeAt(data_pos-1);			
		} 
		if (fac_item->text()!=tr("记录人员") && fac_item->text()!=tr("继续选择人员") && fac_item->text()!=tr("最终选择人员"))
		{
			if (reply_str!=tr("按组序选择") && reply_str!=tr("按时间选择") && reply_str!=tr("按人员选择"))
				texts_hash[fac_item].removeAt(data_pos);
			if (texts_hash[fac_item].size() == 1)
				texts_hash.remove(fac_item);
		}
		if (partner->nestedContainer())
			emit newPlotEditCommand(new TableMgmtPlotEditCmd(this, fac_item, next_item, reply_str, partner));		
		else
			qobject_cast<TablesGroup *>(partner->qmlAgent()->widget()) -> sendSigToToolbar(this, fac_item, next_item, reply_str);		
	}
	else
	{
		if (fac_item->text() == tr("组范围") || fac_item->text()==tr("继续组范围") || fac_item->text()==tr("最终组范围"))
		{
			if (fac_item->rowCount() > 1)
				return;
			initDelegateNumberViewer(fac_item);
		}	
		else if (fac_item->text() == tr("时间范围") || fac_item->text()==tr("继续时间范围") || fac_item->text()==tr("最终时间范围"))
		{
			if (fac_item->rowCount() > 1)
				return;
			initCalandarViewer(fac_item);
		}
		else if (fac_item->text()==tr("记录人员") || fac_item->text()==tr("继续选择人员") || fac_item->text()==tr("最终选择人员"))
		{
			if (texts_hash.contains(fac_item) && names_list[fac_item].isEmpty())
				return;
			ensureRecordersForViewer(fac_item);
			if (fac_item->hasChildren() && fac_item->child(0)->text().contains(tr("无")))
				return;
			initDelegateStrViewer(fac_item);
		}
		else if (fac_item->text()== tr("继续缩小选择范围") || fac_item->text()==tr("再次缩小选择范围"))
		{
			if (fac_item->hasChildren())
			{
				if (texts_hash.keys().contains(fac_item))
					texts_hash.remove(fac_item);
				return;
			}
			QStringList str_list;
			redefineStrlistByCtrlItem(fac_item, str_list);		
			texts_hash.insert(fac_item, str_list);
			initDelegateStrViewer(fac_item);
		}
	}
}

void TableManagement::initDelegateStrViewer(QStandardItem * in_item, const QStringList & specials)
{
	if (strs_viewer)
		return;
	strs_viewer = new EditLongToolsViewer(this);
	connect(strs_viewer, SIGNAL(killMyself(QWidget *)), this, SLOT(clearLongToolPt(QWidget *)));
	QDeclarativeContext * ctxt = strs_viewer->rootContext();
	strs_viewer -> setOrientation(EditLongToolsViewer::ScreenOrientationLockLandscape);
	QStringList val_list;
	if (in_item->text()==tr("记录人员") || in_item->text()==tr("继续选择人员") || in_item->text()==tr("最终选择人员"))
		val_list = names_list.value(in_item);
	else
	{
		if (specials.isEmpty())
			val_list = texts_hash.value(in_item);
		else
			val_list = specials;	
	}
	ctxt -> setContextProperty("StringesList", QVariant::fromValue(val_list));
	strs_viewer -> setMainQmlFile(QLatin1String("spc_qml/slideSelector.qml"), 10);
	strs_viewer -> setRelatedTreePt(t_type, this);
	strs_viewer -> setNestedPosItem(in_item);
	strs_viewer -> setGeometry(visualRect(in_item->index()));
	int height = 0;
	QFontMetrics fm(font());	
	height = fm.height()*10;
	if (in_item->text().contains("\n"))
		openNestedWidgetInBranch(strs_viewer, biggerWidthBetweenTextList(fm, val_list)*1.5, height);
	else
		openNestedWidgetInBranch(strs_viewer, biggerWidthBetweenTextList(fm, val_list, in_item->text())*1.2, height);	
}

void TableManagement::initDelegateNumberViewer(QStandardItem * in_item)
{
	strs_viewer = new EditLongToolsViewer(this);
	connect(strs_viewer, SIGNAL(killMyself(QWidget *)), this, SLOT(clearLongToolPt(QWidget *)));
	strs_viewer -> setNestedPosItem(in_item);	
	QDeclarativeContext * ctxt = strs_viewer->rootContext();	
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QStandardItem * tkey_item = t_model->findTimeParentItem(in_item);
	QString et_stamp;	
	if (t_type < 6 && t_type > 2)
		et_stamp = QString("%1").arg(QDateTime::fromString(tkey_item->child(0)->child(0)->text()).toTime_t());
	else
		et_stamp = QString("%1").arg(QDateTime::fromString(t_model->timesSavedHash().value(tkey_item->child(0)), Qt::ISODate).toTime_t());
	QList<QStandardItem *> all_lines;	
	t_model -> findItemsSingleLine(in_item, all_lines, tr("日常数据"));	
	QString daily_info;
	QStringList groups_list; 	
	if (all_lines.at(1)->text()!=tr("周期测量数据") && all_lines.at(1)->text()!=tr("直方图") && all_lines.at(1)->text()!=tr("正态概率纸"))
	{ 
		QSqlQueryModel dest_indb(this);
		QString dest_table;
		if (all_lines.at(1)->text().contains(tr("均值")))
			dest_table = "dailyavr";			
		else if  (all_lines.at(1)->text() == tr("离散值") || all_lines.at(1)->text() == tr("西格玛图"))
			dest_table = "dailydev";
		else
			dest_table = "dailyrng";
		t_model -> findExistedRelatedTable(et_stamp, dest_table, source->allTables(), daily_info);
		QString normal_state;		
		if (all_lines.at(2)->text().contains(tr("受控")))
			normal_state = QString("%")+"endnormal"+QString("%");
		else if (all_lines.at(2)->text().contains(tr("异常")))
			normal_state = QString("%")+"endunormal"+QString("%");
		else
			normal_state = "all";
		if (in_item->text() == tr("组范围"))
		{
			if (!texts_hash.contains(in_item))
			{
				source -> dataesByRelatedStringFromDb(daily_info, &dest_indb, "state", normal_state);			
				for (int i = 0; i < dest_indb.rowCount(); i++)
					groups_list << dest_indb.data(dest_indb.index(i, 0)).toString();	
				dest_indb.clear();													
				texts_hash.insert(in_item, groups_list);			
			}
			else
			{
				groups_list = texts_hash.value(in_item);
				QString head_num;
				if (!in_item -> hasChildren())
					head_num = groups_list[0];
				else
					head_num = texts_hash.value(in_item->child(0)).at(0);
				if (in_item -> hasChildren())
				{
					foreach (QString str, groups_list)
					{
						if (str == head_num)
							break;
						groups_list.removeAll(str);
					}
				}
			}
		}
		else
		{
			if (in_item->parent()->parent()->text().contains(tr("时间到：")))
			{
				QString from_time = texts_hash.value(in_item->parent()->parent()->parent()->child(0)).at(0);
				QString to_time = texts_hash.value(in_item->parent()->parent()->parent()->child(1)).at(0);
				QDateTime from = QDateTime::fromString(from_time, Qt::ISODate);
				QDateTime to = QDateTime::fromString(to_time, Qt::ISODate);
				if (from == to)
					to = to.addDays(1); 
				to_time = to.toString(Qt::ISODate);
				if (in_item->text() == tr("继续组范围"))
				{
					if (!texts_hash.contains(in_item))	
					{
						source -> dataesByTimeVarFromDb(from, to, daily_info, &dest_indb);
						normal_state.remove(0, 1); 
						normal_state.remove(normal_state.size()-1, 1);
						for (int i = 0; i < dest_indb.rowCount(); i++)
						{
							if (dest_indb.data(dest_indb.index(i, 2)).toString().contains(normal_state))
								groups_list << dest_indb.data(dest_indb.index(i, 0)).toString();
						}
						dest_indb.clear();
						texts_hash.insert(in_item, groups_list);
					}
					else	
					{
						groups_list = texts_hash.value(in_item);
						QString head_num;
						if (!in_item -> hasChildren())
							head_num = groups_list[0];
						else
							head_num = texts_hash.value(in_item->child(0)).at(0);
						if (in_item -> hasChildren())
						{
							foreach (QString str, groups_list)
							{
								if (str == head_num)
									break;
								groups_list.removeAll(str);
							}
						}				
					}
				}
				else
				{
					if (!texts_hash.contains(in_item))	
					{
						QStringList real_name = texts_hash.value(all_lines[6]);
						QDateTime from = QDateTime::fromString(from_time, Qt::ISODate);
						QDateTime to = QDateTime::fromString(to_time, Qt::ISODate);
						if (from == to)
							to = to.addDays(1); 
						QString empty;
						QStringList name_groups;
						source -> getDataesFromTable(daily_info, "groupnum", name_groups, empty, "person", real_name[0]);
						source -> dataesByTimeVarFromDb(from, to, daily_info, &dest_indb, false, "state", normal_state);
						normal_state.remove(0, 1); 
						normal_state.remove(normal_state.size()-1, 1);
						for (int i = 0; i < dest_indb.rowCount(); i++)
						{
							if (name_groups.contains(dest_indb.data(dest_indb.index(i, 0)).toString()) && dest_indb.data(dest_indb.index(i, 2)).toString().contains(normal_state))
								groups_list << dest_indb.data(dest_indb.index(i, 0)).toString();
						}
						dest_indb.clear();
						texts_hash.insert(in_item, groups_list);								
					}
					else
					{
						groups_list = texts_hash.value(in_item);
						QString head_num;
						if (!in_item -> hasChildren())
							head_num = groups_list[0];
						else
							head_num = texts_hash.value(in_item->child(0)).at(0);
						if (in_item -> hasChildren())
						{
							foreach (QString str, groups_list)
							{
								if (str == head_num)
									break;
								groups_list.removeAll(str);
							}
						}
					}			
				}
			}
			else
			{
				if (in_item->text() == tr("继续组范围"))
				{
					if (!texts_hash.contains(in_item))	
					{			
						QString empty;
						QStringList name_groups;
						QString name = texts_hash.value(in_item->parent()->parent()).at(0);
						source -> getDataesFromTable(daily_info, "groupnum", name_groups, empty, "person", name);
						source -> dataesByRelatedStringFromDb(daily_info, &dest_indb, "state", normal_state);
						for (int i = 0; i < dest_indb.rowCount(); i++)
						{
							if (name_groups.contains(dest_indb.data(dest_indb.index(i, 0)).toString()))
								groups_list << dest_indb.data(dest_indb.index(i, 0)).toString();
						}
						dest_indb.clear();			
						texts_hash.insert(in_item, groups_list);	
					}
					else
					{
						groups_list = texts_hash.value(in_item);
						QString head_num;
						if (!in_item -> hasChildren())
							head_num = groups_list[0];
						else
							head_num = texts_hash.value(in_item->child(0)).at(0);
						if (in_item -> hasChildren())
						{
							foreach (QString str, groups_list)
							{
								if (str == head_num)
									break;
								groups_list.removeAll(str);
							}
						}
					}
				}
				else
				{
					if (!texts_hash.contains(in_item))	
					{			
						QString from_time = texts_hash.value(all_lines[5]->child(0)).at(0);
						QString to_time = texts_hash.value(all_lines[5]->child(1)).at(0);
						QDateTime from = QDateTime::fromString(from_time, Qt::ISODate);
						QDateTime to = QDateTime::fromString(to_time, Qt::ISODate);
						if (from == to)
							to = to.addDays(1); 
						QStringList recorder_name = texts_hash.value(in_item->parent()->parent());
						QString empty;
						QStringList name_groups;
						source -> getDataesFromTable(daily_info, "groupnum", name_groups, empty, "person", recorder_name[0]);
						source -> dataesByTimeVarFromDb(from, to, daily_info, &dest_indb, false, "state", normal_state);
						normal_state.remove(0, 1); 
						normal_state.remove(normal_state.size()-1, 1);
						for (int i = 0; i < dest_indb.rowCount(); i++)
						{
							if (name_groups.contains(dest_indb.data(dest_indb.index(i, 0)).toString()) && dest_indb.data(dest_indb.index(i, 2)).toString().contains(normal_state))
								groups_list << dest_indb.data(dest_indb.index(i, 0)).toString();
						}
						dest_indb.clear();
						texts_hash.insert(in_item, groups_list);					
					}
					else
					{
						groups_list = texts_hash.value(in_item);
						QString head_num;
						if (!in_item -> hasChildren())
							head_num = groups_list[0];
						else
							head_num = texts_hash.value(in_item->child(0)).at(0);
						if (in_item -> hasChildren())
						{
							foreach (QString str, groups_list)
							{
								if (str == head_num)
									break;
								groups_list.removeAll(str);
							}
						}
					}														
				}	
			}
		}
	}
	else
	{
		t_model -> findExistedRelatedTable(et_stamp, "dailydataes", source->allTables(), daily_info);			
		if (in_item->text() == tr("组范围"))
		{
			if (!texts_hash.contains(in_item))	
			{			
				QString end_num = source->lastDataFromTable(daily_info, "groupnum").toString();
				for (int i = 1; i < end_num.toInt()+1; i++)
					groups_list << QString("%1").arg(i);
				texts_hash.insert(in_item, groups_list);		
			}
			else
			{
				groups_list = texts_hash.value(in_item);
				QString head_num;
				if (!in_item -> hasChildren())
					head_num = groups_list[0];
				else
					head_num = texts_hash.value(in_item->child(0)).at(0);
				if (in_item -> hasChildren())
				{
					foreach (QString str, groups_list)
					{
						if (str == head_num)
							break;
						groups_list.removeAll(str);
					}
				}
			}			
		}
		else
		{
			if (in_item->parent()->parent()->text().contains(tr("时间到：")))
			{
				QString from_time = texts_hash.value(in_item->parent()->parent()->parent()->child(0)).at(0);
				QString to_time = texts_hash.value(in_item->parent()->parent()->parent()->child(1)).at(0);
				if (in_item->text() == tr("继续组范围"))
				{
					QStringList from_to;
					if (!texts_hash.contains(in_item))	
					{
						QString from_var = "time="+from_time;
						QString to_var = "time="+to_time;
						QString head_num = source->dataFromTable(daily_info, "groupnum", from_var).toString();
						QString end_num = source->dataFromTable(daily_info, "groupnum", to_var).toString();
						from_to << head_num << end_num;
						texts_hash.insert(in_item, from_to);	
					}	
				}
				else
				{
					if (!texts_hash.contains(in_item))	
					{
						QStringList real_name;
						if (t_type == 0)
						{
							if (all_lines[3]->text()==tr("组均值") || all_lines[3]->text()==tr("组离散值") || all_lines[3]->text()==tr("组极差值"))
								real_name = texts_hash.value(all_lines[6]);
							else
								real_name = texts_hash.value(all_lines[5]);
						}
						if (t_type>2 && t_type<6)
							real_name = texts_hash.value(all_lines[3]);
						QSqlQueryModel dest_indb(this);
						QDateTime from = QDateTime::fromString(from_time, Qt::ISODate);
						QDateTime to = QDateTime::fromString(to_time, Qt::ISODate);
						if (from == to)
							to = to.addDays(1); 
						QString recorder("person");
						source -> dataesByTimeVarFromDb(from, to, daily_info, &dest_indb, true, recorder, real_name[0]);
						for (int i = 0; i < dest_indb.rowCount(); i++)
							groups_list << dest_indb.data(dest_indb.index(i, 0)).toString();	
						dest_indb.clear();									
					}
					else
					{
						groups_list = texts_hash.value(in_item);
						if (in_item -> hasChildren())
						{
							QString head_num = texts_hash.value(in_item->child(0)).at(0);
							foreach (QString str, groups_list)
							{
								if (str == head_num)
									break;
								groups_list.removeAll(str);
							}
						}
					}
					texts_hash.insert(in_item, groups_list);				
				}
			}
			else
			{
				QStringList groups_list;
				if (in_item->text() == tr("继续组范围"))
				{
					if (!texts_hash.contains(in_item))	
					{			
						QSqlTableModel dest_indb(this, source->currentConnectionDb()->database(source->currentConnectionDb()->connectionName()));
						source -> varsModelFromTable(daily_info, &dest_indb);
						QString name = texts_hash.value(in_item->parent()->parent()).at(0);
						QString select_name = QString("%1='%2'").arg("person").arg(name);
						dest_indb.setFilter(select_name);
						for (int i = 0; i < dest_indb.rowCount(); i++)
							groups_list << dest_indb.data(dest_indb.index(i, 0)).toString();			
						texts_hash.insert(in_item, groups_list);	
					}
					else
					{
						groups_list = texts_hash.value(in_item);
						if (in_item -> hasChildren())
						{
							QString head_num = texts_hash.value(in_item->child(0)).at(0);
							foreach (QString str, groups_list)
							{
								if (str == head_num)
									break;
								groups_list.removeAll(str);
							}
						}
					}
				}
				else
				{
					if (!texts_hash.contains(in_item))	
					{			
						QSqlQueryModel dest_indb(this);
						QString from_time;
						QString to_time;
						if (t_type == 0)
						{
							if (all_lines[3]->text()==tr("组均值") || all_lines[3]->text()==tr("组离散值") || all_lines[3]->text()==tr("组极差值"))
							{
								from_time = texts_hash.value(all_lines[5]->child(0)).at(0);
								to_time = texts_hash.value(all_lines[5]->child(1)).at(0);
							}
							else
							{
								from_time = texts_hash.value(all_lines[4]->child(0)).at(0);
								to_time = texts_hash.value(all_lines[4]->child(1)).at(0);
							}
						}
						if (t_type>2 && t_type<6)
						{
							from_time = texts_hash.value(all_lines[3]->child(0)).at(0);
							to_time = texts_hash.value(all_lines[3]->child(1)).at(0);
						}
						QDateTime from = QDateTime::fromString(from_time, Qt::ISODate);
						QDateTime to = QDateTime::fromString(to_time, Qt::ISODate);
						if (from == to)
							to = to.addDays(1); 
						QStringList recorder_name = texts_hash.value(in_item->parent()->parent());
						source -> dataesByTimeVarFromDb(from, to, daily_info, &dest_indb, true, "person", recorder_name[0]);	
						for (int i = 0; i < dest_indb.rowCount(); i++)
							groups_list << dest_indb.data(dest_indb.index(i, 0)).toString();
						texts_hash.insert(in_item, groups_list);
						dest_indb.clear();						
					}
					else
					{
						groups_list = texts_hash.value(in_item);
						if (in_item -> hasChildren())
						{
							QString head_num = texts_hash.value(in_item->child(0)).at(0);
							foreach (QString str, groups_list)
							{
								if (str == head_num)
									break;
								groups_list.removeAll(str);
							}
						}
					}														
				}	
			}
		}
	}
	ctxt -> setContextProperty("StringesList", QVariant::fromValue(groups_list));	
	strs_viewer -> setMainQmlFile("spc_qml/numberselector.qml", 9);
	strs_viewer -> setRelatedTreePt(t_type, this);
	QFontMetrics fm(font());
	int num_width = fm.width(groups_list.back())*1.5;
	int end_height =  fm.height()*15;
	QRect v_rect = visualRect(in_item->index());
	strs_viewer -> setGeometry(v_rect.left(), v_rect.top(), num_width+100, end_height);
	openNestedWidgetInBranch(strs_viewer, num_width+100, end_height);	
}

void TableManagement::initCalandarViewer(QStandardItem * in_item)
{
	strs_viewer = new EditLongToolsViewer(this);
	connect(strs_viewer, SIGNAL(killMyself(QWidget *)), this, SLOT(clearLongToolPt(QWidget *)));
	setAllDatesList(in_item);
	strs_viewer -> setMainQmlFile(QLatin1String("spc_qml/calandar.qml"), 8);
	strs_viewer -> setOrientation(EditLongToolsViewer::ScreenOrientationLockLandscape);
	strs_viewer -> setRelatedTreePt(t_type, this);
	strs_viewer -> setNestedPosItem(in_item);
	strs_viewer -> setGeometry(visualRect(in_item->index()));
	openNestedWidgetInBranch(strs_viewer, 200, 100);
	QString time_title;
	if (!in_item -> hasChildren())
		time_title = tr("时间从：");
	if (in_item->rowCount()==1)	
		time_title = tr("时间到：");
	emit timeDefineTitle(time_title);
}

void TableManagement::setAllDatesList(QStandardItem * mark_item)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QStandardItem * tkey_item = t_model->findTimeParentItem(mark_item);
	QString et_stamp;
	if (t_type < 6 && t_type > 2)
		et_stamp = QString("%1").arg(QDateTime::fromString(tkey_item->child(0)->child(0)->text()).toTime_t());
	else
		et_stamp = QString("%1").arg(QDateTime::fromString(t_model->timesSavedHash().value(tkey_item->child(0)), Qt::ISODate).toTime_t());
	QList<QStandardItem *> all_lines;
	t_model -> findItemsSingleLine(mark_item, all_lines, tr("日常数据"));		
	QString daily_info;
	QString from_year;
	QString end_year;
	QDateTime f_datatime;
	QString f_strtime;
	QDateTime e_datatime;
	QStringList all_times; 
	if (all_lines.at(1)->text() != tr("周期测量数据") && all_lines.at(1)->text()!=tr("直方图") && all_lines.at(1)->text()!=tr("正态概率纸"))
	{
		QSqlQueryModel dest_indb(this);
		QString dest_table;
		if (all_lines.at(1)->text().contains(tr("均值")))
			dest_table = "dailyavr";
		else if  (all_lines.at(1)->text()==tr("离散值") || all_lines.at(1)->text()==tr("西格玛图"))
			dest_table = "dailydev";
		else
			dest_table = "dailyrng";
		t_model -> findExistedRelatedTable(et_stamp, dest_table, source->allTables(), daily_info);		
		QString normal_state;
		if (all_lines.at(2)->text().contains(tr("受控")))
			normal_state = QString("%")+"endnormal"+QString("%");
		else
			normal_state = QString("%")+"endunormal"+QString("%");
		if (mark_item->text() == tr("时间范围"))
		{
			if (!texts_hash.contains(mark_item))
			{
				source -> dataesByRelatedStringFromDb(daily_info, &dest_indb, "state", normal_state);
				normal_state.remove(0, 1); 
				normal_state.remove(normal_state.size()-1, 1);
				for (int i = 0; i < dest_indb.rowCount(); i++)
				{
					if (dest_indb.data(dest_indb.index(i, 2)).toString().contains(normal_state))
						all_times << dest_indb.data(dest_indb.index(i, 3)).toString();
				}						
				dest_indb.clear();									
				texts_hash.insert(mark_item, all_times);			
			}
			else
			{
				if (mark_item->hasChildren() && mark_item->child(0)->text().contains(tr("无")))
					return;							
				all_times = texts_hash.value(mark_item);
				if (mark_item->rowCount() == 1)
				{
					QString head_date = texts_hash.value(mark_item->child(0)).at(0);
					foreach (QString str, all_times)
					{
						if (str == head_date)	
							break;
						all_times.removeOne(str);			
					}			
				}
			}
		}
		else
		{
			if (!texts_hash.contains(mark_item))
			{
				QStringList from_list;
				QStringList to_list;
				QStringList real_name;
				QStandardItem * grp_item = existedStrOnItemlist(tr("组到："), all_lines);				
				if (mark_item->text()==tr("继续时间范围"))
				{
					if (grp_item)
					{
						from_list = texts_hash.value(grp_item->parent()->child(0));
						to_list = texts_hash.value(grp_item->parent()->child(1));
						source -> dataesByGroupsFromDb(from_list[0].toInt(), to_list[0].toInt(), daily_info, &dest_indb);
						normal_state.remove(0, 1); 
						normal_state.remove(normal_state.size()-1, 1);
						for (int i = 0; i < dest_indb.rowCount(); i++)
						{
							if (dest_indb.data(dest_indb.index(i, 2)).toString().contains(normal_state))
								all_times << dest_indb.data(dest_indb.index(i, 3)).toString();
						}
						dest_indb.clear();					
					}
					else
					{
						real_name = texts_hash.value(mark_item->parent());
						QString empty;
						QStringList name_groups;
						source -> getDataesFromTable(daily_info, "groupnum", name_groups, empty, "person", real_name[0]);
						source -> dataesByGroupsFromDb(name_groups[0].toInt(), name_groups[name_groups.size()-1].toInt(), daily_info, &dest_indb, false, "state", normal_state);	
						normal_state.remove(0, 1); 
						normal_state.remove(normal_state.size()-1, 1);
						for (int i = 0; i < dest_indb.rowCount(); i++)
						{
							if (name_groups.contains(dest_indb.data(dest_indb.index(i, 0)).toString()) && dest_indb.data(dest_indb.index(i, 2)).toString().contains(normal_state))
								all_times << dest_indb.data(dest_indb.index(i, 3)).toString();
						}
						dest_indb.clear();
					}
				}
				else
				{
					QStringList person_list;
					QString empty;
					if (grp_item)
					{
						real_name = texts_hash.value(mark_item->parent());	
						from_list = texts_hash.value(grp_item->parent()->child(0));
						to_list = texts_hash.value(grp_item->parent()->child(1));				
					}
					else
					{
						QStandardItem * time_item = existedStrOnItemlist(tr("时间到："), all_lines);					  
						from_list = texts_hash.value(time_item->parent()->child(0));
						to_list = texts_hash.value(time_item->parent()->child(1));
						real_name = texts_hash.value(mark_item->parent()->parent());						
					}
					source -> getDataesFromTable(daily_info, "groupnum", person_list, empty, "person", real_name[0]);
					source -> dataesByGroupsFromDb(from_list[0].toInt(), to_list[0].toInt(), daily_info, &dest_indb);
					normal_state.remove(0, 1); 
					normal_state.remove(normal_state.size()-1, 1);	
					for (int i = 0; i < dest_indb.rowCount(); i++)
					{
						if (person_list.contains(dest_indb.data(dest_indb.index(i, 0)).toString()) && dest_indb.data(dest_indb.index(i, 2)).toString().contains(normal_state))
							all_times << dest_indb.data(dest_indb.index(i, 3)).toString();
					}
					dest_indb.clear();		
				}
				texts_hash.insert(mark_item, all_times);
			}
			else
			{
				all_times = texts_hash.value(mark_item);
				if (mark_item->rowCount() == 1)
				{
					QString head_date = texts_hash.value(mark_item->child(0)).at(0);
					foreach (QString str, all_times)
					{
						if (str == head_date)	
							break;
						all_times.removeOne(str);			
					}			
				}
			}		
		}
	}
	else
	{
		t_model -> findExistedRelatedTable(et_stamp, "dailydataes", source->allTables(), daily_info);		  
		if (mark_item->text() == tr("时间范围"))//need more precision to hour and min?
		{
			if (!texts_hash.contains(mark_item))
			{
				from_year = QString("groupnum=%1").arg("1");
				f_datatime = source->dataFromTable(daily_info, "time", from_year).toDateTime();
				f_strtime = source->dataFromTable(daily_info, "time", from_year).toString();
				e_datatime = source->lastDataFromTable(daily_info, "time").toDateTime();
				source -> getDataesFromTable(daily_info, "time", all_times);
				texts_hash.insert(mark_item, all_times);
			}
			else
			{
				all_times = texts_hash.value(mark_item);
				if (mark_item->rowCount() == 1)
				{
					QString head_date = texts_hash.value(mark_item->child(0)).at(0);
					foreach (QString str, all_times)
					{
						if (str == head_date)	
							break;
						all_times.removeOne(str);			
					}			
				}
			}
		}
		else
		{
			if (!texts_hash.contains(mark_item))
			{
				QStringList from_list;
				QStringList to_list;
				QStringList real_name;
				if (mark_item->text()==tr("继续时间范围"))
				{
					if (mark_item->parent()->parent()->text().contains(tr("组到：")))
					{
						from_list = texts_hash.value(mark_item->parent()->parent()->parent()->child(0));
						to_list = texts_hash.value(mark_item->parent()->parent()->parent()->child(1));
						all_times = source->desVarDataesFromTable(daily_info, "time", from_list[0].toInt(), to_list[0].toInt());	
					}
					else
					{
						if (t_type == 0)
						{
							if (all_lines[3]->text()==tr("组均值") || all_lines[3]->text()==tr("组离散值") || all_lines[3]->text()==tr("组极差值"))
								real_name = texts_hash.value(all_lines[6]);
							else
								real_name = texts_hash.value(all_lines[5]);
						}
						if (t_type>2 && t_type<6)
							real_name = texts_hash.value(all_lines[3]);	
						QSqlTableModel dest_indb(this, source->currentConnectionDb()->database(source->currentConnectionDb()->connectionName()));
						source -> varsModelFromTable(daily_info, &dest_indb);
						QString select_name = QString("%1='%2'").arg("person").arg(real_name[0]);
						dest_indb.setFilter(select_name);
						for (int i = 0; i < dest_indb.rowCount(); i++)
							all_times << dest_indb.data(dest_indb.index(i, 2)).toString();								
					}
				}
				else
				{
					QSqlQueryModel dest_indb(this);
					if (mark_item->parent()->parent()->text().contains(tr("组到：")))
					{
						if (t_type == 0)
						{
							if (all_lines[3]->text()==tr("组均值") || all_lines[3]->text()==tr("组离散值") || all_lines[3]->text()==tr("组极差值"))
								real_name = texts_hash.value(all_lines[6]);
							else
								real_name = texts_hash.value(all_lines[5]);
						}
						if (t_type>2 && t_type<6)
							real_name = texts_hash.value(all_lines[3]);	
						from_list = texts_hash.value(mark_item->parent()->parent()->parent()->child(0));
						to_list = texts_hash.value(mark_item->parent()->parent()->parent()->child(1));				
					}
					else
					{
						if (t_type == 0)
						{
							if (all_lines[3]->text()==tr("组均值") || all_lines[3]->text()==tr("组离散值") || all_lines[3]->text()==tr("组极差值"))
							{
								from_list = texts_hash.value(all_lines[5]->child(0));
								to_list = texts_hash.value(all_lines[5]->child(1));
							}
							else
							{
								from_list = texts_hash.value(all_lines[4]->child(0));
								to_list = texts_hash.value(all_lines[4]->child(1));
							}
						}
						if (t_type>2 && t_type<6)
						{
							from_list = texts_hash.value(all_lines[3]->child(0));
							to_list = texts_hash.value(all_lines[3]->child(1));
						}
						real_name = texts_hash.value(mark_item->parent()->parent());									
					}
					source -> dataesByGroupsFromDb(from_list[0].toInt(), to_list[0].toInt(), daily_info, &dest_indb, true, "person", real_name[0]);	
					for (int i = 0; i < dest_indb.rowCount(); i++)
						all_times << dest_indb.data(dest_indb.index(i, 2)).toString();
					dest_indb.clear();	
				}
				texts_hash.insert(mark_item, all_times);
			}
			else
			{
				all_times = texts_hash.value(mark_item);
				if (mark_item->rowCount() == 1)
				{
					QString head_date = texts_hash.value(mark_item->child(0)).at(0);
					foreach (QString str, all_times)
					{
						if (str == head_date)	
							break;
						all_times.removeOne(str);			
					}			
				}
			}		
		}
	}
	f_datatime = QDateTime::fromString(all_times[0], Qt::ISODate);//bug here not often? why
	f_strtime = all_times[0];
	e_datatime = QDateTime::fromString(all_times[all_times.size()-1], Qt::ISODate);	
	int first_year = f_datatime.date().year();
	int last_year = e_datatime.date().year();
	if (!year_list.isEmpty())
		year_list.clear();
	if (!month_list.isEmpty())
		month_list.clear();
	if (!days_list.isEmpty())
		days_list.clear();
	year_list << "" << "";
	month_list << "" << "";
	days_list << "" << "";
	for (int i = first_year; i < last_year+1; i++)
	{
		QStringList ys_list;
		if (i > first_year)
			ys_list = all_times.filter(QString("%1").arg(i));
		else
			ys_list << QString("%1").arg(i);
		if (!ys_list.isEmpty())
			year_list.push_back(QString("%1").arg(i));
	}
	QStringList editting_month;
	redefineDatesListForNewSelection(f_strtime, all_times, true, editting_month);
	foreach (QString mon, editting_month)
	{
		QStringList str = mon.split("-");
		if (month_list.back() != str[1])
			month_list.push_back(str[1]);
	}
	QStringList editting_day;
	redefineDatesListForNewSelection(f_strtime, all_times, false, editting_day);		
	foreach (QString mon, editting_day)
	{
		QStringList str = mon.split("-");
		if (days_list.back() != str[2])
			days_list.push_back(str[2]);
	}
	days_list << "" << "";
	month_list << "" << "";
	year_list << "" << "";
	QDeclarativeContext * ctxt = strs_viewer->rootContext();	
	ctxt -> setContextProperty("YearsList", QVariant::fromValue(year_list));
	ctxt -> setContextProperty("MonthsList", QVariant::fromValue(month_list));
	ctxt -> setContextProperty("DaysList", QVariant::fromValue(days_list));
}

void TableManagement::ensureRecordersForViewer(QStandardItem * r_item)
{
	if (names_list.contains(r_item) && !names_list.value(r_item).isEmpty())
		return;
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QStandardItem * tkey_item = t_model->findTimeParentItem(r_item);	
	QString et_stamp;
	if (t_type < 6 && t_type > 2)
		et_stamp = QString("%1").arg(QDateTime::fromString(tkey_item->child(0)->child(0)->text()).toTime_t());
	else
		et_stamp = QString("%1").arg(QDateTime::fromString(t_model->timesSavedHash().value(tkey_item->child(0)), Qt::ISODate).toTime_t());
	QList<QStandardItem *> all_lines;	
	t_model -> findItemsSingleLine(r_item, all_lines, tr("日常数据"));		
	QString daily_info;
	QStringList all_recorder;
	QString person_table;
	t_model -> findExistedRelatedTable(et_stamp, "dailydataes", source->allTables(), person_table);	
	if (all_lines.at(1)->text() != tr("周期测量数据"))// bug here for 主辅图的childrenItem 问题
	{
		QSqlQueryModel dest_indb(this);
		QString group("groupnum=");
		QString dest_table;
		if (all_lines.at(1)->text().contains(tr("均值")))
			dest_table = "dailyavr";
		else if  (all_lines.at(1)->text()==tr("离散值") || all_lines.at(1)->text()==tr("西格玛图"))
			dest_table = "dailydev";
		else
			dest_table = "dailyrng";
		t_model -> findExistedRelatedTable(et_stamp, dest_table, source->allTables(), daily_info);
		QString normal_state;
		if (all_lines.at(2)->text().contains(tr("受控")))
			normal_state = QString("%")+"endnormal"+QString("%");
		else
			normal_state = QString("%")+"endunormal"+QString("%");		
		QStringList real_all;
		source -> getDataesFromTable(person_table, "person", real_all);
		if (r_item->text() == tr("记录人员"))	
		{
			source -> dataesByRelatedStringFromDb(daily_info, &dest_indb, "state", normal_state);
			for (int i = 0; i < dest_indb.rowCount(); i++)
				all_recorder << real_all.at(dest_indb.data(dest_indb.index(i, 0)).toInt()-1);
			dest_indb.clear();		
		}
		else
		{
			QStandardItem * grp_item = existedStrOnItemlist(tr("组到："), all_lines);		  
			QStringList from_list;
			QStringList to_list;		
			if (grp_item)
			{
				from_list = texts_hash.value(grp_item->parent()->child(0));
				to_list = texts_hash.value(grp_item->parent()->child(1));			  
				source -> dataesByGroupsFromDb(from_list[0].toInt(), to_list[0].toInt(), daily_info, &dest_indb);	
			}
			else
			{
				QStandardItem * time_item = existedStrOnItemlist(tr("时间到："), all_lines);	
				from_list = texts_hash.value(time_item->parent()->child(0));
				to_list = texts_hash.value(time_item->parent()->child(1));				
				QDateTime from_time = QDateTime::fromString(from_list[0], Qt::ISODate);
				QDateTime to_time = QDateTime::fromString(to_list[0], Qt::ISODate);
				if (from_time == to_time)
					to_time = to_time.addDays(1); 
				source -> dataesByTimeVarFromDb(from_time, to_time, daily_info, &dest_indb);
			}
			normal_state.remove(0, 1); 
			normal_state.remove(normal_state.size()-1, 1);
			for (int i = 0; i < dest_indb.rowCount(); i++)
			{
				if (dest_indb.data(dest_indb.index(i, 2)).toString().contains(normal_state))
					all_recorder << real_all.at(dest_indb.data(dest_indb.index(i, 0)).toInt()-1);
			}
			dest_indb.clear();					
		}
	}
	else
	{	  
		if (r_item->text() == tr("记录人员"))	
			source -> getDataesFromTable(person_table, "person", all_recorder);		
		else
		{
			QStandardItem * grp_item = existedStrOnItemlist(tr("组到："), all_lines);		  
			QSqlTableModel dest_indb(this, source->currentConnectionDb()->database(source->currentConnectionDb()->connectionName()));
			source -> varsModelFromTable(daily_info, &dest_indb);
			QStringList from_list;
			QStringList to_list;		
			if (grp_item)
			{
				from_list = texts_hash.value(grp_item->parent()->child(0));
				to_list = texts_hash.value(grp_item->parent()->child(1));			  
				QString group_range = QString("groupnum <= '%1' and groupnum > '%2'").arg(to_list[0]).arg(from_list[0]);
				dest_indb.setFilter(group_range);					
			}
			else
			{
				QStandardItem * time_item = existedStrOnItemlist(tr("时间到："), all_lines);	
				from_list = texts_hash.value(time_item->parent()->child(0));
				to_list = texts_hash.value(time_item->parent()->child(1));			  
				QString time_range = QString("time <= '%1' and time > '%2'").arg(to_list[0]).arg(from_list[0]);
				dest_indb.setFilter(time_range);								
			}
			for (int i = 0; i < dest_indb.rowCount(); i++)
				all_recorder << dest_indb.data(dest_indb.index(i, 3)).toString();					
		}
	}
	QStringList real_recorder;
	QStringList show_recorder;
	show_recorder << tr("不选择");
	foreach (QString str, all_recorder)
	{
		if (!real_recorder.contains(str))
		{
			real_recorder << str;
			show_recorder << source->curDBmanageTable().value(str).at(0);
		}
	}
	names_list.insert(r_item, show_recorder);
	texts_hash.insert(r_item, real_recorder);
}

void TableManagement::redefineRelatedItemHash(QStandardItem * item_key)//?
{
	if (item_key->text() == tr("周期测量数据"))
	{}
	else if (item_key->text() == tr("组均值"))
	{}
	else if (item_key->text() == tr("均值组状态"))
	{}
	else if (item_key->text() == tr("组离散值"))
	{}
	else if (item_key->text() == tr("离散值组状态"))
	{}
	else if (item_key->text() == tr("组极差值"))
	{}
	else if (item_key->text() == tr("极差值组状态"))
	{}
	else if (item_key->text() == tr("测量者"))
	{}
}

void TableManagement::redefineDatesListForNewSelection(const QString & base_time, const QStringList & all_time, bool type, QStringList & def_time)
{
	QString edit_date = base_time;
	if (type)
	{
		QStringList edit_list = edit_date.split("-");
		edit_date = edit_list[0];
	}
	else
	{
		QStringList edit_list = edit_date.split("-");
		edit_date = edit_list[0]+"-"+edit_list[1];
	}
	def_time = all_time.filter(edit_date);
	int i = 0;
	foreach (QString str, def_time)
	{
		QStringList next_str = str.split("T");
		def_time[i] = next_str[0];
		i++;
	}	
}

void TableManagement::redefineStrlistByCtrlItem(QStandardItem * ctrl_item, QStringList & strlist)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QList<QStandardItem *> parents;
	t_model -> findItemsSingleLine(ctrl_item, parents, tr("日常数据"));
	QStandardItem * type_item = 0;
	int i = 0;
	while (parents.at(0)->child(i))
	{
		if (parents.contains(parents.at(0)->child(i)))
		{
			type_item = parents.at(0)->child(i);
			break;
		}
		i++;
	}	
	QString ref_str = type_item->text();
	QString first_str(tr("不继续选择"));
	strlist << first_str;
	if (ctrl_item->text() == tr("继续缩小选择范围"))
	{
		if (ctrl_item->parent()->text().contains(tr("时间到：")))
			strlist << tr("按组序继续选择") << tr("按人员继续选择");			
		else if (ctrl_item->parent()->text().contains(tr("组到：")))	
			strlist << tr("按时间继续选择") << tr("按人员继续选择");
		else
			strlist << tr("按时间继续选择") << tr("按组序继续选择");		
	}
	else
	{
		if (ctrl_item->parent()->text().contains(tr("时间到：")))
		{
			if (ref_str.contains(tr("按组序选择")))
				strlist << QString(tr("最终选择人员"));
			else
				strlist << QString(tr("最终组范围"));				
		}
		else if (ctrl_item->parent()->text().contains(tr("组到：")))	
		{
			if (ref_str.contains(tr("按时间选择")))
				strlist << QString(tr("最终选择人员"));
			else
				strlist << QString(tr("最终时间范围"));
		}
		else
		{
			if (ref_str.contains(tr("按组序选择")))
				strlist << QString(tr("最终时间范围"));
			else
				strlist << QString(tr("最终组范围"));
		}
	}
}

void TableManagement::defineNextSelectString(QStandardItem * ctrl_item, QString & next_str)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(ctrl_item->model());
	QList<QStandardItem *> parents;
	QString hint_str(tr("继续缩小选择范围"));
	t_model -> findItemsSingleLine(ctrl_item, parents, hint_str);
	if (parents.at(0)->text().contains(hint_str))
		next_str = tr("再次缩小选择范围");
	else
		next_str = tr("继续缩小选择范围");
}

void TableManagement::actionForManualDataesShow(QStandardItem * manual_item, const QString & t_name, QSqlTableModel * db_model)
{
	if (manual_item->hasChildren())
		return;
	edittingEvironmentTrans(showingItemFind(), manual_item);
	if (t_name.contains("manualtable8") || t_name.contains("plots"))
	{
		QString items_infoes = db_model->data(db_model->index(0, 1)).toString();
		QString texts_infoes = db_model->data(db_model->index(0, 2)).toString();
		QString names_infoes = db_model->data(db_model->index(0, 3)).toString();
		QHash<QString, QStringList> strs_hash;
		QHash<QString, QStringList> names_hash;
		mBackStringsTransFromDB(texts_infoes, names_infoes, strs_hash, names_hash);
		mBackItemsTransFromDB(manual_item, items_infoes, strs_hash, names_hash);			
	}
}

void TableManagement::mBackStringsTransFromDB(const QString & hash, const QString & names, QHash<QString, QStringList> & db_hash, QHash<QString, QStringList> & db_names)
{
 	QStringList hash_list = hash.split(tr("；"));
	foreach (QString str, hash_list)
	{
		if (str.isEmpty())
			break;
		QStringList every_hash = str.split(tr("，"));
		foreach (QString h_str, every_hash)
		{
			if (h_str.isEmpty())
				break;			
			QStringList every_strs = h_str.split(";");
			QStringList detail_list = every_strs[1].split("%");
			db_hash.insert(every_strs[0], detail_list);
		}
	}
	QStringList trans_names = names.split(tr("；"));	
	foreach (QString str, trans_names)
	{
		if (str.isEmpty())
			break;
		QStringList every_hash = str.split(tr("，"));
		foreach (QString h_str, every_hash)
		{
			if (h_str.isEmpty())
				break;			
			QStringList every_strs = h_str.split(";");
			QStringList detail_list = every_strs[1].split(",");
			db_names.insert(every_strs[0], detail_list);
		}
	} 
}

void TableManagement::treeProjsFresh(QStandardItem * new_item, QStandardItem * old_item)
{	
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QList<QStandardItem *> take_list = db_items.value(old_item);
	QList<QStandardItem *> append_list = db_items.value(new_item);	
	foreach (QStandardItem * o_item, take_list)
	{
		QStandardItem * editted_item = lookingForMatchItem(o_item, tr("按时间"));
		int i = 0;
		while (editted_item->child(i))
		{
			o_item -> appendRow(editted_item->takeRow(editted_item->child(i)->row()));
			i++;
		}
	}
	QList<QStandardItem *> test_unempty;
	t_model -> findAllChildItems(t_model->item(0), test_unempty);
	foreach (QStandardItem * t_item, test_unempty)
	{
		if (t_item->parent() && (t_item->parent()->text()==tr("均值图") || t_item->parent()->text()==tr("西格玛图") || t_item->parent()->text()==tr("直方图") || t_item->parent()->text()==tr("正态概率纸")))
		{			
			QString title = t_item->parent()->parent()->parent()->child(0)->child(0)->text()+tr("，")+t_item->parent()->parent()->text()+tr("，，～")+t_item->parent()->text();
			QStandardItem * add_to = new QStandardItem(title);
			add_to -> appendRow(t_item->parent()->takeRow(t_item->row()));
			db_items[old_item] << add_to;
		}
	}
	foreach (QStandardItem * n_item, append_list)
	{
		QStandardItem * editting_item = lookingForMatchItem(n_item, tr("按时间"));		
		int i = 0;
		while (n_item->child(i))
		{
			editting_item -> appendRow(n_item->takeRow(i));
			i++;
		}		
	}	
}

void TableManagement::createPartnerForClickedItem(QStandardItem * c_item, QSqlTableModel * sql_model, const QString & db_name)
{
	LittleWidgetsView * new_leftcon = new LittleWidgetsView(qobject_cast<QWidget *>(partner->parent()));
	new_leftcon -> setTblManager(this);
	QRect o_rect = partner->initRect();
	new_leftcon -> initMarginForParent(o_rect);
	QStringList name_list = db_name.split(tr("，，。"));
	QString add_name;
	if (t_type == 2)
		add_name = "backuptree";
	setPartnerListForQmlTrans(new_leftcon);
	connect(new_leftcon, SIGNAL(sendNewFrozenState(bool)), this, SLOT(syncPartnersFrozenState(bool)));	
	if (name_list[1].contains("manualtable"))
	{
		if (!add_name.isEmpty())
			add_name += "l_container";
		else
			add_name = "l_container";
		new_leftcon -> setObjectName(add_name);
		new_leftcon -> setMainQmlFile("spc_qml/tablegroupviewer.qml");
		TablesGroup * manual_table = new TablesGroup;
		tools_bar -> setEditingTable(manual_table);		
		manual_table -> setObjectName("manualtable");
		manual_table -> setInspectors(back_calculator, source);
		QString all_contents = sql_model->data(sql_model->index(0, 0)).toString();
		QString all_rcs = sql_model->data(sql_model->index(0, 1)).toString();
		QString all_frames = sql_model->data(sql_model->index(0, 2)).toString();
		manual_table -> initModelViewFromDB(this, all_rcs, all_contents);
		manual_table -> initFrameHashFromDB(all_frames);
		new_leftcon -> setViewForRelatedTable(manual_table);	
	}
	else
	{
		if (!add_name.isEmpty())
			add_name += "p_container";
		else
			add_name = "p_container";		
		new_leftcon -> setObjectName(add_name);
		tools_bar -> setPlotsContainerPt(new_leftcon);		
		new_leftcon -> setMainQmlFile("spc_qml/plotsviewer.qml");
		new_leftcon -> setManualNameFromDB(db_name);
		QString m_matrix = sql_model->data(sql_model->index(0, 0)).toString();
		QStringList matrix_infoes = m_matrix.split(tr("；"));
		QStringList rc_infoes = matrix_infoes[0].split(",");
		QStringList tail_infoes = matrix_infoes[1].split(";");
		QStringList size_infoes = tail_infoes[0].split(",");
		QRect rect(0, 0, size_infoes[0].toInt(), size_infoes[1].toInt());
		new_leftcon -> initViewMatrix(rc_infoes[0].toInt()+1, rc_infoes[1].toInt()+1, rect);
		new_leftcon->qmlAgent()->resetMatrixForSavePlots(this, sql_model, partner, back_calculator, source);		  
	}
	QList<int> m_keys = bk_viewers.keys();
	QPair<QStandardItem *, LittleWidgetsView *> i_pair(c_item, new_leftcon);
	bk_viewers.insert(m_keys.back()+1, i_pair);
	if (t_type != 2)
	{
		QStringList r_list = qobject_cast<QStandardItemModel *>(model())->item(0)->text().split("\n");
		QString new_text = r_list[0]+"\n"+tr("（编辑")+c_item->text()+tr("中...）");
		qobject_cast<QStandardItemModel *>(model())->item(0) -> setText(new_text);				
	}
	emit showSavedManualPlots(this, new_leftcon, true);
}

void TableManagement::setPartnerListForQmlTrans(LittleWidgetsView * new_partner)
{
 	new_partner -> resetQmlFrozenState(partner->frozenSate()); 
	partner = new_partner;
	if (t_type>2 && t_type<6)
	{
		connect(this, SIGNAL(plotFeildToBeChanged(int, QStandardItem *)), partner, SIGNAL(changingPlot(int, QStandardItem *)));
		connect(partner, SIGNAL(dealSigFromAgentHidePlot(PlotWidget *)), this, SLOT(sendsigOrderFromPartner(PlotWidget *)));		
	}
}

void TableManagement::edittingEvironmentTrans(QStandardItem * transed_item, QStandardItem * transing_item)
{
	bk_names.insert(transed_item, names_list);
	bk_texts.insert(transed_item, texts_hash);
	if (bk_names.contains(transing_item))
		names_list = bk_names.value(transing_item);
	else
		names_list.clear();
	if (bk_texts.contains(transing_item))
		texts_hash = bk_texts.value(transing_item);
	else
		texts_hash.clear();
}

void TableManagement::manualDBnameByItem(QStandardItem * key_item, QString & dest_name)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QHashIterator<QPair<QStandardItem *, QString>, QSqlTableModel *> m_hash(t_model->listForManualTableNames());
	while (m_hash.hasNext())
	{
		m_hash.next();
		if (m_hash.key().first == key_item)
		{
			dest_name = m_hash.key().second;
			break;
		}
	}
}

void TableManagement::recalSizeForOpeningNestedViewer(QStandardItem * pos_item, const QRect & n_rect)
{
 	QRect pos_rect = visualRect(pos_item->index());
	if (n_rect.width()+pos_rect.left()>width() || n_rect.height()+pos_rect.top()>height())
	{
		if (n_rect.width()+pos_rect.left() > width())
			setFixedWidth(n_rect.width()+pos_rect.left());
		if (n_rect.height()+pos_rect.top() > height())
			setFixedHeight(n_rect.height()+pos_rect.top());		
		emit sizeChanged();
	}
}

void TableManagement::openNestedWidgetInBranch(EditLongToolsViewer * nesting, int end_width, int end_height)
{
	nested_animation = new QPropertyAnimation(this);
	connect(nested_animation, SIGNAL(valueChanged(const QVariant &)), nesting, SLOT(rectangleAnimating(const QVariant &)));
	connect(nested_animation, SIGNAL(finished()), this, SLOT(clearNestedAnimation()));
	nested_animation -> setTargetObject(nesting);
	nested_animation -> setPropertyName("geometry");
	nested_animation->setDuration(500);
	QRect nested_startRect(QRect(nesting->frameGeometry().topLeft().x(), nesting->frameGeometry().topLeft().y(), 0, 0));
	QRect nested_endRect(QRect(nesting->frameGeometry().topLeft().x(), nesting->frameGeometry().topLeft().y(), end_width, end_height));
	nested_animation->setStartValue(nested_startRect);
	nested_animation->setEndValue(nested_endRect);
	recalSizeForOpeningNestedViewer(nesting->currentNesttingItem(), nested_endRect);
	nested_animation -> start();
	nesting -> showExpanded();
}

void TableManagement::closeNestedWidgetInBranch(EditLongToolsViewer * nested)
{
	nested_animation = new QPropertyAnimation(this);
	connect(nested_animation, SIGNAL(valueChanged(const QVariant &)), nested, SLOT(rectangleAnimating(const QVariant &)));
	connect(nested_animation, SIGNAL(finished()), nested, SLOT(killSelf()));
	connect(nested_animation, SIGNAL(finished()), this, SLOT(clearNestedAnimation()));
	nested_animation -> setTargetObject(nested);
	nested_animation -> setPropertyName("geometry");
	nested_animation->setDuration(500);
	QRect nested_startRect(QRect(nested->frameGeometry().topLeft().x(), nested->frameGeometry().topLeft().y(), nested->width(), 100));
	QRect nested_endRect(QRect(nested->frameGeometry().topLeft().x(), nested->frameGeometry().topLeft().y(), 0, 0));
	nested_animation->setStartValue(nested_startRect);
	nested_animation->setEndValue(nested_endRect);
	nested_animation -> start();	
}

bool TableManagement::plotExistedInViewer(const QModelIndex & check_index)//?
{
	QStandardItem * item = qobject_cast<DataSelectModel *>(model())->itemFromIndex(check_index);
	int i = 0;
	while (item->child(i))
	{
		if (item->child(i)->text().contains(tr("视图位置：")))
			return true;
		i++;
	}
	return false;	
}

bool TableManagement::curIndexMatchForMoveToViewer(const QModelIndex & match_index)//?
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(model());
	QStandardItem * item = t_model->itemFromIndex(match_index);
	if (item->parent()->text()!=tr("均值图") && item->parent()->text()!=tr("西格玛图") && item->parent()->text()!=tr("极差图") && item->parent()->text()!=tr("正态概率图") && item->parent()->text()!=tr("直方图"))
		return false;
	int i = 0;
	while (item->child(i))
	{
		if (item->child(i)->text().contains(tr("时间从：")) || item->child(i)->text().contains(tr("点从：")) || item->child(i)->text().contains(tr("当前人员：")))
			return true;
		i++;
	}
	return false;	
}

bool TableManagement::compareShowingClickingItem(QStandardItem * key_item)
{
	bool equal = false;
	QList<QPair<QStandardItem *, LittleWidgetsView *> > judge_pairs = bk_viewers.values();
	for (int i = 0; i < judge_pairs.size(); i++)
	{
		if (judge_pairs.at(i).first==key_item && judge_pairs.at(i).second==partner)
			return true;			
	}
	return equal;
}

bool TableManagement::newDbGenerationForItem(QStandardItem * bk_item, const QDateTime & db_time)
{
	QString old_db = source->currentConnectionDb()->connectionName();
	QString new_name("/home/dapang/workstation/spc-tablet/qsqlitebkdb/"+bk_item->text());			
	*source->currentConnectionDb() = QSqlDatabase::addDatabase("QSQLITE", bk_item->text());
	source->currentConnectionDb()->setDatabaseName(new_name);		
	if (!source->currentConnectionDb()->open()) 
	{
		*source->currentConnectionDb() = QSqlDatabase::database(old_db);
		return false;
	}
	if (!source->createParasTable(*source->currentConnectionDb()))
	{
		source->currentConnectionDb()->database(bk_item->text()).close();
		*source->currentConnectionDb() = QSqlDatabase::database(old_db);
		QSqlDatabase::removeDatabase(bk_item->text());
		return false;
	}
	DataSelectModel * tree_model = qobject_cast<DataSelectModel *>(model());
	QString t_str = db_time.toString();
	QPair<QString, QString> db_constructor = tree_model->savedUserName();
	QString constructor_merge = db_constructor.first+","+db_constructor.second;
	QString bk_from = source->currentConnectionDb()->connectionName();
	QString name_from("name="+bk_from);
	QString from_time = source->dataFromTable("dbInformations", "constructtime", name_from).toString();
	QStringList fill_total;
	fill_total << bk_item->text() << constructor_merge << t_str << bk_from+from_time << "";
	if (!source->initFillForDbProperty(fill_total, source->currentConnectionDb()))
	{
		source->currentConnectionDb()->database(bk_item->text()).close();
		*source->currentConnectionDb() = QSqlDatabase::database(old_db);
		QSqlDatabase::removeDatabase(bk_item->text());			
		return false;
	}
	QStandardItem * real_item = db_items.value(bk_item->child(0)).at(0);		
	if (real_item->parent() == tree_model->item(0))
	{
		QString add_engi;
		*source->currentConnectionDb() = QSqlDatabase::database(old_db);
		if (!source->replaceProjBetweenDbsAction(real_item->text(), bk_item->text(), add_engi))
		{
			source->currentConnectionDb()->database(bk_item->text()).close();
			*source->currentConnectionDb() = QSqlDatabase::database(old_db);
			QSqlDatabase::removeDatabase(bk_item->text());
			return false;
		}
		*source->currentConnectionDb() = QSqlDatabase::database(bk_item->text());
	}
	else
	{
		QList<QPair<QStandardItem *, QString> > name_list = tree_model->listForManualTableNames().keys();
		QString tbl_name;
		for (int i = 0; i < name_list.size(); i++)
		{
			if (name_list.at(i).first == real_item)
			{
				tbl_name = name_list.at(i).second;
				break;
			}
		}
		if (!source->storeWholeTableFromOutside(old_db, tbl_name, tbl_name))
		{
			source->currentConnectionDb()->database(bk_item->text()).close();
			*source->currentConnectionDb() = QSqlDatabase::database(old_db);
			QSqlDatabase::removeDatabase(bk_item->text());
			return false;
		}		
	}
	source->currentConnectionDb()->database(bk_item->text()).close();
	*source->currentConnectionDb() = QSqlDatabase::database(old_db);
	QSqlDatabase::removeDatabase(bk_item->text());
	return true;
}

int TableManagement::maxCurrentWidth()
{
 	DataSelectModel * show_model = qobject_cast<DataSelectModel *>(model());
	QList<QStandardItem *> all_items;
	for (int i = 0; i < show_model->rowCount(); i++)
	{
		QList<QStandardItem *> root_children;
		show_model -> findAllChildItems(show_model->item(i), root_children);
		root_children.push_front(show_model->item(i));
		all_items += root_children;
	}
	int w_width = 0;
	foreach (QStandardItem * e_item, all_items)
	{
		if (isExpanded(e_item->index()))
		{
			resizeColumnToContents(e_item->column());
			int c_pos = columnViewportPosition(e_item->column())+columnWidth(e_item->column());
			if (w_width < c_pos)
				w_width = c_pos;
		}
	}
	return w_width; 
}

QStandardItem * TableManagement::showingItemFind()
{
	QList<QPair<QStandardItem *, LittleWidgetsView *> > judge_pairs = bk_viewers.values();
	for (int i = 0; i < judge_pairs.size(); i++)
	{
		if (judge_pairs.at(i).second == partner)
			return judge_pairs.at(i).first;
	}
	return 0;
}

QStandardItem * TableManagement::existedStrOnItemlist(const QString & chk_str, QList<QStandardItem *> & on_list)
{
	for (int i = on_list.size()-1; i > 0; i--)
	{
		if (on_list.at(i)->text()==chk_str || on_list.at(i)->text().contains(chk_str))
			return on_list.at(i);
	}
	return 0;
}

LittleWidgetsView * TableManagement::containsPartnerGuideItem(QStandardItem * key_item)
{
	QList<QPair<QStandardItem *, LittleWidgetsView *> > judge_pairs = bk_viewers.values();
	for (int i = 0; i < judge_pairs.size(); i++)
	{
		if (judge_pairs.at(i).first==key_item)
			return judge_pairs.at(i).second;
	}
	return 0; 
}

QModelIndex TableManagement::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(cursorAction);
	Q_UNUSED(modifiers);
	return currentIndex();
}