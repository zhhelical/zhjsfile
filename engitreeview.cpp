#include <QtGui>
#include <QtSql/QSqlTableModel>
#include "engitreeview.h"
#include "nestedmodel.h"
#include "dataestableview.h"
#include "spcdatabase.h"
#include "spcnum.h"

EngiTreeView::EngiTreeView(QWidget * parent)
:QTreeView(parent), open_selection(0), bk_model(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setAttribute(Qt::WA_TranslucentBackground);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setStyleSheet(QString("font-family: %1; font-size:18px; border: 0px").arg(tr("宋体")));	//must here else segment
	setHeaderHidden(true); 
	setAnimated(true);
      	connect(this, SIGNAL(clicked (const QModelIndex &)), this, SLOT(resizeRow(const QModelIndex &)));
	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(relatedEngiClickedInFileView(const QModelIndex &)));	
      	connect(this, SIGNAL(expanded (const QModelIndex &)), this, SLOT(recalculateSize(const QModelIndex &))); 
      	connect(this, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(recalculateSize(const QModelIndex &))); 		
}

EngiTreeView::~EngiTreeView()
{
	foreach (QStandardItem * d_item, del_items)
	{
		if (!d_item->parent())
			delete d_item;
	}
}

void EngiTreeView::setInspector(SpcDataBase * val)
{
	base_db = val;
}

void EngiTreeView::initTreeInformation(QPair<QString, QString> & manager, int type)
{
	user_info = manager;
	tree_type = type;
	QStringList projs;
	if (base_db->isDatabaseConstructor(manager))
		projs = base_db->curDBprojectTable().keys();
	else
		base_db -> managerOwnedProjs(manager, projs);
	ownerEngiesAttributions(projs);
}

void EngiTreeView::setInitViewRect(const QRect & rect)
{
	orig_rect = rect;
}

void EngiTreeView::columnWidthMatchText()
{

}

void EngiTreeView::findIndexParents(const QModelIndex & index, QList<QStandardItem *> & parents)
{
	QModelIndex loop_index = index;
	QStandardItemModel * tree_model = qobject_cast<QStandardItemModel *>(model());
	while(loop_index.isValid())
	{
		if (!tree_model->itemFromIndex(loop_index)->parent())
			break;
		parents.push_front(tree_model->itemFromIndex(loop_index));		
		loop_index = tree_model->parent(loop_index);
	}
}

void EngiTreeView::findAllChildItems(QStandardItem * parent_item, QList<QStandardItem *> & child_items)
{
	int i = 0;
	while (parent_item->child(i))
	{
		child_items << parent_item->child(i);
		findAllChildItems(parent_item->child(i), child_items);
		i++;
	}
}

void EngiTreeView::findChildrenNotAll(QStandardItem * parent_item, QList<QStandardItem *> & sibling_items)
{	
	int i = 0;
	while (parent_item->child(i))
	{
		sibling_items << parent_item->child(i);
		i++;
	}		
}

void EngiTreeView::addItemForNewTest(QStandardItem * engi_item, DataesTableView * dest_tbl)
{
	QList<QStandardItem *> engi_children;
	findChildrenNotAll(engi_item, engi_children);
	int back_num = 0;
	if (engi_children.size() > 1)
	{
		if (engi_children.back()->text().contains(tr("新增版本")))
		{
			QStringList find_num = engi_children.back()->text().split(tr("新增版本"));
			back_num = find_num[1].toInt();
		}
	}
	QString lbl_str(tr("新增版本"));
	lbl_str += QString("%1").arg(back_num+1);
	QStandardItem * add_item = new QStandardItem(lbl_str);
	QStandardItem * def_vars = new QStandardItem(tr("设定参数"));
	QStandardItem * ctime_item = new QStandardItem(tr("设定时间"));
	ctime_item -> appendRow(new QStandardItem(QDateTime::currentDateTime().toString()));
	def_vars -> appendRow(ctime_item);	
	QStandardItem * unit_item = new QStandardItem(tr("测量单位"));
	unit_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(1, 0)).toString()));
	def_vars -> appendRow(unit_item);
	QStandardItem * mark_item = new QStandardItem(tr("测量精度"));
	mark_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(2, 0)).toString()));
	def_vars -> appendRow(mark_item);
	QStandardItem * ctrl_item = new QStandardItem(tr("控图选择"));
	ctrl_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(3, 0)).toString()));
	def_vars -> appendRow(ctrl_item);
	QStandardItem * up_item = new QStandardItem(tr("上规范限"));
	up_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(4, 0)).toString()));
	def_vars -> appendRow(up_item);
	QStandardItem * low_item = new QStandardItem(tr("下规范限"));
	low_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(5, 0)).toString()));
	def_vars -> appendRow(low_item);
	QStandardItem * specs_item = new QStandardItem(tr("每组样本"));
	specs_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(6, 0)).toString()));
	def_vars -> appendRow(specs_item);
	QStandardItem * grps_item = new QStandardItem(tr("样本组数"));
	grps_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(7, 0)).toString()));
	def_vars -> appendRow(grps_item);
	QStandardItem * dest_item = new QStandardItem(tr("目标值"));
	dest_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(8, 0)).toString()));
	def_vars -> appendRow(dest_item);
	QStandardItem * grp_item = new QStandardItem(tr("结果集"));	
	add_item -> appendRow(def_vars);	
	add_item -> appendRow(grp_item);
	engi_item -> appendRow(add_item);
	dest_tbl -> tmpSaveProjsModels(engi_item->text(), ctime_item->child(0)->text());
}

