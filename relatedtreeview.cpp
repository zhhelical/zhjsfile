#include <QtGui>
#include <QtSql>
#include "relatedtreeview.h"
#include "nestedmodel.h"
#include "dialog.h"
#include "editlongtoolsviewer.h"
#include "relatedtreecommand.h"
#include "spcdatabase.h"
#include "mainwindow.h"

RelatedTreeView::RelatedTreeView(QWidget * parent)
:QTreeView(parent), edit_free(false), empower_state(false), special_clicked(0), press_time(0), cons_viewer(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setAnimated(true);
	setDragDropMode(QAbstractItemView::NoDragDrop);
	setStyleSheet("font-family: Microsoft YaHei; font-size:18px; border: 0px");
	setHeaderHidden(true);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_commandStack = new QUndoStack(this);
	connect(this, SIGNAL(itemEditting(const QModelIndex &)), this, SLOT(edit(const QModelIndex &)));
	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(cellClicked(const QModelIndex &)));
	connect(this, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandedToResizeQmlView(const QModelIndex &)));
	connect(this, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(collapsedToResizeQmlView(const QModelIndex &)));	
}

RelatedTreeView::~RelatedTreeView()
{}

void RelatedTreeView::initTree(SpcDataBase * db_ptr, const QPair<QString, QString> & ctrl_name, int type)
{
	t_type = type;
	using_name = ctrl_name;
	inspector = db_ptr;
	NestedModel * roots_model = new NestedModel(this);
	roots_model -> initModel(inspector, ctrl_name);	
	if (t_type == 6)
	{
		QStringList tables_free = inspector->allTables().filter("freeconstruct");
		if (!tables_free.isEmpty())
			roots_model -> initFreeModel(tables_free.at(0), inspector);
	}
	setModel(roots_model);
}

void RelatedTreeView::initHelpTree(int type)
{
	t_type = type;
	NestedModel * helps_model = new NestedModel(this);
	helps_model -> initModel(0, QPair<QString, QString>("", ""));
	setModel(helps_model);
}

void RelatedTreeView::setInitViewRect(const QRect & rect)
{
	if (origin_rect != rect)
		origin_rect = rect;
}

void RelatedTreeView::setFreeForEdit(bool free)
{
	edit_free = free;
}

void RelatedTreeView::initStrListViewer(NestedModel * transed_model, const QModelIndex & pos_index)
{
	if (cons_viewer)
		return;
	cons_viewer = new EditLongToolsViewer(this);
	connect(cons_viewer, SIGNAL(killMyself(QWidget *)), this, SLOT(clearQmlViewStrsPt(QWidget *)));
	QDeclarativeContext * ctxt = cons_viewer->rootContext();
	cons_viewer -> setOrientation(EditLongToolsViewer::ScreenOrientationLockLandscape);
	QStandardItem * cur_item = transed_model->itemFromIndex(pos_index);
	QStringList val_list;
	if (texts_hash.contains(cur_item))
		val_list = texts_hash.value(cur_item);
	else
	{
		if (t_type == 0 || t_type == 1 || t_type == 2)
		{
			val_list << tr("不操作");
			if (t_type == 0)
			{
				QStandardItem * root_item = transed_model->destParentItem(cur_item);		  
				if (!cur_item->parent()->parent() && cur_item->row()==0)
					val_list << tr("添加授权代表") << tr("删除所有");
				else if (cur_item->parent()->parent() == root_item)
					val_list << tr("删除此授权代表");	
			}
			else if (t_type == 1)
			{
				QStandardItem * click_parent = cur_item->parent();	
				if (click_parent->text() == tr("所属部门"))
				{
					val_list += inspector->curDBdepartTable().keys();
					val_list << tr("新部门");
				}
				else if (click_parent->text() == tr("工作职位"))
					val_list << tr("经理") << tr("工程师") << tr("操作者");
				else if (click_parent->text() == tr("待加入工程"))
					val_list << tr("撤销选择");			
				else if (click_parent == transed_model->item(1))
					val_list << tr("选择加入");	
			}
			else
			{
				if (cur_item->parent()->text() == tr("获授权者") || cur_item->parent()->text() == tr("待授权者"))
					val_list << tr("删除加入者");
				else if (cur_item->text() == tr("授予权限") || cur_item->text() == tr("获授权限"))
					val_list << tr("只读") << tr("读写");	
				else if (cur_item->text() == tr("待授权者"))
					val_list << tr("删除所有申请者");
				else
					val_list.clear();
			}
		}
		else if (t_type == 4 || t_type == 5)
		{
			val_list << tr("不操作");		  
			if (!cur_item->parent())
			{
				val_list << tr("添加子项");
				if (t_type == 5)
					val_list << tr("修改工程名称") << tr("删除其他子项");
				else
					val_list << tr("删除所有");
			}
			else
			{
				QStandardItem * root_item = transed_model->destParentItem(cur_item);
				if (cur_item == root_item)
				{
					val_list << tr("添加子项");
					if (t_type==4 && root_item->row()<6)
						val_list << tr("删除子项");
					else
						val_list << tr("添加兄弟项") << tr("修改标题") << tr("删除此项") << tr("删除子项");
				}
				else if (cur_item->parent() == root_item)
				{
					if (t_type==5 && root_item->row()==0)
						val_list << tr("在线") << tr("下线");
					else
						val_list << tr("修改标题") << tr("删除此项");
				}
			}
		}		
		else if (t_type == 6)
		{
			val_list << tr("不操作") << tr("添加兄弟项") << tr("添加子项");
			QStringList projs = inspector->curDBprojectTable().keys();
			QStringList all_tables = inspector->allTables();
			QStringList selecttings;
			foreach (QString n_str, all_tables)
			{
				if (n_str.contains(tr("，，。plots")) || n_str.contains(tr("，，。manualtable")))
				{
					QStringList tol_name = n_str.split(tr("，，。"));
					selecttings << tol_name.back();
				}
			}				
			if (!projs.contains(cur_item->text()) && !selecttings.contains(cur_item->text()))
				val_list << tr("修改标题"); 
			val_list  << tr("工程选择") << tr("表图选择") << tr("删除子项") << tr("删除当前"); 				
		}
		else
		{
			val_list << tr("不操作");
			bool origin_item = transed_model->originHashItems().contains(cur_item);
			if (cur_item->parent() == transed_model->item(0))
			{				  
				if (cur_item->text() != tr("在此创建数据库\n或在”选择数据库”\n选择"))
				{				
					if (origin_item)
					{
						QString add_chk = transed_model->originHashItems().value(cur_item).second;
						if (inspector->isDatabaseConstructor(using_name, add_chk))
							val_list << tr("编辑数据库名");
					}
					val_list << tr("移出当前");
				}
				else
					val_list << tr("编辑数据库名");
			}
			else
			{
				if (transed_model->item(0)->child(0)->text() == tr("在此创建数据库\n或在”选择数据库”\n选择"))
					val_list << tr("作为当前数据库");
				else
				{
					val_list << tr("交换数据库");
					if (origin_item)
					{
						QString add_chk = transed_model->originHashItems().value(cur_item).second;
						if (inspector->isDatabaseConstructor(using_name, add_chk))
							val_list << tr("合并入当前数据库");
					}					
				}
			}
			if (origin_item)
			{
				QString add_chk = transed_model->originHashItems().value(cur_item).second;
				if (inspector->isDatabaseConstructor(using_name, add_chk))
					val_list << tr("删除数据库");
			}						
		}
	}
	if (val_list.isEmpty())
	{
		delete cons_viewer;
		cons_viewer = 0;
		return;
	}
	if (!val_list.isEmpty() && !texts_hash.contains(cur_item))
		texts_hash.insert(cur_item, val_list);
	ctxt -> setContextProperty("StringesList", QVariant::fromValue(val_list));
	cons_viewer -> setMainQmlFile(QLatin1String("spc_qml/slideSelector.qml"), 10);
	cons_viewer -> setOtherEditingTable(this);
	cons_viewer -> setNestedPosItem(cur_item);
	cons_viewer -> setGeometry(visualRect(pos_index));
	int height = 0;
	QFontMetrics fm(font());
	height = fm.height()*5;
	if (cur_item->text().contains("\n"))
		openNestedQmlViewerOnCell(cons_viewer, biggerWidthBetweenTextList(fm, val_list)*1.2, height);
	else
		openNestedQmlViewerOnCell(cons_viewer, biggerWidthBetweenTextList(fm, val_list, cur_item->text())*1.2, height);		
}

