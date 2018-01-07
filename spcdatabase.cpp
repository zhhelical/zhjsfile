#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQueryModel>
#include "spcnum.h"
#include "dataestableview.h"
#include "plotwidget.h"
#include "tablesgroup.h"
#include "tablemanagement.h"
#include "dataselectmodel.h"
#include "nestedmodel.h"
#include "littlewidgetsview.h"
#include "littlewidgetitem.h"
#include "plotwidget.h"
#include "mainwindow.h"
#include "spcdatabase.h"

SpcDataBase::SpcDataBase(QObject * guide)
:QObject(guide), spc_sqldb(0)
{
	inspector = qobject_cast<SpcNum *>(guide);
}

SpcDataBase::~SpcDataBase()
{
	closeCurDB();
}

void SpcDataBase::closeCurDB()
{
	QString db_connection;
	if (spc_sqldb)
	{		
		db_connection = spc_sqldb->connectionName();	
		QSqlDatabase::database(db_connection).close();
		delete spc_sqldb;
		spc_sqldb = 0;
	}
	if (!db_connection.isEmpty())
		QSqlDatabase::removeDatabase(db_connection);		
}

void SpcDataBase::getDataesFromTable(const QString & tablename, const QString & varname1, QStringList & dataes_list, const QString & val1, const QString & varname2, const QString & val2)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));	
	if (!val1.isEmpty())
	{
		QSqlTableModel destvar_indb(this, spc_sqldb->database(spc_sqldb->connectionName()));
		varsModelFromTable(tablename, &destvar_indb);
		QString select_name = QString("%1='%2'").arg(varname1).arg(val1);
		destvar_indb.setFilter(select_name);
		for (int i = 0; i < destvar_indb.columnCount(); i++)
			dataes_list << destvar_indb.data(destvar_indb.index(0, i)).toString();
	}
	else
	{	
		if (varname2.isEmpty())
		{
			QString which_data = QString("select %1 from %2").arg(varname1).arg(tablename);	
			if (!query.prepare(which_data))
				return;
			query.exec();
			while (query.next()) 
			{
				QString var = query.value(0).toString();
				dataes_list << var;
			}		
		}
		else
		{
			QString which_data = QString("select %1 from %2 where %3 = '%4'").arg(varname1).arg(tablename).arg(varname2).arg(val2);	
			if (!query.prepare(which_data))
				return;			  
			query.exec();
			while (query.next()) 
			{
				QString var = query.value(0).toString();
				dataes_list << var;
			}			
		}
	}
}

void SpcDataBase::getProjsConstructores(QStringList & constructors)
{
	QStringList cons_times;
	getDataesFromTable("projectskeys", "constructtime", cons_times);
	foreach (QString time, cons_times)
	{
		QString t_entirely = QDateTime::fromString(time, Qt::ISODate).toString();
		QString row_pos("constructtime="+time);
		QStringList testors = dataFromTable("projectskeys", "constructor", row_pos).toString().split(tr("；"));
		QStringList real_con = testors.filter(t_entirely);
		constructors << real_con.at(0).split(tr("，")).back();
	}	
}

void SpcDataBase::constructorOwnedProjs(const QPair<QString, QString> & constructor, QStringList & p_owned)
{
	QSqlTableModel projs_indb(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("projectskeys", &projs_indb);
	QString merge_constructor(constructor.first+tr("，")+constructor.second);
	for (int i = 0; i < projs_indb.rowCount(); i++)
	{
		QString proj_time = projs_indb.data(projs_indb.index(i, projs_indb.columnCount()-1)).toDateTime().toString();
		QStringList f_constructor = projs_indb.data(projs_indb.index(i, projs_indb.columnCount()-2)).toString().split(tr("；"));
		QStringList judge_constructor = f_constructor.filter(proj_time);
		if (judge_constructor.at(0).contains(merge_constructor))
			p_owned.push_back(projs_indb.data(projs_indb.index(i, 0)).toString());
	}
}

void SpcDataBase::managerOwnedProjs(const QPair<QString, QString> & manager, QStringList & proj_list, const QString & dest_db)
{
	QStringList crude_projs;
	if (dest_db.isEmpty())
	{
		QMapIterator<QString, QStringList> i_map(db_manageList);
		while (i_map.hasNext())
		{
			i_map.next();
			if (i_map.key() == manager.second)
			{
				crude_projs = i_map.value().at(3).split(";");
				break;
			}
		}
	}
	else
	{
		QSqlTableModel db_model(this, spc_sqldb->database(spc_sqldb->connectionName()));
		varsModelFromTable("dbInformations", &db_model);
		QString this_hint = db_model.data(db_model.index(0, 2)).toString()+","+manager.second;
		QString find_lbl;
		findMngPasswdsFromOutsideDb(dest_db, this_hint, find_lbl);
		QString origin_conn = spc_sqldb->connectionName();
		*spc_sqldb = QSqlDatabase::database(dest_db);
		QStringList mng_list;
		getDataesFromTable("managerinfo", "password", mng_list, find_lbl);
		crude_projs = mng_list.at(3).split(";");		
		*spc_sqldb = QSqlDatabase::database(origin_conn);
	}
	foreach (QString p_str, crude_projs)
	{
		QString proj_alone = p_str;
		if (p_str.contains(","))
			proj_alone = p_str.split(",").at(0);
		proj_list << proj_alone;
	}	
}

void SpcDataBase::agentOwnedProjs(const QPair<QString, QString> & agent, QStringList & proj_list, const QString & dest_db)
{
 	if (dest_db.isEmpty()) 
	{
		QMapIterator<QString, QStringList> p_map(db_projectList);
		while (p_map.hasNext())
		{
			p_map.next();
			if (p_map.value().at(3).contains(agent.second))
				proj_list << p_map.value().at(0);
		}
	}
	else
	{
		QString origin_conn = spc_sqldb->connectionName();	
		*spc_sqldb = QSqlDatabase::database(dest_db);		
		QSqlTableModel projs_model(this, spc_sqldb->database(spc_sqldb->connectionName()));
		varsModelFromTable("projectsinfo", &projs_model);
		for (int i = 0; i < projs_model.rowCount(); i++)
		{
			if (projs_model.data(projs_model.index(i, 3)).toString().contains(agent.second))
				proj_list << projs_model.data(projs_model.index(i, 0)).toString();
		}
		*spc_sqldb = QSqlDatabase::database(origin_conn);
	}	
}

void SpcDataBase::waitingPowerProjs(const QPair<QString, QString> & wait_pair, QStringList & w_projs)
{
	QMapIterator<QString, QStringList> i_map(db_projectList);
	while (i_map.hasNext())
	{
		i_map.next();
		QStringList judge_list = i_map.value();
		QStringList name_list = judge_list[5].split(";");
		foreach (QString c_str, name_list)
		{
			if (c_str.contains(wait_pair.second))
			{
				w_projs.push_back(i_map.key());
				break;
			}
		}
	}	
}

void SpcDataBase::waitingPowerManagers(const QString & project, QStringList & manager_list)
{
	QString managers = db_projectList.value(project).at(5);
	QStringList m_list;
	if (!managers.isEmpty())
	{
		m_list = managers.split(";");
		foreach (QString w_info, m_list)
		{
			QString new_name;
			QStringList name_list = w_info.split(",");
			if (name_list.size() == 2)
			{
				QStringList add_list = db_manageList.value(name_list.at(1));
				name_list << add_list.at(2) << add_list.at(4);
			}
			new_name = name_list.join(",");
			manager_list << new_name;
		}		
	}
}

void SpcDataBase::varsModelFromTable(const QString & tablename, QSqlTableModel * t_submit)
{			
	t_submit -> setTable(tablename);
	t_submit -> setEditStrategy(QSqlTableModel::OnManualSubmit);
	t_submit -> select();
}

void SpcDataBase::dataesByUserFromDb(const QString & person, const QString & proj, QSqlTableModel * t_submit)
{
	t_submit -> setTable(proj);
	t_submit -> setEditStrategy(QSqlTableModel::OnManualSubmit);
	t_submit -> select();
	QString fil_str = "person="+person;
	t_submit -> setFilter(fil_str);		
}

void SpcDataBase::dataesByProductFromDb(const QString & product, const QString & proj, int type, QSqlTableModel * t_submit)
{}	

void SpcDataBase::dataesByGroupsFromDb(int from, int to, const QString & proj, QSqlQueryModel * t_submit, bool equal, const QString & var, const QString & real_var)
{
	QString inte_range;	
	if (real_var == "")
		inte_range = QString("select * from %1 where groupnum between ? and ?").arg(proj);
	else
	{
		if (equal)			
			inte_range = QString("select * from %1 where (groupnum between ? and ?) and %2 = '%3'").arg(proj).arg(var).arg(real_var);
		else
			inte_range = QString("select * from %1 where (groupnum between ? and ?) and %2 like '%3'").arg(proj).arg(var).arg(real_var);
	}
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));	
	if (!query.prepare(inte_range))
		return;
	query.addBindValue(from);
	query.addBindValue(to);
	query.exec();
	t_submit -> setQuery(query);
}

void SpcDataBase::dataesByTimeVarFromDb(const QDateTime & from_time, const QDateTime & to_time, const QString & proj, QSqlQueryModel * t_submit, bool equal, const QString & var, const QString & real_var)
{
	QString time_range;
	if (real_var == "")
		time_range = QString("select * from %1 where time between ? and ?").arg(proj);
	else
	{
		if (equal)			
			time_range = QString("select * from %1 where (time between ? and ?) and %2 = '%3'").arg(proj).arg(var).arg(real_var);
		else
			time_range = QString("select * from %1 where (time between ? and ?) and %2 like '%3'").arg(proj).arg(var).arg(real_var);
	}
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));	
	if (!query.prepare(time_range))
		return;
	query.addBindValue(from_time);
	query.addBindValue(to_time);
	query.exec();
	t_submit -> setQuery(query);
}

void SpcDataBase::dataesByRelatedStringFromDb(const QString & proj, QSqlQueryModel * t_submit, const QString & var_str, const QString & real_str, const QString & var, const QString & real_var)
{
	QString str_range;
	if (real_var == "")
	{
		if (real_str == "all")
			str_range = QString("select * from %1").arg(proj);
		else
			str_range = QString("select * from %1 where %2 like '%3'").arg(proj).arg(var_str).arg(real_str);
	}
	else			
		str_range = QString("select * from %1 where %2 like '%3' and %4 = '%5'").arg(proj).arg(var_str).arg(real_str).arg(var).arg(real_var);
	QSqlQuery query(str_range, spc_sqldb->database(spc_sqldb->connectionName()));	
	if (!query.prepare(str_range))
		return;
	query.exec();
	t_submit -> setQuery(query);	
}

void SpcDataBase::dataSearchByCrudeTime(const QString & s_time, const QString & table, const QString & s_var, QString & dest_data)//need promotion
{
	QDate date = QDate::fromString(s_time, Qt::ISODate);
	QString which_data = QString("select %1 from %2 where time > ?").arg(s_var).arg(table);
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));	
	if (!query.prepare(which_data))
		return;
	query.addBindValue(date);
	query.exec();
	query.next();
	dest_data = query.value(0).toString();	
}

void SpcDataBase::selectDestPicture(const QString & p_name, QVariantList & pic)
{
	
}

void SpcDataBase::projectManagersIndb(const QString & proj_name, QList<QPair<QString, QString> > & proj_usrs, QSqlDatabase * dest_db)
{
	QStringList proj_infoes;
	if (!dest_db)
		proj_infoes = db_projectList.value(proj_name);
	else
		getDataesFromTable("projectsinfo", "name", proj_infoes, proj_name);		
	QStringList proj_mngs = proj_infoes.at(2).split(";");
	foreach (QString mng, proj_mngs)
	{
		QStringList mng_detail = mng.split(",");
		QPair<QString, QString> usr_pair(mng_detail.at(0), mng_detail.at(1));
		proj_usrs << usr_pair;
	}	
}

void SpcDataBase::chkUserInOtherDb(const QPair<QString, QString> cur_usr, const QString & other_db, QString & chk_result)
{
	QString ttime_pwds = db_manageList.value(cur_usr.second).at(1);
	if (mngExistedTwoDbs(ttime_pwds, other_db))
		return;
	QString chk_that;
	if (mngOwnedSameDbTime(ttime_pwds, chk_that, other_db))
		return;
	if (theSamePasswdIndb(cur_usr.second, other_db))
		chk_result = tr("密码需要修改");
}

void SpcDataBase::relatedTblsNameCollection(const QString & key_str, QStringList & collectings, QSqlDatabase * dest_db, const QString & fur_def)
{
	QStringList all_tbls;
	if (!dest_db)
		all_tbls = spc_sqldb->tables();
	else
		all_tbls = dest_db->tables();
	foreach (QString t_name, all_tbls)
	{
		if (fur_def.isEmpty())
		{
			if (t_name.contains(key_str))
				collectings << t_name;
		}
		else
		{
			if (t_name.contains(key_str) && t_name.contains(fur_def))
				collectings << t_name;		  
		}
	}
}

void SpcDataBase::findMngPasswdsFromOutsideDb(const QString & outside_db, const QString & find_hint, QString & find_result)
{
	QSqlTableModel db_model(this, QSqlDatabase::database(outside_db));
	varsModelFromTable("dbInformations", &db_model);
	QString fur_hint = db_model.data(db_model.index(0, 2)).toString();	
	QString origin_conn = spc_sqldb->connectionName();
	*spc_sqldb = QSqlDatabase::database(outside_db);
	QStringList ms_pswds;
	getDataesFromTable("managerinfo", "password", ms_pswds);
	foreach (QString pwd_str, ms_pswds)
	{	  
		if (pwd_str.contains(find_hint) && pwd_str.contains(fur_hint))
		{
			find_result = pwd_str;
			break;
		}
	}	
	*spc_sqldb = QSqlDatabase::database(origin_conn);
}

void SpcDataBase::resetManagersDbMap()
{
 	if (!db_manageList.isEmpty())
 		db_manageList.clear();
	QSqlTableModel p_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("managerinfo", &p_sql);
	if (p_sql.rowCount() == 0)
		return;
	QSqlTableModel db_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("dbInformations", &db_sql);
	for (int i = 0; i < p_sql.rowCount(); i++)
	{
		if (p_sql.data(p_sql.index(i, 0)).isNull() || p_sql.data(p_sql.index(i, 1)).isNull() || p_sql.data(p_sql.index(i, 2)).isNull() || p_sql.data(p_sql.index(i, 4)).isNull())
		{
			p_sql.removeRows(i, 1);
			p_sql.submitAll();
			continue;
		}	
		QStringList passwds = p_sql.data(p_sql.index(i, 1)).toString().split(";").filter(db_sql.data(db_sql.index(0, 2)).toString());
		QString passwd = passwds.at(0).split(",").at(1);
		QStringList infos;
		infos << p_sql.data(p_sql.index(i, 0)).toString();
		infos << p_sql.data(p_sql.index(i, 1)).toString();
		infos << p_sql.data(p_sql.index(i, 2)).toString();
		infos << p_sql.data(p_sql.index(i, 3)).toString();
		infos << p_sql.data(p_sql.index(i, 4)).toString();
		infos << p_sql.data(p_sql.index(i, 5)).toString();
		db_manageList.insert(passwd, infos);
	}	
}

void SpcDataBase::resetDepartmentsDbMap()
{
 	if (!db_departList.isEmpty())
		db_departList.clear(); 
	
	QSqlTableModel p_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("departments", &p_sql);
	if (p_sql.rowCount() == 0)
		return;
	for (int i = 0; i < p_sql.rowCount(); i++)
	{
		if (p_sql.data(p_sql.index(i, 0)).isNull() || p_sql.data(p_sql.index(i, 1)).isNull() || p_sql.data(p_sql.index(i, 2)).isNull())
		{
			p_sql.removeRows(i, 1);
			p_sql.submitAll();
			continue;
		}	  
		QString name = p_sql.data(p_sql.index(i, 0)).toString();
		QStringList infos;
		infos << p_sql.data(p_sql.index(i, 0)).toString();
		infos << p_sql.data(p_sql.index(i, 1)).toString();
		infos << p_sql.data(p_sql.index(i, 2)).toString();
		infos << p_sql.data(p_sql.index(i, 3)).toString();
		db_departList.insert(name, infos);
	}	
}

void SpcDataBase::resetProjsDbMap()
{
 	if (!db_projectList.isEmpty())
		db_projectList.clear(); 	
 	QSqlTableModel p_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("projectsinfo", &p_sql);
	if (p_sql.rowCount() == 0)
		return;	
	for (int i = 0; i < p_sql.rowCount(); i++)
	{
		if (p_sql.data(p_sql.index(i, 0)).isNull() || p_sql.data(p_sql.index(i, 1)).isNull() || p_sql.data(p_sql.index(i, 2)).isNull())
		{
			p_sql.removeRows(i, 1);
			p_sql.submitAll();
			continue;
		}	  
		QString name = p_sql.data(p_sql.index(i, 0)).toString();
		QStringList infos;
		infos << p_sql.data(p_sql.index(i, 0)).toString();
		infos << p_sql.data(p_sql.index(i, 1)).toString();
		infos << p_sql.data(p_sql.index(i, 2)).toString();
		infos << p_sql.data(p_sql.index(i, 3)).toString();
		infos << p_sql.data(p_sql.index(i, 4)).toString();
		infos << p_sql.data(p_sql.index(i, 5)).toString();
		db_projectList.insert(name, infos);
	} 
}

bool SpcDataBase::resetOpenDb(const QString & dbfile)
{
	QSqlDatabase * tmp_db = spc_sqldb;
	if (!dbConnection(dbfile))
	{
		spc_sqldb = tmp_db;
		return false;
	}
	tmp_db -> database(tmp_db->connectionName()).close();
	delete tmp_db;
	return true;
}

bool SpcDataBase::dbConnection(const QString & fileinfoes)
{
	QStringList db_path = fileinfoes.split("/");
	spc_sqldb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", db_path.back()));
	spc_sqldb -> setDatabaseName(fileinfoes);
	spc_sqldb -> setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE");
	if (!spc_sqldb -> open())
	{
		spc_sqldb -> setConnectOptions();
		delete spc_sqldb;
		spc_sqldb = 0;
		return false;
	}
	QStringList table_list = spc_sqldb->tables();
	qDebug() << table_list;
	if (table_list.empty())
	{
		if (!createParasTable(*spc_sqldb))
		{
			closeCurDB();
			QFile related_file("/"+db_path.join("/"));
			bool del = false;
			while (!related_file.remove() && !del)
				del = true; 
			return false;
		}
	}
	initPowerTablesList();	
	return true;
}