void EngiTreeView::addItemForNewSamp(DataesTableView * dest_tbl, DataesTableView * din_tbl, DataesTableView * key_tbl)
{
	QString cur_version = key_tbl->getInfoByModel(qobject_cast<QStandardItemModel *>(key_tbl->model()), tr("时间"));  
	QList<QStandardItem *> test_finds = qobject_cast<QStandardItemModel *>(model())->findItems(cur_version, Qt::MatchRecursive);
	QList<QStandardItem *> test_children;
	QStandardItem * fr_item = test_finds.at(0);
	if (fr_item->parent()->text() != tr("设定时间"))
		fr_item = test_finds.at(1);
	findChildrenNotAll(fr_item->parent()->parent()->parent()->child(1), test_children);
	int back_num = 0;
	if (test_children.size() > 0)
	{
		if (test_children.back()->text().contains(tr("新增样本")))
		{
			QStringList find_num = test_children.back()->text().split(tr("新增样本"));
			back_num = find_num[1].toInt();
		}
	}
	QString lbl_str(tr("新增样本"));
	lbl_str += QString("%1").arg(back_num+1);
	QStandardItem * add_item = new QStandardItem(lbl_str);
	QStandardItem * stime_item = new QStandardItem(tr("测试时间"));
	stime_item -> appendRow(new QStandardItem(QDateTime::currentDateTime().toString()));
	add_item -> appendRow(stime_item);
	QStandardItem * cpk_item = new QStandardItem(tr("CPK结果"));
	cpk_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(2, 0)).toString()));
	add_item -> appendRow(cpk_item);
	QStandardItem * avrup_item = new QStandardItem(tr("均值上限"));
	avrup_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(5, 0)).toString()));
	add_item -> appendRow(avrup_item);
	QStandardItem * avrlow_item = new QStandardItem(tr("均值下限"));
	avrlow_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(6, 0)).toString()));
	add_item -> appendRow(avrlow_item);
	QStandardItem * up_item = new QStandardItem(dest_tbl->model()->headerData(8, Qt::Vertical).toString());
	up_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(8, 0)).toString()));
	add_item -> appendRow(up_item);
	QStandardItem * low_item = new QStandardItem(dest_tbl->model()->headerData(9, Qt::Vertical).toString());
	low_item -> appendRow(new QStandardItem(dest_tbl->model()->data(dest_tbl->model()->index(9, 0)).toString()));
	add_item -> appendRow(low_item);	
	fr_item->parent()->parent()->parent()->child(1)->appendRow(add_item);
	din_tbl -> tmpSaveProjsModels(cur_version, stime_item->child(0)->text()); 
	dest_tbl -> tmpSaveProjsModels(stime_item->child(0)->text(), stime_item->child(0)->text());
}