void RelatedTreeView::closeNestedQmlViewerOnCell(EditLongToolsViewer * nested)
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

void RelatedTreeView::setQmlViewerPtr(EditLongToolsViewer * view_ptr)
{
	if (cons_viewer != view_ptr)
		cons_viewer = view_ptr;
}

void RelatedTreeView::resetSpecialClickedItem(QStandardItem * s_item)
{
	special_clicked = s_item;
}

void RelatedTreeView::saveFreeConstructionTodb()
{
	NestedModel * n_model = qobject_cast<NestedModel *>(model());
	QString save_res;
	n_model -> saveFreeConstructionWork(save_res);
	if (save_res == tr("未保存"))
	{
		// warn dialog
	}
}

bool RelatedTreeView::dbEditorVerifyPermission(const QPair<QString, QString> & verify_pair, const QSqlDatabase & chk_db, bool constrctor_authgent)
{
	QSqlTableModel db_sql(this, chk_db.database(chk_db.connectionName()));
	inspector -> varsModelFromTable("dbInformations", &db_sql);	
	QString chk_editor;
	if (constrctor_authgent)
		chk_editor = db_sql.data(db_sql.index(0, 1)).toString();
	else
		chk_editor = db_sql.data(db_sql.index(0, 4)).toString();	 
	if (chk_editor.isEmpty())
		return false;
	QStringList c_details = chk_editor.split(",");
	if (c_details[0]==verify_pair.first && c_details[1]==verify_pair.second)
		return true;
	return false;
}

bool RelatedTreeView::usingNameNull()
{
	QPair<QString, QString> null_pair;
	if (using_name == null_pair)
		return true;
	return false;
}

bool RelatedTreeView::checkInputState()
{
	NestedModel * n_model = qobject_cast<NestedModel *>(model());	
	return !n_model->checkModelWithDbStored();	
}