bool SpcDataBase::relationshipBetweenDbs(const QSqlDatabase & merged_db, QString & from_to)
{
	QSqlTableModel m_table(this, merged_db.database(merged_db.connectionName()));
	varsModelFromTable("dbInformations", &m_table);
	QSqlTableModel cur_dbtable(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("dbInformations", &cur_dbtable);
	QString bk_froms = m_table.data(m_table.index(0, 3)).toString();
	QString con_time = cur_dbtable.data(cur_dbtable.index(0, 2)).toString();	
	QStringList from_list = bk_froms.split(tr("，，。"));
	bool found = false;
	foreach (QString db_from, from_list)
	{
		if (db_from.contains(cur_dbtable.data(cur_dbtable.index(0, 0)).toString()))
		{
			QStringList detailed_infoes = db_from.split(tr("，，；"));
			if (con_time == detailed_infoes[1])
			{
				found = true;
				from_to = spc_sqldb->connectionName()+tr("，，；")+merged_db.connectionName();
				break;
			}
		}
	}
	if (!found)
	{
		QString bk_tos = m_table.data(m_table.index(0, 4)).toString();
		QStringList to_list = bk_tos.split(tr("，，。"));
		foreach (QString db_to, to_list)
		{
			if (db_to.contains(cur_dbtable.data(cur_dbtable.index(0, 0)).toString()))
			{
				QStringList detailed_infoes = db_to.split(tr("，，；"));
				if (con_time == detailed_infoes[1])
				{
					found = true;
					from_to = merged_db.connectionName()+tr("，，；")+spc_sqldb->connectionName();
					break;
				}
			}
		}		
	}
	return found;
}

bool SpcDataBase::canMergeDifferentDbs(const QPair<QString, QString> cur_operator, const QSqlDatabase & merged_db, QString & merge_result)
{
	QSqlTableModel db_model(this, merged_db.database(merged_db.connectionName()));
	varsModelFromTable("dbInformations", &db_model);
	QStringList get_constructor = db_model.data(db_model.index(0, 1)).toString().split(",");
	if (get_constructor.at(1) != cur_operator.second)
	{
		merge_result = tr("需要密码");
		return false;
	}
	return true;
}

bool SpcDataBase::mergeActionForDbs(const QString & merged_db, QString & merge_result)
{
	QStringList ts_tbls = spc_sqldb->tables();		
	QStringList ts_projs;
	getDataesFromTable("projectskeys", "name", ts_projs);	
	foreach (QString t_proj, ts_projs)
	{
		QString merge_proj = t_proj;
		if (projs_conflict.contains(merge_proj))
			merge_proj = projs_conflict.value(t_proj);
		QString can_merge;
		if (!canMergeProjBetweenDbs(merge_proj, can_merge, merged_db))
		{	  
			if (can_merge == tr("在数据库内备份过"))
				continue;			
		}
		if (can_merge == tr("无此工程"))
		{
			if (!replaceProjBetweenDbsAction(merge_proj, merged_db, merge_result))
				return false;		  
		}
		else
		{
			if (!mergePureProjBetweenDbs(merge_proj, merged_db, merge_result))
				return false;
		}
	}
	if (!mergeOtherTblsBetweenDb(merged_db, merge_result))
		return false;
	return true;
}

bool SpcDataBase::canMergeProjBetweenDbs(const QString & merged_proj, QString & can_merge, const QString & to_db)
{
	QStringList same_names;
	QString judge_name("name="+merged_proj);
	QDateTime time_indb = dataFromTable("projectskeys", "constructtime", judge_name).toDateTime();
	if (existedEntirelySameProject(merged_proj, time_indb, to_db))
	{
		can_merge = merged_proj+tr("在数据库内备份过");			
		return false;			  
	}
	if (existedProjectNameIndb(merged_proj, to_db))
	{	  
		if (existedSameProjectIndb(time_indb, to_db))
		{
			can_merge = tr("存在相同工程");				
			return true;
		}
		can_merge = merged_proj+tr("在数据库中同名");		
		return false;
	}
	can_merge = tr("无此工程");
	return true;
}

bool SpcDataBase::replaceProjBetweenDbsAction(const QString & this_proj, const QString & replaced_db, QString & replace_result)
{ 
 	QString origin_conn = spc_sqldb->connectionName(); 	
	QStringList this_keys;
	getDataesFromTable("projectskeys", "name", this_keys, this_proj);
	QStringList this_infoes;
	getDataesFromTable("projectsinfo", "name", this_infoes, this_proj);
	QSqlTableModel db_model(this, QSqlDatabase::database(origin_conn));
	QString db_tbl("dbInformations");
	varsModelFromTable(db_tbl, &db_model);
	QString replace_time = db_model.data(db_model.index(0, 2)).toString();	
	QSqlTableModel replaced_model(this, QSqlDatabase::database(replaced_db));
	varsModelFromTable(db_tbl, &replaced_model);
	QString replaced_time = replaced_model.data(replaced_model.index(0, 2)).toString();
	*spc_sqldb = QSqlDatabase::database(replaced_db);	
	QStringList replaced_infoes;		
	QString find_row("constructtime="+this_keys.at(10));
	QString replaced_proj = dataFromTable("projectskeys", "name", find_row).toString();	
	QStringList replaced_keys;	
	if (!replaced_proj.isEmpty())
	{
		getDataesFromTable("projectskeys", "constructtime", replaced_keys, this_keys.at(10));
		getDataesFromTable("projectsinfo", "name", replaced_infoes, replaced_proj);
		if (!deleteProject(replaced_proj))
		{
			replace_result = tr("删除替换")+replaced_proj+tr("失败");
			*spc_sqldb = QSqlDatabase::database(origin_conn);						
			return false;
		}		
	}
	QString is_pconflicted = findConflictedProj(replace_time, this_proj);
	if (is_pconflicted.isEmpty())
		replaced_proj = this_proj;
	else
	{
		replaced_proj = is_pconflicted;
		this_keys[0] = replaced_proj;
		this_infoes[0] = replaced_proj;						
	}
	QStringList chk_cflts = this_keys.at(this_keys.size()-2).split(tr("；"));
	foreach (QString chk_mng, chk_cflts)
	{
		QStringList f_pswds= chk_mng.split(tr("，"));
		QString is_mconflicted = findConflictedMngr(replace_time, f_pswds.back());
		if (!is_mconflicted.isEmpty())
		{
			f_pswds[f_pswds.size()-1] = is_mconflicted;
			chk_cflts[chk_cflts.indexOf(chk_mng)] = f_pswds.join(tr("，"));
		}
	}
	this_keys[this_keys.size()-2] = chk_cflts.join(tr("；"));
	chk_cflts = this_infoes.at(2).split(";");
	foreach (QString chk_mng, chk_cflts)
	{
		QStringList f_pswds= chk_mng.split(",");
		QString is_mconflicted = findConflictedMngr(replace_time, f_pswds.back());
		QString find_froms;
		findMngPasswdsFromOutsideDb(origin_conn, f_pswds.at(1), find_froms);
		bool chk_opwd = mngExistedTwoDbs(find_froms, replaced_db);
		QString chk_that;		
		if (!is_mconflicted.isEmpty())
		{
			f_pswds[1] = is_mconflicted;
			chk_cflts[chk_cflts.indexOf(chk_mng)] = f_pswds.join(",");
		}
		else if (!replaced_proj.isEmpty() && (chk_opwd || mngOwnedSameDbTime(find_froms, chk_that, replaced_db)))
		{
			if (chk_opwd) 
			{
				QString f_topwds;
				findMngPasswdsFromOutsideDb(replaced_db, f_pswds.at(1), f_topwds);
				f_pswds[1] = f_topwds.split(";").filter(replaced_time).at(0).split(",").at(1);
			}
			else
				f_pswds[1] = chk_that;
			chk_cflts[chk_cflts.indexOf(chk_mng)] = f_pswds.join(",");
		}		
	}
	this_infoes[2] = chk_cflts.join(";");
	chk_cflts.clear();
	if (!this_infoes.at(3).isEmpty())
		chk_cflts = this_infoes.at(3).split(";");
	foreach (QString chk_mng, chk_cflts)
	{
		QStringList f_pswds= chk_mng.split(",");
		QString is_mconflicted = findConflictedMngr(replace_time, f_pswds.back());
		QString find_froms;
		findMngPasswdsFromOutsideDb(origin_conn, f_pswds.at(1), find_froms);
		bool chk_opwd = mngExistedTwoDbs(find_froms, replaced_db);
		QString chk_that;
		if (!is_mconflicted.isEmpty())
		{
			f_pswds[1] = is_mconflicted;
			chk_cflts[chk_cflts.indexOf(chk_mng)] = f_pswds.join(",");
		}
		else if (!replaced_proj.isEmpty() && (chk_opwd || mngOwnedSameDbTime(find_froms, chk_that, replaced_db)))
		{
			if (chk_opwd) 
			{
				QString f_topwds;
				findMngPasswdsFromOutsideDb(replaced_db, f_pswds.at(1), f_topwds);
				f_pswds[1] = f_topwds.split(";").filter(replaced_time).at(0).split(",").at(1);
			}
			else
				f_pswds[1] = chk_that;
			chk_cflts[chk_cflts.indexOf(chk_mng)] = f_pswds.join(",");
		}
	}
	this_infoes[3] = chk_cflts.join(";");				
	if (!storeProjectsKeyDataes(this_keys))
	{
		replace_result = tr("添加")+this_proj+tr("关键数据失败");
		*spc_sqldb = QSqlDatabase::database(origin_conn);				
		return false;		  
	}		
	if (!toldataesInsertIndbProjectsinfoTable(this_infoes))
	{
		replace_result = tr("添加")+this_proj+tr("工程信息失败");
		*spc_sqldb = QSqlDatabase::database(origin_conn);				
		return false;		  
	}	
	QStringList add_selections = QSqlDatabase::database(origin_conn).tables();
	foreach (QString tbl, add_selections)
	{
		if (tbl.contains(this_proj))
		{
			QStringList split_tbl = tbl.split(tr("，，。"));	
			if (tbl.contains("product") || tbl.contains("property"))
				split_tbl[0] = replaced_proj;
			else
				split_tbl[1] = replaced_proj;
			if (!storeWholeTableFromOutside(origin_conn, tbl, split_tbl.join(tr("，，。"))))
			{
				replace_result = this_proj+tr("添加工程相关表")+tbl+tr("失败");
				*spc_sqldb = QSqlDatabase::database(origin_conn);	
				return false;				  
			}
		}
	}
	QStringList to_dprs;
	getDataesFromTable("departments", "name", to_dprs);	
	QStringList replace_mngs = this_infoes.at(2).split(";");	
	foreach (QString mng, replace_mngs)
	{  
		QStringList mng_detail = mng.split(",");
		QString find_froms;
		findMngPasswdsFromOutsideDb(origin_conn, mng_detail.at(1), find_froms);		
		QStringList from_mlist;
		*spc_sqldb = QSqlDatabase::database(origin_conn);
		getDataesFromTable("managerinfo", "password", from_mlist, find_froms);
		QStringList m_projs = from_mlist.at(3).split(";");
		QStringList m_psave = m_projs.filter(this_proj);			
		*spc_sqldb = QSqlDatabase::database(replaced_db);			
		QString chk_that;
		if (mngExistedTwoDbs(find_froms, replaced_db) || mngOwnedSameDbTime(find_froms, chk_that, replaced_db))
		{
			QString find_tos;
			findMngPasswdsFromOutsideDb(replaced_db, mng_detail.at(1), find_tos);	  
			QString mp_row("password="+find_tos);
			QString mp_content = dataFromTable("managerinfo", "projects", mp_row).toString();
			if (mp_content.isEmpty())
			{	  
				QString rep_three = replaced_proj;
				if (m_psave.at(0).split(",").size() > 1)
					rep_three += ","+m_psave.at(0).split(",").at(1);
				from_mlist[3] = rep_three;
				if (!toldataesInsertIndbManagersTable(from_mlist))
				{
					replace_result = tr("存储")+from_mlist.at(1)+"managerinfo"+tr("失败");
					return false;		  
				}			  
			}
			else if (!mp_content.contains(replaced_proj))
			{
				mp_content += ";"+replaced_proj;
				if (m_psave.at(0).split(",").size() > 1)
					mp_content += ","+m_psave.at(0).split(",").at(1);
				if (!updateCellData("managerinfo", mp_row, "projects", mp_content))
				{
					replace_result = tr("存储")+mp_content+tr("失败");
					return false;			  
				}
			}
			QString dpr_row("name="+from_mlist.at(2));
			QString dpr_toproj = dataFromTable("departments", "projects", dpr_row).toString();
			if (dpr_toproj.isEmpty())
			{
				QStringList dpr_news;
				dpr_news << from_mlist.at(2) << replaced_proj << mng << "";
				if (!toldataesInsertIndbDepartmentsTable(dpr_news))
				{
					replace_result = tr("添加")+from_mlist.at(2)+"departments"+tr("失败");
					return false;				  
				}		  
			}
			else if (!dpr_toproj.contains(replaced_proj))
			{
				dpr_toproj += ","+replaced_proj;
				if (!updateCellData("departments", dpr_row, "projects", dpr_toproj))
				{
					replace_result = tr("存储")+dpr_toproj+"departments"+tr("失败");
					return false;			  
				}
			}			
		}
		else
		{
			from_mlist[1] += ";"+replaced_time+","+mng_detail.at(1);		  
			QString rep_three = replaced_proj;
			if (m_psave.at(0).split(",").size() > 1)
				rep_three += ","+m_psave.at(0).split(",").at(1);
			from_mlist[3] = rep_three;
			if (!toldataesInsertIndbManagersTable(from_mlist))
			{
				replace_result = tr("存储")+from_mlist.at(1)+"managerinfo"+tr("失败");
				return false;		  
			}
			if (to_dprs.contains(from_mlist.at(2)))
			{
				QString dpr_row("name="+from_mlist.at(2));
				QString dpr_toproj = dataFromTable("departments", "projects", dpr_row).toString();
				if (!dpr_toproj.contains(replaced_proj))
				{
					dpr_toproj += ","+replaced_proj;
					if (!updateCellData("departments", dpr_row, "projects", dpr_toproj))
					{
						replace_result = tr("存储")+dpr_toproj+"departments"+tr("失败");
						return false;			  
					}
				}
				QString dpr_tomngs = dataFromTable("departments", "managers", dpr_row).toString()+";"+mng;
				if (!updateCellData("departments", dpr_row, "managers", dpr_tomngs))
				{
					replace_result = tr("存储")+dpr_tomngs+"departments"+tr("失败");
					return false;			  
				}				
			}
			else
			{
				QStringList dpr_news;
				dpr_news << from_mlist.at(2) << replaced_proj << mng << "";
				if (!toldataesInsertIndbDepartmentsTable(dpr_news))
				{
					replace_result = tr("添加")+from_mlist.at(2)+"departments"+tr("失败");
					return false;				  
				}		  
			}
			*spc_sqldb = QSqlDatabase::database(origin_conn);
			QString mp_row("password="+find_froms);
			if (!updateCellData("managerinfo", mp_row, "password", from_mlist.at(1)))
			{
				replace_result = tr("更新")+origin_conn+"password"+tr("失败");
				return false;			  
			}
			*spc_sqldb = QSqlDatabase::database(replaced_db);	
		}		
	}
	*spc_sqldb = QSqlDatabase::database(origin_conn);
	resetManagersDbMap();
	resetDepartmentsDbMap();
	resetProjsDbMap();	
	return true;
}

bool SpcDataBase::createEngiCarryProductTable(NestedModel * model_todb, bool carr_product)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QStringList table_list = spc_sqldb->tables();
	QString tail;
	if (carr_product)
		tail = tr("，，。")+"property";
	else
		tail = tr("，，。")+"product";	  
	int engi_nums = model_todb->rowCount();
	for (int i = 0; i < engi_nums; i++)
	{
		QString engi_name = model_todb->item(i)->text();		
		if (engi_name != model_todb->originHashItems().value(model_todb->item(i)).first->text())
		{
			if (!renameProject(model_todb->originHashItems().value(model_todb->item(i)).first->text(), engi_name))
				return false;
			QString old_tbl = model_todb->originHashItems().value(model_todb->item(i)).first->text()+tail;	
			QStringList filter_related = table_list.filter(old_tbl);
			if (!filter_related.isEmpty() && !delTable(filter_related.at(0)))
				return false;
		}
		else
		{
			if (model_todb->itemsArchitectureComparation(model_todb->originHashItems().value(model_todb->item(i)).first, model_todb->item(i)))
				continue;
		}
		QString t_name = engi_name+tail;
		QStringList filter_current = table_list.filter(t_name);
		if (!filter_current.isEmpty() && !delTable(filter_current.at(0)))
			return false;		
		QString table_name = QString("create table %1(").arg(t_name);
		QString insert_table = QString("insert into %1(").arg(t_name);
		QString insert_contents = QString(" values(");
		QList<QStandardItem *> tbl_cols;		
		model_todb -> findChildrenNotAll(model_todb->item(i), tbl_cols);		
		foreach (QStandardItem * c_item, tbl_cols)
		{
			QString col_name = c_item->text();
			if (tbl_cols.indexOf(c_item) == tbl_cols.size()-1)
			{
				table_name += QString("%1 varchar)").arg(col_name);
				insert_table += QString("%1)").arg(col_name);
				insert_contents += QString(":%1)").arg(col_name);
			}
			else
			{
				table_name += QString("%1 varchar, ").arg(col_name);
				insert_table += QString("%1, ").arg(col_name);
				insert_contents += QString(":%1, ").arg(col_name);
			}
		}
		query.exec(table_name);
		insert_table += insert_contents;
		if (!query.prepare(insert_table))
			return false;
		foreach (QStandardItem * n_item, tbl_cols)
		{
			QString col_content;
			if (n_item->hasChildren())
			{
				if (n_item->rowCount() > 1)
				{
					QList<QStandardItem *> col_items;
					model_todb -> findChildrenNotAll(n_item, col_items);
					foreach (QStandardItem * nn_item, col_items)
						col_content += nn_item->text()+tr("，！，");
				}
				else
					col_content = n_item->child(0)->text();
			}			
			QString bind = QString(":%1").arg(n_item->text());	
			query.bindValue(bind, col_content);
		}
		query.exec();	
	}
	return true;		
}

bool SpcDataBase::createEngiTables(const QString & projectname, const QString & constructor, const QSqlDatabase & to_db, const QString & origin_time)
{
	QSqlQuery query(to_db.database(to_db.connectionName()));
	QStringList tablelist_old = to_db.tables();
	QStringList next_list = projectname.split(tr("，，。"));
	QStringList time_list = origin_time.split(";"); 
	QString tengi_stamp = QString("%1").arg(QDateTime::fromString(time_list[0]).toTime_t());
	QString tsam_stamp = tengi_stamp+tr("，")+QString("%1").arg(QDateTime::fromString(time_list[1]).toTime_t());
	QString common_name = tr("，，。")+next_list[0]+tr("，，。")+tsam_stamp+tr("，，。")+constructor+tr("，，。");
	if (projectname.contains("cpkdataes"))
	{	  				
		QString cpk_tbl = QString("create table %1 (groupnum int primary key, dataes varchar, avr double, time datetime, person varchar)").arg(common_name+"cpkdataes");
		query.exec(cpk_tbl);
		if (projectname.contains("cpkdev"))
		{
			QString dev_tbl = QString("create table %1 (groupnum int primary key, dev double, dsigma double, cpk double, upavr double, lowavr double, updev double, lowdev double, accept varchar)").arg(common_name+"cpkdev");
			query.exec(dev_tbl);
		}
		else
		{
			QString rng_tbl = QString("create table %1 (groupnum int primary key, rng double, rsigma double, cpk double, upavr double, lowavr double, uprng double, lowrng double, accept varchar)").arg(common_name+"cpkrng");			
			query.exec(rng_tbl);			  
		}
	}
	else
	{
		QString daily_tbl = QString("create table %1 (groupnum int primary key, dataes varchar, time datetime, person varchar)").arg(common_name+"dailydataes");		
		query.exec(daily_tbl);		  
		QString avr_tbl = QString("create table %1 (groupnum int primary key, avr double, state varchar, time datetime)").arg(common_name+"dailyavr");			
		query.exec(avr_tbl);		  
		if (projectname.contains("dailydev"))
		{
			QString ddev_tbl = QString("create table %1 (groupnum int primary key, dev double, state varchar, time datetime)").arg(common_name+"dailydev");			
			query.exec(ddev_tbl);			  
		}
		else
		{
			QString drng_tbl = QString("create table %1 (groupnum int primary key, rng double, state varchar, time datetime)").arg(common_name+"dailyrng");			
			query.exec(drng_tbl);			  
		}
	}
	return true;
}