void EngiTreeView::defaultSettedModelTransaction()
{
 	QStandardItemModel * tmp_model = qobject_cast<QStandardItemModel *>(model());  
	if (!bk_model)
	{
		bk_model = new NestedModel(this);	  
		QSqlTableModel free_model(this, base_db->currentConnectionDb()->database(base_db->currentConnectionDb()->connectionName()));
		base_db -> varsModelFromTable("freeconstruct", &free_model);
		if (free_model.rowCount() == 0)
			return;
		qobject_cast<NestedModel *>(bk_model)->initFreeModel("freeconstruct", base_db);
		copyModelItems(tmp_model, bk_model);
		QString modifier = free_model.data(free_model.index(0, 2)).toString();
		QStandardItem * editor_item = new QStandardItem(tr("最近编辑者"));
		QStandardItem * editor_name = new QStandardItem(base_db->curDBmanageTable().value(modifier).at(0));
		editor_item -> appendRow(editor_name);
		QStandardItem * dpr_item = new QStandardItem(tr("所属部门"));
		dpr_item -> appendRow(new QStandardItem(base_db->curDBmanageTable().value(modifier).at(2)));		
		editor_name -> appendRow(dpr_item);
		QStandardItem * editor_pos = new QStandardItem(tr("职位"));
		editor_pos -> appendRow(new QStandardItem(base_db->curDBmanageTable().value(modifier).at(4)));
		editor_name -> appendRow(editor_pos);
		QStandardItem * editor_time = new QStandardItem(tr("编辑时间"));
		editor_time -> appendRow(new QStandardItem(free_model.data(free_model.index(0, 1)).toDateTime().toString()));
		editor_item -> appendRow(editor_time);
		bk_model -> appendRow(editor_item);
		return;
	} 	
	setModel(bk_model);	
	if (width()>orig_rect.width() || height()>orig_rect.height())
	{
		if (width()>orig_rect.width())
			setFixedWidth(orig_rect.width());
		if (height()>orig_rect.height())
			setFixedHeight(orig_rect.height());	
		emit treeViewSizeChanged();
	}
	bk_model = tmp_model;
}

void EngiTreeView::initModelsForRelatedView(DataesTableView * dest_tbl, DataesTableView * dest_partner)
{
 	if (dest_tbl->tableType() == 4)
		dest_tbl -> initTable(open_selection->text());
	else
	{
		if (dest_tbl->modelHashs().isEmpty())
			dest_tbl -> initTable("", dest_partner);
		dest_tbl -> initTestCpkDataesModels(open_selection);			
	}
}

