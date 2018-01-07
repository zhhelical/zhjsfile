#include <QtGui>
#include <QtCore>
#include <QtSql/QSqlTableModel>
#include "nestedmodel.h"
#include "relatedtreeview.h"
#include "engitreeview.h"
#include "spcdatabase.h"
#include "editlongtoolsviewer.h"

NestedModel::NestedModel(QWidget * parent)
:QStandardItemModel(parent), tree_view(0), engi_view(0)
{
	RelatedTreeView * trans_rltd = qobject_cast<RelatedTreeView *>(parent);
	EngiTreeView * trans_engi = qobject_cast<EngiTreeView *>(parent);
	if (trans_rltd)
		tree_view = trans_rltd;
	else
		engi_view = trans_engi;
}

NestedModel::~NestedModel()
{
	if (!manual_tables.isEmpty())
	{
		QList<QSqlTableModel *> del_list = manual_tables.values();
		foreach (QSqlTableModel * sql, del_list)
			sql -> deleteLater();
	}
	QList<QStandardItem *> child_items;
	QHashIterator<QStandardItem *, QPair<QStandardItem *, QString> > d_hash(origin_condb);
	while (d_hash.hasNext())
	{
		d_hash.next();
		QStandardItem * d_item = d_hash.value().first;
		if (d_item && !d_item->parent())
			child_items << d_item;
	}
	foreach (QStandardItem * d_item, child_items)
		delete d_item;			
	foreach (QStandardItem * d_item, del_unredoes)
	{
		if (!d_item->model())
			delete d_item;		
	}
}

void NestedModel::initModel(SpcDataBase * val, const QPair<QString, QString> & ctrl_name)
{
	inspector = val;
	using_name = ctrl_name;
	int tree_type = tree_view->treeType();
	if (tree_type==0 || tree_type==1 || tree_type==2)
		modelForPowerRelatedTree(tree_type);
	else if (tree_type == 3)
		modelForPrjsKeysTree();
	else if (tree_type == 4)
		modelForProductsTree();
	else if (tree_type == 5)
		modelForEngiPropertyTree();
	else if (tree_type == 7)	
		modelForDbModifier();
	else
		modelForHelpView();
}

void NestedModel::initFreeModel(const QString & free_tbl, SpcDataBase * val)
{
	inspector = val;
	firstOpenFreeModelOndb(free_tbl);
}

void NestedModel::findIndexParentsChidren(const QModelIndex & index, QList<QStandardItem *> & parents)
{
	parents.push_front(itemFromIndex(index));
	QModelIndex loop_index = index;
	while(parent(loop_index).isValid())
	{
		if (itemFromIndex(parent(loop_index))==item(0))
			break;
		parents.push_front(itemFromIndex(parent(loop_index)));
		loop_index = parent(loop_index);
	}
	findAllChildItems(itemFromIndex(index), parents);
}

void NestedModel::findChildrenNotAll(QStandardItem * parent_item, QList<QStandardItem *> & sibling_items)
{
	int i = 0;
	while (parent_item->child(i))
	{
		sibling_items << parent_item->child(i);
		i++;
	}	
}

void NestedModel::findAllChildItems(QStandardItem * parent_item, QList<QStandardItem *> & child_items)
{
	int i = 0;
	while (parent_item->child(i))
	{
		child_items << parent_item->child(i);
		findAllChildItems(parent_item->child(i), child_items);
		i++;
	}
}

void NestedModel::removeSqltableInhash(const QString & key_str)
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

void NestedModel::userInfoIndb(const QPair<QString, QString> & comer, QStringList & infoes)
{
	if (comer.first.isEmpty() || comer.second.isEmpty())
		return;
	if (tree_view->treeType()==0 || tree_view->treeType()==2)
	{
		QPair<QString, QString> chk_dcon = comer;
		if (inspector->isDatabaseConstructor(chk_dcon))
		{
			infoes = inspector->curDBprojectTable().keys();
			return;
		}
		inspector -> managerOwnedProjs(comer, infoes);
	}
	else if (tree_view->treeType() == 1)
	{
		QMap<QString, QStringList> ms_existed = inspector->curDBmanageTable();
		if (ms_existed.contains(comer.second))
			infoes = ms_existed.value(comer.second);
		else
		{
			QMapIterator<QString, QStringList> ws_map(inspector->curDBprojectTable());
			while (ws_map.hasNext())
			{
				ws_map.next();
				if (ws_map.value().at(4).contains(comer.second))
				{
					QStringList w_list = ws_map.value().at(4).split(";");
					QStringList need_list = w_list.filter(comer.second);
					infoes = need_list.at(0).split(",");
					break;					
				}
			}
		}		
	}
}

void NestedModel::saveFreeConstructionWork(QString & is_save)
{
	bool all_same = true;
	for (int i = 0; i < rowCount(); i++)
	{
		QStandardItem * row_item = item(i);		
		if (!origin_condb.contains(row_item))
		{
			all_same = false;
			break;
		}	  
		if (!itemsArchitectureComparation(row_item, origin_condb.value(row_item).first))
		{
			all_same = false;
			break;
		}
	}
	if (all_same)
		return;
	is_save = inspector->saveFreeManualConstruction(using_name.second, this);
	tree_view->commandStack()->clear();
}

void NestedModel::translatedItemInfoes(QString & transed)
{
	for (int i = 0; i < rowCount(); i++)
	{
		QList<QStandardItem *> children;
		findAllChildItems(item(i), children);	
		children.push_front(item(i));
		foreach (QStandardItem * c_item, children)
		{
			QString item_infoes = QString::number((long)c_item, 16)+tr("，，～")+c_item->text()+tr("，，～，");
			if (c_item->hasChildren())
			{
				int i = 0;
				while (c_item->child(i))
				{
					item_infoes += QString::number((long)c_item->child(i), 16)+tr("，！，")+c_item->child(i)->text()+tr("，，～");
					i++;
				}
			}
			if (c_item == item(i))
				transed += item_infoes+tr("，，～："); 
			else
				transed += item_infoes+tr("，，～；");
		}
		transed += tr("，，&；"); 
	} 
}

void NestedModel::relistQmlViewerStrs(QStandardItem * ref_item, QStringList & cur_list)
{
	if (origin_condb.contains(ref_item))
	{
		foreach (QString str, cur_list)
		{
			if (str.contains(origin_condb.value(ref_item).second))
			{
				cur_list.removeOne(str);
				break;
			}
		}
	}	
}

void NestedModel::tmpSavedItemUnshowInfo(QStandardItem * clue_item, const QString & unshow_str, bool do_no)
{
	if (do_no)
		noshow_hash.insert(clue_item, unshow_str);//?
	else
	{
		QString to_hash = clue_item->text()+tr(",")+unshow_str;
		QList<QStandardItem *> key_items = tree_view->itemTextsHash().keys();
		QStandardItem * key_item = 0;
		foreach (QStandardItem * item, key_items)
		{
			if (item->text() == tr("添加授权代表"))
			{
				key_item = item;
				break;
			}
		}
		tree_view->itemTextsHash()[key_item] << to_hash;
		noshow_hash.remove(clue_item);
	}	
}