bool SpcDataBase::storeSpcDataes(const QString & tablename, DataesTableView * t_dataes, DataesTableView * res_dataes, DataesTableView * init_ptbl, const QString & daily_time)
{
	QStringList data_tbls = spc_sqldb->tables();
	QStringList next_list = tablename.split(tr("，，。"));	
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QStringList time_list = daily_time.split(";"); 
	QString tengi_stamp = QString("%1").arg(QDateTime::fromString(time_list[0]).toTime_t());
	QString tsam_stamp = tengi_stamp+tr("，")+QString("%1").arg(QDateTime::fromString(time_list[1]).toTime_t());	
	int i_h = t_dataes->model()->columnCount();
	int i_v = t_dataes->model()->rowCount();
	QString special_name = tr("，，。")+next_list[0]+tr("，，。");		
	if (tablename.contains("cpkdataes"))
	{
		QDateTime samdt(QDateTime::fromString(time_list[1]));	  
		QVariant inputor = init_ptbl->model()->data(init_ptbl->model()->index(10, 0));	  
		QString cpk_table;
		foreach (QString d_tbl, data_tbls)		  
		{
			if (d_tbl.contains(special_name) && d_tbl.contains("cpkdataes") && d_tbl.contains(tsam_stamp))
			{
				cpk_table = d_tbl;
				break;
			}
		}		
		if (!lastSpcDataesSaveResult(cpk_table))
		{
			QString v_num = lastDataFromTable(cpk_table, "groupnum").toString();
			QString empty_col;		
			if (!deleteDataes(cpk_table, v_num, empty_col))
				return false;
		}
		int last_num = 1;
		QString tol_dataes;		
		for (int i = 0; i < i_v; i++)
		{
			for (int j = 0; j < i_h; j++)
			{
				QString tmdata = t_dataes->model()->data(t_dataes->model()->index(i, j)).toString();
				tol_dataes += tmdata;
				if (j == i_h-1)
					tol_dataes += "; ";
				else
					tol_dataes += ", ";
			}
		}
		QString cpk_prepare = QString("insert into %1 (groupnum, dataes, avr, time, person) values(?, ?, ?, ?, ?)").arg(cpk_table);
		if (!query.prepare(cpk_prepare))
			return false;
		query.addBindValue(last_num);
		query.addBindValue(tol_dataes);
		query.addBindValue(res_dataes->model()->data(res_dataes->model()->index(4, 0)).toDouble());
		query.addBindValue(samdt);
		query.addBindValue(inputor);	
		query.exec();		
		if (init_ptbl->model()->data(init_ptbl->model()->index(3, 0)).toString().contains(tr("西格玛")))
		{
			QString  next_dev;		  
			foreach (QString d_tbl, data_tbls)		  
			{		  
				if (d_tbl.contains(special_name) && d_tbl.contains("cpkdev") && d_tbl.contains(tsam_stamp))
				{
					next_dev = d_tbl;
					break;
				}
			}
			if (!lastSpcDataesSaveResult(next_dev))
			{
				QString v_num = lastDataFromTable(next_dev, "groupnum").toString();
				QString empty_col;
				if (!deleteDataes(next_dev, v_num, empty_col))
					return false;
			}
			int ldev_num = 1;			
			QString dev_prepare = QString("insert into %1 (groupnum, dev, dsigma, cpk, upavr, lowavr, updev, lowdev, accept) values(:groupnum, :dev, :dsigma, :cpk, :upavr, :lowavr, :updev, :lowdev, :accept)").arg(next_dev);
			if (!query.prepare(dev_prepare))
				return false;
			query.bindValue(":groupnum", ldev_num);
			query.bindValue(":dev", res_dataes->model()->data(res_dataes->model()->index(7, 0)).toDouble());
			query.bindValue(":dsigma", res_dataes->model()->data(res_dataes->model()->index(3, 0)).toDouble());
			query.bindValue(":cpk", res_dataes->model()->data(res_dataes->model()->index(2, 0)).toDouble());
			query.bindValue(":upavr", res_dataes->model()->data(res_dataes->model()->index(5, 0)).toDouble());
			query.bindValue(":lowavr", res_dataes->model()->data(res_dataes->model()->index(6, 0)).toDouble());
			query.bindValue(":updev", res_dataes->model()->data(res_dataes->model()->index(8, 0)).toDouble());
			query.bindValue(":lowdev", res_dataes->model()->data(res_dataes->model()->index(9, 0)).toDouble());
			query.bindValue(":accept", res_dataes->model()->data(res_dataes->model()->index(10, 0)));			
			query.exec();
		}
		else
		{
			QString  next_rng;
			foreach (QString d_tbl, data_tbls)		  
			{			  
				if (d_tbl.contains(special_name) && d_tbl.contains("cpkrng") && d_tbl.contains(tsam_stamp))
				{
					next_rng = d_tbl;
					break;
				}
			}
			if (!lastSpcDataesSaveResult(next_rng))
			{
				QString v_num = lastDataFromTable(next_rng, "groupnum").toString();
				QString empty_col;
				if (!deleteDataes(next_rng, v_num, empty_col))
					return false;
			}
			int lrng_num = 1;			
			QString rng_prepare = QString("insert into %1 (groupnum, rng, rsigma, cpk, upavr, lowavr, uprng, lowrng, accept) values(:groupnum, :rng, :rsigma, :cpk, :upavr, :lowavr, :uprng, :lowrng, :accept)").arg(next_rng);
			if (!query.prepare(rng_prepare))
				return false;
			query.bindValue(":groupnum", lrng_num);
			query.bindValue(":rng", res_dataes->model()->data(res_dataes->model()->index(7, 0)).toDouble());
			query.bindValue(":rsigma", res_dataes->model()->data(res_dataes->model()->index(3, 0)).toDouble());
			query.bindValue(":cpk", res_dataes->model()->data(res_dataes->model()->index(2, 0)).toDouble());
			query.bindValue(":upavr", res_dataes->model()->data(res_dataes->model()->index(5, 0)).toDouble());
			query.bindValue(":lowavr", res_dataes->model()->data(res_dataes->model()->index(6, 0)).toDouble());
			query.bindValue(":uprng", res_dataes->model()->data(res_dataes->model()->index(8, 0)).toDouble());
			query.bindValue(":lowrng", res_dataes->model()->data(res_dataes->model()->index(9, 0)).toDouble());
			query.bindValue(":accept", "accept?");			
			query.exec();
		}
	}
	else if (tablename.contains("dailydataes"))
	{
		QDateTime samdt(QDateTime::fromString(time_list[2]));	  
		QVariant inputor = time_list[3];	 	  
		QString daily_table;
		foreach (QString d_tbl, data_tbls)		  
		{		  
			if (d_tbl.contains(special_name) && d_tbl.contains("dailydataes") && d_tbl.contains(tsam_stamp))
			{
				daily_table = d_tbl;
				break;
			}
		}
		if (!lastSpcDataesSaveResult(daily_table))
		{
			QString v_num = lastDataFromTable(daily_table, "groupnum").toString();
			QString empty_col;
			if (!deleteDataes(daily_table, v_num, empty_col))
				return false;
		}
		QVariant gdaily_last = lastDataFromTable(daily_table, "groupnum");
		int ldaily_num = 1;
		if (!gdaily_last.isNull())
			ldaily_num += gdaily_last.toInt();			
		QString dataes_str;
		for (int i = 1; i < t_dataes->model()->rowCount()-5; i++)
		{
			dataes_str += t_dataes->model()->data(t_dataes->model()->index(i, 0)).toString();
			if (i != t_dataes->model()->rowCount()-6)
				dataes_str += ", ";
			else
				dataes_str += " ";
		}
		QString daily_prepare = QString("insert into %1 (groupnum, dataes, time, person) values(:groupnum, :dataes, :time, :person)").arg(daily_table);
		if (!query.prepare(daily_prepare))
			return false;
		query.bindValue(":groupnum", ldaily_num);
		query.bindValue(":dataes", dataes_str);
		query.bindValue(":time", samdt);
		query.bindValue(":person", inputor);
		query.exec();
		QString daily_avr;
		foreach (QString d_tbl, data_tbls)		  
		{		  
			if (d_tbl.contains(special_name) && d_tbl.contains("dailyavr")  && d_tbl.contains(tsam_stamp))
			{
				daily_avr = d_tbl;
				break;
			}
		}
		if (!lastSpcDataesSaveResult(daily_avr))
		{
			QString v_num = lastDataFromTable(daily_avr, "groupnum").toString();
			QString empty_col;
			if (!deleteDataes(daily_avr, v_num, empty_col))
				return false;
		}
		QVariant gavr_last = lastDataFromTable(daily_avr, "groupnum");
		int lavr_num = 1;
		if (!gavr_last.isNull())
			lavr_num += gavr_last.toInt();			
		QString avr_prepare = QString("insert into %1 (groupnum, avr, state, time) values(:groupnum, :avr, :state, :time)").arg(daily_avr);
		if (!query.prepare(avr_prepare))
			return false;
		query.bindValue(":groupnum", lavr_num);
		query.bindValue(":avr", t_dataes->model()->data(t_dataes->model()->index(t_dataes->model()->rowCount()-2, 0)).toDouble());
		query.bindValue(":state", t_dataes->dailyCalculations().value(t_dataes->model()->index(t_dataes->model()->rowCount()-1, 0)));
		query.bindValue(":time", samdt);
		query.exec();
		updateStateDataes(daily_avr, t_dataes);// here problem check it in future
		if (t_dataes->model()->headerData(t_dataes->model()->rowCount()-3, Qt::Vertical).toString().contains(tr("西格玛")))
		{
			QString daily_dev;
			foreach (QString d_tbl, data_tbls)		  
			{		  
				if (d_tbl.contains(special_name) && d_tbl.contains("dailydev")  && d_tbl.contains(tsam_stamp))
				{
					daily_dev = d_tbl;
					break;
				}
			}
			if (!lastSpcDataesSaveResult(daily_dev))
			{
				QString v_num = lastDataFromTable(daily_dev, "groupnum").toString();
				QString empty_col;
				if (!deleteDataes(daily_dev, v_num, empty_col))
					return false;
			}
			QVariant gddev_last = lastDataFromTable(daily_dev, "groupnum");
			int lddev_num = 1;
			if (!gddev_last.isNull())
				lddev_num += gddev_last.toInt();			
			QString devd_prepare = QString("insert into %1 (groupnum, dev, state, time) values(:groupnum, :dev, :state, :time)").arg(daily_dev);
			if (!query.prepare(devd_prepare))
				return false;
			query.bindValue(":groupnum", lddev_num);			
			query.bindValue(":dev", t_dataes->model()->data(t_dataes->model()->index(t_dataes->model()->rowCount()-4, 0)).toDouble());
			query.bindValue(":state", t_dataes->dailyCalculations().value(t_dataes->model()->index(t_dataes->model()->rowCount()-3, 0)));
			query.bindValue(":time", samdt);
			query.exec();
			updateStateDataes(daily_dev, t_dataes);// here problem
		}
		else
		{
			QString daily_rng;
			foreach (QString d_tbl, data_tbls)		  
			{			  
				if (d_tbl.contains(special_name) && d_tbl.contains("dailyrng")  && d_tbl.contains(tsam_stamp))
				{
					daily_rng = d_tbl;
					break;
				}
			}
			if (!lastSpcDataesSaveResult(daily_rng))
			{
				QString v_num = lastDataFromTable(daily_rng, "groupnum").toString();
				QString empty_col;
				if (!deleteDataes(daily_rng, v_num, empty_col))
					return false;
			}
			QVariant gdrng_last = lastDataFromTable(daily_rng, "groupnum");
			int ldrng_num = 1;
			if (!gdrng_last.isNull())
				ldrng_num += gdrng_last.toInt();				
			QString devr_prepare = QString("insert into %1 (groupnum, rng, state, time) values(:groupnum, :rng, :state, :time)").arg(daily_rng);
			if (!query.prepare(devr_prepare))
				return false;
			query.bindValue(":groupnum", ldrng_num);
			query.bindValue(":rng", t_dataes->model()->data(t_dataes->model()->index(t_dataes->model()->rowCount()-4, 0)).toDouble());
			query.bindValue(":state", t_dataes->model()->data(t_dataes->model()->index(t_dataes->model()->rowCount()-3, 0)));
			query.bindValue(":time", samdt);
			query.exec();
		}
	}	
	return true;
}

bool SpcDataBase::theSameNameInManageList(const QString & name, const QString & dest_db)
{
	if (dest_db.isEmpty())
	{
		QMapIterator<QString, QStringList> i_map(db_manageList);
		while (i_map.hasNext())
		{
			i_map.next();
			if (i_map.value().at(0)==name)
				return true;
		}
	}
	else
	{
		QString origin_conn = spc_sqldb->connectionName();
		*spc_sqldb = QSqlDatabase::database(dest_db);
		QStringList mngs_nms;
		getDataesFromTable("managerinfo", "name", mngs_nms);
		*spc_sqldb = QSqlDatabase::database(origin_conn);		
		if (mngs_nms.contains(name))		  
			return true;
	}
	return false;
}

bool SpcDataBase::theSameNameAndPasswdInManageList(const QPair<QString, QString> & name_passwd, const QString & dest_db)
{
	if (dest_db.isEmpty())
	{
		QMapIterator<QString, QStringList> i_map(db_manageList);
		while (i_map.hasNext())
		{
			i_map.next();
			if (i_map.key()==name_passwd.second && i_map.value().at(0)==name_passwd.first)
				return true;
		}
	}
	else
	{
		QString origin_conn = spc_sqldb->connectionName();
		*spc_sqldb = QSqlDatabase::database(dest_db);
		QSqlTableModel db_model(this, QSqlDatabase::database(dest_db));
		varsModelFromTable("dbInformations", &db_model);
		QString dtime_pwd(db_model.data(db_model.index(0, 2)).toString()+","+name_passwd.second);
		QStringList mngs_pws;
		getDataesFromTable("managerinfo", "password", mngs_pws);
		QStringList mng_pws = mngs_pws.filter(name_passwd.second);
		foreach (QString pwd, mng_pws)
		{
			if (pwd.contains(dtime_pwd))
			{
				QString w_pos = "password="+pwd;
				QString real_name = dataFromTable("managerinfo", "name", w_pos).toString();
				if (real_name == name_passwd.first)
				{
					*spc_sqldb = QSqlDatabase::database(origin_conn);
					return true;
				}
			}
		}	
		*spc_sqldb = QSqlDatabase::database(origin_conn);				
	}
	return false;
}

bool SpcDataBase::theSameNameWaitingPower(const QPair<QString, QString> & name_passwd, const QString & dest_db)
{
	if (dest_db.isEmpty())
	{
		QStringList key_list = db_projectList.keys();
		foreach (QString proj_name, key_list)
		{
			QStringList dest_list = db_projectList.value(proj_name);
			QStringList name_pair = dest_list[5].split(";");
			foreach (QString n_info, name_pair)
			{
				QStringList dest_passwd = n_info.split(",");
				if (dest_passwd[0] == name_passwd.first)
					return true;
			}
		}
	}
	else
	{
		QString origin_conn = spc_sqldb->connectionName();
		*spc_sqldb = QSqlDatabase::database(dest_db);	  
		QStringList mngs_wts;
		getDataesFromTable("projectsinfo", "authorizing", mngs_wts);
		*spc_sqldb = QSqlDatabase::database(origin_conn);	
		foreach (QString wt_str, mngs_wts)
		{
			QStringList name_pair = wt_str.split(";");
			foreach (QString n_info, name_pair)
			{
				QStringList dest_passwd = n_info.split(",");
				if (dest_passwd[0] == name_passwd.first)
					return true;
			}			
		}
	}
	return false;
}

bool SpcDataBase::theSamePasswdIndb(const QString & passwd, const QString & dest_db)
{
	if (dest_db.isEmpty())
	{
		QMapIterator<QString, QStringList> i_map(db_manageList);
		while (i_map.hasNext())
		{
			i_map.next();
			if (i_map.key() == passwd)
				return true;
		}
		QMapIterator<QString, QStringList> p_map(db_projectList);
		while (p_map.hasNext())
		{
			p_map.next();
			QStringList p_infoes = p_map.value().at(4).split(",");
			if (p_infoes.size() > 2)
			{
				if (p_infoes.at(1) == passwd)
					return true;
			}
		}
	}
	else 
	{		
		QString oigin_conn = spc_sqldb->connectionName();		
		*spc_sqldb = QSqlDatabase::database(dest_db);
		QStringList mngs_pws;
		getDataesFromTable("managerinfo", "password", mngs_pws);
		QString other_time;
		findDestInfoFromDbtbl(2, other_time, dest_db);
		QString dtime_pwd(other_time+","+passwd);		
		QStringList mng_pws = mngs_pws.filter(dtime_pwd);
		if (!mng_pws.isEmpty())
		{
			*spc_sqldb = QSqlDatabase::database(oigin_conn);
			return true;
		}	
		QStringList mngs_wts;
		getDataesFromTable("projectsinfo", "authorizing", mngs_wts);
		*spc_sqldb = QSqlDatabase::database(oigin_conn);		
		foreach (QString wt_str, mngs_wts)
		{
			if (wt_str.contains(passwd))
				return true;
		}
	}
	return false;	
}

bool SpcDataBase::storeChangedPasswd(const QPair<QString, QString> & new_passwds, const QString & old_passwd, const QString & dest_db)
{	
	QPair<QString, QString> c_pair(new_passwds.first, old_passwd);	
	QString oigin_conn = spc_sqldb->connectionName();	
	if (!dest_db.isEmpty())
		*spc_sqldb = QSqlDatabase::database(dest_db);
	QSqlTableModel db_sql(this, QSqlDatabase::database(spc_sqldb->connectionName()));
	varsModelFromTable("dbInformations", &db_sql);		
	QStringList m_constructors;
	getProjsConstructores(m_constructors);
	if (m_constructors.contains(old_passwd))
	{
		QStringList ps_constructor;
		constructorOwnedProjs(c_pair, ps_constructor);
		QString key_col("constructor");
		foreach (QString own_proj, ps_constructor)
		{
			QString key_row("name="+own_proj);
			QStringList testors = dataFromTable("projectskeys", key_col, key_row).toString().split(tr("；"));
			testors = testors.replaceInStrings(old_passwd, new_passwds.second);
			if (!updateCellData("projectskeys", key_row, key_col, testors.join(tr("；"))))
				return false;
		}
	}
	QSqlTableModel info_sql(this, QSqlDatabase::database(spc_sqldb->connectionName()));
	varsModelFromTable("projectsinfo", &info_sql);
	for (int i = 0; i < info_sql.rowCount(); i++)
	{
		if (info_sql.data(info_sql.index(i, 2)).toString().contains(old_passwd))
		{
			QStringList p_mngs = info_sql.data(info_sql.index(i, 2)).toString().split(";");
			p_mngs = p_mngs.replaceInStrings(old_passwd, new_passwds.second);
			info_sql.setData(info_sql.index(i, 2), p_mngs.join(";"));
		}
		if (info_sql.data(info_sql.index(i, 3)).toString().contains(old_passwd))
		{
			QStringList p_mngs = info_sql.data(info_sql.index(i, 3)).toString().split(";");
			p_mngs = p_mngs.replaceInStrings(old_passwd, new_passwds.second);
			info_sql.setData(info_sql.index(i, 3), p_mngs.join(";"));
		}	
		if (info_sql.data(info_sql.index(i, 5)).toString().contains(old_passwd))
		{
			QStringList p_mngs = info_sql.data(info_sql.index(i, 5)).toString().split(";");
			p_mngs = p_mngs.replaceInStrings(old_passwd, new_passwds.second);
			info_sql.setData(info_sql.index(i, 5), p_mngs.join(";"));
		}		
	}
	if (!info_sql.submitAll())
		return false;
	QSqlTableModel minfo_sql(this, QSqlDatabase::database(spc_sqldb->connectionName()));
	varsModelFromTable("managerinfo", &minfo_sql);
	for (int i = 0; i < minfo_sql.rowCount(); i++)
	{
		if (minfo_sql.data(minfo_sql.index(i, 1)).toString().contains(old_passwd))
		{
			QStringList dbs_mng = minfo_sql.data(minfo_sql.index(i, 1)).toString().split(";");
			QStringList db_mng = dbs_mng.filter(db_sql.data(db_sql.index(0, 2)).toString());
			db_mng = db_mng.replaceInStrings(old_passwd, new_passwds.second);		
			minfo_sql.setData(minfo_sql.index(i, 1), db_mng.join(";"));
		}		
	}
	if (!minfo_sql.submitAll())
		return false;	
	QSqlTableModel dprinfo_sql(this, QSqlDatabase::database(spc_sqldb->connectionName()));
	varsModelFromTable("departments", &dprinfo_sql);
	for (int i = 0; i < minfo_sql.rowCount(); i++)
	{
		if (dprinfo_sql.data(dprinfo_sql.index(i, 2)).toString().contains(old_passwd))
		{
			QStringList dpr_mngs = dprinfo_sql.data(dprinfo_sql.index(i, 2)).toString().split(";");
			dpr_mngs = dpr_mngs.replaceInStrings(old_passwd, new_passwds.second);
			dprinfo_sql.setData(dprinfo_sql.index(i, 2), dpr_mngs.join(";"));
		}		
	}
	if (!dprinfo_sql.submitAll())
		return false;
	QSqlTableModel pics_sql(this, QSqlDatabase::database(spc_sqldb->connectionName()));
	varsModelFromTable("pictures", &pics_sql);
	for (int i = 0; i < pics_sql.rowCount(); i++)
	{
		if (pics_sql.data(pics_sql.index(i, 3)).toString().contains(old_passwd))
		{
			QStringList nesteds = pics_sql.data(pics_sql.index(i, 3)).toString().split(tr("，，。"));
			nesteds = nesteds.replaceInStrings(old_passwd, new_passwds.second);
			pics_sql.setData(pics_sql.index(i, 3), nesteds.join(tr("，，。")));
		}
		if (pics_sql.data(pics_sql.index(i, 4)).toString() == old_passwd)
			pics_sql.setData(pics_sql.index(i, 4), new_passwds.second);		
	}
	if (!pics_sql.submitAll())
		return false;	
	QStringList all_tbls = spc_sqldb->tables();
	all_tbls = all_tbls.replaceInStrings(old_passwd, new_passwds.second);
	foreach (QString tbl, all_tbls)
	{
		if (tbl.contains(new_passwds.second))
		{
			if (!renameTable(spc_sqldb->tables().at(all_tbls.indexOf(tbl)), tbl))
				return false;
		}
		if (tbl.contains(tr("，，。")))
		{
			QStringList tbl_detail = tbl.split(tr("，，。"));
			if (tbl_detail.size()==5 && (tbl_detail[4]=="cpkdataes" || tbl_detail[4]=="dailydataes"))
			{
				if (!changePwdInDifferentProjTbl(old_passwd, new_passwds.second, tbl))
					return false;
			}
		}		
	}
	if (dest_db.isEmpty())
	{
		resetManagersDbMap();
		resetDepartmentsDbMap();
		resetProjsDbMap();
	}
	*spc_sqldb = QSqlDatabase::database(oigin_conn);	
	return true;
}

bool SpcDataBase::isDatabaseConstructor(QPair<QString, QString> & name_pair, const QString & w_db)
{
	bool chk_result = false;
	QString chk_str = name_pair.first+","+name_pair.second;	
	if (!w_db.isEmpty())
	{
		QStringList db_path = w_db.split("/");	
		bool remove_connection = false;
		{
			QSqlDatabase chk_db(QSqlDatabase::addDatabase("QSQLITE", db_path.back()));
			chk_db.setDatabaseName(w_db);		
			if (!chk_db.open())
			{
				name_pair.second += tr("，，。");
				remove_connection = true;
			}
			if (!remove_connection)
			{
				QSqlTableModel db_table(this, chk_db.database(chk_db.connectionName()));							
				varsModelFromTable("dbInformations", &db_table);
				if (db_table.data(db_table.index(0, 1)).toString() == chk_str)
					chk_result = true;
				chk_db.database(chk_db.connectionName()).close();
				remove_connection = true;
			}
		}
		if (remove_connection)
			QSqlDatabase::removeDatabase(db_path.back());
	}
	else
	{
		QSqlTableModel db_table(this, QSqlDatabase::database(spc_sqldb->connectionName()));							
		varsModelFromTable("dbInformations", &db_table);
		if (db_table.data(db_table.index(0, 1)).toString() == chk_str)
			chk_result = true;		
	}
	return chk_result;
}

bool SpcDataBase::isConstructor(const QPair<QString, QString> & name_pair, const QString & project)
{
	QString judge_name = "name="+project;
	QString text_time = dataFromTable("projectskeys", "constructtime", judge_name).toDateTime().toString();
	QStringList tests_constructs = dataFromTable("projectskeys", "constructor", judge_name).toString().split(tr("；"));
	QStringList c_list = tests_constructs.filter(text_time).at(0).split(tr("，"));
	if (c_list.at(1)==name_pair.first && c_list.at(2)==name_pair.second)
		return true;
	return false;	
}

bool SpcDataBase::existedProjectNameIndb(const QString & project, const QString & dest_db)
{
	if (!dest_db.isEmpty())
	{
		QString origin_conn = spc_sqldb->connectionName();
		*spc_sqldb = QSqlDatabase::database(dest_db);
		QString projects_tol = "projectskeys";
		QString names = "name";
		QStringList all_projs;
		getDataesFromTable(projects_tol, names, all_projs);
		*spc_sqldb = QSqlDatabase::database(origin_conn);
		if (all_projs.contains(project))
			return true;
	}
	else
	{
		if (db_projectList.keys().contains(project))
			return true;
	}
	return false;
}