bool EngiTreeView::saveProjsPromotions(QList<QStandardItem *> & save_items, DataesTableView * din_tbl, DataesTableView * cal_tbl, DataesTableView * key_tbl)
{
	connect(key_tbl, SIGNAL(deleteTreeProjForPromotion(const QString &)), this, SLOT(deleteProjByPromotion(const QString &)));
	foreach (QStandardItem * chk_item, save_items)
	{
		QList<QStandardItem *> chk_list;
		findIndexParents(chk_item->index(), chk_list);
		foreach (QStandardItem * ag_item, chk_list)
		{
			if (ag_item->text().contains(tr("新增")) && ag_item!=chk_item)
			{
				save_items.removeOne(chk_item);
				break;
			}	
		}
	}	
	foreach (QStandardItem * s_item, save_items)
	{
		if (s_item->text().contains(tr("版本")))
		{
			QString key_time = s_item->child(0)->child(0)->child(0)->text();
			QStandardItemModel * k_model = key_tbl->matchTimeKeyModel(key_time);
			key_tbl -> setModel(k_model);
			if (!key_tbl->saveProjKeyDataesCheck(false, key_time))
				return false;
			QList<QStandardItem *> specs_list;
			findChildrenNotAll(s_item->child(1), specs_list);
			foreach (QStandardItem * spec_item, specs_list)
			{
				if (!spec_item->text().contains(tr("新增")))
					specs_list.removeOne(spec_item);
			}
			if (!specs_list.isEmpty())
			{
				foreach (QStandardItem * spec_item, specs_list)
				{
					QString save_cpk;		
					if (key_tbl->model()->data(key_tbl->model()->index(3, 0)).toString().contains(tr("西格玛")))
						save_cpk = k_model->data(k_model->index(0, 0)).toString()+tr("，，。")+"cpkdataes"+tr("，，。")+"cpkdev";
					else
						save_cpk = k_model->data(k_model->index(0, 0)).toString()+tr("，，。")+"cpkdataes"+tr("，，。")+"cpkrng";	
					QString din_time = spec_item->child(0)->child(0)->text();
					QStandardItemModel * din_model = din_tbl->matchTimeKeyModel(din_time);
					din_tbl -> setModel(din_model);
					din_tbl -> changeViewStateForNewModel();
					QStandardItemModel * c_model = cal_tbl->matchTimeKeyModel(din_time);
					cal_tbl -> setModel(c_model);
					QString save_times = key_time+";"+din_time;
					if (!base_db->createEngiTables(save_cpk, k_model->data(k_model->index(10, 0)).toString(), *base_db->currentConnectionDb(), save_times))
						return false;					
					if (!base_db->storeSpcDataes(save_cpk, din_tbl, cal_tbl, key_tbl, save_times))
						return false;	  
				}		  
			}
		}		
		if (s_item->text().contains(tr("样本")))
		{
			QString din_time = s_item->child(0)->child(0)->text();
			QStandardItemModel * din_model = din_tbl->matchTimeKeyModel(din_time);
			din_tbl -> setModel(din_model);
			din_tbl -> changeViewStateForNewModel();
			QString k_time = din_tbl->getInfoByModel(din_model, "");
			QStandardItemModel * k_model = key_tbl->matchTimeKeyModel(k_time);
			key_tbl -> setModel(k_model);			
			QString save_cpk;		
			if (key_tbl->model()->data(key_tbl->model()->index(3, 0)).toString().contains(tr("西格玛")))
				save_cpk = k_model->data(k_model->index(0, 0)).toString()+tr("，，。")+"cpkdataes"+tr("，，。")+"cpkdev";
			else
				save_cpk = k_model->data(k_model->index(0, 0)).toString()+tr("，，。")+"cpkdataes"+tr("，，。")+"cpkrng";	
			QStandardItemModel * c_model = cal_tbl->matchTimeKeyModel(din_time);
			cal_tbl -> setModel(c_model);
			QString save_times = k_time+";"+din_time;
			if (!base_db->createEngiTables(save_cpk, k_model->data(k_model->index(10, 0)).toString(), *base_db->currentConnectionDb(), save_times))
				return false;	
			if (!base_db->storeSpcDataes(save_cpk, din_tbl, cal_tbl, key_tbl, save_times))
				return false;		  
		}
	}
	if (!din_tbl->chkAndDeleteEngiDataesInDb())
		return false;	
	if (!key_tbl->chkAndDeleteEngiDataesInDb())
		return false;
	return true;
}

QStandardItem * EngiTreeView::currentEngiItem()
{
	if (!model()->rowCount())
		return 0;
	if (!open_selection)
		open_selection = qobject_cast<QStandardItemModel *>(model())->item(0)->child(0);
	return open_selection;
}

const QRect & EngiTreeView::originShowRect()
{
 	return orig_rect; 
}

QList<QStandardItem *> & EngiTreeView::deletionItemsList()
{
	return del_items;
}

const QHash<QStandardItem *, QStandardItemModel *> & EngiTreeView::openningItemsModels()
{
	return open_models;
}

void EngiTreeView::resizeRow(const QModelIndex & index)
{
	resizeColumnToContents(index.row());
}

void EngiTreeView::recalculateSize(const QModelIndex & r_index)
{
	QStandardItemModel * tree_model = qobject_cast<QStandardItemModel *>(model());
	QStandardItem * last_item = tree_model->item(tree_model->rowCount()-1);
	QRect pos_rect;	
	QList<QStandardItem *> children_items;
	findAllChildItems(last_item, children_items);
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
	if (isExpanded(r_index))
	{
		if (end_geometry>height() && end_geometry>orig_rect.height())			
			h_chging = true;
	}
	else
	{
		if (end_geometry < orig_rect.height())
		{
			end_geometry = orig_rect.height();
			h_chging = true;
		}
	}
	if (w_chging || h_chging)
	{
		if (w_chging)
			setFixedWidth(max_right);
		if (h_chging)
			setFixedHeight(end_geometry);
		emit treeViewSizeChanged();
	}
}

void EngiTreeView::relatedEngiClickedInFileView(const QModelIndex & index)
{
	QStandardItem * clicked_item = qobject_cast<QStandardItemModel *>(model())->itemFromIndex(index);
	if (!clicked_item->parent() && !open_selection)
		open_selection = qobject_cast<QStandardItemModel *>(model())->item(0)->child(0);
	else
	{
		QList<QStandardItem *> e_list;
		findIndexParents(index, e_list);
		open_selection = e_list.at(0);
	}
}