void NestedModel::ensureConfilctionForDbsMerging(QStandardItem * db_item)
{
 	QString old_db = inspector->currentConnectionDb()->connectionName(); 
	QSqlTableModel dbm_model(this, QSqlDatabase::database(old_db));
	inspector -> varsModelFromTable("dbInformations", &dbm_model);
	QString mdbtime = dbm_model.data(dbm_model.index(0, 2)).toString(); 
	QStringList merged_projs = inspector->curDBprojectTable().keys();
	QStringList merged_times;
	inspector -> getDataesFromTable("projectskeys", "constructtime", merged_times);
	QStringList merged_pswds;
	QMapIterator<QString, QStringList> i_mngs(inspector->curDBmanageTable());
	while (i_mngs.hasNext())
	{
		i_mngs.next();
		merged_pswds << i_mngs.value().at(1);
	}
	QString dest_db = origin_condb.value(db_item).second;
	QString dest_dbname = dest_db.split("/").back();
	*inspector->currentConnectionDb() = QSqlDatabase::addDatabase("QSQLITE", dest_dbname);
	inspector->currentConnectionDb()->setDatabaseName(dest_db);
	QSqlTableModel dbto_model(this, QSqlDatabase::database(dest_dbname));
	inspector -> varsModelFromTable("dbInformations", &dbto_model);
	QString dbtotime = dbto_model.data(dbto_model.index(0, 2)).toString();
	QStringList merge_projs;
	inspector -> getDataesFromTable("projectskeys", "name", merge_projs);
	QStringList merge_times;
	inspector -> getDataesFromTable("projectskeys", "constructtime", merge_times);
	foreach (QString mg_proj, merge_projs)
	{
		QString pcon_time = merge_times.at(merge_projs.indexOf(mg_proj));
		if (!merged_times.contains(pcon_time) && merged_projs.contains(mg_proj))
		{
			QString mc_proj = dbtotime+tr("，，。")+mg_proj;
			inspector->tmpProjsHashForDbsConfliction().insert(mc_proj, "");
		}
	}
	foreach (QString mg_mng, merged_pswds)
	{
		QStringList mc_pswd = mg_mng.split(";");
		QStringList get_pswd = mc_pswd.filter(mdbtime);
		QStringList real_pswd = get_pswd.at(0).split(",");
		QPair<QString, QString> chk_pair(inspector->curDBmanageTable().value(real_pswd.at(1)).at(0), real_pswd.at(1));
		QString chk_feedback;
		inspector -> chkUserInOtherDb(chk_pair, dest_dbname, chk_feedback);
		if (chk_feedback == tr("密码需要修改"))
		{
			QString mc_pswd = dbtotime+tr("，，。")+inspector->curDBmanageTable().value(real_pswd.at(1)).at(0)+tr("，，。")+real_pswd.at(1);
			inspector->tmpMngsHashForDbsConfliction().insert(mc_pswd, "");
		}
	}
	QSqlDatabase::database(dest_dbname).close();
	QSqlDatabase::removeDatabase(dest_dbname);
}

void NestedModel::freshModel()
{
	QList<QStandardItem *> c_siblings;
	findChildrenNotAll(item(0), c_siblings);
	if (!c_siblings[0]->hasChildren() && !using_name.second.isEmpty())
	{
		replyForAppendItem(c_siblings[0], using_name.first);
		replyForAppendItem(c_siblings[1], using_name.second);
	}
	QPair<QString, QString> customer(c_siblings[0]->child(0)->text(), c_siblings[1]->child(0)->text());
	QStringList user_info;
	userInfoIndb(customer, user_info);	
	if (tree_view->treeType() == 1)
	{
		if (user_info.isEmpty() && !item(0)->child(2)->hasChildren())
		{
			item(0)->child(2)->appendRow(new QStandardItem(tr("在此填写部门")));
			item(0)->child(3)->appendRow(new QStandardItem(tr("在此填写职位")));
			tree_view->itemTextsHash()[item(0)->child(0)->child(0)] << tr("数据库里不存在");
			return;
		}		
		QMap<QString, QStringList> ps_map = inspector->curDBprojectTable();		
		if (user_info.isEmpty() && !c_siblings[2]->child(0)->text().contains(tr("在此填写")) && !c_siblings[3]->child(0)->text().contains(tr("在此填写")))
		{
			QMapIterator<QString, QStringList> i_map(ps_map);
			while (i_map.hasNext())
			{
				i_map.next();
				replyForAppendItem(item(1), i_map.key());
			}
			tree_view->itemTextsHash().remove(item(0)->child(0)->child(0));
			return;	
		}
		QStringList constructeds;
		inspector -> constructorOwnedProjs(customer, constructeds);
		foreach (QString t_proj, constructeds)
			ps_map.remove(t_proj);			
		QStringList owned_projs;
		QStringList waiting_projs;
		owned_projs = user_info.at(3).split(";");
		inspector -> waitingPowerProjs(customer, waiting_projs);
		replyForAppendItem(c_siblings[2], user_info.at(2));
		replyForAppendItem(c_siblings[3], user_info.at(4));
		foreach (QString proj, owned_projs)
		{
			if (proj.contains(","))
			{
				QStringList set_data = proj.split(",");
				QStandardItem * e_owned = new QStandardItem(set_data[0]);
				QStandardItem * e_powered = new QStandardItem(tr("操作权限"));
				replyForAppendItem(e_powered, set_data[1]);
				e_owned -> appendRow(e_powered);
				c_siblings[4] -> appendRow(e_owned);
				ps_map.remove(set_data[0]);
			}			
		}
		foreach (QString proj, waiting_projs)
		{
			replyForAppendItem(c_siblings[5], proj);
			ps_map.remove(proj);			
		}
		QMapIterator<QString, QStringList> i_psmap(ps_map);			
		while (i_psmap.hasNext())
		{
			i_psmap.next();
			replyForAppendItem(item(1), i_psmap.key());
		}
		if (!origin_condb.isEmpty())
			origin_condb.clear();
		QList<QStandardItem *> w_items;
		findChildrenNotAll(item(0)->child(item(0)->rowCount()-1), w_items);
		foreach (QStandardItem * w_proj, w_items)
		{
			QStandardItem * w_copy = new QStandardItem(w_proj->text());
			QPair<QStandardItem *, QString> w_pair(w_copy, w_proj->text());
			origin_condb.insert(w_proj, w_pair);
		}
		tree_view->itemTextsHash()[item(0)->child(1)->child(0)] << tr("数据库里已存在");		
	}
	else if (tree_view->treeType() == 2)
	{
		QList<QStandardItem *> append_list;		
		if (user_info.isEmpty())
		{
			QStandardItem * no_constructed = new QStandardItem(tr("授权者没有创建过工程"));
			item(1) -> appendRow(no_constructed);			
			return;
		}
		if (!origin_condb.isEmpty())
			origin_condb.clear();		
		foreach (QString proj, user_info)
		{
			QStandardItem * engi_root = new QStandardItem(proj);
			QStringList waiting_list;
			empoweredUserStatesForTwoModel(proj, true, waiting_list);
			QStringList empowered_list;
			empoweredUserStatesForTwoModel(proj, false, empowered_list);	
			if (!waiting_list.isEmpty())
			{
				QStandardItem * wait_engi = new QStandardItem(tr("待授权者"));
				foreach (QString waiter, waiting_list)
				{
					append_list.clear();
					QStringList w_list = waiter.split(",");
					QStandardItem * wait_name = new QStandardItem(w_list.at(0));
					noshow_hash.insert(wait_name, w_list.at(1));
					QStandardItem * wait_depart = new QStandardItem(tr("所属部门"));
					append_list << wait_depart;
					replyForAppendItem(wait_depart, w_list.at(2));
					QStandardItem * wait_duty = new QStandardItem(tr("人员职务"));
					append_list << wait_duty;
					replyForAppendItem(wait_duty, w_list.at(3));
					QStandardItem * wait_powering = new QStandardItem(tr("授予权限"));
					append_list << wait_powering;
					replyForAppendItemsList(wait_name, append_list);
					wait_engi -> appendRow(wait_name);
					QString empty_power;
					QPair<QStandardItem *, QString> name_pair(wait_powering, empty_power);
					origin_condb.insert(wait_name, name_pair);
				}
				engi_root -> appendRow(wait_engi);
			}
			if (!empowered_list.isEmpty())
			{
				QStandardItem * waited_engi = new QStandardItem(tr("获授权者"));
				foreach (QString waited_str, empowered_list)
				{
					append_list.clear();
					QStringList e_list = waited_str.split(",");
					QStandardItem * waited_name = new QStandardItem(e_list.at(0));
					noshow_hash.insert(waited_name, e_list.at(1));
					QStandardItem * waited_depart = new QStandardItem(tr("所属部门"));
					append_list << waited_depart;
					replyForAppendItem(waited_depart, e_list.at(2));
					QStandardItem * waited_duty = new QStandardItem(tr("人员职务"));
					append_list << waited_duty;
					replyForAppendItem(waited_duty, e_list.at(3));
					QStandardItem * waited_powering = new QStandardItem(tr("获授权限"));
					append_list << waited_powering;
					replyForAppendItem(waited_powering, e_list.at(4));
					replyForAppendItemsList(waited_name, append_list);
					waited_engi -> appendRow(waited_name);
					QPair<QStandardItem *, QString> name_pair(waited_powering, e_list.at(4));
					origin_condb.insert(waited_name, name_pair);			
				}
				engi_root -> appendRow(waited_engi);
			}
			item(1) -> appendRow(engi_root);
		}
	}
}