bool RelatedTreeView::storeDataesTodb()
{
	NestedModel * n_model = qobject_cast<NestedModel *>(model());
	QHash<QStandardItem *, QPair<QStandardItem *, QString> > orig_hash = n_model->originHashItems();
	bool saved = true;
	if (t_type == 0)
	{
		int model_children = n_model->rowCount();
		for (int i = 0; i < model_children; i++)
		{
			QString e_engi = n_model->item(i)->text();
			QStandardItem * agent_items = n_model->item(i)->child(0);
			QList<QStandardItem *> departs_agents;
			n_model -> findAllChildItems(agent_items, departs_agents);
			foreach (QStandardItem * d_agent, departs_agents)
			{
				if (!d_agent->hasChildren())
				{
					if (orig_hash.contains(d_agent))
						orig_hash.remove(d_agent);
					else
					{
						QString r_name = "name="+e_engi;
						QString engi_agents;
						if (inspector->curDBprojectTable().value(e_engi).at(3).isEmpty())
							engi_agents = d_agent->text()+","+n_model->unshowHashItems().value(d_agent);
						else
							engi_agents = inspector->curDBprojectTable().value(e_engi).at(3)+";"+d_agent->text()+","+n_model->unshowHashItems().value(d_agent);				
						if (!inspector->updateCellData("projectsinfo", r_name, "authoagent", engi_agents))
							return false;
						inspector -> resetProjsDbMap();			
					}
				}
			}
		}
		if (!orig_hash.isEmpty())
		{
			QHashIterator<QStandardItem *, QPair<QStandardItem *, QString> > it_hash(orig_hash);
			while (it_hash.hasNext())
			{
				it_hash.next();
				QStringList c_detail = it_hash.key()->text().split(QObject::tr("，，。"));
				QString r_name = "name="+c_detail.at(1);
				QStringList engi_agents = inspector->curDBprojectTable().value(c_detail.at(1)).at(3).split(";");
				QStringList rm_agent = engi_agents.filter(it_hash.value().second);
				engi_agents.removeOne(rm_agent.at(0));
				if (!inspector->updateCellData("projectsinfo", r_name, "authoagent", engi_agents.join(";")))
					return false;				
			}						
			inspector -> resetProjsDbMap();
		}
	}
	else if (t_type == 1)
	{		
		QPair<QString, QString> cur_usr(n_model->item(0)->child(0)->child(0)->text(), n_model->item(0)->child(1)->child(0)->text());
		QList<QStandardItem *> waiting_projs;
		n_model -> findChildrenNotAll(n_model->item(0)->child(n_model->item(0)->rowCount()-1), waiting_projs);
		QStringList add_projs;
		foreach (QStandardItem * w_item, waiting_projs)
		{
			if (!orig_hash.contains(w_item))
				add_projs << w_item->text();
			else
				orig_hash.remove(w_item);
		}
		QPair<QString, QString> dep_pos;
		if (!inspector->theSameNameAndPasswdInManageList(cur_usr))
		{
			dep_pos.first = n_model->item(0)->child(2)->child(0)->text();
			dep_pos.second = n_model->item(0)->child(3)->child(0)->text();
		}
		QString w_manager = cur_usr.first+","+cur_usr.second;
		if (!dep_pos.second.isEmpty())
			w_manager += ","+dep_pos.first+","+dep_pos.second;
		QMap<QString, QStringList> projs_infoes = inspector->curDBprojectTable();
		foreach (QString proj, add_projs)
		{
			QString r_name = "name="+proj;
			QStringList new_waitings = projs_infoes[proj];
			QString old_ws = new_waitings.back();
			if (old_ws.isEmpty())
				old_ws = w_manager;
			else
				old_ws += ";"+w_manager;
			saved = inspector->updateCellData("projectsinfo", r_name, "authorizing", old_ws);
			if (!saved)
				return saved;			
		}		
		if (!orig_hash.isEmpty())//keep_del
		{
			QStringList del_projs;
			QHashIterator<QStandardItem *, QPair<QStandardItem *, QString> > d_hash(orig_hash);
			while (d_hash.hasNext())
			{
				d_hash.next();
				QString r_name = "name="+d_hash.value().second;
				QStringList new_waitings = projs_infoes[d_hash.value().second];
				QString old_ws = new_waitings.back();
				QStringList ws_list = old_ws.split(";");
				foreach (QString str, ws_list)
				{
					if (str.contains(cur_usr.second))
					{
						ws_list.removeAll(str);
						break;
					}
				}
				QString new_saved;
				if (!ws_list.isEmpty())
					new_saved = ws_list.join(";");
				saved = inspector->updateCellData("projectsinfo", r_name, "authorizing", new_saved);
				if (!saved)
					return saved;				
			}			
		}
		inspector -> resetProjsDbMap();
	}
	else if (t_type == 2)
	{				
		QHash<QStandardItem *, QPair<QStandardItem *, QString> > orig_hash = n_model->originHashItems();
		QHash<QStandardItem *, QString> unshow_hash = n_model->unshowHashItems();
		QList<QStandardItem *> tail_infoes;
		n_model -> findChildrenNotAll(n_model->item(1), tail_infoes);
		QList<QStandardItem *> editting_items;
		foreach (QStandardItem * p_item, tail_infoes)
		{
			int i = 0;
			while (p_item->child(i))
			{
				QStandardItem * p_lable = p_item->child(i);
				int j = 0;
				while (p_lable->child(j))
				{
					int cc_row = p_lable->child(j)->rowCount()-1;
					QStandardItem * power_lable = p_lable->child(j)->child(cc_row);
					if ((power_lable->hasChildren() && power_lable->child(0)->text()!=orig_hash.value(p_lable->child(j)).second) || (!power_lable->hasChildren() && !orig_hash.value(p_lable->child(j)).second.isEmpty()))
						editting_items << p_lable->child(j);
					orig_hash.remove(p_lable->child(j));
					j++;
				}				
				i++;
			}
		}
		QSqlTableModel dbm_model(this, QSqlDatabase::database(inspector->currentConnectionDb()->connectionName()));
		inspector -> varsModelFromTable("dbInformations", &dbm_model);			
		foreach (QStandardItem * power_item, editting_items)
		{
			QString proj_name = n_model->destParentItem(power_item)->text();		  
			QString p_pswd = unshow_hash.value(power_item);
			if (inspector->curDBmanageTable().contains(p_pswd))
			{
				QStringList old_powers = inspector->curDBmanageTable().value(p_pswd).at(3).split(";");				
				QStringList old_power = old_powers.filter(proj_name);
				if (old_power.isEmpty())
					old_powers << proj_name+","+power_item->child(power_item->rowCount()-1)->child(0)->text();
				else
				{
					QStringList chg_power = old_power.at(0).split(",");
					chg_power[1] = power_item->child(power_item->rowCount()-1)->child(0)->text();
					old_powers = old_powers.replaceInStrings(old_power.at(0), chg_power.join(","));
				}
				QString key_pawd("password="+inspector->curDBmanageTable().value(p_pswd).at(1));
				saved = inspector->updateCellData("managerinfo", key_pawd, "projects", old_powers.join(";"));
				if (!saved)
					return saved;				
			}
			else
			{
				QString db_time = dbm_model.data(dbm_model.index(0, 2)).toString();
				QString merge_pswd(db_time+","+p_pswd);
				QStringList mng_list;
				mng_list << power_item->text() << merge_pswd << power_item->child(0)->child(0)->text() << proj_name+","+power_item->child(power_item->rowCount()-1)->child(0)->text() << power_item->child(1)->child(0)->text();
				saved = inspector->storeManageInfoTable(mng_list);
				if (!saved)
					return saved;
				QStringList dpr_list;
				if (inspector->curDBdepartTable().contains(power_item->child(0)->child(0)->text()))
				{
					QString dpr_row("name="+power_item->child(0)->child(0)->text());				  
					dpr_list = inspector->curDBdepartTable().value(power_item->child(0)->child(0)->text());
					if (!dpr_list.at(1).contains(proj_name))
					{
						QStringList dpr_projs = dpr_list.at(1).split(","); 
						dpr_projs << proj_name;
						dpr_list[1] = dpr_projs.join(",");
						saved = inspector->updateCellData("departments", dpr_row, "projects", dpr_list.at(1));
						if (!saved)
							return saved;						 
					}
					QStringList dpr_mngs = dpr_list.at(2).split(";"); 
					dpr_mngs << power_item->text()+","+p_pswd;
					dpr_list[2] = dpr_mngs.join(";");
					saved = inspector->updateCellData("departments", dpr_row, "managers", dpr_list.at(2));
					if (!saved)
						return saved;					
				}
				else
				{					
					dpr_list << power_item->child(0)->child(0)->text() << proj_name << power_item->text()+","+p_pswd;
					saved = inspector->storeDepartmentTable(dpr_list);
					if (!saved)
						return saved;					
				}
				inspector -> resetDepartmentsDbMap();		
			}
			inspector -> resetManagersDbMap();	
			QString proj_nrow("name="+proj_name);				
			QStringList proj_infoes = inspector->curDBprojectTable().value(proj_name);
			QString p_dprs = proj_infoes.at(1);
			if (!p_dprs.contains(power_item->child(0)->child(0)->text()))
			{
				QStringList proj_dprs = p_dprs.split(",");
				proj_dprs << power_item->child(0)->child(0)->text();
				saved = inspector->updateCellData("projectsinfo", proj_nrow, "departments", proj_dprs.join(","));
				if (!saved)
					return saved;				
			}
			QString p_powereds = proj_infoes.at(2);
			if (!p_powereds.contains(p_pswd))
			{
				QStringList proj_powereds = p_powereds.split(";");
				proj_powereds << power_item->text()+","+p_pswd;
				saved = inspector->updateCellData("projectsinfo", proj_nrow, "managers", proj_powereds.join(";"));
				if (!saved)
					return saved;
			}
			QStringList p_waitings = proj_infoes.at(5).split(";");
			foreach (QString w_str, p_waitings)
			{
				if (w_str.contains(p_pswd))
				{
					p_waitings.removeOne(w_str);
					break;
				}
			}
			QString waitings = p_waitings.join(";");
			if (p_waitings.isEmpty())
				waitings.clear();
			saved = inspector->updateCellData("projectsinfo", proj_nrow, "authorizing", waitings);
			if (!saved)
				return saved;
			inspector -> resetProjsDbMap();				
		}
		if (!orig_hash.isEmpty())//keep_del
		{
			QHashIterator<QStandardItem *, QPair<QStandardItem *, QString> > m_hash(orig_hash);
			while (m_hash.hasNext())
			{
				m_hash.next();
				QStringList pswd_proj = unshow_hash.value(m_hash.key()).split(",");				
				QString proj_name = pswd_proj.at(1);
				QString pswd = pswd_proj.at(0);
				if (inspector->curDBmanageTable().contains(pswd))
				{
					QString mg_projs = inspector->curDBmanageTable().value(pswd).at(3);
					QPair<QString, QString> r_name(m_hash.key()->text(), pswd);
					QString mng_dpr = inspector->curDBmanageTable().value(pswd).at(2);
					QString is_del;
					saved = inspector->deleteMngInfoesToOtherDb(inspector->currentConnectionDb()->connectionName(), r_name, is_del, proj_name);
					if (!saved)
					{
						//show del_result warn dialog
						return saved;	
					}
					inspector -> resetManagersDbMap();
					inspector -> resetDepartmentsDbMap();
					QString proj_ms = inspector->curDBprojectTable().value(proj_name).at(2);
					QString proj_agts = inspector->curDBprojectTable().value(proj_name).at(3);
					QStringList pms_list = proj_ms.split(";");
					QStringList agts_list = proj_agts.split(";");
					QString rp_name("name="+proj_name);
					bool m_exist = false;
					foreach (QString mng, pms_list)
					{
						if (mng.contains(pswd))
						{
							m_exist = true;
							pms_list.removeOne(mng);
							break;
						}
					}
					if (m_exist)
					{
						proj_ms = pms_list.join(";");						
						saved = inspector->updateCellData("projectsinfo", rp_name, "managers", proj_ms);
						if (!saved)
							return saved;
					}
					bool a_exist = false;						
					foreach (QString agt, agts_list)
					{
						if (agt.contains(pswd))
						{
							a_exist = true;  
							agts_list.removeOne(agt);
							break;
						}
					}
					if (a_exist)
					{						
						proj_agts = agts_list.join(";");	
						saved = inspector->updateCellData("projectsinfo", rp_name, "authoagent", proj_agts);
						if (!saved)
							return saved;
					}					
					QString dpr_projs;
					if (!inspector->curDBmanageTable().value(pswd).isEmpty())
						dpr_projs = inspector->curDBdepartTable().value(mng_dpr).at(1);
					if (!dpr_projs.contains(proj_name))
					{
						QStringList proj_dprs = inspector->curDBprojectTable().value(proj_name).at(1).split(",");
						proj_dprs.removeOne(mng_dpr);
						saved = inspector->updateCellData("projectsinfo", rp_name, "departments", proj_dprs.join(","));
						if (!saved)
							return saved;						
					}
				}
				else
				{				  
					QStringList p_waitings = inspector->curDBprojectTable().value(proj_name).at(5).split(";");
					foreach (QString w_str, p_waitings)
					{
						if (w_str.contains(pswd))
						{
							p_waitings.removeOne(w_str);
							break;
						}
					}
					QString waitings = p_waitings.join(";");
					if (p_waitings.isEmpty())
						waitings.clear();
					QString proj_nrow("name="+proj_name);
					saved = inspector->updateCellData("projectsinfo", proj_nrow, "authorizing", waitings);
					if (!saved)
						return saved;					
				}
				inspector -> resetProjsDbMap();				
			}			
		}		
	}	
	else if (t_type == 4)
		saved = inspector->createEngiCarryProductTable(n_model, false);
	else if (t_type == 5)
		saved = inspector->createEngiCarryProductTable(n_model, true);
	else
	{}	
	return saved;	
}