void EngiTreeView::deleteProjByPromotion(const QString & del_proj)
{
	QList<QStandardItem *> proj_items = qobject_cast<QStandardItemModel *>(model())->findItems(del_proj, Qt::MatchRecursive | Qt::MatchContains);
	model() -> removeRow(proj_items.at(0)->row());
	emit treeViewSizeChanged();
}

void EngiTreeView::ownerEngiesAttributions(QStringList & projs)
{
	QHash<QString, QStringList> next_hash;
	foreach (QString f_str, projs)
	{
		if (!base_db->testEngiPerformance(f_str))
		{
			if (tree_type == 1)
				next_hash[tr("未完善工程")] << f_str;		
			continue;
		}
		QString engi("name="+f_str);		
		QString on_off = base_db->dataFromTable("projectsinfo", "attribution", engi).toString();
		if (on_off.contains(tr("下线")))
			next_hash[tr("下线工程")] << f_str;
		else
			next_hash[tr("在线工程")] << f_str;
	}
	multiRootsTree(next_hash);	 
}
	
void EngiTreeView::multiRootsTree(QHash<QString, QStringList> & roots)
{
	QStandardItemModel * roots_model = new QStandardItemModel(this);
	QHashIterator<QString, QStringList> i_hash(roots);
	QList<QStandardItem *> pos_items;
	pos_items << 0 << 0 << 0;
	while (i_hash.hasNext())
	{
		i_hash.next();		
		if (i_hash.key() == tr("未完善工程"))
		{
			QStandardItem * nofinished_root = new QStandardItem(tr("未完善工程")); 
			appendEngiesAttribution(i_hash.value(), nofinished_root);
			pos_items[0] = nofinished_root;
		}
		if (i_hash.key() == tr("在线工程"))
		{
			QStandardItem * online_root = new QStandardItem(tr("在线工程"));
			appendEngiesAttribution(i_hash.value(), online_root);
			pos_items[1] = online_root;			
		}
		if (i_hash.key() == tr("下线工程"))
		{
			QStandardItem * offline_root = new QStandardItem(tr("下线工程"));
			appendEngiesAttribution(i_hash.value(), offline_root);
			pos_items[2] = offline_root;
		}
	}
	int set_pos = 0;
	foreach (QStandardItem * m_item, pos_items)
	{
		if (m_item)
		{			 
			roots_model -> setItem(set_pos, m_item);	
			set_pos++;
		}
	}
	open_models[roots_model->item(0)] = roots_model;
	setModel(roots_model);
}

