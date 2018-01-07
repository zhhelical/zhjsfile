#include <QtGui>
#include <QtCore>
#include <QtSql/QSqlTableModel>
#include "dataselectmodel.h"
#include "tablemanagement.h"
#include "spcdatabase.h"
#include "spcnum.h"

DataSelectModel::DataSelectModel(TableManagement * parent)
:QStandardItemModel(parent)
{
	tree_view = parent;
}

DataSelectModel::~DataSelectModel()
{
	if (!manual_tables.isEmpty())
	{
		QList<QSqlTableModel *> del_list = manual_tables.values();
		foreach (QSqlTableModel * sql, del_list)
			sql -> deleteLater();
	}
}

void DataSelectModel::initModel(const QPair<QString, QString> & ctrl_name, SpcDataBase * val)
{
	using_name = ctrl_name;
	inspector = val;
	int tree_type = tree_view->treeType();
	if (tree_type == 0)
		modelForSpcDataesSelect();
	else if (tree_type == 1)
		modelForSpcInfoesSelect();
	else if (tree_type == 2)
		modelForSpcDbTblBackup();	
	else if (tree_type < 6 && tree_type > 2)
		modelForSpcPlotsSelect(tree_type);
	else
		modelForSpcTotalSelect(tree_type);
}

void DataSelectModel::findItemsSingleLine(QStandardItem * hint_item, QList<QStandardItem *> & i_line, const QString & head_str)
{
	i_line.push_front(hint_item);
	QStandardItem * loop_item = hint_item;
	while(loop_item->parent())
	{
		if (loop_item->text().contains(head_str))
			break;		
		loop_item = loop_item->parent();
		i_line.push_front(loop_item);		
	}
}

void DataSelectModel::findChildrenNotAll(QStandardItem * parent_item, QList<QStandardItem *> & sibling_items)
{
	int i = 0;
	while (parent_item->child(i))
	{
		sibling_items << parent_item->child(i);
		i++;
	}	
}

void DataSelectModel::findAllChildItems(QStandardItem * parent_item, QList<QStandardItem *> & child_items)
{
	int i = 0;
	while (parent_item->child(i))
	{
		child_items << parent_item->child(i);
		findAllChildItems(parent_item->child(i), child_items);
		i++;
	}
}

void DataSelectModel::removeSqltableInhash(const QString & key_str)
{
	QHashIterator<QPair<QStandardItem *, QString>, QSqlTableModel *> m_hash(manual_tables);
	while (m_hash.hasNext())
	{
		m_hash.next();
		if (m_hash.key().second == key_str)
		{
			QSqlTableModel * del_sql = m_hash.value();
			del_sql -> deleteLater();
			manual_tables.remove(m_hash.key());	
			break;
		}
	}	
}

void DataSelectModel::findExistedDbsBackupTo(QStandardItem * select_item, QStringList & dbs)
{	  
	int i_child = 0;
	while (item(3)->child(i_child))
	{
		QStandardItem * p_test = item(3)->child(i_child);
		QList<QStandardItem *> children_list;
		findChildrenNotAll(p_test, children_list);
		bool found = false;
		foreach (QStandardItem * db_item, children_list)
		{
			if (children_list.indexOf(db_item) == 0)
				continue;
			if (db_item->text() == select_item->text())
			{
				found = true;
				break;
			}
		}
		if (!found)
			dbs << item(3)->child(i_child)->text();
		i_child++;
	}	
}

void DataSelectModel::projBackupJudgeMngs(QStandardItem * bk_item, QStandardItem * n_db)
{
	QString proj_name;
	if (bk_item->text().contains(","))
	{
		QStringList details = bk_item->text().split(",");
		proj_name = details.at(0);
	}
	else
		proj_name = bk_item->text();
	QPair<QStandardItem *, QStandardItem *> proj_db(bk_item, n_db);
	QList<QPair<QString, QString> > proj_usrs;
	inspector -> projectManagersIndb(proj_name, proj_usrs);
	QStringList chk_usrs;	
	for (int i = 0; i < proj_usrs.size(); i++)
	{
		QString result;
		if (proj_usrs.at(i).second.isEmpty())// no need to judge waiting mngs because not backup waiting mngs 
			continue;
		inspector -> chkUserInOtherDb(proj_usrs.at(i), n_db->text(), result);
		if (result == tr("密码需要修改"))
			chk_usrs << proj_usrs.at(i).second;
	}
	usr_corruptions.insert(proj_db, chk_usrs);
}

void DataSelectModel::feedSimpleDataFromTbl(const QString & lbl_feed, const QString & feed_source, QStandardItemModel * set_model, QStandardItem * hint_item)
{
	QStringList lbl_list = lbl_feed.split(tr("，，。"));
	QStringList all_tbls = inspector->allTables(); 	
	QStandardItem * time_parent = findTimeParentItem(hint_item);
	if (feed_source == "projectskeys")
	{
		QString var_row("name="+lbl_list.at(0));	  
		QString crude_str = inspector->dataFromTable(feed_source, lbl_list.back(), var_row).toString();
		QDateTime k_time = QDateTime::fromString(key_times.value(time_parent->child(0)), Qt::ISODate);
		QString c_time;
		if (k_time.isValid())
			c_time = k_time.toString();
		else
			c_time = key_times.value(time_parent->child(0));
		QStringList detailed_crude = crude_str.split(tr("；")).filter(c_time);
		QStringList fed_list = detailed_crude.at(0).split(tr("，"));
		if (lbl_list.back() == "constructor")
			set_model -> setData(set_model->index(0, 0), fed_list.at(1));
		else
			set_model -> setData(set_model->index(0, 0), fed_list.back());						
	}
	else
	{
		QString et_tamp;
		if (feed_source == "cpkdataes")
			et_tamp = QString("%1").arg(QDateTime::fromString(key_times.value(time_parent->parent()->parent()->child(0)->child(0))).toTime_t());	
		else
			et_tamp = QString("%1").arg(QDateTime::fromString(key_times.value(time_parent->child(0)), Qt::ISODate).toTime_t());
		QString find_tbl;			
		findExistedRelatedTable(et_tamp, tr("，，。")+feed_source, all_tbls, find_tbl);	
		if (!find_tbl.isEmpty())
		{
			if (lbl_list.back() == "dataes")
			{
				if (hint_item->text() == tr("测量基础数据"))
				{
					find_tbl += tr("，。，")+key_times.value(time_parent->child(0));
					inspector->spcCalculator()->cpkInitModelFromDb(0, find_tbl, set_model);
					set_model -> removeRow(0);
				}
				else
				{}
			}
			else
			{
				QString var_row("groupnum="+QString("%1").arg(1));
				QString feed_str = inspector->dataFromTable(find_tbl, lbl_list.back(), var_row).toString();
				set_model -> setData(set_model->index(0, 0), feed_str);
			}
		}
	}
}