bool NestedModel::checkModelWithDbStored()
{
	bool dif = true;	
	if (tree_view->treeType() == 0)
	{
		QList<QStandardItem *> all_agents;	  
		for (int i = 0; i < rowCount(); i++)
		{
			QStandardItem * agent_item = item(i)->child(0);
			QList<QStandardItem *> agent_children;
			findAllChildItems(agent_item, agent_children);
			if (!agent_children.isEmpty())
			{
				foreach (QStandardItem * a_item, agent_children)
				{
					if (!a_item->hasChildren())
						all_agents << a_item;
				}
			}
		}
		if (all_agents.size() != origin_condb.size())
			return false;
		foreach (QStandardItem * a_item, all_agents)
		{
			if (!origin_condb.contains(a_item))
				return false;
		}				
	}
	else if (tree_view->treeType()==1 || tree_view->treeType()==2)
	{			  
		QList<QStandardItem *> children;
		QList<QStandardItem *> test_orig;
		QList<QStandardItem *> tail_children;
		if (tree_view->treeType() == 1)
		{
			findChildrenNotAll(item(0)->child(item(0)->rowCount()-1), children);
			foreach (QStandardItem * c_proj, children)
			{
				if (origin_condb.contains(c_proj))
					test_orig << c_proj;
				else
					tail_children << c_proj;
			}
		}
		else
		{
			findChildrenNotAll(item(1), children);
			foreach (QStandardItem * c_proj, children)
			{
				int i = 0;
				while (c_proj->child(i))
				{
					QStandardItem * p_lable = c_proj->child(i);
					int j = 0;
					while (p_lable->child(j))
					{
						if (origin_condb.contains(p_lable->child(j)))
						{
							QStandardItem * power_lable = p_lable->child(j)->child(p_lable->child(j)->rowCount()-1);
							if ((power_lable->hasChildren() && power_lable->child(0)->text()==origin_condb.value(p_lable->child(j)).second) || (!power_lable->hasChildren() && origin_condb.value(p_lable->child(j)).second.isEmpty()))
								test_orig << p_lable->child(j);
						}
						j++;
					}
					i++;
				}
			}
		}
		if (test_orig.size()!=origin_condb.size() || !tail_children.isEmpty())
			dif = false;
	}	
	else if (tree_view->treeType()==4 || tree_view->treeType()==5)
	{
		int all_same = 0;	  
		for (int i = 0; i < rowCount(); i++)
		{
			QStandardItem *engi_item = item(i);
			if (engi_item->text() != origin_condb.value(engi_item).first->text())
				return false;
			if (itemsArchitectureComparation(engi_item, origin_condb.value(engi_item).first))
				all_same++;
		}
		if (all_same != rowCount())
			return false;
	}
	return dif;
}

bool NestedModel::fillStateCheckForNewComer()
{
	QList<QStandardItem *> c_siblings;
	findChildrenNotAll(item(0), c_siblings);
	if (c_siblings[0]->child(0)->text().contains(tr("在此填写")) || c_siblings[1]->child(0)->text().contains(tr("在此填写")))
		return false;
	if (tree_view->treeType() == 1)
	{		
		if ((c_siblings[2]->child(0) && c_siblings[2]->child(0)->text().contains(tr("在此填写"))) || (c_siblings[3]->child(0) && c_siblings[3]->child(0)->text().contains(tr("在此填写"))))
			return false;
	}
	return true;
}

bool NestedModel::userExistedIndb(const QString & filled_first, const QString & filled_second)
{
	QPair<QString, QString> judge_pair(filled_first, filled_second);
	if (inspector->theSameNameAndPasswdInManageList(judge_pair) || inspector->theSameNameWaitingPower(judge_pair))
		return true;
	return false;
}

bool NestedModel::dbMigrationAction(QStandardItem * move_item, QString & from, QString & to)
{
	QDir related_dir("/home/dapang/workstation/spc-tablet/"+from);
	QFileInfoList qfi_list = related_dir.entryInfoList(QDir::Files);
	foreach (QFileInfo d_info, qfi_list)
	{
		if (d_info.fileName() == move_item->text())
		{
			QString db_file = "/home/dapang/workstation/spc-tablet/"+from+"/"+move_item->text();
			QString db_move = "/home/dapang/workstation/spc-tablet/"+to+"/"+move_item->text();
			if (!moveRelatedDbfile(db_file, db_move))
			{
				if (db_file.contains(tr("三次失败")))
					from += tr("三次失败");
				else
					to += tr("三次失败");				  
				return false;
			}
			break;
		}
	}
	return true;
}

bool NestedModel::itemsArchitectureComparation(QStandardItem * base_item, QStandardItem * chk_item)
{
	if (base_item->rowCount() != chk_item->rowCount())
		return false;
	int i = 0;	
	bool chk_result = true;
	while (base_item->child(i) && chk_item->child(i))
	{ 
		if (base_item->child(i)->text() != chk_item->child(i)->text())
			return false;
		if (!itemsArchitectureComparation(base_item->child(i), chk_item->child(i)))
		{
			chk_result = false;
			break;
		}
		i++;
	}	
	return chk_result;
}

QStandardItem * NestedModel::findChildItem(QStandardItem * parent_item, QStandardItem * info_item)
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