void EngiTreeView::appendEngiesAttribution(const QStringList & projs, QStandardItem * root_item)
{
	if (projs.isEmpty())
		return;
	QStringList all_tbls = base_db->allTables();		
	foreach (QString e_proj, projs)
	{
		QStandardItem * proj_name = new QStandardItem(e_proj);
		QString name_row("name="+e_proj);	
		QStandardItem * time_branch = new QStandardItem(tr("创建时间"));
		QString ep_ctime = QDateTime::fromString(base_db->dataFromTable("projectskeys", "constructtime", name_row).toString(), Qt::ISODate).toString();
		QStandardItem * ep_time = new QStandardItem(ep_ctime);
		time_branch -> appendRow(ep_time);	
		proj_name -> appendRow(time_branch);
		QString proj_times = base_db->dataFromTable("projectskeys", "unit", name_row).toString();
		QStringList proj_versions = proj_times.split(tr("；"));	
		int finished = 1;	
		int unfinished = 1;		
		foreach (QString fver_str, proj_versions)
		{
			QStringList def_construction = fver_str.split(tr("，"));
			bool found = base_db->testEngiPerformance(e_proj, false, def_construction[0]);			
			if (tree_type == 0)
			{
				if (root_item->text() == tr("在线工程") && found)
				{	
					QStandardItem * version = new QStandardItem(tr("版本")+QString("%1").arg(finished));
					QStandardItem * proj_paraes = new QStandardItem(tr("设定参数"));
					QStandardItem * proj_time = new QStandardItem(tr("设定时间"));
					proj_time -> appendRow(new QStandardItem(def_construction[0]));
					proj_paraes -> appendRow(proj_time);
					QStringList ep_paraes;
					base_db -> getDataesFromTable("projectskeys", "name", ep_paraes, e_proj);
					keyDataesTreeSet(proj_paraes, def_construction[0], ep_paraes);
					version -> appendRow(proj_paraes);				  
					QStandardItem * cpk_paraes = new QStandardItem(tr("结果集"));
					samplesDataesTreeSet(cpk_paraes, def_construction[0], all_tbls);
					version -> appendRow(cpk_paraes);
					proj_name -> appendRow(version);
					finished++;
				}
			}
			else
			{
				if (proj_times.isEmpty())
					continue;			  
				QStandardItem * version = new QStandardItem;
				proj_name -> appendRow(version);			
				QStandardItem * proj_paraes = new QStandardItem(tr("设定参数"));
				QStandardItem * proj_time = new QStandardItem(tr("设定时间"));
				proj_time -> appendRow(new QStandardItem(def_construction[0]));
				proj_paraes -> appendRow(proj_time);				
				QStringList ep_paraes;
				base_db -> getDataesFromTable("projectskeys", "name", ep_paraes, e_proj);
				keyDataesTreeSet(proj_paraes, def_construction[0], ep_paraes);
				version -> appendRow(proj_paraes);								
				if (found)
				{
					version -> setText(tr("完成版本")+QString("%1").arg(finished));	
					QStandardItem * cpk_paraes = new QStandardItem(tr("结果集"));	
					samplesDataesTreeSet(cpk_paraes, def_construction[0], all_tbls);
					version -> appendRow(cpk_paraes);
					finished++;
				}
				else
				{
					version -> setText(tr("未完成版本")+QString("%1").arg(unfinished));
					QStandardItem * cpk_paraes = new QStandardItem(tr("结果集"));	
					version -> appendRow(cpk_paraes);
					unfinished++;
				}	
			}			
		}
		if (tree_type == 1)
		{
			QStandardItem * first_item = proj_name->takeRow(0).at(0);
			sortItemsList(proj_name);
			proj_name -> insertRow(0, first_item);
		}
		root_item -> appendRow(proj_name);
	}	
}

void EngiTreeView::keyDataesTreeSet(QStandardItem * key_root, QString & proj, const QStringList paraes)
{
	QStringList proj_paraes;
	foreach (QString p_paraes, paraes)
	{	  
		if (paraes.indexOf(p_paraes) == 0)
			continue;
		if (paraes.indexOf(p_paraes) == 9)
			break;
		QStringList proj_versions = p_paraes.split(tr("；"));
		QStringList real_version = proj_versions.filter(proj);	
		QStringList real_para = real_version.at(0).split(tr("，"));
		proj_paraes << real_para.at(1);
	}
	QStandardItem * unit_plot = new QStandardItem(tr("测量单位"));
	QStandardItem * unit = new QStandardItem(proj_paraes.at(0));
	unit_plot -> appendRow(unit);
	key_root -> appendRow(unit_plot);
	QStandardItem * precision_plot = new QStandardItem(tr("测量精度"));
	QStandardItem * precision = new QStandardItem(proj_paraes.at(1));
	precision_plot -> appendRow(precision);
	key_root -> appendRow(precision_plot);	
	QStandardItem * ctrl_plot = new QStandardItem(tr("控制图"));
	QStandardItem * ctrl = new QStandardItem(proj_paraes.at(2));
	ctrl_plot -> appendRow(ctrl);
	key_root -> appendRow(ctrl_plot);
	QStandardItem * dest_up = new QStandardItem(tr("目标上限"));
	QStandardItem * up_num = new QStandardItem(proj_paraes.at(3));
	dest_up -> appendRow(up_num);
	key_root -> appendRow(dest_up);
	QStandardItem * dest_low = new QStandardItem(tr("目标下限"));
	QStandardItem * low_num = new QStandardItem(proj_paraes.at(4));
	dest_low -> appendRow(low_num);
	key_root -> appendRow(dest_low);	
	QStandardItem * plot_species = new QStandardItem(tr("每组样本"));
	QStandardItem * species = new QStandardItem(proj_paraes.at(5));
	plot_species -> appendRow(species);
	key_root -> appendRow(plot_species);	
	QStandardItem * plot_grps = new QStandardItem(tr("样本组数"));
	QStandardItem * grps = new QStandardItem(proj_paraes.at(6));
	plot_grps -> appendRow(grps);
	key_root -> appendRow(plot_grps);
	QStandardItem * dest_num = new QStandardItem(tr("目标值"));
	QStandardItem * num = new QStandardItem(proj_paraes.at(7));
	dest_num -> appendRow(num);
	key_root -> appendRow(dest_num);
}