void DataSelectModel::findExistedRelatedTable(const QString & proj_version, const QString & v_name, const QStringList & all_tbls, QString & dest_tbl)
{
	foreach (QString tbl, all_tbls)
	{
		if (tbl.contains(proj_version) && tbl.contains(v_name))
		{
			dest_tbl = tbl;
			break;
		}		
	}
}

QStandardItem * DataSelectModel::findTimeParentItem(QStandardItem * c_item)
{
	QStandardItem * r_item = c_item;
	while (r_item->parent())
	{
		if (r_item->parent()->parent()==item(0) || r_item->text()==tr("设定参数") || r_item->text().contains(tr("数据样本")))
			return r_item;
		r_item = r_item->parent();
	}
	return 0;
}

QStandardItem * DataSelectModel::findChildItem(QStandardItem * parent_item, QStandardItem * info_item)
{
	QStandardItem * parent_info = info_item;
	while (parent_info->parent())
	{
		if (parent_info->parent() == parent_item)
			return parent_info;
		else
			parent_info = parent_info->parent();
	}
	return 0;
}

TableManagement * DataSelectModel::nestedInTreeView()
{
	return tree_view;
}

QString DataSelectModel::findEngiItemText(QStandardItem * info_item)
{
	QList<QStandardItem *> engi_line;
	findItemsSingleLine(info_item, engi_line, item(0)->text());
	QString empty;
	if (tree_view->treeType() == 2)
		return engi_line.at(1)->text();
	else if (tree_view->treeType() != 7)
		return engi_line.at(2)->text();
	return empty;	  
}

const QPair<QString, QString> & DataSelectModel::savedUserName()
{
	return using_name;
}

const QHash<QStandardItem *, QString> & DataSelectModel::timesSavedHash()
{
	return key_times;
}

const QHash<QPair<QStandardItem *, QString>, QSqlTableModel *> & DataSelectModel::listForManualTableNames()
{
	return manual_tables;
}

QHash<QPair<QStandardItem *, QStandardItem *>, QStringList> & DataSelectModel::bkdbUsrsInformation()
{
	return usr_corruptions;
}

void DataSelectModel::modelForSpcDataesSelect()
{
	QStringList manage_projects;
	if (inspector->isDatabaseConstructor(using_name))
		manage_projects = inspector->curDBprojectTable().keys();
	else
		inspector -> managerOwnedProjs(using_name, manage_projects);
	QStringList all_tables = inspector->allTables();
	QStandardItem * engi_root = new QStandardItem(tr("当前数据选项"));
	treeArrangementForEngiDataes(engi_root, manage_projects);
	setItem(0, engi_root);
	QStandardItem * manual_dtables = new QStandardItem(tr("保存的数据表"));
	QString d_lbl("manualtable0");
	QStringList find_manuals;
	findManualTblsFromDb(d_lbl, using_name.second, all_tables, find_manuals);
	if (!find_manuals.isEmpty())
	{
		foreach (QString tbl, find_manuals)
		{
			QStringList manual_list = tbl.split(tr("，，。"));//need add constructor judge
			QStandardItem * idata_name = new QStandardItem(manual_list.back());
			initManualTableModelOnTree(idata_name, tbl);
			manual_dtables -> appendRow(idata_name);
		}	
	}
	setItem(1, manual_dtables);
}
	
void DataSelectModel::modelForSpcInfoesSelect()
{
	QStringList manage_projects;
	if (inspector->isDatabaseConstructor(using_name))
		manage_projects = inspector->curDBprojectTable().keys();
	else
		inspector -> managerOwnedProjs(using_name, manage_projects);
	QStringList all_tables = inspector->allTables();
	QStandardItem * engi_root = new QStandardItem(tr("当前信息选项"));
	foreach (QString m_proj, manage_projects)
	{
		QStandardItem * engi_name = new QStandardItem(m_proj);
		treeArrangementForEngiInfoes(engi_name);
		engi_root -> appendRow(engi_name);
	}
	QStandardItem * manual_dtables = new QStandardItem(tr("保存的信息表"));
	QStringList find_manuals;
	findManualTblsFromDb("manualtable1", using_name.second, all_tables, find_manuals);
	if (!find_manuals.isEmpty())
		manualItemOnTreeAction(manual_dtables, find_manuals);	
	setItem(0, engi_root);
	setItem(1, manual_dtables);
}