QStandardItem * NestedModel::destParentItem(QStandardItem * child)
{
	if (!child)
		return 0;
	QStandardItem * loop_item = child;
	while (loop_item -> parent())
	{
		if (!loop_item->parent()->parent())
			break;
		loop_item = loop_item->parent();
	}
	return loop_item;	
}

QList<QStandardItem *> & NestedModel::resetDeleteUnredoItems()
{
	return del_unredoes;
}

QHash<QStandardItem *, QPair<QStandardItem *, QString> > & NestedModel::originHashItems()
{
	return origin_condb;
}

QHash<QStandardItem *, QString> & NestedModel::unshowHashItems()
{
	return noshow_hash;
}

const QPair<QString, QString> & NestedModel::savedUserName()
{
	return using_name;
}

const QPair<QStandardItem *, QStandardItem *> & NestedModel::dbEdittingOriginPair()
{
	return head_pair;
}

const QHash<QPair<QStandardItem *, QString>, QSqlTableModel *> & NestedModel::listForManualTableNames()
{
	return manual_tables;
}

void NestedModel::modelForPowerRelatedTree(int detail)
{
	QList<QStandardItem *> append_list;
	QStringList user_info;
	if (detail==0)
	{
		userInfoIndb(using_name, user_info);	  
		int i = 0;
		QMap<QString, QStringList> managers_map = inspector->curDBmanageTable();
		foreach (QString proj, user_info)
		{
			QStandardItem * root_engi = new QStandardItem(proj);
			QStringList back_pinfoes = inspector->curDBprojectTable().value(proj);
			QStandardItem * engi_agents = new QStandardItem(tr("授权代表"));
			if (!back_pinfoes.at(3).isEmpty())
			{
				QHash<QString, QStandardItem *> tmp_hash;
				QStringList agent_ms = back_pinfoes.at(3).split(";");
				if (agent_ms.back().isEmpty())
					agent_ms.pop_back();
				foreach (QString e_agent, agent_ms)
				{
					QStringList agent_detail = e_agent.split(",");
					QStandardItem * engi_agent = new QStandardItem(agent_detail.at(0));
					QStringList m_details = managers_map.value(agent_detail.at(1));
					QPair<QStandardItem *, QString> key_pair(0, agent_detail.at(1));
					origin_condb.insert(engi_agent, key_pair);
					QString depart = m_details.at(2);
					if (tmp_hash.contains(depart))
						tmp_hash[depart] -> appendRow(engi_agent);
					else
					{
						QStandardItem * engi_dprt = new QStandardItem(depart);
						engi_dprt -> appendRow(engi_agent);
						engi_agents -> appendRow(engi_dprt);
						tmp_hash.insert(depart, engi_dprt);
					}
				}
			}
			root_engi -> appendRow(engi_agents);
			QStandardItem * engi_departs = new QStandardItem(tr("管理部门"));
			QStringList m_departs = back_pinfoes.at(1).split(",");
			QStringList m_managers = back_pinfoes.at(2).split(";");
			departsManagersMatching(proj, m_departs, m_managers);
			foreach (QString depart, m_departs)
			{
				QStandardItem * engi_depart = new QStandardItem;
				furtherInitZeroModel(engi_depart, depart);
				engi_departs -> appendRow(engi_depart); 
			}
			root_engi -> appendRow(engi_departs);						
			setItem(i, root_engi);
			i++;			
		}
	}
	else if (detail==1)
	{
		QStringList owned_projs;
		QStringList waiting_projs;
		QStandardItem * person_root = new QStandardItem(tr("当前加入者"));
		QStandardItem * other_engis = new QStandardItem(tr("待选工程"));
		QStandardItem * person_name = new QStandardItem(tr("加入者姓名"));
		append_list << person_name;
		QStandardItem * person_passwd = new QStandardItem(tr("加入者密码"));
		append_list << person_passwd;
		QStandardItem * person_depart = new QStandardItem(tr("所属部门"));
		append_list << person_depart;
		QStandardItem * person_duty = new QStandardItem(tr("工作职位"));
		append_list << person_duty;
		QStandardItem * owned_engis = new QStandardItem(tr("获授权工程"));
		append_list << owned_engis;
		QStandardItem * owning_engis = new QStandardItem(tr("待加入工程"));
		append_list << owning_engis;
		replyForAppendItemsList(person_root, append_list);
		setItem(0, person_root);
		setItem(1, other_engis);		
		if (!using_name.second.isEmpty())
			freshModel();
		else
		{
			person_name -> appendRow(new QStandardItem(tr("在此填写姓名")));
			person_passwd -> appendRow(new QStandardItem(tr("在此填写密码")));			
		}
	}
	else
	{
		QStandardItem * name_root = new QStandardItem(tr("授权者"));
		QStandardItem * projs_root = new QStandardItem(tr("创建的工程"));
		QStandardItem * name = new QStandardItem(tr("授权者姓名"));
		QStandardItem * passwd = new QStandardItem(tr("授权者密码"));
		name_root -> appendRow(name);
		name_root -> appendRow(passwd);
		setItem(0, name_root);
		setItem(1, projs_root);		
		if (!using_name.second.isEmpty())		  
			freshModel();		  
		else
		{
			name -> appendRow(new QStandardItem(tr("在此填写姓名")));
			passwd -> appendRow(new QStandardItem(tr("在此填写密码")));
		}		
	}		
}
	
void NestedModel::modelForPrjsKeysTree()
{ 
	QStringList m_tolprojs;
	if (inspector->isDatabaseConstructor(using_name))
		m_tolprojs = inspector->curDBprojectTable().keys();
	else
		inspector -> managerOwnedProjs(using_name, m_tolprojs);
	QHash<QString, QStringList> next_hash;
	foreach (QString f_str, m_tolprojs)
	{
		if (!inspector->testEngiPerformance(f_str))
		{
			next_hash[tr("未完善工程")] << f_str;		
			continue;
		}
		QString engi("name="+f_str);		
		QString on_off = inspector->dataFromTable("projectsinfo", "attribution", engi).toString();
		if (on_off.contains(tr("下线")))
			next_hash[tr("下线工程")] << f_str;
		else
			next_hash[tr("在线工程")] << f_str;
	}
	QHashIterator<QString, QStringList> i_hash(next_hash);
	QList<QStandardItem *> pos_items;
	pos_items << 0 << 0 << 0;
	while (i_hash.hasNext())
	{
		i_hash.next();		
		if (i_hash.key() == tr("未完善工程"))
		{
			QStandardItem * nofinished_root = new QStandardItem(tr("未完善工程")); 
			arrangeAgainForProjKeyTree(i_hash.value(), nofinished_root);
			pos_items[0] = nofinished_root;
		}
		if (i_hash.key() == tr("在线工程"))
		{
			QStandardItem * online_root = new QStandardItem(tr("在线工程"));
			arrangeAgainForProjKeyTree(i_hash.value(), online_root);
			pos_items[1] = online_root;			
		}
		if (i_hash.key() == tr("下线工程"))
		{
			QStandardItem * offline_root = new QStandardItem(tr("下线工程"));
			arrangeAgainForProjKeyTree(i_hash.value(), offline_root);
			pos_items[2] = offline_root;
		}
	}	
	int set_pos = 0;
	foreach (QStandardItem * m_item, pos_items)
	{
		if (m_item)
		{			 
			setItem(set_pos, m_item);	
			set_pos++;
		}
	}	
}
	