void EngiTreeView::samplesDataesTreeSet(QStandardItem * sams_root, QString & proj, const QStringList tbls)
{
	QString cpk_tbl;
	QString cpk_stamp = QString("%1").arg(QDateTime::fromString(proj).toTime_t());
	QStringList sams_tbls = tbls.filter(cpk_stamp);
	QStringList sams_cpks = sams_tbls.filter(tr("，，。cpkdataes"));
	QList<int> cpks_stamps;
	foreach (QString tbl, sams_cpks)
	{
		QStringList name_list = tbl.split(tr("，，。"));
		QStringList times_list = name_list.at(2).split(tr("，"));
		cpks_stamps << times_list.at(1).toInt();
		sams_tbls.removeOne(tbl);
	}
	int specimen = 1;	
	qSort(cpks_stamps);
	foreach (int tbl_stamp, cpks_stamps)
	{
		QStringList e_cpks = sams_cpks.filter(QString("%1").arg(tbl_stamp));	
		sams_cpks.removeOne(e_cpks.at(0));
		QSqlTableModel dcpk_model(this, base_db->currentConnectionDb()->database(base_db->currentConnectionDb()->connectionName()));
		base_db -> varsModelFromTable(e_cpks.at(0), &dcpk_model);		
		QString dt_real = dcpk_model.data(dcpk_model.index(0, 3)).toDateTime().toString();
		QStringList cpk_gathers = tbls.filter(tr("，，。cpk")).filter(QString("%1").arg(tbl_stamp));
		cpk_gathers.removeOne(e_cpks.at(0));
		QString cpk_gather = cpk_gathers.at(0);
		QSqlTableModel cpk_model(this, base_db->currentConnectionDb()->database(base_db->currentConnectionDb()->connectionName()));
		base_db -> varsModelFromTable(cpk_gather, &cpk_model);		
		QStringList daily_lbl = tbls.filter(tr("，，。dailydataes")).filter(QString("%1").arg(tbl_stamp));		
		QStandardItem * e_spec = new QStandardItem(tr("数据样本")+QString("%1").arg(specimen));
		QStandardItem * tcpk_lbl = new QStandardItem(tr("测试时间"));
		QStandardItem * tcpk_time = new QStandardItem(dt_real);
		tcpk_lbl -> appendRow(tcpk_time);
		e_spec -> appendRow(tcpk_lbl);		
		QString cpk_lbl;
		if (cpk_gather.contains("dev"))
			cpk_lbl = tr("西格玛");
		else
			cpk_lbl = tr("级差");
		QStandardItem * item_cpk = new QStandardItem(cpk_lbl+"CPK");
		QStandardItem * val_cpk = new QStandardItem(cpk_model.data(cpk_model.index(0, 3)).toString());
		item_cpk -> appendRow(val_cpk);
		e_spec -> appendRow(item_cpk);
		QStandardItem * avr_uplbl = new QStandardItem(tr("均值上限"));
		QStandardItem * avr_up = new QStandardItem(cpk_model.data(cpk_model.index(0, 4)).toString());
		avr_uplbl -> appendRow(avr_up);
		e_spec -> appendRow(avr_uplbl);
		QStandardItem * avr_lowlbl = new QStandardItem(tr("均值下限"));
		QStandardItem * avr_low = new QStandardItem(cpk_model.data(cpk_model.index(0, 5)).toString());
		avr_lowlbl -> appendRow(avr_low);
		e_spec -> appendRow(avr_lowlbl);
		QStandardItem * dest_uplbl = new QStandardItem(cpk_lbl+tr("上限"));
		QStandardItem * dest_up = new QStandardItem(cpk_model.data(cpk_model.index(0, 6)).toString());
		dest_uplbl -> appendRow(dest_up);
		e_spec -> appendRow(dest_uplbl);
		QStandardItem * dest_lowlbl = new QStandardItem(cpk_lbl+tr("下限"));
		QStandardItem * dest_low = new QStandardItem(cpk_model.data(cpk_model.index(0, 7)).toString());
		dest_lowlbl -> appendRow(dest_low);
		e_spec -> appendRow(dest_lowlbl);
		QStandardItem * daily_records = new QStandardItem(tr("日常记录"));	
		if (!daily_lbl.isEmpty())
		{
			QString cur_drecords = base_db->lastDataFromTable(daily_lbl.at(0), "groupnum").toString();
			QStandardItem * tol_records = new QStandardItem(tr("当前共记录")+QString("%1").arg(cur_drecords)+tr("次"));
			daily_records -> appendRow(tol_records);
		}
		e_spec -> appendRow(daily_records);
		sams_root -> appendRow(e_spec);
		specimen++;
	}	
}