bool SpcDataBase::existedSameProjectIndb(const QDateTime & construct_time, const QString & dest_db)// no finished for proj search in one db?
{
	QVariant time_proj;
	if (!dest_db.isEmpty())
	{
		QString origin_conn = spc_sqldb->connectionName();
		*spc_sqldb = QSqlDatabase::database(dest_db);
		time_proj = dataFromTableByTime(construct_time, "projectskeys", "name", "constructtime");
		*spc_sqldb = QSqlDatabase::database(origin_conn);
	}
	else
		time_proj = dataFromTableByTime(construct_time, "projectskeys", "name", "constructtime");
	if (!time_proj.isNull())
		return true;
	return false;	
}

bool SpcDataBase::existedEntirelySameProject(const QString & this_proj, const QDateTime & construct_time, const QString & dest_db)
{
	QStringList tkey_list;	
	getDataesFromTable("projectskeys", "constructtime", tkey_list, construct_time.toString(Qt::ISODate));	
	QStringList tall_tbls = spc_sqldb->tables();		
	QString origin_conn = spc_sqldb->connectionName(); 	
	*spc_sqldb = QSqlDatabase::database(dest_db);		
	QString p_that = dataFromTableByTime(construct_time, "projectskeys", "name", "constructtime").toString();	
	if (p_that.isEmpty())
	{
		*spc_sqldb = QSqlDatabase::database(origin_conn);
		return false;
	}
	QStringList oall_tbls = spc_sqldb->tables();	
	if (!projsKeyDataesEqualBetweenDbs(dest_db, construct_time.toString(Qt::ISODate)))
	{
		*spc_sqldb = QSqlDatabase::database(origin_conn);	
		return false;
	}
	if (!projsInfoDataesEqualBetweenDbs(dest_db, this_proj, construct_time.toString(Qt::ISODate)))
	{
		*spc_sqldb = QSqlDatabase::database(origin_conn);
		return false;
	}
	QString this_property = this_proj+tr("，，。")+"property";
	QString this_product = this_proj+tr("，，。")+"product";			
	QStringList filter_property1 = tall_tbls.filter(this_property);
	QStringList filter_product1 = tall_tbls.filter(this_product);
	QString that_property = p_that+tr("，，。")+"property";
	QString that_product = p_that+tr("，，。")+"product";	
	QStringList filter_property2 = oall_tbls.filter(that_property);
	QStringList filter_product2 = oall_tbls.filter(that_product);
	if (filter_property1!=filter_property2 || filter_product1!=filter_product2)
	{
		*spc_sqldb = QSqlDatabase::database(origin_conn);
		return false;
	}
	QStringList ttime_list = tkey_list.at(1).split(tr("；"));			
	QStringList tcpk_tbls;	
	QStringList ocpk_tbls;		
	foreach (QString t_time, ttime_list)
	{
		QStringList tcpk_infos = t_time.split(tr("，"));
		QString tengi_stamp = QString("%1").arg(QDateTime::fromString(tcpk_infos.at(0)).toTime_t());	
		tcpk_tbls += tall_tbls.filter(tengi_stamp);
		ocpk_tbls += oall_tbls.filter(tengi_stamp);	
	}
	QStringList tcpk_compares;
	QStringList ocpk_compares;	
	foreach (QString t_tbl, tcpk_tbls)
	{
		QStringList this_tbl = t_tbl.split(tr("，，。"));
		QString this_compare = this_tbl.at(2)+this_tbl.back();
		tcpk_compares << this_compare;
		QStringList that_tbls = ocpk_tbls.filter(this_tbl.at(2)).filter(this_tbl.back());
		if (!that_tbls.isEmpty())
		{
			QStringList that_tbl = that_tbls.at(0).split(tr("，，。"));
			QString that_compare = that_tbl.at(2)+that_tbl.back();
			ocpk_compares << that_compare;		
		}
	}	
	if (tcpk_compares != ocpk_compares)
	{
		*spc_sqldb = QSqlDatabase::database(origin_conn);
		return false;
	}
	QStringList this_dailys = tcpk_tbls.filter(tr("，，。dailydataes"));
	foreach (QString e_tbl, this_dailys)
	{
		QStringList this_stamp = e_tbl.split(tr("，，。"));
		if (!projsDailysEqualBetweenDbs(origin_conn, this_stamp.at(2), tr("，，。dailydataes")))
		{
			*spc_sqldb = QSqlDatabase::database(origin_conn);
			return false;
		}
	}
	*spc_sqldb = QSqlDatabase::database(origin_conn);	
	return true;
}

bool SpcDataBase::existedDepartmentIndb(const QString & department, QSqlDatabase * dest_db)
{
	if (dest_db)
	{
		QSqlDatabase * tmp_db = spc_sqldb;
		spc_sqldb = dest_db;
		QStringList dprs;
		getDataesFromTable("departments", "name", dprs);
		spc_sqldb = tmp_db;
		if (dprs.contains(department))
			return true;		  
	}
	else
	{
		if (db_departList.keys().contains(department))
			return true;
	}
	return false;
}

bool SpcDataBase::existedManualPlotsTblIndb(const QString & manual_tbl)
{
	QStringList table_list = spc_sqldb->tables();
	foreach (QString t_str, table_list)
	{
		if (t_str == manual_tbl)
			return true;
	}
	return false;
}

bool SpcDataBase::existedSameManualTblIndb(const QString & outside_tbl, const QString & construct_time)
{
	QSqlTableModel tbl_model(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable(outside_tbl, &tbl_model);
	QString time_inside;
	if (outside_tbl.contains("manualtable"))//if conclude "manualtable" in tbl_name?
		time_inside = tbl_model.data(tbl_model.index(0, 6)).toString();
	else
		time_inside = tbl_model.data(tbl_model.index(0, 4)).toString();
	if (construct_time == time_inside)
		return true;
	return false;
}

bool SpcDataBase::renameDatabaseName(const QString & old_name, const QString & new_name)
{
	QFile db_file(old_name);
	int i_rename = 0;
	while (!db_file.rename(new_name))
	{
		if (i_rename == 3)
			return false;
		i_rename++;
	}
	return true;
}

bool SpcDataBase::renameProject(const QString & oldname, const QString & newname)
{
	QString replace_name = "name="+oldname;
	if (!updateCellData("projectsinfo", replace_name, "name", newname))
		return false;
	if (db_projectList.contains(oldname))
	{
		QStringList values = db_projectList.value(oldname);
		db_projectList.erase(db_projectList.find(oldname));
		db_projectList.insert(newname, values);
	}
	if (!updateCellData("projectskeys", replace_name, "name", newname))
		return false;
	QMapIterator<QString, QStringList> m_map(db_manageList);
	while (m_map.hasNext())
	{
		m_map.next();
		QString projs = m_map.value().at(3);
		if (projs.contains(oldname))
		{
			QStringList m_projs = projs.split(";");
			foreach (QString m_proj, m_projs)
			{
				if (m_proj.contains(oldname))
				{
					QStringList p_details = m_proj.split(",");
					p_details[0] = newname;
					QString new_pms = p_details.join(",");
					m_projs[m_projs.indexOf(m_proj)] = new_pms;
					break;
				}
			}
			projs = m_projs.join(";");
			QString replace_mng = "password="+m_map.value().at(1);
			QString replace_projs = "projects";
			if (!updateCellData("managerinfo", replace_mng, replace_projs, projs))
				return false;
			db_manageList[m_map.key()][3] = projs;
		}
	}
	QMapIterator<QString, QStringList> d_map(db_departList);
	while (d_map.hasNext())
	{
		d_map.next();
		QString projs = d_map.value().at(1);
		if (projs.contains(oldname))
		{
			QStringList m_projs = projs.split(",");
			foreach (QString m_proj, m_projs)
			{
				if (m_proj == oldname)
				{
					m_projs[m_projs.indexOf(m_proj)] = newname;
					break;
				}
			}
			projs = m_projs.join(",");
			QString replace_dpr = "name="+d_map.key();
			QString replace_projs = "projects";
			if (!updateCellData("departments", replace_dpr, replace_projs, projs))
				return false;
			db_departList[d_map.key()][1] = projs;
		}
	}	
	QStringList tables_list = spc_sqldb->tables();
	QString chk_name(oldname+tr("，，。"));
	foreach (QString tbl, tables_list)
	{
		if (tbl.contains(chk_name))
		{
			QStringList tbl_list = tbl.split(chk_name);
			QString new_tbl = tbl_list.at(0)+newname+tr("，，。")+tbl_list.at(1);			
			if (!renameTable(tbl, new_tbl))
				return false;
		}
	}
	return true;
}

bool SpcDataBase::destProjPassed(const QString & projectname)//no finished
{
	QString rng = projectname+"cpkrng";
	QString dev = projectname+"cpkdev";
	QStringList table_list = spc_sqldb->tables();
	if (table_list.contains(rng))
	{
		QStringList pass_info;
		getDataesFromTable(rng, "accept", pass_info);
		if (pass_info.contains(tr("通过")))	
			return true;	
	}
	if (table_list.contains(dev))
	{
		QStringList pass_info;
		getDataesFromTable(dev, "accept", pass_info);
		if (pass_info.contains(tr("通过")))	
			return true;		
	}
	return false;
}

bool SpcDataBase::storeManageInfoTable(const QStringList & manage_list)
{
	if (manage_list.contains("projkeys"))
	{
		QPair<QString, QString> name_info(manage_list[10], manage_list[11]);
		if (theSameNameAndPasswdInManageList(name_info))
		{
			QString proj("projects");
			QString m_name("password="+db_manageList[name_info.second].at(1));
			QString new_proj(";"+manage_list[1]);
			if (!storeData("managerinfo", m_name, proj, new_proj))
				return false;
			db_manageList[name_info.second][3] += ";"+manage_list.at(1);
		}
		else
		{
			QSqlTableModel db_model(this, spc_sqldb->database(spc_sqldb->connectionName()));
			QString db_tbl("dbInformations");
			varsModelFromTable(db_tbl, &db_model);		  
			QStringList tmps;
			tmps << manage_list.at(10) << db_model.data(db_model.index(0, 2)).toString()+","+manage_list.at(11) << manage_list.at(13) << manage_list.at(1) << manage_list.at(12);
			if (!toldataesInsertIndbManagersTable(tmps))
				return false;			
			db_manageList.insert(manage_list[11], tmps);
		}
	} 
	else
	{
		if (manage_list.size() == 5)
		{
			if (!toldataesInsertIndbManagersTable(manage_list))
				return false;
			db_manageList.insert(manage_list[1], manage_list);
		}
		else//???
		{}
	}
	resetManagersDbMap();
	return true;
}

bool SpcDataBase::storeProjectsInfoTable(const QStringList & project_list) 
{
	if (project_list.contains("projkeys"))
	{
		QStringList tmps1;
		QStringList tmps2;
		tmps1 << project_list.at(1) << project_list.at(13) << project_list.at(10)+","+project_list.at(11);
		tmps2 << project_list.at(1) << project_list.at(13) << project_list.at(10)+","+project_list.at(11) << "";
		if (!toldataesInsertIndbProjectsinfoTable(tmps1))
			return false;
		db_projectList.insert(project_list.at(1), tmps2);
	}
	else
	{
//		foreach (QString)
	}
	resetProjsDbMap();
	return true;
}

bool SpcDataBase::storeDepartmentTable(const QStringList & depart_list)
{
	if (depart_list.contains("projkeys"))
	{
		if (existedDepartmentIndb(depart_list[13]))
		{
			QString d_ms(db_departList[depart_list[13]][2]);
			QString d_name("name="+depart_list[13]);
			if (!d_ms.contains(depart_list[11]))
			{
				QString new_dms(";"+depart_list[10]+","+depart_list[11]);
				QString ms("managers");
				if (!storeData("departments", d_name, ms, new_dms))
					return false;
				db_departList[depart_list[13]][2] = d_ms+new_dms;
			}
			QString new_proj(","+depart_list[1]);
			if (!storeData("departments", d_name, "projects", new_proj))
				return false;
			db_departList[depart_list[13]][1] += ","+depart_list.at(1);
		}
		else
		{
			QStringList tmps;
			tmps << depart_list.at(13) << depart_list.at(1) << depart_list.at(10)+","+depart_list.at(11);
			if (!toldataesInsertIndbDepartmentsTable(tmps))
				return false;
			db_departList.insert(depart_list.at(13), tmps);
		}
	}
	else
	{
		if (depart_list.size() == 3)
		{
			if (!toldataesInsertIndbDepartmentsTable(depart_list))
				return false;
			db_departList.insert(depart_list[0], depart_list);
		}
		else
		{}		
	}
	resetDepartmentsDbMap();
	return true;
}

bool SpcDataBase::storeProjectsKeyDataes(QStringList & keys, bool init_test, const QString & test_dt)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	if (init_test)
	{	  
		if (!query.prepare("insert into projectskeys(name, unit, precision, ctrlplot, uplimit, lowlimit, grpsamles, groups, desnum, constructor, constructtime) values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"))
			return false;
		foreach (QString e_str, keys)
		{
			if (test_dt.isEmpty())
			{
				if (keys.indexOf(e_str) == 11)
				{
					query.addBindValue(QDateTime::fromString(e_str));
					break;
				}
				query.addBindValue(e_str);
			}
			else
			{
				QString d_strs;
				if (keys.indexOf(e_str) == 9)
				{
					d_strs = test_dt+tr("，")+e_str+tr("，")+keys.at(10);
					query.addBindValue(d_strs);
					query.addBindValue(QDateTime::fromString(test_dt));
					break;
				}
				if (keys.indexOf(e_str) != 0)
					d_strs = test_dt+tr("，")+e_str;
				else
					d_strs = e_str;
				query.addBindValue(d_strs);
			}
		}
		query.exec(); 		
	}
	else
	{
		QStringList old_keys;
		getDataesFromTable("projectskeys", "name", old_keys, keys.at(0));
		QString col_name;
		QString new_testers;		
		QString real_proj("name="+keys.at(0)); 
		SpcNum * calculator = qobject_cast<SpcNum *>(parent());
		for (int i = 0; i < 10; i++)
		{
			if (i == 0)
				continue;
			if (i == 1)
				col_name = "unit";
			else if (i == 2)
				col_name = "precision";
			else if (i == 3)
				col_name = "ctrlplot";
			else if (i == 4)
				col_name = "uplimit";
			else if (i == 5)
				col_name = "lowlimit";
			else if (i == 6)
				col_name = "grpsamles";
			else if (i == 7)
				col_name = "groups";
			else if (i == 8)
				col_name = "desnum";
			else if (i == 9)
				col_name = "constructor";
			if (i == 9)
				new_testers = test_dt+tr("，")+keys.at(i)+tr("，")+keys.at(10);
			else
				new_testers = test_dt+tr("，")+keys.at(i);
			QStringList old_strs = old_keys.at(i).split(tr("；"));
			QStringList ins_list;
			ins_list << new_testers;
			calculator -> insertListByTimeSequence(ins_list, old_strs);
			QString update_str = old_strs.join(tr("；"));
			if (!updateCellData("projectskeys", real_proj, col_name, update_str))
				return false;			
		}
	}
	return true;
}

bool SpcDataBase::storePictueInfoes(const QVariantList & pic_infoes)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	if (!query.prepare("insert into pictures(name, pic, type, nested) values(?, ?, ?, ?)"))
		return false;
	query.addBindValue(pic_infoes.at(0));
	query.addBindValue(pic_infoes.at(1));
	query.addBindValue(pic_infoes.at(2));
	query.addBindValue(pic_infoes.at(3));
	query.exec();
	return true;
}

bool SpcDataBase::renameTable(const QString & oldname, const QString & newname)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QString rename_order = QString("ALTER TABLE %1 RENAME TO %2").arg(oldname).arg(newname);		
	return query.exec(rename_order);
}

bool SpcDataBase::storeWholeTableFromOutside(const QString & outside_db, const QString & inside_table, const QString & outside_table)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QString attach_db = QString("ATTACH '%1' AS db2").arg(QSqlDatabase::database(outside_db).databaseName());
	query.exec(attach_db);
	QString insert_whole = QString("create table %1 as select * from db2.%2").arg(inside_table).arg(outside_table);
	query.exec(insert_whole);
	return true;
}

bool SpcDataBase::deleteDatabase(const QString & db_posname)
{	
	QFile db_file(db_posname);
	int i_remove = 0;
	while (!db_file.remove())
	{
		if (i_remove == 3)
			return false;
		i_remove++;
	}
	return true;
}

bool SpcDataBase::deleteProject(const QString & proj_name)
{
	QStringList tablelist = spc_sqldb->tables();
	foreach (QString tbl, tablelist)
	{
		if (tbl.contains(proj_name))
		{
			if (!delTable(tbl))
				return false;
		}	
	}
	QSqlTableModel ms_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("managerinfo", &ms_sql);
	for (int i = 0; i < ms_sql.rowCount(); i++)
	{
		if (ms_sql.data(ms_sql.index(i, 3)).toString().contains(proj_name))
		{
			QString proj_cell(ms_sql.data(ms_sql.index(i, 3)).toString());
			QStringList m_projs = proj_cell.split(";");		
			if (m_projs.size() == 1)
				ms_sql.removeRows(i, 1);
			else
			{
				QStringList remove_proj = m_projs.filter(proj_name);
				m_projs.removeOne(remove_proj[0]);
				QString new_cell(m_projs.join(";"));
				if (!ms_sql.setData(ms_sql.index(i, 3), new_cell))
					return false;
			}
		}
	}
	if (!ms_sql.submitAll())
		return false;
	QSqlTableModel ds_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("departments", &ds_sql);
	for (int i = 0; i < ds_sql.rowCount(); i++)
	{
		if (ds_sql.data(ds_sql.index(i, 1)).toString().contains(proj_name))
		{
			QString proj_cell(ds_sql.data(ms_sql.index(i, 1)).toString());
			QStringList d_projs = proj_cell.split(",");
			if (d_projs.size() == 1)
				ds_sql.removeRows(i, 1);
			else
			{
				QStringList remove_proj = d_projs.filter(proj_name);
				d_projs.removeOne(remove_proj[0]);
				QString new_cell(d_projs.join(","));
				if (!ds_sql.setData(ds_sql.index(i, 1), new_cell))
					return false;
			}
		}
	}
	if (!ds_sql.submitAll())
		return false;	
	QString empty;
	QString proj_row("name="+proj_name);
	if (!deleteDataes("projectskeys", proj_row, empty))
		return false;
	if (!deleteDataes("projectsinfo", proj_row, empty))
		return false;	
	resetManagersDbMap();
	resetDepartmentsDbMap();
	resetProjsDbMap();	
	return true;
}

bool SpcDataBase::deleteMngInfoesToOtherDb(const QString & to_db, const QPair<QString, QString> & manager, QString & del_result, const QString & del_proj)
{
	QString origin_conn = spc_sqldb->connectionName();
	*spc_sqldb = QSqlDatabase::database(to_db);	
 	QStringList omng_list;
	QString m_passwd;
	findMngPasswdsFromOutsideDb(to_db, manager.second, m_passwd);	
	getDataesFromTable("managerinfo", "password", omng_list, m_passwd);
	QStringList omng_projs = omng_list[3].split(";");
	QString mng_row("password="+m_passwd);		
	if (omng_projs.size() == 1)
	{
		QString mng_cols;
		if (!deleteDataes("managerinfo", mng_row, mng_cols))
		{
			del_result = tr("删除")+m_passwd+tr("失败");
			return false;					  
		}					  
	}
	else
	{
		QStringList omng_news = omng_list.at(3).split(";");
		if (!del_proj.isEmpty())
		{
			QStringList omng_rms = omng_news.filter(del_proj);
			omng_news.removeOne(omng_rms.at(0));
		}
		if (!updateCellData("managerinfo", mng_row, "projects", omng_news.join(";")))
		{
			del_result = tr("存储")+"managerinfoprojects"+tr("失败");
			return false;					  
		}				  					
	}
	QStringList odpr_list;
	getDataesFromTable("departments", "name", odpr_list, omng_list.at(2));
	QStringList odpr_news = odpr_list.at(1).split(",");
	if (!del_proj.isEmpty())
		odpr_news.removeOne(del_proj);
	QString dpr_row("name="+odpr_list.at(0));	
	if (odpr_news.isEmpty())
	{
		QString dpr_cols;
		if (!deleteDataes("departments", dpr_row, dpr_cols))
		{
			del_result = tr("删除")+m_passwd+tr("失败");
			return false;					  
		}					
	}
	else
	{
		if (!updateCellData("departments", dpr_row, "projects", odpr_news.join(",")))
		{
			del_result = tr("删除")+"departmentsprojects"+tr("失败");
			return false;					  
		}
	} 
	*spc_sqldb = QSqlDatabase::database(origin_conn);
	return true;
}

bool SpcDataBase::delTable(const QString & tablename)
{
	QStringList tablelist = spc_sqldb->tables();
	if (!tablelist.contains(tablename))
		return true;
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QString del_all = QString("DROP TABLE %1").arg(tablename);
	query.exec(del_all);
	return true;
}

bool SpcDataBase::deleteDataes(const QString & tablename, QString & rows, QString & cols)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	if (!rows.isEmpty())
	{
		QStringList tol_rows = rows.split("=");
		QStringList rows_list = tol_rows[1].split(tr("，，。"));
		foreach (QString n_row, rows_list)
		{
			QString del_row = QString("delete from %1 where %2 = '%3'").arg(tablename).arg(tol_rows[0]).arg(n_row);
			query.exec(del_row);
		}
	}
	if (!cols.isEmpty())
	{
		QStringList tol_cols = cols.split("=");
		QStringList cols_list = tol_cols[1].split(tr("，，。"));
		foreach (QString n_col, cols_list)
		{
			QString del_col = QString("ALTER TABLE %1 DROP COLUMN %2").arg(tablename).arg(n_col);
			query.exec(del_col);
		}		
	}
	return true;	
}