void NestedModel::modelForProductsTree()
{
	QStringList table_list = inspector->allTables();
	QString tail = tr("，，。")+"product"+tr("，，。")+"tbl";
	QSqlTableModel t_saved(this, inspector->currentConnectionDb()->database(inspector->currentConnectionDb()->connectionName()));
	QStringList m_tolprojs;
	if (inspector->isDatabaseConstructor(using_name))
		m_tolprojs = inspector->curDBprojectTable().keys();
	else
		inspector -> managerOwnedProjs(using_name, m_tolprojs);
	QList<QStandardItem *> append_list;	
	int i = 0;
	foreach (QString proj, m_tolprojs)
	{
		QString e_name = proj+tail;
		QStandardItem * e_engi = new QStandardItem(proj);
		QStandardItem * engi_copy = new QStandardItem(proj);
		QPair<QStandardItem *, QString> ec_pair(engi_copy, proj);
		origin_condb.insert(e_engi, ec_pair);
		if (table_list.contains(e_name))
		{
			t_saved.clear();
			inspector -> varsModelFromTable(e_name, &t_saved);
			for (int j = 0; j < t_saved.columnCount(); j++)
			{
				QString c_name = t_saved.headerData(j, Qt::Horizontal).toString();
				QStandardItem * c_item = new QStandardItem(c_name);
				QStandardItem * c_copy = new QStandardItem(c_name);
				QPair<QStandardItem *, QString> cc_pair(c_copy, c_name);
				origin_condb.insert(c_item, cc_pair);
				QString c_contents = t_saved.data(t_saved.index(0, j)).toString();
				if (!c_contents.isEmpty())
				{
					if (c_contents.contains(tr("，！，")))
					{
						QStringList chd_list = c_contents.split(tr("，！，"));
						chd_list.pop_back();
						foreach (QString str, chd_list)
						{
							QStandardItem * c_children = new QStandardItem(str);
							QStandardItem * cc_copy = new QStandardItem(str);
							QPair<QStandardItem *, QString> ccc_pair(cc_copy, str);
							origin_condb.insert(c_children, ccc_pair);
							c_item -> appendRow(c_children);
							c_copy -> appendRow(cc_copy);
						}
					}
					else
					{
						QStandardItem * c_child = new QStandardItem(c_contents);
						QStandardItem * cc_copy = new QStandardItem(c_contents);
						QPair<QStandardItem *, QString> ccc_pair(cc_copy, c_contents);
						origin_condb.insert(c_child, ccc_pair);
						c_item -> appendRow(c_child);
						c_copy -> appendRow(cc_copy);
					}
				}
				e_engi -> appendRow(c_item);
				engi_copy -> appendRow(c_copy);
			}
		}
		else
		{
			append_list.clear();	  			
			QStandardItem * e_pname = new QStandardItem(tr("产品名称"));			
			append_list << e_pname;
			QStandardItem * e_type = new QStandardItem(tr("产品型号"));		
			append_list << e_type;
			QStandardItem * e_line = new QStandardItem(tr("所属产线"));
			append_list << e_line;
			QStandardItem * e_supplier = new QStandardItem(tr("原料供商"));
			append_list << e_supplier;
			QStandardItem * e_equip = new QStandardItem(tr("生产设备"));
			append_list << e_equip;
			QStandardItem * e_customer = new QStandardItem(tr("对应客户"));
			append_list << e_customer;
			replyForAppendItemsList(e_engi, append_list);			
		}
		setItem(i, e_engi);
		i++;		
	}
}
	
void NestedModel::modelForEngiPropertyTree()
{
	QStringList table_list = inspector->allTables();
	QString tail = tr("，，。")+"property";
	QSqlTableModel t_saved(this, QSqlDatabase::database(inspector->currentConnectionDb()->connectionName()));
	QStringList m_tolprojs;
	if (inspector->isDatabaseConstructor(using_name))
		m_tolprojs = inspector->curDBprojectTable().keys();
	else
		inspector -> managerOwnedProjs(using_name, m_tolprojs);
	QList<QStandardItem *> append_list;	
	int i = 0;
	foreach (QString proj, m_tolprojs)
	{
		QString e_name = proj+tail;	  	  
		QStandardItem * p_engi = new QStandardItem(proj);
		QStandardItem * engi_copy = new QStandardItem(proj);
		QPair<QStandardItem *, QString> e_pair(engi_copy, proj);
		origin_condb.insert(p_engi, e_pair);
		if (table_list.contains(e_name))
		{
			t_saved.clear();
			inspector -> varsModelFromTable(e_name, &t_saved);
			for (int j = 0; j < t_saved.columnCount(); j++)
			{
				QString c_name = t_saved.headerData(j, Qt::Horizontal).toString();
				QStandardItem * c_item = new QStandardItem(c_name);
				QStandardItem * c_copy = new QStandardItem(c_name);
				QPair<QStandardItem *, QString> c_pair(c_copy, c_name);
				origin_condb.insert(c_item, c_pair);
				QString c_contents = t_saved.data(t_saved.index(0, j)).toString();
				if (!c_contents.isEmpty())
				{
					if (c_contents.contains(tr("，！，")))
					{
						QStringList chd_list = c_contents.split(tr("，！，"));
						chd_list.pop_back();
						foreach (QString str, chd_list)
						{
							QStandardItem * c_children = new QStandardItem(str);
							QStandardItem * cc_copy = new QStandardItem(str);
							QPair<QStandardItem *, QString> cc_pair(cc_copy, str);
							origin_condb.insert(c_children, cc_pair);
							c_item -> appendRow(c_children);
							c_copy -> appendRow(cc_copy);
						}
					}
					else
					{
						QStandardItem * c_child = new QStandardItem(c_contents);
						QStandardItem * cc_copy = new QStandardItem(c_contents);
						QPair<QStandardItem *, QString> cc_pair(cc_copy, c_contents);
						origin_condb.insert(c_child, cc_pair);
						c_item -> appendRow(c_child);
						c_copy -> appendRow(cc_copy);
					}
				}
				p_engi -> appendRow(c_item);
				engi_copy -> appendRow(c_copy);
			}
		}
		else
		{
			append_list.clear();	
			QStandardItem * onoff_state = new QStandardItem(tr("在线下线"));	
			onoff_state -> appendRow(new QStandardItem(inspector->curDBprojectTable().value(proj).at(4)));
			append_list << onoff_state;		
			QStandardItem * p_perform = new QStandardItem(tr("作业内容"));
			append_list << p_perform;
			QStandardItem * p_standard = new QStandardItem(tr("依据标准"));
			append_list << p_standard;
			QStandardItem * p_insm = new QStandardItem(tr("度量仪器"));
			append_list << p_insm;
			QStandardItem * p_freq = new QStandardItem(tr("检验频率"));
			append_list << p_freq;
			QStandardItem * p_unnorm = new QStandardItem(tr("异常处理"));
			append_list << p_unnorm;
			QStandardItem * p_rec = new QStandardItem(tr("记录表单"));
			append_list << p_rec;
			QStandardItem * p_noter = new QStandardItem(tr("执行人员"));
			append_list << p_noter;
			QStandardItem * p_trakor = new QStandardItem(tr("异常跟踪"));
			append_list << p_trakor;
			replyForAppendItemsList(p_engi, append_list);	
		}		
		setItem(i, p_engi);
		i++;	
	}	
}