void EngiTreeView::sortItemsList(QStandardItem * parent_root)
{
	QList<QStandardItem *> sibling_finishs;
	QList<QStandardItem *> sibling_unfinishs;
	while (parent_root->child(0))
	{
		if (!parent_root->child(0)->text().contains(tr("未")))
			sibling_finishs << parent_root->takeRow(0).at(0);
		else
			sibling_unfinishs << parent_root->takeRow(0).at(0);
	}
	SpcNum * calculator = qobject_cast<SpcNum *>(base_db->parent());
	QMap<int, QStandardItem *> sort_map;
	QHash<QStandardItem *, QStandardItem *> dt_hash;
	QList<QStandardItem *> cmp_items;
	foreach (QStandardItem * f_item, sibling_finishs)
	{		
		dt_hash.insert(f_item, f_item->child(0)->child(0)->child(0));
		cmp_items << f_item->child(0)->child(0)->child(0);
	}
	calculator -> sortItemSequenceByTime(cmp_items, sort_map);
	QMapIterator<int, QStandardItem *> i_map(sort_map);
	while (i_map.hasNext())
	{
		i_map.next();
		QStandardItem * k_item = dt_hash.key(i_map.value());
		k_item -> setText(tr("完成版本")+QString("%1").arg(i_map.key()));
		parent_root -> insertRow(i_map.key()-1, k_item);
	}
	dt_hash.clear();
	sort_map.clear();
	cmp_items.clear();
	foreach (QStandardItem * f_item, sibling_unfinishs)
	{
		dt_hash.insert(f_item, f_item->child(0)->child(0)->child(0));
		cmp_items << f_item->child(0)->child(0)->child(0);
	}
	calculator -> sortItemSequenceByTime(cmp_items, sort_map);
	QMapIterator<int, QStandardItem *> ii_map(sort_map);
	while (ii_map.hasNext())
	{
		ii_map.next();
		QStandardItem * k_item = dt_hash.key(ii_map.value());
		k_item -> setText(tr("未完成版本")+QString("%1").arg(ii_map.key()));
		parent_root -> insertRow(ii_map.key()-1, k_item);
	}	
}

void EngiTreeView::copyModelItems(QStandardItemModel * from, QStandardItemModel * to)
{
	QList<QStandardItem *> engi_froms;
	for (int i = 0; i < from->rowCount(); i++)
	{
		QList<QStandardItem *> root_children;
		findChildrenNotAll(from->item(i), root_children);
		engi_froms += root_children;
	}	
	foreach (QStandardItem * e_item, engi_froms)
	{
		QList<QStandardItem *> engi_toes = to->findItems(e_item->text(), Qt::MatchRecursive);
		QList<QStandardItem *> engi_children;
		findAllChildItems(e_item, engi_children);
		foreach (QStandardItem * t_item, engi_toes)
		{
			QHash<QStandardItem *, QStandardItem *> tmp_hash;
			foreach (QStandardItem * c_item, engi_children)
			{
				QStandardItem * copy_item = new QStandardItem(c_item->text());
				tmp_hash[c_item] = copy_item;
				if (c_item->parent() == e_item)
					t_item -> appendRow(copy_item);
				else
					tmp_hash[c_item->parent()]->appendRow(copy_item);
			}
		}
	}
}

int EngiTreeView::maxCurrentWidth()
{
	QStandardItemModel * show_model = qobject_cast<QStandardItemModel *>(model());
	QList<QStandardItem *> all_items;
	for (int i = 0; i < show_model->rowCount(); i++)
	{
		QList<QStandardItem *> root_children;
		findAllChildItems(show_model->item(i), root_children);
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