bool RelatedTreeView::userIndb()
{
	QPair<QString, QString> judge_pair;
	judge_pair.first = model()->data(model()->index(0, 0)).toString(); 
	judge_pair.second = model()->data(model()->index(1, 0)).toString();
	if (judge_pair.first.isEmpty() || judge_pair.first.isEmpty())
		return false;
	if (inspector->theSameNameAndPasswdInManageList(judge_pair) || inspector->theSameNameWaitingPower(judge_pair))
		return true;
	return false;
}

bool RelatedTreeView::dbsEdittingAction(QString & act_result)
{ 
	NestedModel * db_model = qobject_cast<NestedModel *>(model());
	QList<QStandardItem *> all_items;
	all_items.push_front(db_model->item(0)->child(0)); 
	QList<QStandardItem *> one_items;
	db_model -> findAllChildItems(db_model->item(1), one_items);
	if (!one_items.isEmpty())
		all_items += one_items;
 	MainWindow * mw = qobject_cast<MainWindow *>(inspector->parent()->parent());
	QProgressDialog save_following(mw, Qt::FramelessWindowHint);	  
	save_following.setWindowModality(Qt::WindowModal);
	save_following.setRange(0, all_items.size()+db_model->unshowHashItems().size());
	save_following.setLabel(0);
	save_following.setCancelButton(0);
	save_following.setGeometry(mw->width()/2-100, mw->height()/2-15, 200, 15);	
	save_following.setFixedSize(QSize(200, 15));
	save_following.setStyleSheet("color: white; background: rgb(50, 50, 50)"); 	
	save_following.show(); 	
	QHash<QStandardItem *, QPair<QStandardItem *, QString> > origin_hash = db_model->originHashItems();
	QList<QStandardItem *> origin_items = origin_hash.keys();		
	if (all_items.size() == origin_items.size())
	{
		bool found_different = false;	  
		foreach (QStandardItem * item, all_items)
		{			
			if (origin_items.contains(item))
			{
				if (all_items.indexOf(item) == 0)
				{
					if (item!=db_model->dbEdittingOriginPair().second)
					{
						found_different = true;
						break;
					}
				}
				QStringList db_path = origin_hash.value(item).second.split("/");
				if (item->text() != db_path.back())
				{
					found_different = true;
					break;				  
				}
				continue;
			}
			found_different = true;
			break;			  
		}
		if (!found_different)
		{
			act_result = tr("没有编辑项");
			return false;
		}
	}	
	QDateTime c_time = QDateTime::currentDateTime();
	foreach (QStandardItem * d_item, all_items)
	{
		save_following.setValue(all_items.indexOf(d_item)+1);
		if (origin_hash.contains(d_item))
		{		  
			QStringList detail_name = origin_hash.value(d_item).second.split("/");
			if (d_item->text() != detail_name.back() && d_item->text().contains(".db"))
			{					  
				detail_name.pop_back();
				detail_name.pop_front();
				QString db_new = "/"+detail_name.join("/")+"/"+d_item->text();
				if (!inspector->renameDatabaseName(origin_hash.value(d_item).second, db_new))
				{
					act_result = d_item->text()+tr("重命名失败");
					return false;
				}				
			}			
			if (d_item->parent() == db_model->item(0))
			{
				if (detail_name[detail_name.size()-2] != "qlitedb")
				{	
					QString db_from("qsqlitebkdb");
					QString db_to("qlitedb");
					if (!db_model->dbMigrationAction(d_item, db_from, db_to))
					{
						if (db_from.contains(tr("三次失败")))
							act_result = d_item->text()+tr("拷贝三次失败");
						else
							act_result = d_item->text()+tr("删除三次失败");
						return false;
					}					
				}
				else
				{
					if (d_item->text() == tr("在此创建数据库\n或在”选择数据库”\n选择"))
					{					  
						if (!inspector->deleteDatabase(origin_hash.value(d_item).second))
						{
							act_result = origin_hash.value(d_item).second+tr("删除失败");
							return false;
						}
						db_model->unshowHashItems().remove(d_item);						
					}						
				}						
			}
			else
			{
				if (detail_name[detail_name.size()-2] == "qlitedb")
				{
					QString db_from("qlitedb");
					QString db_to("qsqlitebkdb");				  
					if (!db_model->dbMigrationAction(d_item, db_from, db_to))
					{
						if (db_from.contains(tr("三次失败")))
							act_result = d_item->text()+tr("拷贝三次失败");
						else
							act_result = d_item->text()+tr("删除三次失败");
						return false;
					}
				}				
			}
		}
		else
		{	  
			if (d_item->text() == tr("在此创建数据库\n或在”选择数据库”\n选择"))
				continue;		
			QString db_where;
			if (d_item->parent() == db_model->item(0))
				db_where = "/home/dapang/workstation/spc-tablet/qlitedb/"+d_item->text();
			else
				db_where = "/home/dapang/workstation/spc-tablet/qsqlitebkdb/"+d_item->text();				
			QString old_db;
			if (inspector->currentConnectionDb())
			{
				old_db = inspector->currentConnectionDb()->connectionName();	
				*inspector->currentConnectionDb() = QSqlDatabase::addDatabase("QSQLITE", d_item->text());
				inspector->currentConnectionDb()->setDatabaseName(db_where);
				if (!inspector->currentConnectionDb()->open())
				{
					act_result = tr("新数据库")+d_item->text()+tr("打开失败");	
					QSqlDatabase::database(d_item->text()).close();							
					*inspector->currentConnectionDb() = QSqlDatabase::database(old_db);						
					QSqlDatabase::removeDatabase(d_item->text());
					return false;						
				}
				if (!inspector->createParasTable(*inspector->currentConnectionDb()))
				{
					act_result = tr("新数据库")+d_item->text()+tr("创建参数表失败");
					QSqlDatabase::database(d_item->text()).close();					
					*inspector->currentConnectionDb() = QSqlDatabase::database(old_db);						
					QSqlDatabase::removeDatabase(d_item->text());
					return false;				
				}					
			}
			else
			{
				if (!inspector->dbConnection(db_where))
				{
					act_result = tr("新数据库")+d_item->text()+tr("创建失败");
					inspector -> closeCurDB();
					return false;				
				}
			}
			QStringList dbm_list;
			dbm_list << d_item->text() << using_name.first+","+using_name.second << c_time.toString();
			if (!inspector->initFillForDbProperty(dbm_list, inspector->currentConnectionDb()))
			{
				act_result = tr("新数据库")+d_item->text()+tr("创建标识失败");
				if (old_db.isEmpty())
					inspector -> closeCurDB();
				else
				{
					QSqlDatabase::database(d_item->text()).close();					  
					*inspector->currentConnectionDb() = QSqlDatabase::database(old_db);						
					QSqlDatabase::removeDatabase(d_item->text());
				}
				return false;
			}				
			if (old_db.isEmpty())
				inspector -> closeCurDB();
			else
			{
				QSqlDatabase::database(d_item->text()).close();				  
				*inspector->currentConnectionDb() = QSqlDatabase::database(old_db);
			}
			c_time.addMSecs(200);
		}			
		QSqlDatabase::removeDatabase(d_item->text());			
	}
	int i_again = 0;
	QHashIterator<QStandardItem *, QString> u_hash(db_model->unshowHashItems());	
	while (u_hash.hasNext())
	{
		u_hash.next();
		i_again++;
		save_following.setValue(save_following.value()+i_again);
		if (origin_hash.contains(u_hash.key()))
		{
			QString old_db;
			if (u_hash.value().contains(tr("合并入")))//maybe need promotion
			{
				QString merge_name = origin_hash.value(u_hash.key()).second;							  
				if (!inspector->dbConnection(merge_name))
				{
					act_result = u_hash.key()->text()+tr("打开失败");
					return false;
				}				
				QStringList find_db = u_hash.value().split(";");
				QString todb_description;
				foreach (QStandardItem * fdb_item, origin_hash.keys())
				{
					if (QString::number((long)fdb_item, 16) == find_db.at(1))
					{
						todb_description = origin_hash.value(fdb_item).second;
						break;
					}
				}
				if (todb_description.isEmpty())
				{	
					if (QString::number((long)db_model->item(0)->child(0), 16) == find_db.at(1))
						todb_description = "/home/dapang/workstation/spc-tablet/qlitedb/"+db_model->item(0)->child(0)->text();
					else
					{
						int i = 0;
						while (db_model->item(1)->child(i))
						{
							if (QString::number((long)db_model->item(1)->child(i), 16) == find_db.at(1))
							{
								todb_description = "/home/dapang/workstation/spc-tablet/qsqlitebkdb/"+db_model->item(1)->child(i)->text();
								break;
							}
							i++;
						}
					}
				}				
				*inspector->currentConnectionDb() = QSqlDatabase::addDatabase("QSQLITE", todb_description.split("/").back());
				inspector->currentConnectionDb()->setDatabaseName(todb_description);				  
				int i_open = 0;
				while (!inspector->currentConnectionDb()->open())
				{
					if (i_open == 3)
					{
						act_result = todb_description+tr("打开三次失败");
						return false;
					}
					i_open++;
				}
				QString merging;
				*inspector->currentConnectionDb() = QSqlDatabase::database(merge_name.split("/").back());
				if (!inspector->mergeActionForDbs(todb_description.split("/").back(), merging))
				{
					act_result = tr("合并")+u_hash.key()->text()+merging;
					QSqlDatabase::database(todb_description.split("/").back()).close();
					QSqlDatabase::removeDatabase(todb_description.split("/").back());
					inspector -> closeCurDB();
					return false;
				}
				inspector -> closeCurDB();
				QSqlDatabase::database(todb_description.split("/").back()).close();
				QSqlDatabase::removeDatabase(todb_description.split("/").back());
			}
			else
			{
				QStringList detail_name = origin_hash.value(u_hash.key()).second.split("/");
				QSqlDatabase::database(detail_name.back()).close();	
				QSqlDatabase::removeDatabase(origin_hash.value(u_hash.key()).second);
				if (!inspector->deleteDatabase(origin_hash.value(u_hash.key()).second))
				{
					act_result = u_hash.key()->text()+tr("删除失败");					
					return false;
				}				
			}			
		}
		else//???
		{
			QStandardItem * d_item = u_hash.key();
			delete d_item;				
		}		
	}		
	QDir related_dir; 
	related_dir.setFilter(QDir::Files);	  
	related_dir.setPath("/home/dapang/workstation/spc-tablet/qlitedb");
	QFileInfoList qfi_list = related_dir.entryInfoList();
	if (!qfi_list.isEmpty())
	{
		QStringList file_details = qfi_list[0].path().split("/");
		if (inspector->currentConnectionDb() && inspector->currentConnectionDb()->connectionName()!=file_details.back())
			inspector -> closeCurDB();
		emit secondOpenDatabase(qfi_list[0].filePath());
	}
	save_following.setValue(all_items.size()+db_model->unshowHashItems().size());
	return true;
}