bool SpcDataBase::deleteCelldata(const QString & tablename, QString & row, const QString & col, const QString & data)
{
	QString dest_data = dataFromTable(tablename, col, row).toString();
	QStringList match_list1 = dest_data.split(";");
	for (int i = 0; i < match_list1.size(); i++)
	{
		if (match_list1.at(i) == data)
		{
			match_list1.removeAll(data);
			QString end_str = match_list1.join(";");
			return updateCellData(tablename, row, col, end_str);
		}
		if (match_list1.at(i).contains(data) && match_list1.at(i)!=data)
		{
			QStringList end_list = match_list1[i].split(",");
			end_list.removeAll(data);
			match_list1[i] = end_list.join(",");
			QString end_str = match_list1.join(";");
			return updateCellData(tablename, row, col, end_str);
		}
	}
	return false;
}

bool SpcDataBase::updateCellData(const QString & tablename, QString & rowname, const QString & column, const QString & data)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QStringList tables_db = spc_sqldb->tables();
	if (!tables_db.contains(tablename))
		return false;
	QStringList trans_list = rowname.split("=");
	QString split_one = trans_list[0];
	QString split_two = trans_list[1];
	if (data.isEmpty())
		query.exec(QString("update %1 set %2 = null where %3 = '%4'").arg(tablename).arg(column).arg(split_one).arg(split_two));
	else
		query.exec(QString("update %1 set %2 = '%3' where %4 = '%5'").arg(tablename).arg(column).arg(data).arg(split_one).arg(split_two));
	return true;
}

bool SpcDataBase::storeData(const QString & tablename, QString & rowname, const QString & column, const QString & data)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QStringList tables_db = spc_sqldb->tables();
	if (!tables_db.contains(tablename))
		return false;
	QStringList trans_list = rowname.split("=");
	QString split_one = trans_list[0];
	QString split_two = trans_list[1];
	query.exec(QString("select * from %1 where %2 = '%3'").arg(tablename).arg(split_one).arg(split_two));
	QString old_data = dataFromTable(tablename, column, rowname).toString();
	if (old_data.contains(data))
		return true;
	if (dataFromTable(tablename, column, rowname).isNull())
		query.exec(QString("update %1 set %2 = '%3' where %4 = '%5'").arg(tablename).arg(column).arg(data).arg(split_one).arg(split_two));
	QString old_info = dataFromTable(tablename, column, rowname).toString();
/*	if (!old_info.contains(";"))
		old_info += ",";
	else
		old_info += ";";*/
	if (data == "null")
		query.exec(QString("update %1 set %2 = null where %3 = '%4'").arg(tablename).arg(column).arg(split_one).arg(split_two));
	else
	{
		old_info += data;
		query.exec(QString("update %1 set %2 = '%3' where %4 = '%5'").arg(tablename).arg(column).arg(old_info).arg(split_one).arg(split_two));	
	}
	return true;
}

bool SpcDataBase::databaseSavePlots(const QString & person, QString & input_name, TableManagement * detailed, LittleWidgetsView * viewer, const QString & construct_time)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QMap<QPair<int, int>, PlotWidget *> plots_map = viewer->qmlAgent()->allCurrentPlots();
	if (plots_map.isEmpty())
		return false;
	QDateTime creat_time(QDateTime::currentDateTime());	
	QString t_stamp;
	if (construct_time.isEmpty())
		t_stamp = QString("%1").arg(QDateTime::currentDateTime().toTime_t());
	else
		t_stamp = QString("%1").arg(QDateTime::fromString(construct_time).toTime_t());	
	QString old_name = person+tr("，，。")+"plots"+QString("%1").arg(detailed->treeType())+tr("，，。")+t_stamp+tr("，，。")+input_name;
	QPair<int, int> plots_rcs = viewer->qmlAgent()->realRowsColsInGridLayout();
	QString tol_Matrix = QString("%1").arg(plots_rcs.first)+","+QString("%1").arg(plots_rcs.second);
	PlotWidget * cell_plot = plots_map.values().at(0);
	tol_Matrix += tr("；")+QString("%1").arg(cell_plot->endRect().width())+","+QString("%1").arg(cell_plot->endRect().height());
	QMapIterator<QPair<int, int>, PlotWidget *> i_pm(plots_map);
	while (i_pm.hasNext())
	{
		i_pm.next();
		if (i_pm.value()->width() > cell_plot->endRect().width())
			tol_Matrix += ";"+QString("%1").arg(i_pm.key().first)+","+QString("%1").arg(i_pm.key().second)+","+QString("%1").arg(qRound(i_pm.value()->width()/cell_plot->endRect().width()));		
	}	
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(detailed->model());
	QStandardItem * root_item = t_model->item(0);
	QList<QStandardItem *> find_roots;
	t_model -> findChildrenNotAll(root_item, find_roots);
	QList<QStandardItem *> plots_list;
	QString insert_pics = QString("insert into pictures(name, pic, type, nested) values(?, ?, ?, ?)");	
	foreach(QStandardItem * item, find_roots)
	{
		QList<QStandardItem *> general_types;
		t_model -> findAllChildItems(item, general_types);
		foreach(QStandardItem * t_item, general_types)
		{
			if (t_item->text().contains(tr("视图位置：")))
			{
				QStringList pos_list = t_item->text().split(tr("，"));
				QStringList row_list = pos_list[0].split(tr("视图位置："));
				QPair<int, int> plot_key(row_list[1].toInt()-1, pos_list[1].toInt()-1);	
				PlotWidget * s_plot = plots_map.value(plot_key);
				s_plot->exsitedBtn()->hide();
				QString p_name = tr("单元图")+QString("%1").arg(inspector->randNumberGenerate());
				QPixmap pixmap(s_plot->size());
				pixmap.fill(Qt::transparent);
				s_plot -> render(&pixmap);					
				QByteArray bytes;
				QBuffer buffer(&bytes);
				buffer.open(QIODevice::WriteOnly);
				pixmap.save(&buffer, "PNG");
				QString nested_tbl(old_name+tr("，。，")+t_item->text());
				query.prepare(insert_pics);
				query.addBindValue(p_name);
				query.addBindValue((QVariant)bytes);
				query.addBindValue("png");
				query.addBindValue(nested_tbl);
				query.exec();
				query.next();
				bytes.clear();
				buffer.close();		
				plots_list << t_item->parent();
				s_plot->exsitedBtn()->show();
			}
		}			
	}
	QString t_create = QString("create table ")+old_name+QString("(matrix varchar, items varchar, texthash varchar, namehash varchar, modeify datetime)");
	query.exec(t_create);
	QString plots_items;
	QString plots_texts;
	QString plots_makers;
	foreach (QStandardItem * plot, plots_list)
	{
		QString mid_items;
		detailed -> itemsInfoesTransToDB(plot, mid_items, plots_texts, plots_makers);
		QString key_lbl(tr("测量基础数据"));
		QList<QStandardItem *> judge_list;
		t_model -> findItemsSingleLine(plot, judge_list, key_lbl);
		if (judge_list.at(0)->text() == key_lbl)
		{
			QStringList redef_strs = mid_items.split(tr("，，～："));
			QStringList redef_agns = redef_strs.at(0).split(tr("，，～"));
			QString end_str = tr("，")+plot->parent()->text()+tr("，，～：")+redef_strs.at(1);
			plots_items += plot->parent()->parent()->child(0)->child(0)->text()+end_str+tr("，，&；");
		}
		else
			plots_items += plot->parent()->parent()->parent()->child(0)->child(0)->text()+tr("，")+plot->parent()->parent()->text()+tr("，，～：")+mid_items+tr("，，&；");		
	}
	QString insert_str = QString("insert into %1(matrix, items, texthash, namehash, modeify) values(?, ?, ?, ?, ?)").arg(old_name);
	query.prepare(insert_str);
	query.addBindValue(tol_Matrix);
	query.addBindValue(plots_items);
	query.addBindValue(plots_texts);
	query.addBindValue(plots_makers);
	query.addBindValue(creat_time);
	query.exec();
	return true;
}

bool SpcDataBase::saveManualTable(const QString & person, QString & input_name, TableManagement * detailed, LittleWidgetsView * viewer, const QString & construct_time)// maybe the same time for different file??
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	TablesGroup * g_save = qobject_cast<TablesGroup *>(viewer->qmlAgent()->widget());
	QStandardItemModel * save_model = qobject_cast<QStandardItemModel *>(g_save->model());
	bool empty_model = true;
	for (int i = 0; i < save_model->columnCount(); i++)
	{
		for (int j = 0; j < save_model->rowCount(); j++)
		{
			if (!save_model->data(save_model->index(i, j)).isNull() || g_save->indexWidget(save_model->index(i, j)))
				empty_model = false;
		}
	}
	if (empty_model)
		return false;
	QString t_stamp;
	if (construct_time.isEmpty())
		t_stamp = QString("%1").arg(QDateTime::currentDateTime().toTime_t());
	else
		t_stamp = QString("%1").arg(QDateTime::fromString(construct_time).toTime_t());
	QString t_name = person+tr("，，。")+"manualtable"+QString("%1").arg(detailed->treeType())+tr("，，。")+t_stamp+tr("，，。")+input_name;
	QString table_name = QString("create table %1(contents varchar, rcnum varchar, frameline varchar, modeify datetime, modeifier varchar)").arg(t_name);
	QHash<QString, QModelIndexList> save_hash;
	g_save -> resetFrameHashForSave(save_hash);
	QString frame_infoes;
	QHashIterator<QString, QModelIndexList> i_hash(save_hash);
	while (i_hash.hasNext())
	{
		i_hash.next();
		QString f_strs = i_hash.key()+tr("，（");
		foreach (QModelIndex index, i_hash.value())
			f_strs += QString("%1").arg(index.row())+","+QString("%1").arg(index.column())+";";
		frame_infoes += f_strs+tr("）；");
	}	
	QString all_rows;
	QString all_cols;
	QString all_contents;
	QString insert_pics = QString("insert into pictures(name, pic, type, nested) values(?, ?, ?, ?)");
	for (int i = 0; i < save_model->rowCount(); i++)
	{
		all_rows += QString("%1").arg(g_save->rowHeight(i))+","; 
		for (int j = 0; j < save_model->columnCount(); j++)
		{
			if (i == 0)
				all_cols += QString("%1").arg(g_save->columnWidth(j))+",";
			if (!g_save->indexHidden(save_model->index(i, j)))
			{
				QString m_pos = "*"+QString("%1").arg(i)+","+QString("%1").arg(j)+"*";
				all_contents += m_pos;			  
				QString m_content;
				QMap<int, QVariant> c_dataes = save_model->itemData(save_model->index(i, j));
				QMapIterator<int, QVariant> c_map(c_dataes);
				while (c_map.hasNext())
				{
					c_map.next();
					QString v_str = c_map.value().toString();					
					if (c_map.key() == Qt::DecorationRole)
					{
						QString p_name = tr("单元图")+QString("%1").arg(inspector->randNumberGenerate());
						QByteArray bytes;
						QBuffer buffer(&bytes);
						buffer.open(QIODevice::WriteOnly);
						c_map.value().value<QPixmap>().save(&buffer, "PNG");						
						query.prepare(insert_pics);
						query.addBindValue(p_name);
						query.addBindValue((QVariant)bytes);
						query.addBindValue("png");
						query.addBindValue(t_name);
						query.exec();
						query.next();
						bytes.clear();
						buffer.close();						
						v_str = p_name;
					}					
					m_content += QString("%1").arg(c_map.key())+tr("，^，")+v_str+tr("，^；");
				}
				if (!m_content.isEmpty())
					all_contents += tr("，，～，")+m_content;				
				QString cell_span = QString("%1").arg(g_save->rowSpan(i, j))+"$&"+QString("%1").arg(g_save->columnSpan(i, j));
				all_contents += tr("，，～，")+cell_span+tr("，，～；");
			}
		}
	}
	QString all_rc = all_rows+";"+all_cols;
	query.exec(table_name);
	QString insert_strs = QString("insert into %1(contents, rcnum, frameline, modeify, modeifier) values(?, ?, ?, ?, ?)").arg(t_name);
	query.prepare(insert_strs);
	query.addBindValue(all_contents);
	query.addBindValue(all_rc);
	query.addBindValue(frame_infoes);
	query.addBindValue(t_stamp);
	query.addBindValue(person);
	query.exec();
	query.next();
	return true;		
}

bool SpcDataBase::databaseResaveRelativeManuals(const QString & person, QString & input_name, TableManagement * detailed, LittleWidgetsView * viewer)
{
	if (!delTable(input_name))
		return false;
	QStringList real_input = input_name.split(tr("，，。"));
	if (detailed->treeType()<6 && detailed->treeType()>2)
	{
		QSqlTableModel pics_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
		varsModelFromTable("pictures", &pics_sql);
		for (int i = 0; i < pics_sql.rowCount(); i++)
		{
			if (pics_sql.data(pics_sql.index(i, pics_sql.columnCount()-2)).toString().contains(input_name))
				pics_sql.removeRows(i, 1);
		}
		if (!pics_sql.submitAll())
			return false;
		if (!databaseSavePlots(person, real_input.back(), detailed, viewer, real_input[2]))
			return false;
	}
	else
	{
		if (!saveManualTable(person, real_input.back(), detailed, viewer, real_input[2]))
			return false;
	}
	return true;	
}

bool SpcDataBase::saveBackGroundsForManualTable(TablesGroup * back_source) // qmodelindex to qvariant is in qt5.0 //no finished
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QString t_title = back_source->model()->data(back_source->model()->index(0, 0)).toString();
	QString source_name = t_title+"_backgrounds";
	return true;
}

bool SpcDataBase::saveAloneFreeConstruction(const QString & person, const QString & t_name, NestedModel * detailed_model)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));  
	QString table_name = QString("create table %1(items varchar, modeify datetime, modeifier varchar)").arg(t_name);
	query.exec(table_name);	
	QDateTime exact_time(QDateTime::currentDateTime());
	QString insert_strs = QString("insert into %1(items, modeify, modeifier) values(?, ?, ?)").arg(t_name);
	QString item_infoes;
	detailed_model -> translatedItemInfoes(item_infoes);
	query.prepare(insert_strs);
	query.addBindValue(item_infoes);
	query.addBindValue(exact_time);
	query.addBindValue(person);
	query.exec();
	query.next();
	return true;
}

bool SpcDataBase::theSameProjFile(const QString & name, QSqlDatabase * dest_db)
{
	QStringList tbls_list;
	if (dest_db)
		tbls_list = dest_db->tables();
	else		
		tbls_list = spc_sqldb->tables();
	if (tbls_list.contains(name))
		return true;
	QStringList fur_chk = name.split(tr("，，。"));
	foreach (QString e_tbl, tbls_list)
	{
		if (e_tbl.contains(fur_chk[0]) && e_tbl.contains(fur_chk[1]))
			return true;
	}
	return false;
}
	