void NestedModel::modelForDbModifier()
{
 	QStandardItem * current_db = new QStandardItem(tr("当前数据库")); 
	QDir related_dir1; 
	related_dir1.setFilter(QDir::Files);	  
	related_dir1.setPath("/home/dapang/workstation/spc-tablet/qlitedb");
	QFileInfoList qfi_list1 = related_dir1.entryInfoList();
	QStandardItem * cur_name = 0;
	QStandardItem * cur_copy = 0;
	if (qfi_list1.isEmpty())
	{
		cur_name = new QStandardItem(tr("在此创建数据库\n或在”选择数据库”\n选择"));
		cur_copy = new QStandardItem(tr("在此创建数据库\n或在”选择数据库”\n选择"));
	}
	else
	{
		cur_name = new QStandardItem(qfi_list1[0].fileName());
		cur_copy = new QStandardItem(qfi_list1[0].fileName());
		QPair<QStandardItem *, QString> cur_pair(cur_copy, qfi_list1[0].filePath());
		origin_condb.insert(cur_name, cur_pair);
		head_pair.first = current_db;
		head_pair.second = cur_name;		
	}
	current_db -> appendRow(cur_name);
	setItem(0, current_db);		
	QStandardItem * modify_db = new QStandardItem(tr("选择数据库"));
	QDir related_dir2("/home/dapang/workstation/spc-tablet/qsqlitebkdb");
	QFileInfoList qfi_list2 = related_dir2.entryInfoList(QDir::Files);
	foreach (QFileInfo f_info, qfi_list2)
	{	  
		QString db_file = f_info.filePath();
		QStringList db_path = db_file.split("/");
		QStandardItem * e_db = new QStandardItem(db_path.back());
		QStandardItem * e_copy = new QStandardItem(db_path.back());
		QPair<QStandardItem *, QString> e_pair(e_copy, db_file);		
		modify_db -> appendRow(e_db);
		origin_condb.insert(e_db, e_pair);		
	}
	setItem(1, modify_db);	
}

void NestedModel::modelForHelpView()
{
	
}

void NestedModel::replyForAppendItem(QStandardItem * parent, const QString & item_text)
{
	QStandardItem * new_item = new QStandardItem(item_text);
	parent -> appendRow(new_item);
}

void NestedModel::replyForAppendItemsList(QStandardItem * parent, const QList<QStandardItem *> & child_items)
{
	foreach (QStandardItem * item, child_items)
	{
		if (tree_view->treeType()==4 || tree_view->treeType()==5)
		{
			QStandardItem * i_copy = new QStandardItem(item->text());
			QPair<QStandardItem *, QString> i_pair(i_copy, item->text());				
			origin_condb.insert(item, i_pair);
		}
		parent -> appendRow(item);
	}
}

void NestedModel::departsManagersMatching(const QString & proj, QStringList & departments, QStringList & managers)
{
	QStringList d_departs = departments;
	departments.clear();
	foreach (QString dp_str, d_departs)
	{
		QStringList manager_info;
		QStringList engineer_info;
		QStringList operator_info;
		foreach (QString m_str, managers)
		{
			QStringList m_detail = m_str.split(",");
			QPair<QString, QString> construct_pair;
			construct_pair.first = m_detail[0];
			construct_pair.second = m_detail[1];
			if (inspector->isConstructor(construct_pair, proj))
				continue;
			if (inspector->curDBdepartTable().value(dp_str).size()>2 && inspector->curDBdepartTable().value(dp_str).at(2).contains(m_detail[1]))
			{
				QStringList mp_power = inspector->curDBmanageTable()[m_detail[1]][3].split(";");
				QStringList tol_power = mp_power.filter(proj);
				QStringList power = tol_power[0].split(",");
				QString n_manager = m_str+","+power[1];
				if (inspector->curDBmanageTable().value(m_detail[1]).at(4) == tr("经理"))
					manager_info << n_manager;
				else if (inspector->curDBmanageTable().value(m_detail[1]).at(4) == tr("工程师"))
					engineer_info << n_manager;
				else
					operator_info << n_manager;
			}
		}
		QString manager_strs;
		QString engineer_strs;
		QString operator_strs;
		if (!manager_info.isEmpty())
		{
			QString join_mstr = manager_info.join(";");
			manager_strs = tr("经理")+"@"+join_mstr;
		}
		if (!engineer_info.isEmpty())
		{
			QString join_estr = engineer_info.join(";");
			engineer_strs = tr("工程师")+"@"+join_estr;
		}
		if (!operator_info.isEmpty())
		{
			QString join_ostr = operator_info.join(";");
			operator_strs = tr("操作者")+"@"+join_ostr;
		}
		QString tol_strs;
		if (!manager_strs.isEmpty())
			tol_strs = manager_strs;
		if (!engineer_strs.isEmpty())
			tol_strs += "-"+engineer_strs;
		if (!operator_strs.isEmpty())
			tol_strs += "-"+operator_strs;
		if (!tol_strs.isEmpty())
		{
			tol_strs = dp_str+"*"+tol_strs;
			departments << tol_strs;	
		}
	}
}

void NestedModel::furtherInitZeroModel(QStandardItem * dest_depart, const QString & content)
{
	QStringList d_every = content.split("*");
	dest_depart -> setText(d_every[0]);	
	QStringList meo_list = d_every[1].split("-");
	QString managers;
	QString engineers;
	QString operators;
	foreach (QString meo_str, meo_list)
	{
		if (meo_str.contains(tr("经理@")))
			managers = meo_str;
		else if (meo_str.contains(tr("工程师@")))
			engineers = meo_str;
		else
			operators = meo_str;
	}
	if (!managers.isEmpty())	
	{
		QStringList manager_list = managers.split("@");
		QStringList n_managers = manager_list[1].split(";");
		foreach (QString m_str, n_managers)
		{
			QStringList m_every = m_str.split(",");
			QStandardItem * m_title = new QStandardItem(tr("经理"));
			QStandardItem * m_name = new QStandardItem(m_every[0]);
			noshow_hash.insert(m_name, m_every[1]);
			QStandardItem * m_power = new QStandardItem(tr("权限"));
			m_power -> appendRow(new QStandardItem(m_every[2]));
			m_name -> appendRow(m_power);
			m_title -> appendRow(m_name);
			dest_depart -> appendRow(m_title); 
		}
	}
	if (!engineers.isEmpty())	
	{
		QStringList engineer_list = engineers.split("@");
		QStringList n_engineers = engineer_list[1].split(";");
		foreach (QString e_str, n_engineers)
		{
			QStringList e_every = e_str.split(",");
			QStandardItem * e_title = new QStandardItem(tr("工程师"));
			QStandardItem * e_name = new QStandardItem(e_every[0]);
			noshow_hash.insert(e_name, e_every[1]);		
			QStandardItem * e_power = new QStandardItem(tr("权限"));
			e_power -> appendRow(new QStandardItem(e_every[2]));
			e_name -> appendRow(e_power);
			e_title -> appendRow(e_name); 
			dest_depart -> appendRow(e_title);
		}
	}
	if (!operators.isEmpty())	
	{
		QStringList operator_list = operators.split("@");
		QStringList n_operators = operator_list[1].split(";");
		foreach (QString o_str, n_operators)
		{
			QStringList o_every = o_str.split(",");
			QStandardItem * o_title = new QStandardItem(tr("操作者"));
			QStandardItem * o_name = new QStandardItem(o_every[0]);
			noshow_hash.insert(o_name, o_every[1]);			
			QStandardItem * o_power = new QStandardItem(tr("权限"));
			o_power -> appendRow(new QStandardItem(o_every[2]));
			o_name -> appendRow(o_power);
			o_title -> appendRow(o_name);
			dest_depart -> appendRow(o_title);
		}
	}		
}