int RelatedTreeView::treeType()
{
	return t_type;
}

const QPair<QString, QString> & RelatedTreeView::usingPair()
{
	return using_name;
}

EditLongToolsViewer * RelatedTreeView::qmlViewer()
{
	return cons_viewer;
}

QStandardItem * RelatedTreeView::specialClickedItem()
{
	return special_clicked;
}

QUndoStack * RelatedTreeView::commandStack()
{
	return m_commandStack;
}

SpcDataBase * RelatedTreeView::databaseSource()
{
	return inspector;
}

const QRect & RelatedTreeView::originShowRect()
{
	return origin_rect;
}

QHash<QStandardItem *, QStringList> & RelatedTreeView::itemTextsHash()
{
	return texts_hash;
}

void RelatedTreeView::setEmpowerState(bool set_state)
{
	empower_state = set_state;
}

void RelatedTreeView::replyForSelectedStrs(int s_num)
{
	NestedModel * view_model = qobject_cast<NestedModel *>(model());
	QStandardItem * seled_item = cons_viewer->currentNesttingItem();
	QString s_name = texts_hash.value(seled_item).at(s_num);
	if (t_type == 0)
	{
		if (cons_viewer->qmlDelegateType() == 10)
		{
			if (s_name != tr("不操作"))
			{
				if (s_name==tr("添加授权代表") && edit_order.isEmpty())
				{
					QList<QStandardItem *> hash_keys = texts_hash.keys();
					QStandardItem * in_hash = 0;
					foreach (QStandardItem * k_item, hash_keys)
					{
						if (k_item->text() == tr("添加授权代表"))
						{
							in_hash = k_item;
							break;
						}
					}
					QStringList em_tol;
					QStringList em_tmp;
					if (in_hash)
						em_tol = texts_hash.value(in_hash);
					else
					{
						QMap<QString, QStringList> projs_info = inspector->curDBprojectTable();
						QString managers = projs_info.value(seled_item->parent()->text()).at(2);
						em_tol = managers.split(";");
						foreach (QString mng, em_tol)
						{
							QStringList splitted = mng.split(",");
							QPair<QString, QString> mng_pair(splitted.at(0), splitted.at(1));
							if (inspector->isConstructor(mng_pair, seled_item->parent()->text()))
							{
								em_tol.removeOne(mng);
								continue;
							}
							QStringList m_proj = inspector->curDBmanageTable().value(splitted.at(1)).at(3).split(";").filter(seled_item->parent()->text());
							if (!m_proj.at(0).contains(tr("读写")) || projs_info.value(seled_item->parent()->text()).at(3).contains(splitted.at(1)))
								em_tol.removeOne(mng);					
						}
						QList<QStandardItem *> cur_departs;
						view_model -> findChildrenNotAll(seled_item, cur_departs);
						foreach (QStandardItem * depart, cur_departs)
						{
							QList<QStandardItem *> cur_agents;
							view_model -> findChildrenNotAll(depart, cur_agents);
							foreach (QStandardItem * agent, cur_agents)
								view_model -> relistQmlViewerStrs(agent, em_tol);
						}
						in_hash = new QStandardItem(tr("添加授权代表"));
						texts_hash.insert(in_hash, em_tol);
					}
					QMap<QString, QStringList> managers_info = inspector->curDBmanageTable();
					em_tmp = em_tol;
					foreach (QString str, em_tmp)
					{
						QStringList splitted = str.split(",");
						QString m_deprt = managers_info[splitted[1]].at(2);
						QString rep_str = m_deprt+tr("：")+splitted[0];
						em_tmp.replace(em_tol.indexOf(str), rep_str);
					}
					if (!em_tmp.isEmpty())
					{
						edit_order = tr("授权代表选择");
						cons_viewer -> setNewStrList(em_tmp);	
						return;				
					}
				}
				else
					m_commandStack->push(new RelatedTreeCommand(this, seled_item, s_name));
			}
		}
		else
		{
			QString order = edit_order;
			edit_order.clear();	
			QList<QStandardItem *> key_items = texts_hash.keys();
			QStandardItem * key_item = 0;
			foreach (QStandardItem * item, key_items)
			{
				if (item->text() == tr("添加授权代表"))
				{
					key_item = item;
					break;
				}
			}
			s_name = texts_hash.value(key_item).at(s_num);
			m_commandStack->push(new RelatedTreeCommand(this, seled_item, order, s_name));		  
		}
	}	
	if (t_type == 1)
	{
		if (s_name != tr("不操作"))
		{
			QStandardItem * p_item = view_model->destParentItem(seled_item);
			QString order;
			if (p_item && p_item->parent()==view_model->item(0) && p_item->row()<4)
				order = tr("修改标题")+","+s_name;
			else
				order = s_name;
			m_commandStack->push(new RelatedTreeCommand(this, seled_item, order, seled_item->text()));
		}
	}
	else if (t_type == 2)
	{
		if (s_name != tr("不操作"))
		{
			if (seled_item->text()==tr("获授权限"))
			{
				QString old = seled_item->child(0)->text();
				m_commandStack->push(new RelatedTreeCommand(this, seled_item, s_name, old));	
			}
			else
				m_commandStack->push(new RelatedTreeCommand(this, seled_item, s_name));	
		}
	}
	else if (t_type == 4 || t_type == 5)
	{
		if (s_name != tr("不操作") && s_name != tr("修改标题"))
			m_commandStack->push(new RelatedTreeCommand(this, seled_item, s_name));
		if (s_name == tr("修改工程名称") || s_name == tr("修改标题"))
		{
			connect(view_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(activedItemChanged(QStandardItem *)));
			edit_order = s_name;
			cell_oldText = seled_item->text();
			emit itemEditting(seled_item->index());
		}		
	}	
	else if (t_type == 6)
	{
		if (cons_viewer->qmlDelegateType() == 11)
		{
			QString push_name = tr("工程表图选择")+cons_viewer->rootContext()->contextProperty("StringesList").toList().at(s_num).toString();	
			m_commandStack->push(new RelatedTreeCommand(this, seled_item, push_name, seled_item->text()));
		}
		else
		{
			if (s_name!=tr("不操作") && s_name!=tr("修改标题"))
			{
				if (s_name==tr("工程选择") || s_name==tr("表图选择"))
				{
					if (s_name == tr("工程选择"))
					{
						QStringList projs = inspector->curDBprojectTable().keys();
						cons_viewer -> setNewStrList(projs);
					}
					else
					{
						QStringList all_tables = inspector->allTables();
						QStringList selecttings;
						foreach (QString n_str, all_tables)
						{
							if (n_str.contains(tr("，，。plots")) || n_str.contains(tr("，，。manualtable")))
							{
								QStringList tol_name = n_str.split(tr("，，。"));
								selecttings << tol_name.back();
							}
						}
						if (!selecttings.isEmpty())
							cons_viewer -> setNewStrList(selecttings);
					}
					return;
				}
				else
					m_commandStack->push(new RelatedTreeCommand(this, seled_item, s_name));
			}
			if (s_name == tr("修改标题"))
			{
				connect(view_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(activedItemChanged(QStandardItem *)));
				edit_order = s_name;
				cell_oldText = seled_item->text();
				emit itemEditting(seled_item->index());
			}
		}
	}
	else if (t_type == 7)
	{		
		if (s_name == tr("编辑数据库名"))
		{
			connect(view_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(activedItemChanged(QStandardItem *)));
			edit_order = s_name;		  
			cell_oldText = seled_item->text();
			emit itemEditting(seled_item->index());			
		}
		else if (s_name != tr("不操作"))
		{
			if (s_name == tr("合并入当前数据库"))
			{
				MainWindow * mw = qobject_cast<MainWindow *>(inspector->parent()->parent());			  
				QString open_name = view_model->originHashItems().value(seled_item).second;
				if (!inspector->dbConnection(open_name))
				{
					DiaLog * op_dialog = new DiaLog;
					op_dialog -> initWinDialog(mw, QString(tr("warning:数据库未打开\n请再试一次")), 0); 	 
					return;
				}
				if (inspector->curDBprojectTable().isEmpty())
				{
					DiaLog * mg_dialog = new DiaLog;
					mg_dialog -> initWinDialog(mw, QString(tr("hint:合并的数据库内无工程\n合并无意义")), 0); 	  
					inspector -> closeCurDB();
					return;				  
				}				
				view_model->ensureConfilctionForDbsMerging(view_model->item(0)->child(0));
				qDebug() << inspector->tmpProjsHashForDbsConfliction() << inspector->tmpMngsHashForDbsConfliction();
				if (!inspector->tmpProjsHashForDbsConfliction().isEmpty() || !inspector->tmpMngsHashForDbsConfliction().isEmpty())
				{
					DiaLog * rt_dialog = new DiaLog;
					rt_dialog -> showForCollisionDbsDataes(inspector->tmpProjsHashForDbsConfliction(), inspector->tmpMngsHashForDbsConfliction(), mw); 	  
					inspector -> closeCurDB();
					return;
				}
				inspector -> closeCurDB();
			}
			m_commandStack->push(new RelatedTreeCommand(this, seled_item, s_name));	
		}
	}
	closeNestedQmlViewerOnCell(cons_viewer);	
}