bool SpcDataBase::createParasTable(QSqlDatabase & dest_db)//need promotion for db constructor
{
	QSqlQuery query(dest_db.database(dest_db.connectionName()));
	query.exec("create table spcparaes(groupnum int primary key, A double, A2 double, A3 double, c4 double, B3 double, B4 double, B5 double, B6 double, d2 double, d3 double, D1 double, DD2 double, DD3 double, D4 double)");	
	query.exec("insert into spcparaes values(2, 2.121, 1.88, 2.659, 0.7979, 0, 3.267, 0, 2.606, 1.128, 0.853, 0, 3.686, 0, 3.267)");
	query.exec("insert into spcparaes values(3, 1.732, 1.023, 1.954, 0.8862, 0, 2.568, 0, 2.276, 1.693, 0.888, 0, 4.358, 0, 2.574)");
	query.exec("insert into spcparaes values(4, 1.5, 0.729, 1.628, 0.9213, 0, 2.266, 0, 2.088, 2.059, 0.88, 0, 4.698, 0, 2.282)");	
	query.exec("insert into spcparaes values(5, 1.342, 0.577, 1.427, 0.94, 0, 2.089, 0, 1.964, 2.326, 0.864, 0, 4.918, 0, 2.114)");
	query.exec("insert into spcparaes values(6, 1.225, 0.483, 1.287, 0.9515, 0.03, 1.97, 0.029, 1.874, 2.534, 0.848, 0, 5.078, 0, 2.004)");
	query.exec("insert into spcparaes values(7, 1.134, 0.419, 1.182, 0.9594, 0.118, 1.882, 0.113, 1.806, 2.704, 0.833, 0.204, 5.204, 0.076, 1.924)");
	query.exec("insert into spcparaes values(8, 1.061, 0.373, 1.099, 0.965, 0.185, 1.815, 0.179, 1.751, 2.847, 0.82, 0.388, 5.306, 0.136, 1.864)");
	query.exec("insert into spcparaes values(9, 1, 0.337, 1.032, 0.9693, 0.239, 1.761, 0.232, 1.707, 2.97, 0.808, 0.547, 5.393, 0.184, 1.816)");
	query.exec("insert into spcparaes values(10, 0.949, 0.308, 0.975, 0.9727, 0.284, 1.716, 0.276, 1.669, 3.078, 0.797, 0.687, 5.469, 0.223, 1.777)");
	query.exec("insert into spcparaes values(11, 0.905, 0.285, 0.927, 0.9754, 0.321, 1.679, 0.313, 1.637, 3.173, 0.787, 0.811, 5.535, 0.256, 1.744)");
	query.exec("insert into spcparaes values(12, 0.866, 0.266, 0.886, 0.9776, 0.354, 1.646, 0.346, 1.61, 3.258, 0.778, 0.922, 5.594, 0.283, 1.717)");
	query.exec("insert into spcparaes values(13, 0.832, 0.249, 0.85, 0.9794, 0.382, 1.618, 0.374, 1.585, 3.336, 0.77, 1.025, 5.647, 0.307, 1.693)");
	query.exec("insert into spcparaes values(14, 0.802, 0.235, 0.817, 0.981, 0.406, 1.594, 0.399, 1.563, 3.407, 0.763, 1.118, 5.696, 0.328, 1.672)");
	query.exec("insert into spcparaes values(15, 0.775, 0.223, 0.789, 0.9823, 0.428, 1.572, 0.421, 1.544, 3.472, 0.756, 1.203, 5.741, 0.347, 1.653)");
	query.exec("insert into spcparaes values(16, 0.75, 0.212, 0.763, 0.9835, 0.448, 1.552, 0.44, 1.526, 3.532, 0.75, 1.282, 5.782, 0.363, 1.637)");
	query.exec("insert into spcparaes values(17, 0.728, 0.203, 0.739, 0.9845, 0.466, 1.534, 0.458, 1.511, 3.588, 0.744, 1.356, 5.82, 0.378, 1.622)");
	query.exec("insert into spcparaes values(18, 0.707, 0.194, 0.718, 0.9854, 0.482, 1.518, 0.475, 1.496, 3.64, 0.739, 1.424, 5.856, 0.391, 1.608)");
	query.exec("insert into spcparaes values(19, 0.688, 0.187, 0.698, 0.9862, 0.497, 1.503, 0.49, 1.483, 3.689, 0.734, 1.487, 5.891, 0.403, 1.597)");
	query.exec("insert into spcparaes values(20, 0.671, 0.18, 0.68, 0.9869, 0.51, 1.49, 0.504, 1.47, 3.735, 0.729, 1.549, 5.921, 0.415, 1.585)");
	query.exec("insert into spcparaes values(21, 0.655, 0.173, 0.663, 0.9876, 0.523, 1.477, 0.516, 1.459, 3.778, 0.724, 1.605, 5.951, 0.425, 1.575)");
	query.exec("insert into spcparaes values(22, 0.64, 0.167, 0.647, 0.9882, 0.534, 1.466, 0.528, 1.448, 3.819, 0.72, 1.659, 5.979, 0.434, 1.566)");
	query.exec("insert into spcparaes values(23, 0.626, 0.162, 0.633, 0.9887, 0.545, 1.455, 0.539, 1.438, 3.858, 0.716, 1.71, 6.006, 0.443, 1.557)");
	query.exec("insert into spcparaes values(24, 0.612, 0.157, 0.619, 0.9892, 0.555, 1.445, 0.549, 1.429, 3.895, 0.712, 1.759, 6.031, 0.451, 1.548)");
	query.exec("insert into spcparaes values(25, 0.6, 0.153, 0.606, 0.9896, 0.565, 1.435, 0.559, 1.42, 3.931, 0.708, 1.806, 6.056, 0.459, 1.541)");
	query.exec("create table norproparaes(frequency double primary key, fractile double)");	
	query.exec("insert into norproparaes values(0.01, 1.19)");
	query.exec("insert into norproparaes values(0.02, 1.36)");
	query.exec("insert into norproparaes values(0.03, 1.47)");
	query.exec("insert into norproparaes values(0.04, 1.55)");	
	query.exec("insert into norproparaes values(0.05, 1.61)");
	query.exec("insert into norproparaes values(0.06, 1.66)");
	query.exec("insert into norproparaes values(0.07, 1.71)");
	query.exec("insert into norproparaes values(0.08, 1.74)");
	query.exec("insert into norproparaes values(0.09, 1.78)");
	query.exec("insert into norproparaes values(0.1, 1.81)");
	query.exec("insert into norproparaes values(0.2, 2.02)");
	query.exec("insert into norproparaes values(0.3, 2.15)");
	query.exec("insert into norproparaes values(0.4, 2.25)");
	query.exec("insert into norproparaes values(0.5, 2.32)");
	query.exec("insert into norproparaes values(0.6, 2.39)");
	query.exec("insert into norproparaes values(0.7, 2.44)");
	query.exec("insert into norproparaes values(0.8, 2.49)");
	query.exec("insert into norproparaes values(0.9, 2.53)");
	query.exec("insert into norproparaes values(1.0, 2.58)");
	query.exec("insert into norproparaes values(2.0, 2.85)");
	query.exec("insert into norproparaes values(3.0, 3.02)");
	query.exec("insert into norproparaes values(4.0, 3.15)");
	query.exec("insert into norproparaes values(5.0, 3.26)");
	query.exec("insert into norproparaes values(6.0, 3.35)");
	query.exec("insert into norproparaes values(7.0, 3.42)");
	query.exec("insert into norproparaes values(8.0, 3.49)");
	query.exec("insert into norproparaes values(9.0, 3.56)");
	query.exec("insert into norproparaes values(10.0, 3.62)");
	query.exec("insert into norproparaes values(11.0, 3.67)");
	query.exec("insert into norproparaes values(12.0, 3.73)");
	query.exec("insert into norproparaes values(13.0, 3.77)");
	query.exec("insert into norproparaes values(14.0, 3.82)");
	query.exec("insert into norproparaes values(15.0, 3.86)");
	query.exec("insert into norproparaes values(16.0, 3.91)");
	query.exec("insert into norproparaes values(17.0, 3.95)");
	query.exec("insert into norproparaes values(18.0, 3.98)");
	query.exec("insert into norproparaes values(19.0, 4.02)");
	query.exec("insert into norproparaes values(20.0, 4.06)");
	query.exec("insert into norproparaes values(21.0, 4.09)");
	query.exec("insert into norproparaes values(22.0, 4.13)");
	query.exec("insert into norproparaes values(23.0, 4.16)");
	query.exec("insert into norproparaes values(24.0, 4.19)");
	query.exec("insert into norproparaes values(25.0, 4.23)");
	query.exec("insert into norproparaes values(26.0, 4.26)");
	query.exec("insert into norproparaes values(27.0, 4.29)");
	query.exec("insert into norproparaes values(28.0, 4.32)");
	query.exec("insert into norproparaes values(29.0, 4.35)");
	query.exec("insert into norproparaes values(30.0, 4.38)");
	query.exec("insert into norproparaes values(31.0, 4.41)");
	query.exec("insert into norproparaes values(32.0, 4.43)");
	query.exec("insert into norproparaes values(33.0, 4.46)");
	query.exec("insert into norproparaes values(34.0, 4.49)");
	query.exec("insert into norproparaes values(35.0, 4.51)");
	query.exec("insert into norproparaes values(36.0, 4.53)");
	query.exec("insert into norproparaes values(37.0, 4.57)");
	query.exec("insert into norproparaes values(38.0, 4.59)");
	query.exec("insert into norproparaes values(39.0, 4.62)");
	query.exec("insert into norproparaes values(40.0, 4.65)");
	query.exec("insert into norproparaes values(41.0, 4.67)");
	query.exec("insert into norproparaes values(42.0, 4.7)");
	query.exec("insert into norproparaes values(43.0, 4.72)");
	query.exec("insert into norproparaes values(44.0, 4.75)");
	query.exec("insert into norproparaes values(45.0, 4.77)");
	query.exec("insert into norproparaes values(46.0, 4.8)");
	query.exec("insert into norproparaes values(47.0, 4.82)");
	query.exec("insert into norproparaes values(48.0, 4.85)");
	query.exec("insert into norproparaes values(49.0, 4.87)");
	query.exec("insert into norproparaes values(50.0, 4.9)");
	query.exec("insert into norproparaes values(51.0, 4.92)");
	query.exec("insert into norproparaes values(52.0, 4.95)");
	query.exec("insert into norproparaes values(53.0, 4.98)");
	query.exec("insert into norproparaes values(54.0, 5.0)");
	query.exec("insert into norproparaes values(55.0, 5.03)");
	query.exec("insert into norproparaes values(56.0, 5.05)");
	query.exec("insert into norproparaes values(57.0, 5.08)");
	query.exec("insert into norproparaes values(58.0, 5.1)");
	query.exec("insert into norproparaes values(59.0, 5.13)");
	query.exec("insert into norproparaes values(60.0, 5.15)");
	query.exec("insert into norproparaes values(61.0, 5.18)");
	query.exec("insert into norproparaes values(62.0, 5.21)");
	query.exec("insert into norproparaes values(63.0, 5.23)");
	query.exec("insert into norproparaes values(64.0, 5.26)");
	query.exec("insert into norproparaes values(65.0, 5.29)");
	query.exec("insert into norproparaes values(66.0, 5.31)");
	query.exec("insert into norproparaes values(67.0, 5.34)");
	query.exec("insert into norproparaes values(68.0, 5.37)");
	query.exec("insert into norproparaes values(69.0, 5.4)");
	query.exec("insert into norproparaes values(70.0, 5.42)");
	query.exec("insert into norproparaes values(71.0, 5.45)");
	query.exec("insert into norproparaes values(72.0, 5.48)");
	query.exec("insert into norproparaes values(73.0, 5.51)");
	query.exec("insert into norproparaes values(74.0, 5.54)");
	query.exec("insert into norproparaes values(75.0, 5.57)");
	query.exec("insert into norproparaes values(76.0, 5.61)");
	query.exec("insert into norproparaes values(77.0, 5.64)");
	query.exec("insert into norproparaes values(78.0, 5.67)");
	query.exec("insert into norproparaes values(79.0, 5.71)");
	query.exec("insert into norproparaes values(80.0, 5.74)");
	query.exec("insert into norproparaes values(81.0, 5.78)");
	query.exec("insert into norproparaes values(82.0, 5.82)");
	query.exec("insert into norproparaes values(83.0, 5.85)");
	query.exec("insert into norproparaes values(84.0, 5.89)");
	query.exec("insert into norproparaes values(85.0, 5.94)");
	query.exec("insert into norproparaes values(86.0, 5.98)");
	query.exec("insert into norproparaes values(87.0, 6.03)");
	query.exec("insert into norproparaes values(88.0, 6.07)");
	query.exec("insert into norproparaes values(89.0, 6.13)");
	query.exec("insert into norproparaes values(90.0, 6.18)");
	query.exec("insert into norproparaes values(91.0, 6.24)");
	query.exec("insert into norproparaes values(92.0, 6.31)");
	query.exec("insert into norproparaes values(93.0, 6.38)");
	query.exec("insert into norproparaes values(94.0, 6.45)");
	query.exec("insert into norproparaes values(95.0, 6.54)");
	query.exec("insert into norproparaes values(96.0, 6.65)");
	query.exec("insert into norproparaes values(97.0, 6.78)");
	query.exec("insert into norproparaes values(98.0, 6.95)");
	query.exec("insert into norproparaes values(99.0, 7.22)");
	query.exec("insert into norproparaes values(99.1, 7.27)");
	query.exec("insert into norproparaes values(99.2, 7.31)");
	query.exec("insert into norproparaes values(99.3, 7.36)");
	query.exec("insert into norproparaes values(99.4, 7.41)");
	query.exec("insert into norproparaes values(99.5, 7.48)");
	query.exec("insert into norproparaes values(99.6, 7.55)");
	query.exec("insert into norproparaes values(99.7, 7.65)");
	query.exec("insert into norproparaes values(99.8, 7.78)");
	query.exec("insert into norproparaes values(99.9, 7.99)");
	query.exec("insert into norproparaes values(99.91, 8.02)");
	query.exec("insert into norproparaes values(99.92, 8.06)");
	query.exec("insert into norproparaes values(99.93, 8.09)");
	query.exec("insert into norproparaes values(99.94, 8.14)");
	query.exec("insert into norproparaes values(99.95, 8.19)");
	query.exec("insert into norproparaes values(99.96, 8.25)");
	query.exec("insert into norproparaes values(99.97, 8.33)");
	query.exec("insert into norproparaes values(99.98, 8.44)");
	query.exec("insert into norproparaes values(99.99, 8.62)");
	query.exec("create table managerinfo(name varchar primary key, password varchar, department varchar, projects varchar, title varchar)");
	query.exec("create table projectsinfo(name varchar primary key, departments varchar, managers varchar, authoagent varchar, attribution varchar, authorizing varchar)");
	query.exec("create table departments(name varchar primary key, projects varchar, managers varchar, company varchar)");
	query.exec("create table projectskeys(name varchar primary key, unit varchar, precision varchar, ctrlplot varchar, uplimit varchar, lowlimit varchar, grpsamles varchar, groups varchar, desnum varchar, constructor varchar, constructtime datetime)");
	query.exec("create table pictures(name varchar primary key, pic image, type varchar, nested varchar, constructor varchar)");
	query.exec("create table dbInformations(name varchar primary key, constructor varchar, constructtime datetime, authoagent varchar, bkfrom varchar, bkto varchar)");
	QStringList tablelist = dest_db.tables();
	if (!tablelist.contains("spcparaes") || !tablelist.contains("norproparaes") || !tablelist.contains("managerinfo") || !tablelist.contains("projectsinfo") || !tablelist.contains("departments") || !tablelist.contains("projectskeys") || !tablelist.contains("pictures") || !tablelist.contains("dbInformations"))
	{
 		QMessageBox::critical(0, tr("严重错误"), tr("该数据库参数表格未正确创建！"));
		return false;
	}
	return true;
}

bool SpcDataBase::initFillForDbProperty(const QStringList & info_list, QSqlDatabase * dest_db)
{
	QSqlQuery query(dest_db->database(dest_db->connectionName()));
	if (!query.prepare("insert into dbInformations(name, constructor, constructtime) values(:name, :constructor, :constructtime)"))
		return false;
	QDateTime c_time = QDateTime::fromString(info_list[2]);
	query.bindValue(":name", info_list.at(0));
	query.bindValue(":constructor", info_list.at(1));
	query.bindValue(":constructtime", c_time);
	query.exec();
	return true;
}

bool SpcDataBase::lastSpcDataesSaveResult(const QString & tablename)
{
	QSqlTableModel p_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable(tablename, &p_sql);
	if (p_sql.rowCount() == 0)
		return true;
	for (int col = 0; col < p_sql.columnCount(); col++)
	{
		if (p_sql.data(p_sql.index(p_sql.rowCount()-1,  col)).isNull())
			return false;
	}
	return true;
}

bool SpcDataBase::testEngiPerformance(const QString & proj, bool next, const QString & engi_version)
{
	QStringList all_tbls = spc_sqldb->tables(); 	
	QString name_row("name="+proj);
	QStringList keys_cell = dataFromTable("projectskeys", "unit", name_row).toString().split(tr("；"));
	if (keys_cell.isEmpty())
		return false;	
	if (engi_version.isEmpty())
	{  
		foreach (QString k_str, keys_cell)
		{
			QStringList ft_key = k_str.split(tr("，"));
			QString tengi_stamp = QString("%1").arg(QDateTime::fromString(ft_key[0]).toTime_t());
			QStringList proj_cpks = all_tbls.filter(tengi_stamp);			
			if (!next)
			{
				if (!proj_cpks.isEmpty())
					return true;
			}
			else
			{
				QStringList time_list;
				getDataesFromTable(proj_cpks.at(0), "time", time_list);				
				foreach (QString d_time, time_list)
				{
					QString daily_stamp = QString("%1").arg(QDateTime::fromString(d_time, Qt::ISODate).toTime_t());
					QStringList crude_daily = all_tbls.filter(daily_stamp);
					QStringList proj_daily = crude_daily.filter(tr("，，。dailydataes"));
					if (!proj_daily.isEmpty())
						return true;	
				}
			}
		}
	}
	else
	{
		QString tengi_stamp = QString("%1").arg(QDateTime::fromString(engi_version).toTime_t());
		QStringList version_cpks = all_tbls.filter(tengi_stamp);
		if (!version_cpks.isEmpty())
			return true;
	}
	return false;
}

bool SpcDataBase::projectContainsTimeStr(const QString & time_str, const QString & c_proj)
{
	bool contained = false;
	QStringList all_tables = allTables();
	QStringList dest_tbls;
	foreach (QString tbl, all_tables)
	{
		if (tbl.contains(c_proj) && tbl.contains(tr("，，。cpkdataes")))
			dest_tbls << tbl;
	}
	if (dest_tbls.isEmpty())
		return contained;
	QString iso_time = QDateTime::fromString(time_str).toString(Qt::ISODate);
	QStringList time_list;	
	foreach (QString tbl, dest_tbls)
	{
		time_list.clear();
		getDataesFromTable(tbl, "time", time_list);
		if (time_list.contains(iso_time))
			return true;
	}
	return contained;
}

bool SpcDataBase::mergePureProjBetweenDbs(const QString & merged_proj, const QString & to_db, QString & merge_result)
{
	QSqlTableModel odb_model(this, QSqlDatabase::database(to_db));
	varsModelFromTable("dbInformations", &odb_model);	
	QString odb_time = odb_model.data(odb_model.index(0, 2)).toString(); 
	QString origin_conn = spc_sqldb->connectionName(); 
	QString judge_name("name="+merged_proj);			
	QVariant v_time = dataFromTable("projectskeys", "constructtime", judge_name);
	QStringList merged_keys;
	getDataesFromTable("projectskeys", "name", merged_keys, merged_proj);
	*spc_sqldb = QSqlDatabase::database(to_db);
	QString that_proj = dataFromTableByTime(v_time.toDateTime(), "projectskeys", "name", "constructtime").toString();	
	*spc_sqldb = QSqlDatabase::database(origin_conn);	
	if (!projsKeyDataesEqualBetweenDbs(to_db, v_time.toString()))
	{
		QDateTime time_indb = v_time.toDateTime();	
		QSqlTableModel projs_indb(this, QSqlDatabase::database(to_db));
		varsModelFromTable("projectskeys", &projs_indb);
		bool act = false;
		for (int i = 0; i < projs_indb.rowCount(); i++)
		{
			if (projs_indb.data(projs_indb.index(i, projs_indb.columnCount()-1)).toDateTime() == time_indb)
			{			  
				for (int j = 1; j < projs_indb.columnCount()-1; j++)
				{
					QString pos_contents = projs_indb.data(projs_indb.index(i, j)).toString();
					QStringList insed_tests = pos_contents.split(tr("；"));
					QStringList insing_tests = merged_keys.at(j).split(tr("；"));
					if (j == 1)
					{
						QString comparing;			  
						inspector -> compareTwoTimeLists(insing_tests, insed_tests, comparing);
						if (comparing == tr("时间串被包含"))
							break;
						act = true;
					}
					inspector -> insertListByTimeSequence(insing_tests, insed_tests);
					QString new_data = insed_tests.join(tr("；"));
					projs_indb.setData(projs_indb.index(i, j), new_data);
					merged_keys[j] = new_data;
				}
				break;
			}
		}
		if (act && !projs_indb.submitAll())
		{
			merge_result = tr("存储projectskeys失败");
			return false;
		}
	}
	*spc_sqldb = QSqlDatabase::database(origin_conn);	
	if (!projsInfoDataesEqualBetweenDbs(to_db, merged_proj, v_time.toString()))	
	{
		if (!mergePureProjManagersBetweenDbs(merged_proj, to_db, merge_result))
			return false;
	}
	if (!projAssistTblsActionBetweenDbs(to_db, merged_proj, that_proj, merge_result, false))
		return false;
	QStringList inside_tbls = spc_sqldb->tables();
	QStringList todb_tbls = QSqlDatabase::database(to_db).tables();	
	QStringList chk_cdtbls = merged_keys.at(1).split(tr("；"));		
	foreach (QString stamp, chk_cdtbls)
	{
		QStringList real_stamps = stamp.split(tr("，"));
		QString real_stamp = QString("%1").arg(QDateTime::fromString(real_stamps.at(0)).toTime_t());
		QStringList inside_cdtbls = inside_tbls.filter(real_stamp);
		QStringList inside_dailys = inside_cdtbls.filter(tr("，，。daily"));
		foreach (QString tbl, inside_dailys)
			inside_cdtbls.removeOne(tbl);
		QStringList todb_cdtbls = todb_tbls.filter(real_stamp);	
		QStringList todb_dailys = todb_cdtbls.filter(tr("，，。daily"));
		foreach (QString cd_tbl, todb_dailys)
			todb_cdtbls.removeOne(cd_tbl);
		foreach (QString rm_tbl, inside_cdtbls)
		{
			QStringList ch_strs = rm_tbl.split(tr("，，。"));
			if (todb_cdtbls.contains(ch_strs.at(2)))
				inside_cdtbls.removeOne(rm_tbl);
		}		
		if (!inside_cdtbls.isEmpty())
		{
			foreach (QString s_tbl, inside_cdtbls)
			{
				QStringList cpk_names = s_tbl.split(tr("，，。"));
				cpk_names[1] = that_proj;
				QString that_pswd;
				findMngPasswdsFromOutsideDb(to_db, cpk_names.at(3), that_pswd);
				if (!that_pswd.isEmpty())
				{
					QStringList that_list = that_pswd.split(";");
					QStringList replaced_that = that_list.filter(odb_time);	
					QString replaced_pswd = replaced_that.at(0).split(",").at(1);	
					cpk_names[3] = replaced_pswd;
				}
				QString todb_cpktbl = cpk_names.join(tr("，，。"));	
				*spc_sqldb = QSqlDatabase::database(to_db);
				if (!storeWholeTableFromOutside(origin_conn, s_tbl, todb_cpktbl))
				{
					*spc_sqldb = QSqlDatabase::database(origin_conn);
					merge_result = tr("存储cpk相关表失败");
					return false;			  
				}
				*spc_sqldb = QSqlDatabase::database(origin_conn);
			}
		}
		foreach (QString daily_only, inside_dailys)
		{
			QStringList new_ndaily = daily_only.split(tr("，，。"));		
			QStringList ofdaily_tbl  = todb_dailys.filter(new_ndaily.at(2)).filter(new_ndaily.back());
			if (!ofdaily_tbl.isEmpty())
			{
				if (!projsDailysEqualBetweenDbs(to_db, new_ndaily.at(2), new_ndaily.back()))
				{
					QStringList tt_list;
					getDataesFromTable(daily_only, "time", tt_list);				
					*spc_sqldb = QSqlDatabase::database(to_db);				
					QStringList ot_list;
					getDataesFromTable(ofdaily_tbl.at(0), "time", ot_list);
					QString comparing;			  
					inspector -> compareTwoTimeLists(tt_list, ot_list, comparing);
					if (comparing == tr("时间串包含"))
					{
						if (!delTable(ofdaily_tbl.at(0)))
						{
							*spc_sqldb = QSqlDatabase::database(origin_conn);
							merge_result = tr("存储被合并")+ofdaily_tbl.at(0)+tr("失败");
							return false;						
						}
						*spc_sqldb = QSqlDatabase::database(to_db);
						if (!storeWholeTableFromOutside(origin_conn, daily_only, ofdaily_tbl.at(0)))
						{
							*spc_sqldb = QSqlDatabase::database(origin_conn);
							merge_result = tr("存储日常表")+daily_only+tr("失败");
							return false;			  
						}
					}
					*spc_sqldb = QSqlDatabase::database(origin_conn);
				}
			}
			else
			{
				new_ndaily[1] = that_proj;
				QString that_pswd;
				findMngPasswdsFromOutsideDb(to_db, new_ndaily.at(3), that_pswd);
				if (!that_pswd.isEmpty())
				{
					QStringList that_list = that_pswd.split(";");
					QStringList replaced_that = that_list.filter(odb_time);	
					QString replaced_pswd = replaced_that.at(0).split(",").at(1);	
					new_ndaily[3] = replaced_pswd;
				}
				QString todb_odaily = new_ndaily.join(tr("，，。"));	
				*spc_sqldb = QSqlDatabase::database(to_db);
				if (!storeWholeTableFromOutside(origin_conn, daily_only, todb_odaily))
				{	
					*spc_sqldb = QSqlDatabase::database(origin_conn);
					merge_result = tr("存储日常表")+daily_only+tr("失败");
					return false;			  
				}
				*spc_sqldb = QSqlDatabase::database(origin_conn);
			}
		}
	}
	return true;	
}

bool SpcDataBase::mergeOtherTblsBetweenDb(const QString & to_db, QString & merge_result)
{
  	QString origin_conn = spc_sqldb->connectionName();
	QSqlTableModel odb_model(this, QSqlDatabase::database(to_db));
	varsModelFromTable("dbInformations", &odb_model);	
	QString odb_time = odb_model.data(odb_model.index(0, 2)).toString();	
	QSqlTableModel pics_thedb(this, QSqlDatabase::database(origin_conn));
	varsModelFromTable("pictures", &pics_thedb);
	QStringList tdb_pics;
	getDataesFromTable("pictures", "name", tdb_pics);
	*spc_sqldb = QSqlDatabase::database(to_db);
	QStringList odb_pics;
	getDataesFromTable("pictures", "name", odb_pics);	
	QSqlTableModel pics_todb(this, QSqlDatabase::database(to_db));
	varsModelFromTable("pictures", &pics_todb);
	foreach (QString n_pic, tdb_pics)
	{
		  if (!odb_pics.contains(n_pic))
		  {
			QSqlRecord pic_add = pics_thedb.record(tdb_pics.indexOf(n_pic));
			QString nested_col = pic_add.value(3).toString();
			QString constructor_col = pic_add.value(4).toString();
			QStringList nested_list = nested_col.split(tr("，，。"));
			QString todb_pswd1;
			findMngPasswdsFromOutsideDb(to_db, nested_list.at(0), todb_pswd1);
			QStringList pswd1_list = todb_pswd1.split(";");
			QStringList replaced_pswds1 = pswd1_list.filter(odb_time);
			nested_list[0] = replaced_pswds1.at(0).split(",").at(1);
			pic_add.setValue(3, nested_list.join(tr("，，。")));
			QString todb_pswd2;
			findMngPasswdsFromOutsideDb(to_db, constructor_col, todb_pswd2);
			QStringList pswd2_list = todb_pswd2.split(";");
			QStringList replaced_pswds2 = pswd2_list.filter(odb_time);	
			pic_add.setValue(4, replaced_pswds2.at(0).split(",").at(1));
			pics_todb.insertRecord(-1, pic_add);
		  }
	}
	if (!pics_todb.submitAll())
	{
		merge_result = tr("合并图库失败");
		return false;
	}
	QStringList fromdb_tbls = spc_sqldb->tables();
	QStringList todb_tbls = QSqlDatabase::database(to_db).tables();
	foreach (QString f_tbl, fromdb_tbls)
	{
		if (!todb_tbls.contains(f_tbl) && (f_tbl.contains(tr("，，。manualtable")) || f_tbl.contains(tr("，，。plots"))))
		{
			QStringList otbl_list = f_tbl.split(tr("，，。"));
			QString todb_pswd;
			findMngPasswdsFromOutsideDb(to_db, otbl_list.at(0), todb_pswd);
			QStringList pswd_list = todb_pswd.split(";");
			QStringList replaced_pswds = pswd_list.filter(odb_time);
			otbl_list[0] = replaced_pswds.at(0).split(",").at(1);
			if (!storeWholeTableFromOutside(to_db, f_tbl, otbl_list.join(tr("，，。"))))
			{
				merge_result = tr("合并手工失败")+f_tbl;
				return false;
			}
		}
	}
	return true;
}