void DataSelectModel::modelForSpcDbTblBackup()
{
	QStandardItem * engi_bks = new QStandardItem(tr("选择备份工程"));
	QStandardItem * mtbl_bks = new QStandardItem(tr("选择备份自制表"));
	QStandardItem * alone_bks = new QStandardItem(tr("备份为单独数据库"));
	QStandardItem * indb_bks = new QStandardItem(tr("备份到数据库"));	
	QStringList ps_constructed;
	inspector -> constructorOwnedProjs(using_name, ps_constructed);	
	QStringList ps_agented;
	inspector -> agentOwnedProjs(using_name, ps_agented);
	QStringList manage_projects = ps_constructed+ps_agented;	
	foreach (QString m_proj, manage_projects)
	{
		QStandardItem * engi_branch = new QStandardItem(m_proj);
		QStandardItem * engi_time = new QStandardItem(tr("创建时间"));
		QString sel_name("name="+m_proj);
		QString fetch_time = inspector->dataFromTable("projectskeys", "constructtime", sel_name).toString();
		QString is_online = inspector->curDBprojectTable().value(m_proj).at(4);
		QString trans_str = QDateTime::fromString(fetch_time, Qt::ISODate).toString();
		engi_time -> appendRow(new QStandardItem(trans_str));
		engi_branch -> appendRow(engi_time);
		if (is_online.contains(tr("下线")))
			engi_branch -> appendRow(new QStandardItem(tr("已下线")));
		engi_bks -> appendRow(engi_branch);
	}
	QStringList all_tables = inspector->allTables();
	foreach (QString tbl, all_tables)
	{
		if (tbl.contains(using_name.second))
		{
			QStringList manual_list = tbl.split(tr("，，。"));
			if (manual_list.at(1).contains("manualtable") || manual_list.at(1).contains("plots"))
			{
				QStandardItem * idata_name = new QStandardItem(manual_list.back());
				initManualTableModelOnTree(idata_name, tbl);
				mtbl_bks -> appendRow(idata_name);
			}
		}	
	}	
	QDir related_dir("/home/dapang/workstation/spc-tablet/qsqlitebkdb");
	QFileInfoList qfi_list = related_dir.entryInfoList(QDir::Files);
	QString old_db = inspector->currentConnectionDb()->connectionName();	
	foreach (QFileInfo db_file, qfi_list)
	{
		QStandardItem * db_name = new QStandardItem(db_file.fileName());
		QStandardItem * inner_engis = new QStandardItem(tr("内部工程"));
		db_name -> appendRow(inner_engis);
		indb_bks -> appendRow(db_name);
		QString odb_name = "/home/dapang/workstation/spc-tablet/qsqlitebkdb/"+db_name->text();			
		*inspector->currentConnectionDb() = QSqlDatabase::addDatabase("QSQLITE", db_name->text());
		inspector->currentConnectionDb()->setDatabaseName(odb_name);	
		if (!inspector->currentConnectionDb()->open())
		{
					//warning dailog
			*inspector->currentConnectionDb() = QSqlDatabase::database(old_db);
			return;					
		}		
		QStringList db_projs;
		inspector -> getDataesFromTable("projectskeys", "name", db_projs);
		foreach (QString proj, db_projs)
		{
			QStandardItem * db_engi = new QStandardItem(proj);
			QStandardItem * engi_time = new QStandardItem(tr("创建时间"));			
			QString sel_name("name="+proj);
			QString fetch_time = inspector->dataFromTable("projectskeys", "constructtime", sel_name).toString();
			QString is_online = inspector->dataFromTable("projectsinfo", "attribution", sel_name).toString();
			QString trans_str = QDateTime::fromString(fetch_time, Qt::ISODate).toString();
			engi_time -> appendRow(new QStandardItem(trans_str));
			db_engi -> appendRow(engi_time);
			if (is_online.contains(tr("下线")))
				db_engi -> appendRow(new QStandardItem(tr("已下线")));
			inner_engis -> appendRow(db_engi);
		}
		QSqlDatabase::database(db_name->text()).close();		
	}
	QList<QStandardItem *> gen_dbs;
	findChildrenNotAll(indb_bks, gen_dbs);
	*inspector->currentConnectionDb() = QSqlDatabase::database(old_db);	
	foreach (QStandardItem * db_item, gen_dbs)	
		QSqlDatabase::removeDatabase(db_item->text());
	setItem(0, engi_bks);
	setItem(1, mtbl_bks);	
	setItem(2, alone_bks);
	setItem(3, indb_bks);		
}

void DataSelectModel::modelForSpcPlotsSelect(int detail)
{
	QStringList manage_projects;
	if (inspector->isDatabaseConstructor(using_name))
		manage_projects = inspector->curDBprojectTable().keys();
	else
		inspector -> managerOwnedProjs(using_name, manage_projects);
	QStringList all_tables = inspector->allTables();
	QStandardItem * engi_root = 0;
	QStandardItem * manual_plots = 0;
	if (detail == 3)
	{
		engi_root = new QStandardItem(tr("当前主图选项"));
		manual_plots = new QStandardItem(tr("保存的主图"));
	}
	else if (detail == 4)
	{
		engi_root = new QStandardItem(tr("当前辅图选项")); 
		manual_plots = new QStandardItem(tr("保存的辅图"));
	}
	else
	{
		engi_root = new QStandardItem(tr("当前复合图选项")); 
		manual_plots = new QStandardItem(tr("保存的复合图"));	  
	}
	treeArrangementForEngiPlots(engi_root, manage_projects);
	QString p_lbl("plots"+QString("%1").arg(detail));
	QStringList find_manuals;
	findManualTblsFromDb(p_lbl, using_name.second, all_tables, find_manuals);	
	if (!find_manuals.isEmpty())
		manualItemOnTreeAction(manual_plots, find_manuals);
	setItem(0, engi_root);
	setItem(1, manual_plots);
}
	