void RelatedTreeView::listenFreezingState(bool freezing)
{
	emit freezingNestCell(freezing);
}

void RelatedTreeView::activedItemChanged(QStandardItem * a_item)
{
	if ((edit_order==tr("修改标题") || edit_order==tr("修改工程名称")) && a_item->text()!=cell_oldText)
	{
		if (t_type == 1)
		{
			NestedModel * view_model = qobject_cast<NestedModel *>(model());
			QList<QStandardItem *> c_siblings;
			view_model -> findChildrenNotAll(view_model->item(0), c_siblings);  
			QPair<QString, QString> t_pair(c_siblings.at(0)->child(0)->text(), c_siblings.at(1)->child(0)->text());
			if (inspector->theSamePasswdIndb(c_siblings.at(1)->child(0)->text()) && !inspector->theSameNameAndPasswdInManageList(t_pair) && !inspector->theSameNameWaitingPower(t_pair))
			{
				a_item->setText(tr("在此填写密码"));
				edit_order.clear();
				cell_oldText.clear();			
				return;
			}
		}
		m_commandStack->push(new RelatedTreeCommand(this, a_item, edit_order, cell_oldText));	
	}
	else if (edit_order==tr("编辑数据库名") && a_item->text()!=cell_oldText)
	{
		QStringList content = a_item->text().split(".db");
		if (content.size() == 1)
			a_item -> setText(a_item->text()+".db");
		m_commandStack->push(new RelatedTreeCommand(this, a_item, edit_order, cell_oldText));	
	}
	edit_order.clear();
	cell_oldText.clear();		
}

void RelatedTreeView::cellClicked(const QModelIndex & index)
{
	if (!edit_free || t_type==3)
		return;
	int time_passed = 0;
	if (press_time)
	{
		time_passed = press_time->elapsed();
		delete press_time;
		press_time = 0;			
	}
	NestedModel * view_model = qobject_cast<NestedModel *>(model());	
	QStandardItem * clicked_item = view_model->itemFromIndex(index);
	QStandardItem * fill_root = view_model->destParentItem(clicked_item);	
	if (t_type!=1 && t_type!=2 && t_type!=6 && t_type!=7)
	{
		QString c_proj;
		if (!fill_root->parent())
			c_proj = fill_root->text();
		else
			c_proj = fill_root->parent()->text();
		QStringList m_proj = inspector->curDBmanageTable().value(using_name.second).at(3).split(";").filter(c_proj);
		if (!inspector->isConstructor(using_name, c_proj) || !m_proj.contains(c_proj) || (!inspector->isConstructor(using_name, c_proj) && m_proj.contains(c_proj) && !m_proj.at(0).contains(tr("读写"))))
			return;
	}	
	if (t_type == 0)
	{
		if (!clicked_item->parent())	
			return;		
		if (fill_root->row()!=0 || clicked_item->parent()==fill_root)
			return;
		QString c_proj = fill_root->parent()->text();
		QString authes = inspector->curDBprojectTable().value(c_proj).at(3);
		if (!authes.contains(using_name.second))
			return;
	}
	else if (t_type == 1)
	{		
		if (view_model->item(0)==clicked_item || view_model->item(1)==clicked_item)
			return;	  
		if ((fill_root==clicked_item && fill_root->parent()==view_model->item(0)) || fill_root->row()==4)
			return;
		if (clicked_item->parent()==fill_root && fill_root->row()<4)
		{
			if (fill_root->hasChildren() && fill_root->row()>1)
			{
				QList<QStandardItem *> children_items;
				view_model -> findChildrenNotAll(view_model->item(0), children_items);
				if (!children_items[2]->child(0)->text().contains(tr("在此填写")) && !children_items[3]->child(0)->text().contains(tr("在此填写")))
					return;
			}		
			if (using_name.second.isEmpty() && (clicked_item->text()==tr("在此填写新部门名称") || fill_root->row()<2))
			{
				connect(view_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(activedItemChanged(QStandardItem *)));				
				cell_oldText = clicked_item->text();
				edit_order = tr("修改标题");
				emit itemEditting(index);
				return;				
			}
		}
	}
	else if (t_type == 2)
	{
		if (view_model->item(0)==clicked_item || view_model->item(1)==clicked_item)
			return;	
		if (fill_root==clicked_item || (clicked_item->parent()==fill_root && clicked_item->text()==tr("获授权者")))
			return;
		if (clicked_item->parent()==fill_root && fill_root->parent()==view_model->item(0))
		{
			if (clicked_item->text().contains(tr("在此填写")) || clicked_item->text().contains(tr("在此填写")))
			{
				connect(view_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(activedItemChanged(QStandardItem *)));
				cell_oldText = clicked_item->text();
				edit_order = tr("修改标题");
				emit itemEditting(index);
			}
			return;			
		}		
	}	
	else if (t_type == 5)
	{	  
		if (clicked_item->parent() && !clicked_item->parent()->parent() && clicked_item->row()==0)
			return;
	}
	else if (t_type==7 && !clicked_item->parent())
		return;	
	if (time_passed > 500)	  
		initStrListViewer(view_model, index);
	else
		return;
	cell_oldText = model()->data(index).toString();	
}