void NestedModel::empoweredUserStatesForTwoModel(const QString & proj, bool waiting, QStringList & empowereds)
{
	if (waiting)
		inspector -> waitingPowerManagers(proj, empowereds);
	else
	{
		QString empowered = inspector->curDBprojectTable().value(proj).at(2);
		QStringList e_listed = empowered.split(";");
		foreach (QString name_str, e_listed)
		{
			QStringList cell_list = name_str.split(",");
			if (inspector->isConstructor(QPair<QString, QString>(cell_list.at(0), cell_list.at(1)), proj))
				continue;
			QStringList m_infos = inspector->curDBmanageTable().value(cell_list[1]);
			cell_list << m_infos.at(2) << m_infos.at(4);				
			QStringList proj_list = m_infos.at(3).split(";");
			QStringList proj_dest = proj_list.filter(proj);
			QStringList power = proj_dest.at(0).split(",");			  
			cell_list << power.at(1);								
			QString new_name = cell_list.join(",");
			empowereds.push_back(new_name);
		}
	}
}

void NestedModel::arrangeAgainForProjKeyTree(const QStringList & projs, QStandardItem * tree_item)
{
	if (projs.isEmpty())
		return;		
	foreach (QString e_proj, projs)
	{
		QStandardItem * proj_name = new QStandardItem(e_proj);
		QString name_row("name="+e_proj);
		QStringList ep_tcons = inspector->dataFromTable("projectskeys", "constructor", name_row).toString().split(tr("；"));
		QStandardItem * time_branch = new QStandardItem(tr("创建时间"));
		QString ep_ctime = QDateTime::fromString(inspector->dataFromTable("projectskeys", "constructtime", name_row).toString(), Qt::ISODate).toString();
		QStandardItem * ep_time = new QStandardItem(ep_ctime);
		time_branch -> appendRow(ep_time);
		proj_name -> appendRow(time_branch);		
		QStandardItem * proj_constructor = new QStandardItem(tr("创建者"));
		QString ep_con = ep_tcons.filter(ep_ctime).at(0).split(tr("，")).at(1);
		QStandardItem * epcon_item = new QStandardItem(ep_con);
		proj_constructor -> appendRow(epcon_item);		
		proj_name -> appendRow(proj_constructor);
		QString proj_times = inspector->dataFromTable("projectskeys", "unit", name_row).toString();
		QStringList proj_versions = proj_times.split(tr("；"));	
		foreach (QString fver_str, proj_versions)
		{
			QStringList def_construction = fver_str.split(tr("，"));
			bool found = inspector->testEngiPerformance(e_proj, false, def_construction[0]);
			int finished = 1;			
			int unfinished = 1;
			QStandardItem * version = new QStandardItem;
			QStandardItem * proj_paraes = new QStandardItem(tr("设定参数"));
			QStandardItem * proj_time = new QStandardItem(tr("设定时间"));
			proj_time -> appendRow(new QStandardItem(def_construction[0]));
			proj_paraes -> appendRow(proj_time);		
			QStandardItem * proj_testor = new QStandardItem(tr("设定者"));
			QString ep_testor = ep_tcons.filter(def_construction[0]).at(0).split(tr("，")).at(1);
			proj_testor -> appendRow(new QStandardItem(ep_testor));
			proj_paraes -> appendRow(proj_testor);				
			QStringList ep_paraes;
			inspector -> getDataesFromTable("projectskeys", "name", ep_paraes, e_proj);
			keyDataesTreeSet(proj_paraes, def_construction[0], ep_paraes);
			version -> appendRow(proj_paraes);					
			if (found)
			{
				version -> setText(tr("完成版本")+QString("%1").arg(finished));			  
				finished++;
			}
			else
			{
				version -> setText(tr("未完成版本")+QString("%1").arg(unfinished));
				unfinished++;
			}
			proj_name -> appendRow(version);			
		}
		QStandardItem * first_item = proj_name->takeRow(0).at(0);
		QStandardItem * second_item = proj_name->takeRow(0).at(0);
		sortItemsList(proj_name);
		proj_name -> insertRow(0, first_item);
		proj_name -> insertRow(1, second_item);
		QMap<QString, QStringList> projs_infoes = inspector->curDBprojectTable();
		QMap<QString, QStringList> mngs_infoes = inspector->curDBmanageTable();		
		if (inspector->isDatabaseConstructor(using_name))
		{
			QStandardItem * mngs_branch = new QStandardItem(tr("当前管理者"));
			QStringList mngs = projs_infoes.value(e_proj).at(2).split(";");
			foreach (QString mng, mngs)
			{
				QStringList name_pwd = mng.split(",");
				QStandardItem * mng_name = new QStandardItem(name_pwd.at(0));
				QStandardItem * mng_dpr = new QStandardItem(tr("所属部门"));
				mng_dpr -> appendRow(new QStandardItem(mngs_infoes.value(name_pwd.at(1)).at(2)));
				QStandardItem * mng_position = new QStandardItem(tr("职位"));
				mng_position -> appendRow(new QStandardItem(mngs_infoes.value(name_pwd.at(1)).at(4)));				
				QStandardItem * mng_pwd = new QStandardItem(tr("密码"));
				mng_pwd -> appendRow(new QStandardItem(name_pwd.at(1)));
				mng_name -> appendRow(mng_dpr);
				mng_name -> appendRow(mng_position);
				mng_name -> appendRow(mng_pwd);
				mngs_branch -> appendRow(mng_name);				
			}
			QStandardItem * proj_agents = new QStandardItem(tr("授权代表"));
			QStringList agents = projs_infoes.value(e_proj).at(3).split(";");
			foreach (QString agent, agents)
			{
				if (agent.isEmpty())
					continue;
				QStringList name_pwd = agent.split(",");
				QStandardItem * agent_name = new QStandardItem(name_pwd.at(0));				
				QStandardItem * agent_pwd = new QStandardItem(tr("密码"));
				agent_pwd -> appendRow(new QStandardItem(name_pwd.at(1)));
				agent_name -> appendRow(agent_pwd);
				proj_agents -> appendRow(agent_name);				
			}			
			proj_name -> appendRow(mngs_branch);
			proj_name -> appendRow(proj_agents);
		}
		else if (inspector->isConstructor(using_name, e_proj))
		{
			QStandardItem * mngs_branch = new QStandardItem(tr("当前管理者")); 
			QStringList mngs = projs_infoes.value(e_proj).at(2).split(";");
			foreach (QString mng, mngs)
			{
				QStringList name_pwd = mng.split(",");
				QStandardItem * mng_name = new QStandardItem(name_pwd.at(0));
				QStandardItem * mng_dpr = new QStandardItem(tr("所属部门"));
				mng_dpr -> appendRow(new QStandardItem(mngs_infoes.value(name_pwd.at(1)).at(2)));
				QStandardItem * mng_position = new QStandardItem(tr("职位"));
				mng_position -> appendRow(new QStandardItem(mngs_infoes.value(name_pwd.at(1)).at(4)));				
				mng_name -> appendRow(mng_dpr);
				mng_name -> appendRow(mng_position);
				mngs_branch -> appendRow(mng_name);				
			}			
			QStandardItem * proj_agents = new QStandardItem(tr("授权代表"));
			QStringList agents = projs_infoes.value(e_proj).at(3).split(";");
			foreach (QString agent, agents)
			{
				if (agent.isEmpty())
					continue;			  
				QStringList name_pwd = agent.split(",");				
				proj_agents -> appendRow(new QStandardItem(name_pwd.at(0)));				
			}			
			proj_name -> appendRow(mngs_branch);
			proj_name -> appendRow(proj_agents);			
		}		
		tree_item -> appendRow(proj_name);
	}	
}