void DataSelectModel::modelForSpcTotalSelect(int detail)
{
	QStringList manage_projects;
	if (inspector->isDatabaseConstructor(using_name))
		manage_projects = inspector->curDBprojectTable().keys();
	else
		inspector -> managerOwnedProjs(using_name, manage_projects);
	QStringList all_tables = inspector->allTables();
	QStandardItem * engi_root = 0;
	QStandardItem * manual_related = 0;
	if (detail == 6)
	{
		engi_root = new QStandardItem(tr("当前数据信息选项"));
		treeArrangementForEngiDataes(engi_root, manage_projects);		
		manual_related = new QStandardItem(tr("保存的数据信息"));
	}
	else if (detail == 7)
	{
		engi_root = new QStandardItem(tr("当前综合图表选项"));
		QStandardItem * manual_dtables = new QStandardItem(tr("保存的数据表"));
		engi_root -> appendRow(manual_dtables);		
		QString d_lbl("manualtable0");
		QStringList find_manuals;
		findManualTblsFromDb(d_lbl, using_name.second, all_tables, find_manuals);
		if (!find_manuals.isEmpty())
			manualItemOnTreeAction(manual_dtables, find_manuals);			
		QStandardItem * manual_itables = new QStandardItem(tr("保存的信息表"));
		engi_root -> appendRow(manual_itables);		
		QString i_lbl("manualtable1");
		find_manuals.clear();
		findManualTblsFromDb(i_lbl, using_name.second, all_tables, find_manuals);
		if (!find_manuals.isEmpty())
			manualItemOnTreeAction(manual_itables, find_manuals);			
		QStandardItem * manual_infodata = new QStandardItem(tr("保存的数据信息"));
		engi_root -> appendRow(manual_infodata);		
		QString di_lbl("manualtable6");
		find_manuals.clear();
		findManualTblsFromDb(di_lbl, using_name.second, all_tables, find_manuals);
		if (!find_manuals.isEmpty())
			manualItemOnTreeAction(manual_infodata, find_manuals);		
		QStandardItem * manual_plots = new QStandardItem(tr("保存的主图"));
		engi_root -> appendRow(manual_plots);		
		QString mp_lbl("plots3");
		find_manuals.clear();
		findManualTblsFromDb(mp_lbl, using_name.second, all_tables, find_manuals);
		if (!find_manuals.isEmpty())
			manualItemOnTreeAction(manual_plots, find_manuals);
		QStandardItem * tool_plots = new QStandardItem(tr("保存的辅图"));
		engi_root -> appendRow(tool_plots);		
		QString tp_lbl("plots4");
		find_manuals.clear();
		findManualTblsFromDb(tp_lbl, using_name.second, all_tables, find_manuals);
		if (!find_manuals.isEmpty())
			manualItemOnTreeAction(tool_plots, find_manuals);
		QStandardItem * merge_plots = new QStandardItem(tr("保存的复合图"));
		engi_root -> appendRow(merge_plots);		
		QString multip_lbl("plots5");
		find_manuals.clear();
		findManualTblsFromDb(multip_lbl, using_name.second, all_tables, find_manuals);		
		if (!find_manuals.isEmpty())
			manualItemOnTreeAction(merge_plots, find_manuals);	
		QStandardItem * package_images = new QStandardItem(tr("其他图源"));
		engi_root -> appendRow(package_images);
		recurseAppendDirsInFolder(package_images, tr("/home/dapang/workstation/image文件"));			
		manual_related = new QStandardItem(tr("保存的综合图表"));
	}
	QString m_lbl("manualtable"+QString("%1").arg(detail));
	QStringList find_manuals;
	findManualTblsFromDb(m_lbl, using_name.second, all_tables, find_manuals);
	setItem(0, engi_root);
	setItem(1, manual_related);	
	if (!find_manuals.isEmpty())
		manualItemOnTreeAction(manual_related, find_manuals);	
}
	
void DataSelectModel::treeArrangementForEngiDataes(QStandardItem * root_item, const QStringList & engis_list)
{  	
	QStringList all_tbls = inspector->allTables();  
	QHash<QString, QStringList> next_hash;
	foreach (QString m_proj, engis_list)
	{		  
		if (!inspector->testEngiPerformance(m_proj))
		{
			next_hash[tr("未完善工程")] << m_proj;		
			continue;
		}
		QString engi("name="+m_proj);		
		QString on_off = inspector->dataFromTable("projectsinfo", "attribution", engi).toString();
		if (on_off.contains(tr("下线")))
			next_hash[tr("下线工程")] << m_proj;
		else
			next_hash[tr("在线工程")] << m_proj;	  
	}
	QHashIterator<QString, QStringList> i_ehash(next_hash);
	while (i_ehash.hasNext())
	{
		i_ehash.next();
		QStandardItem * n_branch = new QStandardItem(i_ehash.key());
		foreach (QString proj, i_ehash.value())
		{
			QStandardItem * e_proj = new QStandardItem(proj);			
			if (tree_view->treeType() == 6)
			{
				QStandardItem * all_dataes = new QStandardItem(tr("工程数据"));
				engiVersionDistribution(all_dataes, proj, all_tbls);
				e_proj -> appendRow(all_dataes);				  
				QStandardItem * all_infoes = new QStandardItem(tr("工程信息"));
				treeArrangementForEngiInfoes(all_infoes);
				e_proj -> appendRow(all_infoes);	
			}
			else
				engiVersionDistribution(e_proj, proj, all_tbls);
			n_branch -> appendRow(e_proj);
		}		
		root_item -> appendRow(n_branch);
	}
}
	