int SpcDataBase::userClass(QPair<QString, QString> & name_pswd)
{
	if (isDatabaseConstructor(name_pswd))
		return 4;
	if (!db_manageList.contains(name_pswd.second))
		return 0;
	QString val = db_manageList.value(name_pswd.second).at(4);
	if (val == tr("经理"))
		return 3;
	else if (val == tr("工程师"))
		return 2;
	else if (val == tr("操作者"))
		return 1;
	return 0;
}

SpcNum * SpcDataBase::spcCalculator()
{
	return inspector;
}

const QString SpcDataBase::saveFreeManualConstruction(const QString & person, NestedModel * detailed)
{
	QString save_state(tr("保存完毕"));
	QStringList tbls = allTables();
	if (tbls.contains("freeconstruct"))		
		save_state = resaveFreeManualConstruction(person, detailed);		  
	else if (!saveAloneFreeConstruction(person, "freeconstruct", detailed))
		save_state = tr("未保存");					  
	return save_state;		
}

const QString SpcDataBase::resaveFreeManualConstruction(const QString & person, NestedModel * detailed)
{
	QString save_state(tr("保存完毕"));  
	delTable("freeconstruct");
	if (!saveAloneFreeConstruction(person, "freeconstruct", detailed))
		save_state = tr("未保存");
	return save_state;	
}

QVariant SpcDataBase::dataFromTable(const QString & tablename, const QString & varname, QString & which)
{
	QStringList trans_list = which.split("=");
	QString split_one = trans_list[0];
	QString split_two = trans_list[1];
	QString which_data = QString("select %1 from %2 where %3 = '%4'").arg(varname).arg(tablename).arg(split_one).arg(split_two);
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));	
	if (!query.prepare(which_data))
	{
		QVariant null;
		return null;
	}	  
	query.exec();
	query.next();
	QVariant find_var = query.value(0);
	query.finish();
	return find_var;
}

QVariant SpcDataBase::lastDataFromTable(const QString & tablename, const QString & varname)
{
	QString which_data = QString("select %1 from %2").arg(varname).arg(tablename);
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));	
	if (!query.prepare(which_data))
	{
		QVariant null;
		return null;
	}
	query.exec();
	if (query.last())
		return query.value(0);
	return tablename;
}

QVariant SpcDataBase::dataFromTableByTime(const QDateTime & dest_time, const QString & tablename, const QString & varname, const QString & other_name)
{
	QString which_data;
	if (other_name.isEmpty())
		which_data = QString("select %1 from %2 where time = ?").arg(varname).arg(tablename);
	else
		which_data = QString("select %1 from %2 where %3 = ?").arg(varname).arg(tablename).arg(other_name);	  
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	if (!query.prepare(which_data))
	{
		QVariant null;
		return null;
	}	
	query.addBindValue(dest_time);
	query.exec();
	query.next();
	return query.value(0);	
}

QString SpcDataBase::projectAcceptInfo(const QString & proj)
{
	QStringList proj_info;
	getDataesFromTable("projectsinfo", "name", proj_info, proj);
	return proj_info[4];	
}

QStringList SpcDataBase::desVarDataesFromTable(const QString & tablename, const QString & varname, int from, int to)
{
	QStringList varGroup;
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));		
	QString which_pos = QString("select %1 from %2").arg(varname).arg(tablename);
	if (!query.prepare(which_pos))
	{
		QStringList null_list;
		return null_list;
	}	
	query.exec();
	if (query.seek(from-1))
	{
		do
		{
			if (query.at() == to)
				break;
			QString var = query.value(0).toString();
			varGroup << var;
		}
		while (query.next());
	}
	return  varGroup;		
}

QStringList SpcDataBase::allTables()
{
	return spc_sqldb->database(spc_sqldb->connectionName()).tables();
}

QPixmap SpcDataBase::foundPictureFromDb(const QString & tablename, const QString & lbl)
{
	QString var_pos;
	if (lbl.contains(tr("视图位置")))
		var_pos = "nested="+tablename+tr("，。，")+lbl;
	else
		var_pos = "name="+lbl;
	QVariant f_picture = dataFromTable("pictures", "pic", var_pos);
	QPixmap photo;
	photo.loadFromData(f_picture.toByteArray(), "PNG");	
	return photo;
}

QMap<QString, QStringList> & SpcDataBase::curDBmanageTable()
{
	return db_manageList;
}
	
QMap<QString, QStringList> & SpcDataBase::curDBdepartTable()
{
	return db_departList;
}
	
QMap<QString, QStringList> & SpcDataBase::curDBprojectTable()
{
	return db_projectList;
}

QHash<QString, QString> & SpcDataBase::tmpProjsHashForDbsConfliction()
{
	return projs_conflict;
}

QHash<QString, QString> & SpcDataBase::tmpMngsHashForDbsConfliction()
{
 	return mngs_conflict; 
}

QSqlDatabase * SpcDataBase::currentConnectionDb()
{
	return spc_sqldb;
}

void SpcDataBase::initPowerTablesList()
{
	if (!db_manageList.isEmpty())
 		db_manageList.clear();
	if (!db_projectList.isEmpty())
		db_projectList.clear();
	if (!db_departList.isEmpty())
		db_departList.clear(); 
	QSqlTableModel p_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("managerinfo", &p_sql);
	if (p_sql.rowCount() == 0)
		return;		 
	QSqlTableModel db_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("dbInformations", &db_sql);	
	for (int i = 0; i < p_sql.rowCount(); i++)
	{
		if (p_sql.data(p_sql.index(i, 0)).isNull() || p_sql.data(p_sql.index(i, 1)).isNull() || p_sql.data(p_sql.index(i, 2)).isNull() || p_sql.data(p_sql.index(i, 4)).isNull())
		{
			p_sql.removeRows(i, 1);
			p_sql.submitAll();
			continue;
		}	
		QStringList passwds = p_sql.data(p_sql.index(i, 1)).toString().split(";").filter(db_sql.data(db_sql.index(0, 2)).toString());
		QString passwd = passwds[0].split(",").at(1);
		QStringList infos;
		infos << p_sql.data(p_sql.index(i, 0)).toString();
		infos << p_sql.data(p_sql.index(i, 1)).toString();
		infos << p_sql.data(p_sql.index(i, 2)).toString();
		infos << p_sql.data(p_sql.index(i, 3)).toString();
		infos << p_sql.data(p_sql.index(i, 4)).toString();
		infos << p_sql.data(p_sql.index(i, 5)).toString();
		db_manageList.insert(passwd, infos);
	}	
	QSqlTableModel p1_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("departments", &p1_sql);
	for (int i = 0; i < p1_sql.rowCount(); i++)
	{
		if (p1_sql.data(p1_sql.index(i, 0)).isNull() || p1_sql.data(p1_sql.index(i, 1)).isNull() || p1_sql.data(p1_sql.index(i, 2)).isNull())
		{
			p1_sql.removeRows(i, 1);
			p1_sql.submitAll();
			continue;
		}	  
		QString name = p1_sql.data(p1_sql.index(i, 0)).toString();
		QStringList infos;
		infos << p1_sql.data(p1_sql.index(i, 0)).toString();
		infos << p1_sql.data(p1_sql.index(i, 1)).toString();
		infos << p1_sql.data(p1_sql.index(i, 2)).toString();
		infos << p1_sql.data(p1_sql.index(i, 3)).toString();
		db_departList.insert(name, infos);
	}
	QSqlTableModel p2_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("projectsinfo", &p2_sql);
	for (int i = 0; i < p2_sql.rowCount(); i++)
	{
		if (p2_sql.data(p2_sql.index(i, 0)).isNull() || p2_sql.data(p2_sql.index(i, 1)).isNull() || p2_sql.data(p2_sql.index(i, 2)).isNull())
		{
			p2_sql.removeRows(i, 1);
			p2_sql.submitAll();
			continue;
		}	  
		QString name = p2_sql.data(p2_sql.index(i, 0)).toString();
		QStringList infos;
		infos << p2_sql.data(p2_sql.index(i, 0)).toString();
		infos << p2_sql.data(p2_sql.index(i, 1)).toString();
		infos << p2_sql.data(p2_sql.index(i, 2)).toString();
		infos << p2_sql.data(p2_sql.index(i, 3)).toString();
		infos << p2_sql.data(p2_sql.index(i, 4)).toString();
		infos << p2_sql.data(p2_sql.index(i, 5)).toString();
		db_projectList.insert(name, infos);
	}
	QSqlTableModel p3_sql(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable("projectskeys", &p3_sql);
	for (int i = 0; i < p3_sql.rowCount(); i++)
	{
		if (p3_sql.data(p3_sql.index(i, 0)).isNull() || p3_sql.data(p3_sql.index(i, 1)).isNull() || p3_sql.data(p3_sql.index(i, 2)).isNull() || p3_sql.data(p3_sql.index(i, 3)).isNull() || p3_sql.data(p3_sql.index(i, 4)).isNull() || p3_sql.data(p3_sql.index(i, 5)).isNull() || p3_sql.data(p3_sql.index(i, 6)).isNull() || p3_sql.data(p3_sql.index(i, 7)).isNull() || p3_sql.data(p3_sql.index(i, 8)).isNull() || p3_sql.data(p3_sql.index(i, 9)).isNull() || p3_sql.data(p3_sql.index(i, 10)).isNull())
		{
			p3_sql.removeRows(i, 1);
			p3_sql.submitAll();
			continue;
		}	  
	}
}

void SpcDataBase::findTblNameFromOtherDb(const QSqlDatabase & outside_db, const QString & construct_time, QString & f_name, const QString & special_str)
{
	QStringList tables_list = outside_db.tables();
	foreach (QString tbl, tables_list)
	{
		if (!special_str.isEmpty())
		{
			if (tbl.contains(construct_time) && tbl.contains(special_str))
			{
				f_name = tbl;
				break;
			}
		}
		else
		{
			if (tbl.contains(construct_time))
			{
				f_name = tbl;
				break;
			}			
		}
	}
}

void SpcDataBase::findDestInfoFromDbtbl(int f_pos, QString & f_result, const QString & dest_db)
{  
	if (!dest_db.isEmpty())
	{
		QSqlTableModel dest_dbmodel(this, QSqlDatabase::database(dest_db));
		varsModelFromTable("dbInformations", &dest_dbmodel);
		f_result = dest_dbmodel.data(dest_dbmodel.index(0, f_pos)).toString();	
	}
	else
	{
		QSqlTableModel this_dbmodel(this, spc_sqldb->database(spc_sqldb->connectionName()));
		varsModelFromTable("dbInformations", &this_dbmodel);
		f_result = this_dbmodel.data(this_dbmodel.index(0, f_pos)).toString();		  
	}
}

void SpcDataBase::differentPasswdInDiffrentDb(const QString & this_passwd, const QString & outside_db, QString & that_passwd)
{
	QStringList m_pswds = db_manageList.value(this_passwd).at(1).split(";");
	QSqlTableModel db_sql(this, QSqlDatabase::database(outside_db));
	varsModelFromTable("dbInformations", &db_sql);
	QString db_time(db_sql.data(db_sql.index(0, 2)).toString());
	foreach (QString pswd, m_pswds)
	{
		if (pswd.contains(db_time))
		{
			that_passwd = pswd.split(",").at(1);
			break;
		}
	}
}

void SpcDataBase::mergeFreeTblsBetweenDbs(const QString & this_passwd, const QString & outside_db, QString & m_res)
{
	QString origin_conn = spc_sqldb->connectionName();
	QStringList out_free = QSqlDatabase::database(outside_db).tables().filter("freeconstruct");
	QStringList in_free = spc_sqldb->tables().filter("freeconstruct");
	NestedModel out_model;
	*spc_sqldb = QSqlDatabase::database(outside_db);	
	out_model.initFreeModel(out_free.at(0), this);
	NestedModel in_model;
	*spc_sqldb = QSqlDatabase::database(origin_conn);
	in_model.initFreeModel(in_free.at(0), this);
	QList<QStandardItem *> append_items;
	for (int i = 0; i < out_model.rowCount(); i++)
	{
		QStandardItem * out_item = out_model.item(i);
		bool differnent = false;
		for (int j = 0; j < in_model.rowCount(); j++)
		{
			if (in_model.itemsArchitectureComparation(out_item, in_model.item(j)))
			{
				differnent = true;
				break;
			}
		}
		if (!differnent)
			append_items << out_item;
	}
	if (append_items.isEmpty())
		return;
	foreach (QStandardItem * ap_item, append_items)
		in_model.appendRow(ap_item);
	m_res = resaveFreeManualConstruction(this_passwd, &in_model);
}

bool SpcDataBase::updateStateDataes(const QString & pro_name, DataesTableView * guider)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QString tmp_state;
	QString row;
	int cur_group = guider->model()->data(guider->model()->index(0, 0)).toInt();
	if (pro_name.contains("dailyavr"))
	{
		if (cur_group < 3)
			return true;
		for (int i = 0; i < 14; i++)
		{
			if (cur_group-i < 1)
				break;
			else
			{
				tmp_state = guider->plotsInfomations()->avrDataCurState(cur_group-i);
				QString sel_pos = QString("groupnum=%1").arg(cur_group-i);
				QString db_state = dataFromTable(pro_name, "state", sel_pos).toString();
				if (tmp_state != db_state)
				{
					row = QString("groupnum=%1").arg(cur_group-i);
					if (!updateCellData(pro_name, row, "state", tmp_state))
						return false;
				}
			}	
		}
	}
	else
	{
		if (cur_group < 8)
			return true;
		for (int i = 0; i < 15; i++)
		{
			if (cur_group-i < 1)
				break;
			else
			{
				QString tmp_state = guider->plotsInfomations()->devDataCurState(cur_group-i);
				QString sel_pos = QString("groupnum=%1").arg(cur_group-i);
				QString db_state = dataFromTable(pro_name, "state", sel_pos).toString();
				if (tmp_state != db_state)
				{
					row = QString("groupnum=%1").arg(cur_group-i);
					if (!updateCellData(pro_name, row, "state", tmp_state))
						return false;
				}
			}
	
		}
	}
	return true;
}

bool SpcDataBase::toldataesInsertIndbManagersTable(const QStringList & list)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	if (!query.prepare("insert into managerinfo(name, password, department, projects, title) values(:name, :password, :department, :projects, :title)"))
		return false;
	query.bindValue(":name", list.at(0));
	query.bindValue(":password", list.at(1));
	query.bindValue(":department", list.at(2));
	query.bindValue(":projects", list.at(3));
	query.bindValue(":title", list.at(4));
	query.exec();
	return true;
}
	
bool SpcDataBase::toldataesInsertIndbProjectsinfoTable(const QStringList & list)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	if (!query.prepare("insert into projectsinfo(name, departments, managers, authoagent, attribution, authorizing) values(:name, :departments, :managers, :authoagent, :attribution, :authorizing)"))
		return false;
	query.bindValue(":name", list.at(0));
	query.bindValue(":departments", list.at(1));
	query.bindValue(":managers", list.at(2));
	QString empty;
	if (list.size() == 4)
		query.bindValue(":authoagent", list.at(3));
	else
		query.bindValue(":authoagent", empty);
	if (list.size() == 5)	
		query.bindValue(":attribution", list.at(4));
	else
		query.bindValue(":attribution", tr("在线"));	
	if (list.size() == 6)
		query.bindValue(":authorizing", list.at(5));	
	else
		query.bindValue(":authorizing", empty);		
	query.exec();
	return true;
}
	
bool SpcDataBase::toldataesInsertIndbDepartmentsTable(const QStringList & list)
{
	QSqlQuery query(spc_sqldb->database(spc_sqldb->connectionName()));
	QString com = "";
	if (!query.prepare("insert into departments(name, projects, managers, company) values(:name, :projects, :managers, :company)"))
		return false;
	query.bindValue(":name", list.at(0));
	query.bindValue(":projects", list.at(1));
	query.bindValue(":managers", list.at(2));
	query.bindValue(":company", com); // :company is null , attention! not add null str to db, because db will act null str as space
	query.exec();
	return true;
}

bool SpcDataBase::managerUpdateInDifferentDb(const QString & outside_db, const QString & mng, const QString & update_info)
{
	QString origin_conn = spc_sqldb->connectionName();
	*spc_sqldb = QSqlDatabase::database(outside_db);
	QStringList update_details0 = update_info.split("=");
	QStringList update_details1 = update_details0.at(1).split(";;;");
	QString w_pos = "password="+mng;
	QString m_updates = dataFromTable("managerinfo", update_details0.at(0), w_pos).toString();
	QStringList update_olds;
	if (m_updates.contains(";"))
		update_olds = m_updates.split(";");
	else
		update_olds = m_updates.split(tr(","));
	if (update_details1.size() > 1)
	{
		update_olds[update_olds.indexOf(update_details1.at(0))] = update_details1.at(1);
		QString r_str;
		if (m_updates.contains(";"))
			r_str = update_olds.join(";");
		else
			r_str = update_olds.join(",");
		if (!updateCellData("managerinfo", w_pos, update_details0.at(0), r_str))
			return false;
	}
	else
	{
		if (m_updates.contains(update_details1.at(0)))
			return true;
		else
		{
			update_olds << update_details1.at(0);
			QString r_str;
			if (m_updates.contains(";"))
				r_str = update_olds.join(";");
			else
				r_str = update_olds.join(",");
			if (!updateCellData("managerinfo", w_pos, update_details0.at(0), r_str))
				return false;			
		}
	}
	*spc_sqldb = QSqlDatabase::database(origin_conn);
	return true;	
}

bool SpcDataBase::projinfoUpdateInDifferentDb(QSqlDatabase & outside_db, const QString & proj, const QString & update_info)
{
	QSqlDatabase * tmp_db = spc_sqldb;
	spc_sqldb = &outside_db;
	QStringList update_details0 = update_info.split("=");
	QStringList update_details1 = update_details0[1].split(";;;");
	QString proj_tbl = "projectsinfo";
	QString w_pos = "name="+proj;
	QString p_updates = dataFromTable(proj_tbl, update_details0[0], w_pos).toString();
	QStringList update_olds;
	if (p_updates.contains(";"))
		update_olds = p_updates.split(";");
	else
		update_olds = p_updates.split(",");
	if (update_details1.size() > 1)
	{
		update_olds[update_olds.indexOf(update_details1[0])] = update_details1[1];
		QString r_str;
		if (p_updates.contains(";"))
			r_str = update_olds.join(";");
		else
			r_str = update_olds.join(",");
		if (!updateCellData(proj_tbl, w_pos, update_details0[0], r_str))
			return false;
	}
	else
	{
		if (p_updates.contains(update_details1[0]))
			return true;
		else
		{
			update_olds << update_details1[0];
			QString r_str;
			if (p_updates.contains(";"))
				r_str = update_olds.join(";");
			else
				r_str = update_olds.join(",");
			if (!updateCellData(proj_tbl, w_pos, update_details0[0], r_str))
				return false;			
		}
	}
	spc_sqldb = tmp_db;
	return true;	
}

bool SpcDataBase::departmentUpdateInDifferentDb(const QString & outside_db, const QString & dpr, const QString & update_info)
{
	QString origin_conn = spc_sqldb->connectionName();
	*spc_sqldb = QSqlDatabase::database(outside_db);
	QStringList update_details0 = update_info.split("=");
	QStringList update_details1 = update_details0[1].split(";;;");
	QString dpr_tbl = "departments";
	QString w_pos = "name="+dpr;
	QString d_updates = dataFromTable(dpr_tbl, update_details0[0], w_pos).toString();
	QStringList update_olds;
	if (d_updates.contains(";"))
		update_olds = d_updates.split(";");
	else
		update_olds = d_updates.split(",");	
	if (update_details1.size() > 1)
	{
		update_olds[update_olds.indexOf(update_details1[0])] = update_details1[1];
		QString r_str;
		if (d_updates.contains(";"))
			r_str = update_olds.join(";");
		else
			r_str = update_olds.join(",");
		if (!updateCellData(dpr_tbl, w_pos, update_details0[0], r_str))
			return false;
	}
	else
	{
		if (d_updates.contains(update_details1[0]))
			return true;
		else
		{
			update_olds << update_details1[0];
			QString r_str;
			if (d_updates.contains(";"))
				r_str = update_olds.join(";");
			else
				r_str = update_olds.join(",");
			if (!updateCellData(dpr_tbl, w_pos, update_details0[0], r_str))
				return false;			
		}
	}
	*spc_sqldb = QSqlDatabase::database(origin_conn);
	return true;
}