void NestedModel::firstOpenFreeModelOndb(const QString & n_tbl)
{
	QSqlTableModel m_sql(this, inspector->currentConnectionDb()->database(inspector->currentConnectionDb()->connectionName()));
	inspector -> varsModelFromTable(n_tbl, &m_sql);
	QString items_str = m_sql.data(m_sql.index(0, 0)).toString();
	QStringList model_heads = items_str.split(tr("，，&；"));
	model_heads.pop_back();
	foreach (QString m_child, model_heads)
	{
		QStringList cycle_list;
		QStringList head_tails = m_child.split(tr("，，～："));
		QStringList head_infoes = head_tails.at(0).split(tr("，，～，"));	
		QStandardItem * head_item = new QStandardItem(head_infoes.at(0).split(tr("，，～")).at(1));
		QStandardItem * head_copy = new QStandardItem(head_item->text());
		QPair<QStandardItem *, QString> head_pair(head_copy, head_infoes.at(0).split(tr("，，～")).at(0));
		origin_condb.insert(head_item, head_pair);		
		setItem(model_heads.indexOf(m_child), head_item);
		QStringList tails_items = head_tails.at(1).split(tr("，，～；"));
		foreach (QString t_str, tails_items)
		{
			if (!t_str.isEmpty())
			{
				cycle_list << t_str;
				QStringList ts_details = t_str.split(tr("，，～，"));
				QStringList tail_details = ts_details.at(0).split(tr("，，～"));
				QStandardItem * tail_item = new QStandardItem(tail_details.at(1));
				QStandardItem * tail_copy = new QStandardItem(tail_item->text());
				QPair<QStandardItem *, QString> t_pair(tail_copy, ts_details.at(0));
				origin_condb.insert(tail_item, t_pair);
				if (head_tails.at(0).contains(tail_details.at(0)))
				{
					head_item -> appendRow(tail_item);
					head_copy -> appendRow(tail_copy);
				}
			}
		}
		foreach (QString item_strs, cycle_list)
		{
			if (item_strs.contains(tr("，！，")))
			{
				QStringList strs_list = item_strs.split(tr("，，～，"));	
				QStandardItem * head_origin = findItemFromOrigincondb(strs_list.at(0));	
				QStandardItem * origin_copy = origin_condb.value(head_origin).first;	
				QStringList strs_chdls = strs_list.at(1).split(tr("，，～"));
				foreach (QString c_strs, strs_chdls)
				{
					if (!c_strs.isEmpty())
					{
						QStringList c_list = c_strs.split(tr("，！，"));
						QStandardItem * child_item = findItemFromOrigincondb(c_list.at(0));
						QStandardItem * child_copy = origin_condb.value(child_item).first;
						head_origin -> appendRow(child_item);
						origin_copy -> appendRow(child_copy);
					}
				}
			}
		}
	}
}

void NestedModel::keyDataesTreeSet(QStandardItem * key_root, QString & proj, const QStringList paraes)
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
		QStringList real_para = real_version[0].split(tr("，"));
		proj_paraes << real_para[1];
	}
	QStandardItem * unit_plot = new QStandardItem(tr("测量单位"));
	QStandardItem * unit = new QStandardItem(proj_paraes[0]);
	unit_plot -> appendRow(unit);
	key_root -> appendRow(unit_plot);
	QStandardItem * precision_plot = new QStandardItem(tr("测量精度"));
	QStandardItem * precision = new QStandardItem(proj_paraes[1]);
	precision_plot -> appendRow(precision);
	key_root -> appendRow(precision_plot);	
	QStandardItem * ctrl_plot = new QStandardItem(tr("控制图"));
	QStandardItem * ctrl = new QStandardItem(proj_paraes[2]);
	ctrl_plot -> appendRow(ctrl);
	key_root -> appendRow(ctrl_plot);
	QStandardItem * dest_up = new QStandardItem(tr("目标上限"));
	QStandardItem * up_num = new QStandardItem(proj_paraes[3]);
	dest_up -> appendRow(up_num);
	key_root -> appendRow(dest_up);
	QStandardItem * dest_low = new QStandardItem(tr("目标下限"));
	QStandardItem * low_num = new QStandardItem(proj_paraes[4]);
	dest_low -> appendRow(low_num);
	key_root -> appendRow(dest_low);	
	QStandardItem * plot_species = new QStandardItem(tr("每组样本"));
	QStandardItem * species = new QStandardItem(proj_paraes[5]);
	plot_species -> appendRow(species);
	key_root -> appendRow(plot_species);	
	QStandardItem * plot_grps = new QStandardItem(tr("样本组数"));
	QStandardItem * grps = new QStandardItem(proj_paraes[6]);
	plot_grps -> appendRow(grps);
	key_root -> appendRow(plot_grps);
	QStandardItem * dest_num = new QStandardItem(tr("目标值"));
	QStandardItem * num = new QStandardItem(proj_paraes[7]);
	dest_num -> appendRow(num);
	key_root -> appendRow(dest_num); 
}

void NestedModel::sortItemsList(QStandardItem * parent_root)
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

bool NestedModel::reselectedConflictWithdb(QStandardItem * dest_test)
{
	bool conflicted = false;
	if (origin_condb.contains(dest_test))
	{
		QString content = origin_condb.value(dest_test).second;
		QStringList all_tables = inspector->allTables();
		bool old_name = false;
		bool new_name = false;
		if (content != dest_test->text())
		{
			foreach (QString t_name, all_tables)
			{
				if (t_name.contains(content))
					old_name = true;
				if (t_name.contains(dest_test->text()))
					new_name = true;				
			}
			if (old_name && new_name)
				conflicted = true;
		}
	}
	return conflicted;
}

bool NestedModel::moveRelatedDbfile(QString & from, QString & to)
{
	QFile related_file(from);
	int cp = 0;
	while (!related_file.copy(to))
	{
		if (cp == 3)
		{
			from += tr("三次失败");
			return false;
		}
		cp++; 
	}
	int del = 0;
	while (!related_file.remove())
	{
		if (del == 3)
		{
			to += tr("三次失败");
			return false;
		}		
		del++; 	  
	}
	return true;
}

QStandardItem * NestedModel::findItemFromOrigincondb(const QString & valu)
{
	QStandardItem * r_item = 0;
	QHashIterator<QStandardItem *, QPair<QStandardItem *, QString> > f_hash(origin_condb);
	while (f_hash.hasNext())
	{
		f_hash.next();
		QString redef_str = f_hash.value().second;
		if (redef_str.contains(valu))
			return f_hash.key();
	}
	return r_item;
}