void DataSelectModel::treeArrangementForEngiInfoes(QStandardItem * r_item)
{ 
	r_item -> appendRow(new QStandardItem(tr("管理部门")));
	r_item -> appendRow(new QStandardItem(tr("授权代表")));
	r_item -> appendRow(new QStandardItem(tr("待授权者")));
	r_item -> appendRow(new QStandardItem(tr("质量描述")));
	r_item -> appendRow(new QStandardItem(tr("管理产品")));
	r_item -> appendRow(new QStandardItem(tr("属性数据")));  
}
	
void DataSelectModel::treeArrangementForEngiPlots(QStandardItem * root_item, const QStringList & engis_list)
{	
	QStringList all_tbls = inspector->allTables();  
	QHash<QString, QStringList> next_hash;
	foreach (QString m_proj, engis_list)
	{
		if (!inspector->testEngiPerformance(m_proj))		
			continue;
		QString engi("name="+m_proj);		
		QString on_off = inspector->dataFromTable("projectsinfo", "attribution", engi).toString();
		if (on_off.contains(tr("下线")))
			next_hash[tr("下线工程")] << m_proj;
		else
			next_hash[tr("在线工程")] << m_proj;	  
	}
	QHashIterator<QString, QStringList> i_ehash(next_hash);
	QList<QStandardItem *> copy_list;
	while (i_ehash.hasNext())
	{
		i_ehash.next();
		QStandardItem * n_branch = new QStandardItem(i_ehash.key());
		foreach (QString proj, engis_list)			  
			engiVersionDistriForPlots(n_branch, proj, all_tbls, copy_list);
		if (n_branch->rowCount() == 0)
			delete n_branch;
		else
			root_item -> appendRow(n_branch);
	}
	tree_view->backupManualFileItems()[root_item] << copy_list;
}

void DataSelectModel::manualItemOnTreeAction(QStandardItem * branch, const QStringList & db_manuals)
{
	foreach (QString tbl, db_manuals)
	{
		QStringList manual_list = tbl.split(tr("，，。"));
		QStandardItem * m_item = new QStandardItem(manual_list.back());
		branch -> appendRow(m_item);
		initManualTableModelOnTree(m_item, tbl);
	}	
}

void DataSelectModel::initManualTableModelOnTree(QStandardItem * branch, const QString & db_manual)
{
	QSqlTableModel * data_model = new QSqlTableModel(this, inspector->currentConnectionDb()->database(inspector->currentConnectionDb()->connectionName()));
	inspector -> varsModelFromTable(db_manual, data_model);
	QPair<QStandardItem *, QString> key_pair(branch, db_manual);
	manual_tables.insert(key_pair, data_model);
	if (tree_view->treeType()==7 && branch->parent()->text().contains(tr("图")) && branch->parent()!=item(1))
		completePlotsManualTblTree(branch, data_model);	  
}

void DataSelectModel::engiVersionDistribution(QStandardItem * engi_item, const QString & proj_name, const QStringList & db_tbls)
{
	QString name_row("name="+proj_name);
	QStandardItem * engi_time = new QStandardItem(tr("创建时间"));
	engi_item -> appendRow(engi_time);	
	QString cst_time(inspector->dataFromTable("projectskeys", "constructtime", name_row).toString());
	key_times.insert(engi_time, cst_time);
	QString ep_ctime = QDateTime::fromString(cst_time, Qt::ISODate).toString();
	engi_item -> appendRow(new QStandardItem(tr("创建者")));	
	QString proj_times = inspector->dataFromTable("projectskeys", "unit", name_row).toString();
	QStringList proj_versions = proj_times.split(tr("；"));	
	foreach (QString fver_str, proj_versions)
	{
		QStringList def_construction = fver_str.split(tr("，"));
		QStandardItem * version = new QStandardItem;
		QStandardItem * proj_paraes = new QStandardItem(tr("设定参数"));
		QStandardItem * setp_time = new QStandardItem(tr("设定时间"));		
		proj_paraes -> appendRow(setp_time);
		QStandardItem * setp_testor = new QStandardItem(tr("设定者"));		
		proj_paraes -> appendRow(setp_testor);		
		proj_paraes -> appendRow(new QStandardItem(tr("测量单位")));
		proj_paraes -> appendRow(new QStandardItem(tr("测量精度")));	
		proj_paraes -> appendRow(new QStandardItem(tr("控制图")));
		proj_paraes -> appendRow(new QStandardItem(tr("目标上限")));
		proj_paraes -> appendRow(new QStandardItem(tr("目标下限")));	
		proj_paraes -> appendRow(new QStandardItem(tr("每组样本")));
		proj_paraes -> appendRow(new QStandardItem(tr("样本组数")));
		proj_paraes -> appendRow(new QStandardItem(tr("目标值")));	
		version -> appendRow(proj_paraes);
		key_times.insert(setp_time, def_construction[0]);
		bool found = inspector->testEngiPerformance(proj_name, false, def_construction[0]);
		int finished = 1;	
		int unfinished = 1;		
		if (found)
		{
			version -> setText(tr("完成版本")+QString("%1").arg(finished));			  
			QStandardItem * cpk_paraes = new QStandardItem(tr("结果集"));
			engiSamplesDistribution(cpk_paraes, def_construction[0], db_tbls);
			version -> appendRow(cpk_paraes);
			finished++;
		}
		else
		{
			version -> setText(tr("未完成版本")+QString("%1").arg(unfinished));
			unfinished++;
		}
		engi_item -> appendRow(version);			
		QStandardItem * first_item = engi_item->takeRow(0).at(0);
		QStandardItem * second_item = engi_item->takeRow(0).at(0);
		sortItemsList(engi_item);	
		engi_item -> insertRow(0, second_item);
		engi_item -> insertRow(0, first_item);		
	}	
}