void RelatedTreeView::expandedToResizeQmlView(const QModelIndex & e_index)
{
	NestedModel * t_model = qobject_cast<NestedModel *>(model());	
	QStandardItem * last_item = t_model->item(t_model->rowCount()-1);
	QRect pos_rect;	
	QList<QStandardItem *> children_items;
	t_model -> findAllChildItems(last_item, children_items);
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

void RelatedTreeView::collapsedToResizeQmlView(const QModelIndex & c_index)
{
	Q_UNUSED(c_index);
	NestedModel * t_model = qobject_cast<NestedModel *>(model());
	QStandardItem * last_item = t_model->item(t_model->rowCount()-1);
	QStandardItem * last_child = 0;
	if (last_item->hasChildren())
	{
		QList<QStandardItem *> children_items;
		t_model -> findAllChildItems(last_item, children_items);
		if (children_items[children_items.size()-1]->parent())//need promotion for end lastitem
		{
			QStandardItem * c_parent = children_items[children_items.size()-1]->parent();
			if (isExpanded(c_parent->index()))
				last_child = children_items[children_items.size()-1];
			else
				last_child = c_parent;
		}
	}
	else
		last_child = last_item;
	QRect pos_rect = visualRect(last_child->index());
	int end_geometry = pos_rect.y()+pos_rect.height();
	if (end_geometry > origin_rect.height())
	{
		setFixedHeight(end_geometry);
		emit sizeChanged();
	}
	else
	{
		setFixedHeight(origin_rect.height());
		emit sizeChanged();
	}		
}

void RelatedTreeView::clearQmlViewStrsPt(QWidget * clear_pt)
{
	Q_UNUSED(clear_pt);
	cons_viewer = 0;
}

void RelatedTreeView::clearNestedAnimation()
{
	if (cons_viewer)
		cons_viewer -> setFixedWidth(nested_animation->endValue().toRect().width());
	nested_animation -> deleteLater();
	nested_animation = 0;
	update();
}

void RelatedTreeView::closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint)
{
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	QTreeView::closeEditor(editor, hint);	
}

void RelatedTreeView::mousePressEvent(QMouseEvent * event)
{
	QTreeView::mousePressEvent(event);
 	NestedModel * n_model = qobject_cast<NestedModel *>(model()); 
	special_clicked = n_model->itemFromIndex(indexAt(event->pos()));
	press_time = new QTime;
	press_time -> start();	
}

void RelatedTreeView::mouseReleaseEvent(QMouseEvent * event)
{
	QTreeView::mouseReleaseEvent(event);
	if (edit_free && t_type==6 && model()->rowCount()==0 && press_time->elapsed()>500)
	{
		delete press_time;
		press_time = 0;
		if (cons_viewer)
			return;
		cons_viewer = new EditLongToolsViewer(this);
		connect(cons_viewer, SIGNAL(killMyself(QWidget *)), this, SLOT(clearQmlViewStrsPt(QWidget *)));
		QDeclarativeContext * ctxt = cons_viewer->rootContext();
		cons_viewer -> setOrientation(EditLongToolsViewer::ScreenOrientationLockLandscape);
		QStringList val_list;
		val_list << tr("不操作") << tr("建立新项");	
		ctxt -> setContextProperty("StringesList", QVariant::fromValue(val_list));
		cons_viewer -> setMainQmlFile(QLatin1String("spc_qml/slideSelector.qml"), 10);
		cons_viewer -> setOtherEditingTable(this);
		QStandardItem * origin_item = new QStandardItem(tr("请修改为您想添加的"));
		cons_viewer -> setNestedPosItem(origin_item);
		texts_hash.insert(origin_item, val_list);
		QFontMetrics fm(font());
		int height = fm.height()*5;
		QString empty;		
		int c_width = biggerWidthBetweenTextList(fm, val_list, empty)*1.2;
		cons_viewer -> setGeometry(event->pos().x(), event->pos().y(), c_width, height);
		openNestedQmlViewerOnCell(cons_viewer, c_width, height);			
	}  
}

void RelatedTreeView::stringlistMinus(QStringList & tol_list, const QStringList & m_list, bool equal)
{
	if (equal)
	{
		for (int i = 0; i < m_list.size(); i++)
			tol_list.removeAll(m_list[i]);
	}
	else
	{
		for (int i = 0; i < m_list.size(); i++)// const ?
		{
			QStringList fur_list = m_list[i].split(",");
			if (fur_list.size() > 1)
				tol_list.removeAll(fur_list[0]);
			else
				tol_list.removeAll(m_list[i]);
		}
	}
}

void RelatedTreeView::fillQmlViewerStrs(QStringList & fill_list)
{
	QString depart;
	for (int i = 3; i < model()->rowCount(); i++)
	{
		if (model()->headerData(i, Qt::Vertical).toString()==tr("管理部门"))
		{
			if (depart != model()->data(model()->index(i, 0)).toString())
				depart = model()->data(model()->index(i, 0)).toString();
		}
		else if (model()->headerData(i, Qt::Vertical).toString()==tr("经理"))
		{
			QStringList test_list = model()->data(model()->index(i, 0)).toString().split(" ");
			if (test_list.size() > 1)
				continue;
			if (model()->data(model()->index(i+1, 1)).toString() == tr("授权代表"))
				continue;
			QString m_title = depart+tr("经理")+" "+model()->data(model()->index(i, 0)).toString();;
			fill_list << m_title;	
		}
		else if (model()->headerData(i, Qt::Vertical).toString()==tr("工程师"))
		{
			QStringList test_list = model()->data(model()->index(i, 0)).toString().split(" ");
			if (test_list.size() > 1)
				continue;
			if (model()->data(model()->index(i+1, 1)).toString() == tr("授权代表"))
				continue;
			QString e_title = depart+tr("工程师")+" "+model()->data(model()->index(i, 0)).toString();;
			fill_list << e_title;	
		}
		else
			continue;
	}
}

void RelatedTreeView::openNestedQmlViewerOnCell(EditLongToolsViewer * nesting, int end_width, int end_height)
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
	nesting -> showExpanded();
	nested_animation -> start();	
}