bool SpcDataBase::changePwdInDifferentProjTbl(const QString & old_pwd, const QString & new_pwd, const QString & proj_tbl)
{
	QSqlTableModel proj_tblmodel(this, spc_sqldb->database(spc_sqldb->connectionName()));
	varsModelFromTable(proj_tbl, &proj_tblmodel);	
	int col_pos = 0;
	if (proj_tbl.contains(tr("，，。cpkdataes")))
		col_pos = 4;
	else if (proj_tbl.contains(tr("，，。dailydataes")))
		col_pos = 3;
	for (int i = 0; i < proj_tblmodel.rowCount(); i++)
	{
		QString e_pwd = proj_tblmodel.data(proj_tblmodel.index(i, col_pos)).toString();
		if (e_pwd == old_pwd)
			proj_tblmodel.setData(proj_tblmodel.index(i, col_pos), new_pwd);
	}
	if (!proj_tblmodel.submitAll())
		return false;
	return true;
}

bool SpcDataBase::mergePureProjManagersBetweenDbs(const QString & merged_proj, const QString & to_db, QString & merge_result)
{
 	QString origin_conn = spc_sqldb->connectionName();	
	QString judge_name("name="+merged_proj);	
	QStringList proj_infoes;	
	QSqlTableModel dbm_model(this, QSqlDatabase::database(origin_conn));
	varsModelFromTable("dbInformations", &dbm_model);	
	QString mdbtime = dbm_model.data(dbm_model.index(0, 2)).toString();
	QSqlTableModel dbto_model(this, QSqlDatabase::database(to_db));
	varsModelFromTable("dbInformations", &dbto_model);	
	QString todbtime = dbto_model.data(dbto_model.index(0, 2)).toString();
	getDataesFromTable("projectsinfo", "name", proj_infoes, merged_proj);				
	QDateTime time_indb = dataFromTable("projectskeys", "constructtime", judge_name).toDateTime();	
	*spc_sqldb = QSqlDatabase::database(to_db);
	QString that_proj = dataFromTableByTime(time_indb, "projectskeys", "name", "constructtime").toString();		
	QStringList that_pinfo;
	getDataesFromTable("projectsinfo", "name", that_pinfo, that_proj);	
	QStringList chged_that = that_pinfo;
	foreach (QString p_info, proj_infoes)
	{
		if (proj_infoes.indexOf(p_info) == 0)
			continue;
		else if (proj_infoes.indexOf(p_info) == 1)
		{
			QStringList that_einfoes = chged_that.at(1).split(",");
			QStringList merged_einfoes = p_info.split(",");
			foreach (QString t_dpr, that_einfoes)
				merged_einfoes.removeOne(t_dpr);
			if (!merged_einfoes.isEmpty())
				chged_that[1] += ","+merged_einfoes.join(",");
		}
		else if (proj_infoes.indexOf(p_info)==2 || proj_infoes.indexOf(p_info)==3)
		{
			QStringList merged_einfoes;
			if (!p_info.isEmpty())
				merged_einfoes = p_info.split(";");
			foreach (QString t_mng, merged_einfoes)
			{
				QStringList merged_minfoes = t_mng.split(",");
				QString find_froms;
				findMngPasswdsFromOutsideDb(origin_conn, merged_minfoes.at(1), find_froms);
				if (!mngExistedTwoDbs(find_froms, to_db))
				{
					QString chk_that;
					if (mngOwnedSameDbTime(find_froms, chk_that, to_db))
					{
						QStringList omngs_list = that_pinfo.at(2).split(";");
						if (!omngs_list.filter(chk_that).isEmpty())
							merged_einfoes.removeOne(t_mng);
						else
							merged_einfoes[merged_einfoes.indexOf(t_mng)] = merged_minfoes.at(0)+","+chk_that;
					}
					if (mngs_conflict.contains(merged_minfoes.at(1)))
						merged_einfoes[merged_einfoes.indexOf(t_mng)] = merged_minfoes.at(0)+","+mngs_conflict.value(merged_minfoes.at(1));
				}
				else
					merged_einfoes.removeOne(t_mng);
			}			
			if (!merged_einfoes.isEmpty())
				chged_that[proj_infoes.indexOf(p_info)] += ";"+merged_einfoes.join(";");
		}
		else if (proj_infoes.indexOf(p_info) == 4)
		{
			if (p_info!=chged_that.at(4) && p_info==tr("在线"))
				chged_that[4] = p_info;
		}
		else
			break;
	}
	chged_that.pop_front();
	chged_that.pop_back();
	QString info_row("name="+that_proj);
	foreach (QString info_cell, chged_that)
	{
		QString colname;
		if (chged_that.indexOf(info_cell) == 0)
			colname = "departments";
		else if (chged_that.indexOf(info_cell) == 1)
			colname = "managers";
		else if (chged_that.indexOf(info_cell) == 2)
			colname = "authoagent";	
		else
			colname = "attribution";
		if (!updateCellData("projectsinfo", info_row, colname, info_cell))
		{
			merge_result = tr("更新")+that_proj+"projectsinfo"+tr("失败");
			return false;
		}
	}
	QStringList to_dprs;
	getDataesFromTable("departments", "name", to_dprs);	
	QStringList mng_infoes = chged_that[1].split(";");	
	foreach (QString mng, mng_infoes)
	{
		if (that_pinfo.at(2).contains(mng))
			continue;	  
		QStringList mng_detail = mng.split(",");
		QString find_froms;
		findMngPasswdsFromOutsideDb(origin_conn, mng_detail.at(1), find_froms);		
		QStringList from_mlist;
		*spc_sqldb = QSqlDatabase::database(origin_conn);
		getDataesFromTable("managerinfo", "password", from_mlist, find_froms);
		QStringList m_projs = from_mlist.at(3).split(";");
		QStringList m_psave = m_projs.filter(merged_proj);			
		*spc_sqldb = QSqlDatabase::database(to_db);			
		QString chk_that;
		if (mngExistedTwoDbs(find_froms, to_db) || mngOwnedSameDbTime(find_froms, chk_that, to_db))
		{
			QString find_tos;
			findMngPasswdsFromOutsideDb(to_db, mng_detail.at(1), find_tos);	  
			QString mp_row("password="+find_tos);
			QString mp_content = dataFromTable("managerinfo", "projects", mp_row).toString();
			mp_content += ";"+that_proj;
			if (m_psave.at(0).split(",").size() > 1)
				mp_content += ","+m_psave.at(0).split(",").at(1);
			if (!updateCellData("managerinfo", mp_row, "projects", mp_content))
			{
				merge_result = tr("存储")+mp_content+tr("失败");
				return false;			  
			}
			QString dpr_row("name="+from_mlist.at(2));
			QString dpr_toproj = dataFromTable("departments", "projects", dpr_row).toString();
			if (!dpr_toproj.contains(that_proj))
			{
				dpr_toproj += ","+that_proj;
				if (!updateCellData("departments", dpr_row, "projects", dpr_toproj))
				{
					merge_result = tr("存储")+dpr_toproj+"departments"+tr("失败");
					return false;			  
				}
			}			
		}
		else
		{
			from_mlist[1] += ";"+todbtime+","+mng_detail.at(1);
			QString rep_three = that_proj;
			if (m_psave.at(0).split(",").size() > 1)
				rep_three += ","+m_psave.at(0).split(",").at(1);		  
			from_mlist[3] = rep_three;
			if (!toldataesInsertIndbManagersTable(from_mlist))
			{
				merge_result = tr("存储")+from_mlist.at(1)+"managerinfo"+tr("失败");
				return false;		  
			}
			if (to_dprs.contains(from_mlist.at(2)))
			{
				QString dpr_row("name="+from_mlist.at(2));
				QString dpr_toproj = dataFromTable("departments", "projects", dpr_row).toString();
				if (!dpr_toproj.contains(that_proj))
				{
					dpr_toproj += ","+that_proj;
					if (!updateCellData("departments", dpr_row, "projects", dpr_toproj))
					{
						merge_result = tr("存储")+dpr_toproj+"departments"+tr("失败");
						return false;			  
					}
				}
				QString dpr_tomngs = dataFromTable("departments", "managers", dpr_row).toString()+";"+mng;
				if (!updateCellData("departments", dpr_row, "managers", dpr_tomngs))
				{
					merge_result = tr("存储")+dpr_tomngs+"departments"+tr("失败");
					return false;			  
				}				
			}
			else
			{
				QStringList dpr_news;
				dpr_news << from_mlist.at(2) << that_proj << mng << "";
				if (!toldataesInsertIndbDepartmentsTable(dpr_news))
				{
					merge_result = tr("添加")+from_mlist.at(2)+"departments"+tr("失败");
					return false;				  
				}		  
			}
			*spc_sqldb = QSqlDatabase::database(origin_conn);
			QString mp_row("password="+find_froms);
			if (!updateCellData("managerinfo", mp_row, "password", from_mlist.at(1)))
			{
				merge_result = tr("更新")+origin_conn+"password"+tr("失败");
				return false;			  
			}
			*spc_sqldb = QSqlDatabase::database(to_db);			
		}		
	}
	*spc_sqldb = QSqlDatabase::database(origin_conn);
	resetManagersDbMap();
	resetDepartmentsDbMap();
	resetProjsDbMap();	
	return true;	
}

bool SpcDataBase::addNewTblsTodbAction(const QString & out_db, const QString & from_proj, const QString & to_proj, const QStringList & from_tbls, const QStringList & current_tbls, const QStringList & key_lbls, bool merge_add, QString & act_result)
{
 	foreach (QString k_str, key_lbls)
	{
		QStringList real_test = k_str.split(tr("，"));
		QString tengi_stamp = QString("%1").arg(QDateTime::fromString(real_test[0]).toTime_t());
		foreach (QString tbl, from_tbls)
		{
			if (tbl.contains(tengi_stamp))
			{
				QString trans_name(tbl);
				if (to_proj != from_proj)
				{
					QStringList real_names = tbl.split(tr("，，。"));
					real_names[1] = to_proj;
					trans_name = tr("，，。")+real_names.join(tr("，，。"));
				}
				if (current_tbls.contains(trans_name))
					continue;
				if (!storeWholeTableFromOutside(out_db, trans_name, tbl))
				{
					if (merge_add)
						act_result = tr("合并新表失败");	
					else
						act_result = tr("添加新表失败");						  
					return false;
				}
			}
		}
	} 
	return true;
}

bool SpcDataBase::mngExistedTwoDbs(const QString & this_pwds, const QString & out_db)
{
	QSqlTableModel dbo_model(this, QSqlDatabase::database(out_db));
	varsModelFromTable("dbInformations", &dbo_model);	
	QString odbtime = dbo_model.data(dbo_model.index(0, 2)).toString();
	if (this_pwds.contains(odbtime))
		return true;
	return false;
}

bool SpcDataBase::mngOwnedSameDbTime(const QString & this_pwds, QString & that_pwds, const QString & out_db)
{
	QString origin_conn = spc_sqldb->connectionName(); 	
	QStringList tpwds_list = this_pwds.split(";");
	*spc_sqldb = QSqlDatabase::database(out_db);
	QStringList opwds_list;
	getDataesFromTable("managerinfo", "password", opwds_list);
	foreach (QString t_pwd, tpwds_list)
	{
		QStringList time_pwds = t_pwd.split(",");
		QStringList chk_opwds = opwds_list.filter(time_pwds.at(0));
		foreach (QString o_pwds, chk_opwds)
		{
			QStringList opwds_real = o_pwds.split(";");
			foreach (QString real_pwd, opwds_real)
			{
				if (t_pwd == real_pwd)
				{
					QSqlTableModel dbo_model(this, QSqlDatabase::database(out_db));
					varsModelFromTable("dbInformations", &dbo_model);	
					QString odbtime = dbo_model.data(dbo_model.index(0, 2)).toString();
					QStringList oreal_list = opwds_list.filter(real_pwd).at(0).split(";").filter(odbtime);
					that_pwds = oreal_list.at(0).split(",").at(1);
					*spc_sqldb = QSqlDatabase::database(origin_conn);  
					return true;
				}
			}
		}
	}
	*spc_sqldb = QSqlDatabase::database(origin_conn);	
	return false;
}

bool SpcDataBase::projsKeyDataesEqualBetweenDbs(const QString & out_db, const QString & proj_time)
{
 	QString origin_conn = spc_sqldb->connectionName(); 	
	QStringList tkey_list;	
	getDataesFromTable("projectskeys", "constructtime", tkey_list, proj_time);	
	tkey_list.pop_front();
	*spc_sqldb = QSqlDatabase::database(out_db);	
	QStringList okey_list;	
	getDataesFromTable("projectskeys", "constructtime", okey_list, proj_time);	
	okey_list.pop_front();	
	*spc_sqldb = QSqlDatabase::database(origin_conn);	
	if (tkey_list != okey_list)
		return false;	
	return true;
}


bool SpcDataBase::projsInfoDataesEqualBetweenDbs(const QString & out_db, const QString & this_proj, const QString & this_time)
{
 	QString origin_conn = spc_sqldb->connectionName(); 	
	QString judge_key = "constructtime="+this_time;		
	QStringList tinfo_list;	
	getDataesFromTable("projectsinfo", "name", tinfo_list, this_proj);
	tinfo_list.pop_front();
	tinfo_list.pop_back();
	QStringList t_mngs = tinfo_list.at(1).split(";");	
	foreach (QString t_mng, t_mngs)
	{
		QStringList name_pwd = t_mng.split(",");
		QString t_pwds = db_manageList.value(name_pwd.at(1)).at(1);
		if (!mngExistedTwoDbs(t_pwds, out_db))
			return false;
		t_mngs.removeOne(t_mng);
	}	
	*spc_sqldb = QSqlDatabase::database(out_db);	
	QString that_proj = dataFromTable("projectskeys", "name", judge_key).toString();	
	QStringList oinfo_list;	
	getDataesFromTable("projectsinfo", "name", oinfo_list, that_proj);	
	oinfo_list.pop_front();	
	oinfo_list.pop_back();
	QStringList o_mngs = oinfo_list.at(1).split(";");
	*spc_sqldb = QSqlDatabase::database(origin_conn);	
	foreach (QString t_mng, t_mngs)
	{
		QStringList name_pwd = t_mng.split(",");
		QString t_pwds = db_manageList.value(name_pwd.at(1)).at(1);
		QString chk_that;
		if (mngOwnedSameDbTime(t_pwds, chk_that, out_db) && !o_mngs.filter(chk_that).isEmpty())
			t_mngs.removeOne(t_mng);
	}
	if (!t_mngs.isEmpty())
		return false;
	if (tinfo_list.at(2)!=oinfo_list.at(2) || tinfo_list.at(3)!=oinfo_list.at(3))
		return false;
	return true;
}

bool SpcDataBase::projsDailysEqualBetweenDbs(const QString & out_db, const QString & time_constructed, const QString & sel_lbl)
{
	QString origin_conn = spc_sqldb->connectionName(); 
	QString this_dailys;
	foreach (QString tbl, allTables())
	{
		if (tbl.contains(sel_lbl) && tbl.contains(time_constructed))
		{
			this_dailys = tbl;
			break;
		}
	}
	QStringList this_times;
	getDataesFromTable(this_dailys, "time", this_times);
	*spc_sqldb = QSqlDatabase::database(out_db);
	QString that_dailys;	
	foreach (QString tbl, allTables())
	{
		if (tbl.contains(sel_lbl) && tbl.contains(time_constructed))
		{
			that_dailys = tbl;
			break;
		}
	}
	QStringList that_times;
	getDataesFromTable(that_dailys, "time", that_times);
	*spc_sqldb = QSqlDatabase::database(origin_conn);
	if (this_times != that_times)
		return false;	
	return true;	  
}

bool SpcDataBase::projAssistTblsActionBetweenDbs(const QString & out_db, const QString & this_proj, const QString & that_proj, QString & act_result, bool rep_merge)
{
 	QString origin_conn = spc_sqldb->connectionName(); 
	QStringList inside_tbls = spc_sqldb->tables();
	QStringList todb_tbls = QSqlDatabase::database(out_db).tables();		
	QString this_property = this_proj+tr("，，。")+"property";
	QString this_product = this_proj+tr("，，。")+"product";			
	QStringList filter_property1 = inside_tbls.filter(this_property);
	QStringList filter_product1 = inside_tbls.filter(this_product);
	QString that_property = that_proj+tr("，，。")+"property";
	QString that_product = that_proj+tr("，，。")+"product";	
	if (!rep_merge)
	{
		if (!filter_property1.isEmpty())
		{
			QStringList filter_property2 = todb_tbls.filter(that_property);
			if (!filter_property2.isEmpty())
			{
				QDateTime property1_time = QDateTime::fromString(filter_property1.at(0).split(tr("，，。")).back());
				QDateTime property2_time = QDateTime::fromString(filter_property2.at(0).split(tr("，，。")).back());
				if (property1_time > property2_time)
				{
					*spc_sqldb = QSqlDatabase::database(out_db);
					if (!delTable(filter_property2.at(0)))
					{
						*spc_sqldb = QSqlDatabase::database(origin_conn);
						act_result = tr("删除property失败");
						return false;						
					}
					QStringList trans_list = filter_property2.at(0).split(tr("，，。"));					
					trans_list[trans_list.size()-1] = property1_time.toString();
					if (!storeWholeTableFromOutside(out_db, filter_property1.at(0), trans_list.join(tr("，，。"))))
					{
						*spc_sqldb = QSqlDatabase::database(origin_conn);
						act_result = tr("存储新property失败");
						return false;					  
					}
				}
			}
			else
			{
				QStringList trans_list = filter_property1.at(0).split(tr("，，。"));
				trans_list[0] = that_proj;
				if (!storeWholeTableFromOutside(out_db, filter_property1.at(0), trans_list.join(tr("，，。"))))
				{
					act_result = tr("存储新property失败");
					return false;					  
				}			  
			}
		}
		if (!filter_product1.isEmpty())
		{
			QStringList filter_product2 = todb_tbls.filter(that_product);
			if (!filter_product2.isEmpty())
			{
				QDateTime product1_time = QDateTime::fromString(filter_product1.at(0).split(tr("，，。")).back());
				QDateTime product2_time = QDateTime::fromString(filter_product2.at(0).split(tr("，，。")).back());
				if (product1_time > product2_time)
				{
					*spc_sqldb = QSqlDatabase::database(out_db);
					if (!delTable(filter_product2.at(0)))
					{
						*spc_sqldb = QSqlDatabase::database(origin_conn);
						act_result = tr("删除product失败");
						return false;						
					}
					QStringList trans_list = filter_product2.at(0).split(tr("，，。"));					
					trans_list[trans_list.size()-1] = product1_time.toString();
					if (!storeWholeTableFromOutside(out_db, filter_product1.at(0), trans_list.join(tr("，，。"))))
					{
						*spc_sqldb = QSqlDatabase::database(origin_conn);
						act_result = tr("存储新product失败");
						return false;					  
					}
				}
			}
			else
			{
				*spc_sqldb = QSqlDatabase::database(origin_conn);
				QStringList trans_list = filter_product1.at(0).split(tr("，，。"));
				trans_list[0] = that_proj;
				if (!storeWholeTableFromOutside(out_db, filter_product1.at(0), trans_list.join(tr("，，。"))))
				{
					act_result = tr("存储新product失败");
					return false;					  
				}			  
			}			
		}
	}
	else
	{
		QStringList filter_property2 = todb_tbls.filter(that_property);
		if (!filter_property2.isEmpty())
		{
			*spc_sqldb = QSqlDatabase::database(out_db);
			if (!delTable(filter_property2.at(0)))
			{
				*spc_sqldb = QSqlDatabase::database(origin_conn);
				act_result = tr("删除property失败");
				return false;						
			}		  
		}
		QStringList filter_product2 = todb_tbls.filter(that_product);
		if (!filter_product2.isEmpty())
		{
			*spc_sqldb = QSqlDatabase::database(out_db);
			if (!delTable(filter_product2.at(0)))
			{
				*spc_sqldb = QSqlDatabase::database(origin_conn);
				act_result = tr("删除product失败");
				return false;						
			}		  
		}
		*spc_sqldb = QSqlDatabase::database(origin_conn);		
		if (!filter_property1.isEmpty())
		{
			QStringList trans_list = filter_property1.at(0).split(tr("，，。"));
			trans_list[0] = that_proj;
			if (!storeWholeTableFromOutside(out_db, filter_property1.at(0), trans_list.join(tr("，，。"))))
			{
				act_result = tr("存储新property失败");
				return false;					  
			}			
		}
		if (!filter_product1.isEmpty())
		{
			QStringList trans_list = filter_product1.at(0).split(tr("，，。"));
			trans_list[0] = that_proj;
			if (!storeWholeTableFromOutside(out_db, filter_product1.at(0), trans_list.join(tr("，，。"))))
			{
				act_result = tr("存储新product失败");
				return false;					  
			}		  
		}
	}
	*spc_sqldb = QSqlDatabase::database(origin_conn);
	return true;
}

const QString SpcDataBase::findConflictedProj(const QString & db_time, const QString & this_proj)
{
	QStringList conflicts_keys = projs_conflict.keys();
	QStringList conflicts = conflicts_keys.filter(db_time);
	QStringList dest_conflicts = conflicts.filter(this_proj);
	return projs_conflict.value(dest_conflicts.at(0));
}
	
const QString SpcDataBase::findConflictedMngr(const QString & db_time, const QString & this_pswd)
{
 	QStringList conflicts_keys = mngs_conflict.keys(); 
	QStringList conflicts = conflicts_keys.filter(db_time);
	QStringList dest_conflicts = conflicts.filter(this_pswd);
	return mngs_conflict.value(dest_conflicts.at(0));	
}