void DataSelectModel::engiSamplesDistribution(QStandardItem * engi_version, const QString & version_time, const QStringList & db_tbls)
{
	QString cpk_tbl;
	QString cpk_stamp = QString("%1").arg(QDateTime::fromString(version_time).toTime_t());
	findExistedRelatedTable(cpk_stamp, tr("，，。cpkdataes"), db_tbls, cpk_tbl);
	QStringList time_list;
	if (!cpk_tbl.isEmpty())
		inspector -> getDataesFromTable(cpk_tbl, "time", time_list);	
	int specimen = 1;	
	foreach (QString t_str, time_list)
	{
		QString e_stamp = QString("%1").arg(QDateTime::fromString(t_str, Qt::ISODate).toTime_t());
		QString cpk_gather;
		findExistedRelatedTable(e_stamp, tr("，，。cpk"), db_tbls, cpk_gather);
		QString daily_lbl;
		QString lbl_daily();	
		findExistedRelatedTable(e_stamp, tr("，，。dailydataes"), db_tbls, daily_lbl);
		QSqlTableModel cpk_model(this, inspector->currentConnectionDb()->database(inspector->currentConnectionDb()->connectionName()));
		inspector -> varsModelFromTable(cpk_gather, &cpk_model);
		QStandardItem * e_spec = new QStandardItem(tr("数据样本")+QString("%1").arg(specimen));
		QStandardItem * test_time = new QStandardItem(tr("测试时间"));
		e_spec -> appendRow(test_time);		
		e_spec -> appendRow(new QStandardItem(tr("数据建立者")));	
		e_spec -> appendRow(new QStandardItem(tr("测量基础数据")));
		e_spec -> appendRow(new QStandardItem(tr("总平均值")));	
		key_times.insert(test_time, t_str);
		bool dev_rng = false;
		QString f_tbl;
		findExistedRelatedTable(e_stamp, tr("，，。cpkdev"), db_tbls, f_tbl);
		if (!f_tbl.isEmpty())
		{
			dev_rng = true;
			QStandardItem * cpkdev_branch = new QStandardItem(tr("西格玛"));
			cpkdev_branch -> appendRow(new QStandardItem(tr("离散值")));
			cpkdev_branch -> appendRow(new QStandardItem(tr("标准差")));
			cpkdev_branch -> appendRow(new QStandardItem(tr("CPK值")));
			cpkdev_branch -> appendRow(new QStandardItem(tr("均值上控限")));
			cpkdev_branch -> appendRow(new QStandardItem(tr("均值下控限")));
			cpkdev_branch -> appendRow(new QStandardItem(tr("西格玛上控限")));
			cpkdev_branch -> appendRow(new QStandardItem(tr("西格玛下控限")));
			cpkdev_branch -> appendRow(new QStandardItem(tr("西格玛组状态")));
			e_spec -> appendRow(cpkdev_branch);			
		}
		f_tbl.clear();
		findExistedRelatedTable(e_stamp, tr("，，。cpkrng"), db_tbls, f_tbl);		
		if (!f_tbl.isEmpty())
		{
			QStandardItem * cpkrng_branch = new QStandardItem(tr("极差"));		  
			cpkrng_branch -> appendRow(new QStandardItem(tr("极差值")));
			cpkrng_branch -> appendRow(new QStandardItem(tr("标准差")));
			cpkrng_branch -> appendRow(new QStandardItem(tr("CPK值")));
			cpkrng_branch -> appendRow(new QStandardItem(tr("均值上控限")));
			cpkrng_branch -> appendRow(new QStandardItem(tr("均值下控限")));
			cpkrng_branch -> appendRow(new QStandardItem(tr("极差上控限")));
			cpkrng_branch -> appendRow(new QStandardItem(tr("极差下控限")));
			cpkrng_branch -> appendRow(new QStandardItem(tr("极差组状态")));
			e_spec -> appendRow(cpkrng_branch);			
		}
		QStandardItem * daily_root = new QStandardItem(tr("日常数据"));
		if (!daily_lbl.isEmpty())
		{
			daily_root -> appendRow(new QStandardItem(tr("周期测量数据")));
			QStandardItem * daily_avr = new QStandardItem(tr("均值"));
			daily_avr -> appendRow(new QStandardItem(tr("全部均值")));
			daily_avr -> appendRow(new QStandardItem(tr("受控均值")));
			daily_avr -> appendRow(new QStandardItem(tr("异常均值")));
			daily_root -> appendRow(daily_avr);
			if (dev_rng)
			{
				QStandardItem * daily_dev = new QStandardItem(tr("离散值"));
				daily_dev -> appendRow(new QStandardItem(tr("全部离散值")));
				daily_dev -> appendRow(new QStandardItem(tr("受控离散值")));
				daily_dev -> appendRow(new QStandardItem(tr("异常离散值")));	
				daily_root -> appendRow(daily_dev);			  
			}
			else
			{
				QStandardItem * daily_rng = new QStandardItem(tr("极差值"));
				daily_rng -> appendRow(new QStandardItem(tr("全部极差值")));
				daily_rng -> appendRow(new QStandardItem(tr("受控极差值")));
				daily_rng -> appendRow(new QStandardItem(tr("异常极差值")));	
				daily_root -> appendRow(daily_rng);			  
			}
		}
		e_spec -> appendRow(daily_root);
		engi_version -> appendRow(e_spec);
	}	
}