bool RelatedTreeView::judgeToAddNewManagerInfo(const QString & project, QStringList & new_managers)
{
	QString proj_name = "name="+project;
	QStringList m_list = inspector->curDBmanageTable().keys();
	QStringList d_list = inspector->curDBdepartTable().keys();
	for (int i = 0; i < new_managers.size(); i++)
	{
		QStringList n_managers = new_managers[i].split(",");
		if (m_list.contains(n_managers[1]))
		{
			QString passwd_key = "password="+n_managers[1];
			QString name_key = "name="+project;
			QString save_data1 = ";"+project+","+n_managers[4];
			QString cur_ds = inspector->curDBprojectTable().value(project).at(1);
			QString save_data2 = ";"+n_managers[0]+","+n_managers[1];
			if (!inspector->storeData("managerinfo", passwd_key, "projects", save_data1))
				return false;
			QString cur_ps = inspector->curDBmanageTable().value(n_managers[1]).at(3)+";"+project+","+n_managers[4];
			inspector->curDBmanageTable()[n_managers[1]][3] = cur_ps;
			if (!cur_ds.contains(n_managers[2]))
			{
				QString new_depart = ","+n_managers[2];
				if (!inspector->storeData("projectsinfo", name_key, "departments", new_depart))
					return false;
				QString cur_pds = inspector->curDBprojectTable().value(project).at(1)+","+n_managers[2];
				inspector->curDBprojectTable()[project][1] = cur_pds;
			}
			if (!inspector->storeData("projectsinfo", name_key, "managers", save_data2))
				return false;
			QString old_pms = inspector->curDBprojectTable().value(project).at(2)+";"+n_managers[0]+","+n_managers[1];
			inspector->curDBprojectTable()[project][2] = old_pms;
			QString old_dps = inspector->curDBdepartTable().value(n_managers[2]).at(1);
			if (!old_dps.contains(project))
			{
				QString dpart_name = "name="+n_managers[2];
				QString new_proj = ","+project;
				if (!inspector->storeData("departments", dpart_name, "projects", new_proj))
					return false;
				old_dps += ","+project;
				inspector->curDBdepartTable()[n_managers[2]][1] = old_dps;				
			}	
		}
		else
		{
			QStringList new_manager;
			new_manager << n_managers[0] << n_managers[1] << n_managers[2] << project + "," + n_managers[4] << n_managers[3];
			if (!inspector->storeManageInfoTable(new_manager))
				return false;
			QString new_depart = inspector->curDBprojectTable().value(project).at(1);
			if (!new_depart.contains(n_managers[2]))
			{
				QString store_depart = ","+n_managers[2];
				if (!inspector->storeData("projectsinfo", proj_name, "departments", store_depart))
					return false;
				new_depart += store_depart;
				inspector->curDBprojectTable()[project][1] = new_depart;
			}
			QString proj_newm = ";"+n_managers[0]+","+n_managers[1];
			if (!inspector->storeData("projectsinfo", proj_name, "managers", proj_newm))
				return false;
			QString new_managers = inspector->curDBprojectTable().value(project).at(2)+proj_newm;
			inspector->curDBprojectTable()[project][2] = new_managers;
			if (!inspector->existedDepartmentIndb(n_managers[2]))
			{				
				QString d_managers = n_managers[0] + "," + n_managers[1];
				QStringList new_dlist;
				new_dlist << n_managers[2] << project << d_managers;
				if (!inspector->storeDepartmentTable(new_dlist))
					return false;
				inspector->curDBdepartTable().insert(n_managers[2], new_dlist);
			}
			else
			{
				QString dp_str = inspector->curDBdepartTable().value(n_managers[2]).at(1);
				QString d_rname = "name=" + n_managers[2];
				if (!dp_str.contains(project))
				{
					QString store_proj = "," + project;
					if (!inspector->storeData("departments", d_rname, "projects", store_proj))
						return false;
					dp_str += "," + project;
					inspector->curDBdepartTable()[n_managers[2]][1] = dp_str;
				}
				QString dm_str = inspector->curDBdepartTable().value(n_managers[2]).at(2);
				if (!dm_str.contains(n_managers[1]))
				{
					QString new_dmanager = ";" + n_managers[0] + "," + n_managers[1];
					if (!inspector->storeData("departments", d_rname, "managers", new_dmanager))
						return false;
					dm_str += ";" + n_managers[0] + "," + n_managers[1];
					inspector->curDBdepartTable()[n_managers[2]][2] = dm_str;
				}
			}
		}		
	}
	if (!inspector->storeData("projectsinfo", proj_name, "authorizing", "null"))
		return false;
	inspector->curDBprojectTable()[project][3] = "";//position is not correct?
	return true;
}

bool RelatedTreeView::judgeToRemoveManagerForProj(const QString & project, QStringList & del_managers)
{
	QString key_name = "name="+project;
	QString constructor = inspector->dataFromTable("projectskeys", "passwd", key_name).toString();
	QString merge_str = del_managers.join(";");
	QMapIterator<QString, QStringList> m_map1(inspector->curDBmanageTable());
	while (m_map1.hasNext())
	{
		m_map1.next();
		if (merge_str.contains(m_map1.key()))
		{
			QString passwd_which = "password="+m_map1.key();
			QString ps_str = m_map1.value().at(3);
			QStringList mps_list = ps_str.split(";");
			QStringList del_p = mps_list.filter(project);
			mps_list.removeAll(del_p[0]);
			if (mps_list.isEmpty())
			{
				QString m_col = "";
				if (!inspector->deleteDataes("managerinfo", passwd_which, m_col))
					return false;
				QMapIterator<QString, QStringList> d_map(inspector->curDBdepartTable());
				while (d_map.hasNext())
				{
					d_map.next();
					if (d_map.value().at(2).contains(m_map1.key()))
					{
						QString change_dstr = d_map.value().at(2);
						QStringList d2_list = change_dstr.split(";");
						QStringList d2f_list = d2_list.filter(m_map1.key());
						d2_list.removeAll(d2f_list[0]);
						QString d_row = "name="+d_map.key();
						if (d2_list.isEmpty())
						{
							QString null_dcol = "";
							if (!inspector->deleteDataes("departments", d_row, null_dcol))
								return false;
							inspector->curDBdepartTable().remove(d_map.key());
						}
						else
						{
							change_dstr = d2_list.join(";");
							if (!inspector->updateCellData("departments", d_row, "managers", change_dstr))
								return false;
							inspector->curDBdepartTable()[d_map.key()][2] = change_dstr;
						}
					}
//					break;
				}
				inspector->curDBmanageTable().remove(m_map1.key());
			}
			else
			{
				ps_str = mps_list.join(";");
				if (!inspector->updateCellData("managerinfo", passwd_which, "projects", ps_str))
					return false;
				inspector->curDBmanageTable()[m_map1.key()][3] = ps_str;
			}
		}
	}
	QStringList newm_dlist;
	QMapIterator<QString, QStringList> m_map2(inspector->curDBmanageTable());
	while (m_map2.hasNext())
	{
		m_map2.next();
		if (m_map2.value().at(3).contains(project))
			newm_dlist << m_map2.value().at(2);
	}
	QStringList tol_dlist = inspector->curDBdepartTable().keys();
	stringlistMinus(tol_dlist, newm_dlist, true);
	for (int i = 0; i < tol_dlist.size(); i++)
	{
		if (inspector->curDBdepartTable().value(tol_dlist[i]).at(1).contains(project))
		{
			QString delp_dstr = inspector->curDBdepartTable().value(tol_dlist[i]).at(1);
			QStringList delp_dlist = delp_dstr.split(",");
			delp_dlist.removeAll(project);
			QString update_drow = "name="+tol_dlist[i];
			if (delp_dlist.isEmpty())
			{
				QString d_nullcol = "";
				if (!inspector->deleteDataes("departments", update_drow, d_nullcol))
					return false;
				inspector->curDBdepartTable().remove(tol_dlist[i]);
			}
			else
			{
				delp_dstr = delp_dlist.join(",");
				if (!inspector->updateCellData("departments", update_drow, "projects", delp_dstr))
					return false;
				inspector->curDBdepartTable()[tol_dlist[i]][1] = delp_dstr;
			}
		}
	}
	QStringList delm_list;
	for (int i = 0; i < del_managers.size(); i++)
	{
		QStringList key_list = del_managers[i].split(",");
		delm_list << key_list[0]+","+key_list[1];
	}
	QString p_dstr = inspector->curDBprojectTable().value(project).at(1);
	QStringList pd_list = p_dstr.split(",");
	newm_dlist.removeAll(pd_list[0]);
	newm_dlist.push_front(pd_list[0]);
	p_dstr = newm_dlist.join(",");
	QString pd_row = "name="+project;
	if (!inspector->updateCellData("projectsinfo", pd_row, "departments", p_dstr))
		return false;
	inspector->curDBprojectTable()[project][1] = p_dstr;
	QString p_mstr = inspector->curDBprojectTable().value(project).at(2);
	QStringList pm_list = p_mstr.split(";");
	stringlistMinus(pm_list, delm_list, true);	
	p_mstr = pm_list.join(";");
	if (!inspector->updateCellData("projectsinfo", pd_row, "managers", p_mstr))
		return false;
	inspector->curDBprojectTable()[project][2] = p_mstr;
	return true;
}

int RelatedTreeView::biggerWidthBetweenTextList(const QFontMetrics & fm, const QStringList & strs_list, const QString & str)
{	
	int max_listwidth = 0;
	foreach (QString w_str, strs_list)
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

int RelatedTreeView::maxCurrentWidth()
{
	NestedModel * show_model = qobject_cast<NestedModel *>(model());
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

QModelIndex RelatedTreeView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(cursorAction);
	Q_UNUSED(modifiers);
	return currentIndex();
}