void DataSelectModel::engiVersionDistriForPlots(QStandardItem * engi_branch, const QString & proj_name, const QStringList & db_tbls, QList<QStandardItem *> & copy_items)
{
	QString name_row("name="+proj_name);
	QString proj_times = inspector->dataFromTable("projectskeys", "ctrlplot", name_row).toString();
	QStringList proj_versions = proj_times.split(tr("；"));	
	bool found_samples = inspector->testEngiPerformance(proj_name);
	if (found_samples)
	{
		QStandardItem * engi_item = new QStandardItem(proj_name);
		foreach (QString fver_str, proj_versions)
		{
			QStringList def_construction = fver_str.split(tr("，"));
			bool found = inspector->testEngiPerformance(engi_item->text(), false, def_construction.at(0));
			int finished = 1;			
			if (found)
			{
				QStandardItem * version = new QStandardItem(tr("完成版本")+QString("%1").arg(finished));				
				QStandardItem * setp_time = new QStandardItem(tr("设定时间"));
				setp_time -> appendRow(new QStandardItem(def_construction.at(0)));
				version -> appendRow(setp_time);
				engiSamplesDistriForPlots(engi_item, version, def_construction.at(0), db_tbls, copy_items);				
				finished++;			
			}				
		}
		if (engi_item->rowCount() == 0)
			delete engi_item;
		else
			engi_branch -> appendRow(engi_item);
	}
}

void DataSelectModel::engiSamplesDistriForPlots(QStandardItem * engi_root, QStandardItem * engi_version, const QString & version_type, const QStringList & db_tbls, QList<QStandardItem *> & copy_items)
{
	QString cpk_gather;
	QString cpk_stamp = QString("%1").arg(QDateTime::fromString(version_type).toTime_t());
	findExistedRelatedTable(cpk_stamp, tr("，，。cpkdataes"), db_tbls, cpk_gather);				
	QStringList time_list;
	inspector -> getDataesFromTable(cpk_gather, "time", time_list);
	int specimen = 1; 
	foreach (QString t_str, time_list)
	{
		QStandardItem * e_spec = new QStandardItem(tr("数据样本")+QString("%1").arg(specimen));
		QStandardItem * test_time = new QStandardItem(tr("测试时间"));
		test_time -> appendRow(new QStandardItem(QDateTime::fromString(t_str, Qt::ISODate).toString()));
		e_spec -> appendRow(test_time);
		if (tree_view->treeType() != 3)
		{
			QStandardItem * cpk_dataes = new QStandardItem(tr("测量基础数据"));
			cpk_dataes -> appendRow(new QStandardItem(tr("直方图")));
			QStandardItem * child_copy0 = new QStandardItem(test_time->child(0)->text()+tr("，")+cpk_dataes->text()+tr("，，～")+cpk_dataes->child(0)->text());
			copy_items << child_copy0;
			cpk_dataes -> appendRow(new QStandardItem(tr("正态概率纸")));
			QStandardItem * child_copy1 = new QStandardItem(test_time->child(0)->text()+tr("，")+cpk_dataes->text()+tr("，，～")+cpk_dataes->child(1)->text());
			copy_items << child_copy1;
			e_spec -> appendRow(cpk_dataes);	
		}
		QString dataes_stamp = QString("%1").arg(QDateTime::fromString(t_str, Qt::ISODate).toTime_t());
		QString f_tbl;
		findExistedRelatedTable(dataes_stamp, tr("，，。dailydataes"), db_tbls, f_tbl);	
		if (!f_tbl.isEmpty())
		{		  
			QStandardItem * daily_dataes = new QStandardItem(tr("日常数据"));
			if (tree_view->treeType() != 4)
			{
				daily_dataes -> appendRow(new QStandardItem(tr("均值图")));
				QStandardItem * child_copy0 = new QStandardItem(test_time->child(0)->text()+tr("，")+daily_dataes->text()+tr("，，～")+daily_dataes->child(0)->text());
				copy_items << child_copy0;
				if (!f_tbl.isEmpty())
					daily_dataes -> appendRow(new QStandardItem(tr("西格玛图")));	
				else
					daily_dataes -> appendRow(new QStandardItem(tr("极差图")));
				QStandardItem * child_copy1 = new QStandardItem(test_time->child(0)->text()+tr("，")+daily_dataes->text()+tr("，，～")+daily_dataes->child(1)->text());
				copy_items << child_copy1;
				
			}
			if (tree_view->treeType() != 3)
			{
				daily_dataes -> appendRow(new QStandardItem(tr("直方图")));
				daily_dataes -> appendRow(new QStandardItem(tr("正态概率纸")));
				QString hist_text;
				QString gau_text;
				if (tree_view->treeType()==4 || tree_view->treeType()==7)	
				{
					hist_text = daily_dataes->child(0)->text();
					gau_text = daily_dataes->child(1)->text();					
				}
				else
				{
					hist_text = daily_dataes->child(2)->text();
					gau_text = daily_dataes->child(3)->text();					  
				}
				QStandardItem * child_copy2 = new QStandardItem(test_time->child(0)->text()+tr("，")+daily_dataes->text()+tr("，，～")+hist_text);
				copy_items << child_copy2;
				QStandardItem * child_copy3 = new QStandardItem(test_time->child(0)->text()+tr("，")+daily_dataes->text()+tr("，，～")+gau_text);
				copy_items << child_copy3;
			}
			e_spec -> appendRow(daily_dataes);
		}
		if (e_spec->rowCount() > 1)
			engi_version -> appendRow(e_spec);
		else
			delete e_spec;
		specimen++;		
	}
	if (engi_version->rowCount() > 1)
		engi_root -> appendRow(engi_version);
	else
		delete engi_version;
}

void DataSelectModel::findManualTblsFromDb(const QString & m_lbl, const QString & m_noter, const QStringList & db_tbls, QStringList & find_tbls)
{
	foreach (QString tbl, db_tbls)
	{
		if (tbl.contains(m_lbl) && tbl.contains(m_noter))
			find_tbls << tbl;
	}
}

void DataSelectModel::completePlotsManualTblTree(QStandardItem * manual_item, QSqlTableModel * m_sql)
{
	QString items_infoes = m_sql->data(m_sql->index(0, 1)).toString();
	QStringList all_projs = inspector->curDBprojectTable().keys();	
	QHash<QString, QStringList> strs_hash;
	QHash<QString, QStringList> names_hash;	
	tree_view -> mBackItemsTransFromDB(manual_item, items_infoes, strs_hash, names_hash);	
	QList<QStandardItem *> projs_items = tree_view->backupManualFileItems().value(manual_item);	
	QStringList m_projs;
	foreach (QStandardItem * p_item, projs_items)
	{
		QStringList et_list = p_item->text().split(tr("，"));
		foreach (QString proj, all_projs)
		{
			if (inspector->projectContainsTimeStr(et_list.at(0), proj) && !m_projs.contains(proj))
			{
				m_projs << proj;
				all_projs.removeOne(proj);
				break;
			}
		}
	}
	QList<QStandardItem *> tmp_list;
	foreach (QString m_proj, m_projs)
		engiVersionDistriForPlots(manual_item, m_proj, inspector->allTables(), tmp_list);
	foreach (QStandardItem * d_item, tmp_list)
		delete d_item;
	QList<QStandardItem *> children;
	findAllChildItems(manual_item, children);	
	foreach (QStandardItem * p_item, projs_items)
	{
		QStringList f_split = p_item->text().split(tr("，，～"));
		QStringList f_keys = f_split.at(0).split(tr("，"));
		foreach (QStandardItem * f_item, children)
		{
			if (f_item->text() == f_keys.at(0))
			{				
				QList<QStandardItem *> again_children;
				findAllChildItems(f_item->parent()->parent(), again_children);
				foreach (QStandardItem * again_item, again_children)
				{
					if (again_item->parent()->text()==f_keys.at(1) && again_item->text() == f_split.at(1))
					{
						again_item -> appendRow(p_item->takeRow(0));
						break;
					}
				}
			}		
		}
	}
	foreach (QStandardItem * t_item, children)
	{
		if (t_item->text()==tr("均值图") || t_item->text()==tr("西格玛图") || t_item->text()==tr("直方图") || t_item->text()==tr("正态概率纸"))
		{				
			QStandardItem * t_parent = t_item->parent();			  
			if (!t_item->hasChildren())
				t_parent -> removeRow(t_item->row());
			QStandardItem * t_grandf = t_parent->parent();
			if (!t_parent->hasChildren())
				t_grandf->removeRow(t_parent->row());			
			QList<QStandardItem *> h_line;
			findItemsSingleLine(t_grandf, h_line, tr("数据样本"));
			QList<QStandardItem *> spec_children;
			findAllChildItems(h_line.at(0), spec_children);
			bool s_plot = false;
			foreach (QStandardItem * s_item, spec_children)
			{
				if (s_item->text()==tr("均值图") || s_item->text()==tr("西格玛图") || s_item->text()==tr("直方图") || s_item->text()==tr("正态概率纸"))
				{
					s_plot = true;
					break;
				}
			}
			QStandardItem * spec_parent = h_line.at(0)->parent();
			if (!s_plot)
				spec_parent -> removeRow(h_line.at(0)->row());
			h_line.clear();
			findItemsSingleLine(spec_parent, h_line, manual_item->text());
			QList<QStandardItem *> engi_children;
			findAllChildItems(h_line.at(1), engi_children);
			bool engi_specs = false;
			foreach (QStandardItem * e_item, engi_children)
			{
				if (e_item->text().contains(tr("数据样本")))
				{
					engi_specs = true;
					break;
				}
			}
			if (!engi_specs)
				manual_item -> removeRow(h_line.at(1)->row());			
		}
	}
	children.clear();
	findAllChildItems(manual_item, children);
	foreach (QStandardItem * pos_item, children)
	{
		if (pos_item->text().contains(tr("视图位置")))
		{
			QStandardItem * pos_parent = pos_item->parent();		
			tree_view->backupManualFileItems()[pos_parent] << pos_parent->takeRow(pos_item->row());
		}
	}
	foreach (QStandardItem * d_item, projs_items)
		delete d_item;	
	tree_view->backupManualFileItems().remove(manual_item);	
}

void DataSelectModel::recurseAppendDirsInFolder(QStandardItem * dir_item, const QString & n_dir)
{
	QDir nested_dir(n_dir);
	QFileInfoList qfi_list = nested_dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files);	
	foreach (QFileInfo fi, qfi_list)
	{
		QStandardItem * nested_item = new QStandardItem(fi.fileName());
		dir_item -> appendRow(nested_item);
		if (fi.isDir())
			recurseAppendDirsInFolder(nested_item, fi.filePath());
	}	
}

void DataSelectModel::sortItemsList(QStandardItem * parent_root)
{
	QList<QStandardItem *> sibling_finishs;
	QList<QStandardItem *> sibling_unfinishs;
	int i = 0;
	while (parent_root->child(i))
	{
		if (!parent_root->child(i)->text().contains(tr("未")))
			sibling_finishs << parent_root->takeRow(i).at(0);
		else
			sibling_unfinishs << parent_root->takeRow(i).at(0);
		i++;
	}
	foreach (QStandardItem * f_item, sibling_finishs)
		parent_root -> appendRow(f_item);
	foreach (QStandardItem * unf_item, sibling_unfinishs)
		parent_root -> appendRow(unf_item);	
}

bool DataSelectModel::isCurrentIndexContainEngi(const QModelIndex & index, const QString & engi)
{
	QStandardItem * parent = itemFromIndex(index);
	int i = 0;
	while (parent -> child(i))
	{
		if (parent->child(i)->text() == engi)
			return true;
	}
	return false;
}