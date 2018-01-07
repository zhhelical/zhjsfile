#ifndef SPCDATABASE_H
#define SPCDATABASE_H
#include <QtCore>
#include <QtGui>
#include <QtSql/QSqlDatabase>

class TablesGroup;
class QStandardItemModel;
class NestedModel;
class QSqlTableModel;
class QSqlQueryModel;
class TableManagement;
class LittleWidgetsView;
class SpcNum;
class DataesTableView;
class SpcDataBase : public QObject
{
	Q_OBJECT
public:
	SpcDataBase(QObject * guide = 0);
	~SpcDataBase();
	void closeCurDB();
	void getDataesFromTable(const QString & tablename, const QString & varname1, QStringList & dataes_list, const QString & val1 = "", const QString & varname2 = "", const QString & val2 = "");
	void getProjsConstructores(QStringList & constructors);
	void constructorOwnedProjs(const QPair<QString, QString> & constructor, QStringList & p_owned);
	void managerOwnedProjs(const QPair<QString, QString> & manager, QStringList & proj_list, const QString & dest_db = QString());
	void agentOwnedProjs(const QPair<QString, QString> & agent, QStringList & proj_list, const QString & dest_db = QString());
	void waitingPowerProjs(const QPair<QString, QString> & wait_pair, QStringList & w_projs);
	void waitingPowerManagers(const QString & project, QStringList & manager_list);
	void varsModelFromTable(const QString & tablename, QSqlTableModel * t_submit);
	void dataesByUserFromDb(const QString & person, const QString & proj, QSqlTableModel * t_submit);
	void dataesByProductFromDb(const QString & product, const QString & proj, int type, QSqlTableModel * t_submit);	
	void dataesByGroupsFromDb(int from, int to, const QString & proj, QSqlQueryModel * t_submit, bool equal = false, const QString & var = "", const QString & real_var = "");
	void dataesByTimeVarFromDb(const QDateTime & from_time, const QDateTime & to_time, const QString & proj, QSqlQueryModel * t_submit, bool equal = false, const QString & var = "", const QString & real_var = "");
	void dataesByRelatedStringFromDb(const QString & proj, QSqlQueryModel * t_submit, const QString & var_str, const QString & real_str, const QString & var = "", const QString & real_var = "");
	void dataSearchByCrudeTime(const QString & s_time, const QString & table, const QString & s_var, QString & dest_data);
	void selectDestPicture(const QString & p_name, QVariantList & pic_infoes);
	void projectManagersIndb(const QString & proj_name, QList<QPair<QString, QString> > & proj_usrs, QSqlDatabase * dest_db = 0);
	void chkUserInOtherDb(const QPair<QString, QString> cur_usr, const QString & other_db, QString & chk_result);
	void relatedTblsNameCollection(const QString & key_str, QStringList & collectings, QSqlDatabase * dest_db = 0, const QString & fur_def = QString());
	void findMngPasswdsFromOutsideDb(const QString & outside_db, const QString & find_hint, QString & find_result);
	void resetManagersDbMap();
	void resetDepartmentsDbMap();
	void resetProjsDbMap();
	bool resetOpenDb(const QString & dbfile);	
	bool dbConnection(const QString & fileinfoes);
	bool relationshipBetweenDbs(const QSqlDatabase & merged_db, QString & from_to);
	bool canMergeDifferentDbs(const QPair<QString, QString> cur_operator, const QSqlDatabase & merged_db, QString & merge_result);
	bool mergeActionForDbs(const QString & merged_db, QString & merge_result);	
	bool canMergeProjBetweenDbs(const QString & merged_proj, QString & can_merge, const QString & to_db);// not list in odg		
	bool replaceProjBetweenDbsAction(const QString & this_proj, const QString & replaced_db, QString & replace_result);// not list in odg
	bool createEngiCarryProductTable(NestedModel * model_todb, bool carr_product);
	bool createEngiTables(const QString & projectname, const QString & constructor, const QSqlDatabase & to_db, const QString & origin_time);
	bool storeSpcDataes(const QString & tablename, DataesTableView * init_dataes, DataesTableView * res_dataes, DataesTableView * init_ptbl, const QString & daily_time);	//no finished
	bool theSameNameInManageList(const QString & name, const QString & dest_db = QString());
	bool theSameNameAndPasswdInManageList(const QPair<QString, QString> & name_passwd, const QString & dest_db = QString());
	bool theSameNameWaitingPower(const QPair<QString, QString> & name_passwd, const QString & dest_db = QString());
	bool theSamePasswdIndb(const QString & passwd, const QString & dest_db = QString());
	bool storeChangedPasswd(const QPair<QString, QString> & new_passwds, const QString & old_passwd, const QString & dest_db = QString());
	bool isDatabaseConstructor(QPair<QString, QString> & name_pair, const QString & w_db = QString());
	bool isConstructor(const QPair<QString, QString> & name_pair, const QString & project);
	bool existedProjectNameIndb(const QString & project, const QString & dest_db = QString());
	bool existedSameProjectIndb(const QDateTime & construct_time, const QString & dest_db = QString());
	bool existedEntirelySameProject(const QString & this_proj, const QDateTime & construct_time, const QString & dest_db);
	bool existedDepartmentIndb(const QString & department, QSqlDatabase * dest_db = 0);
	bool existedManualPlotsTblIndb(const QString & manual_tbl);
	bool existedSameManualTblIndb(const QString & outside_tbl, const QString & construct_time);
	bool renameDatabaseName(const QString & old_name, const QString & new_name);
	bool renameProject(const QString & oldname, const QString & newname);
	bool destProjPassed(const QString & projectname);
	bool storeManageInfoTable(const QStringList & manage_list);
	bool storeProjectsInfoTable(const QStringList & project_list);
	bool storeDepartmentTable(const QStringList & depart_list);
	bool storeProjectsKeyDataes(QStringList & keys, bool init_test = true, const QString & test_dt = QString());
	bool storePictueInfoes(const QVariantList & pic_infoes);
	bool renameTable(const QString & oldname, const QString & newname);
	bool storeWholeTableFromOutside(const QString & outside_db, const QString & inside_table, const QString & outside_table);
	bool deleteDatabase(const QString & db_posname);
	bool deleteProject(const QString & proj_name);
	bool deleteMngInfoesToOtherDb(const QString & to_db, const QPair<QString, QString> & manager, QString & del_result, const QString & del_proj = QString());	
	bool delTable(const QString & tablename);
	bool deleteDataes(const QString & tablename, QString & rows, QString & cols);
	bool deleteCelldata(const QString & tablename, QString & row, const QString & col, const QString & data);
	bool updateCellData(const QString & tablename, QString & rowname, const QString & column, const QString & data = QString());
	bool storeData(const QString & tablename, QString & rowname, const QString & column, const QString & data);
	bool databaseSavePlots(const QString & person, QString & input_name, TableManagement * detailed, LittleWidgetsView * viewer, const QString & construct_time = QString());
	bool saveManualTable(const QString & person, QString & input_name, TableManagement * detailed, LittleWidgetsView * viewer, const QString & construct_time = QString());
	bool databaseResaveRelativeManuals(const QString & person, QString & input_name, TableManagement * detailed, LittleWidgetsView * viewer);
	bool saveBackGroundsForManualTable(TablesGroup * back_source);
	bool saveAloneFreeConstruction(const QString & person, const QString & t_name, NestedModel * detailed_model);
	bool theSameProjFile(const QString & name, QSqlDatabase * dest_db = 0);// not list in odg
	bool createParasTable(QSqlDatabase & dest_db);
	bool initFillForDbProperty(const QStringList & info_list, QSqlDatabase * dest_db);
	bool lastSpcDataesSaveResult(const QString & tablename);
	bool testEngiPerformance(const QString & proj, bool next = false, const QString & engi_version = QString());
	bool projectContainsTimeStr(const QString & time_str, const QString & c_proj);	
	bool mergePureProjBetweenDbs(const QString & merged_proj, const QString & to_db, QString & merge_result);
	bool mergeOtherTblsBetweenDb(const QString & to_db, QString & merge_result);
	int userClass(QPair<QString, QString> & name_pswd);
	SpcNum * spcCalculator();
	const QString saveFreeManualConstruction(const QString & person, NestedModel * detailed);
	const QString resaveFreeManualConstruction(const QString & person, NestedModel * detailed);
	QVariant dataFromTable(const QString & tablename, const QString & varname, QString & which);
	QVariant lastDataFromTable(const QString & tablename, const QString & varname);
	QVariant dataFromTableByTime(const QDateTime & dest_time, const QString & tablename, const QString & varname, const QString & other_name = QString());
	QString projectAcceptInfo(const QString & proj);
	QStringList desVarDataesFromTable(const QString & tablename, const QString & varname, int from, int to);
	QStringList allTables();
	QPixmap foundPictureFromDb(const QString & tablename, const QString & lbl);
	QMap<QString, QStringList> & curDBmanageTable();
	QMap<QString, QStringList> & curDBdepartTable();
	QMap<QString, QStringList> & curDBprojectTable();
	QHash<QString, QString> & tmpProjsHashForDbsConfliction();
	QHash<QString, QString> & tmpMngsHashForDbsConfliction();	
	QSqlDatabase * currentConnectionDb();
	
private:
  	void initPowerTablesList();
	void findTblNameFromOtherDb(const QSqlDatabase & outside_db, const QString & construct_time, QString & f_name, const QString & special_str = QString());	
	void findDestInfoFromDbtbl(int f_pos, QString & f_result, const QString & dest_db = QString());
	void differentPasswdInDiffrentDb(const QString & this_passwd, const QString & outside_db, QString & that_passwd);
	void mergeFreeTblsBetweenDbs(const QString & this_passwd, const QString & outside_db, QString & m_res);
	bool updateStateDataes(const QString & pro_name, DataesTableView * guider);
	bool toldataesInsertIndbManagersTable(const QStringList & list);
	bool toldataesInsertIndbProjectsinfoTable(const QStringList & list);
	bool toldataesInsertIndbDepartmentsTable(const QStringList & list);
	bool managerUpdateInDifferentDb(const QString & outside_db, const QString & mng, const QString & update_info);
	bool projinfoUpdateInDifferentDb(QSqlDatabase & outside_db, const QString & proj, const QString & update_info);
	bool departmentUpdateInDifferentDb(const QString & outside_db, const QString & dpr, const QString & update_info);	
	bool changePwdInDifferentProjTbl(const QString & old_pwd, const QString & new_pwd, const QString & proj_tbl);
	bool mergePureProjManagersBetweenDbs(const QString & merged_proj, const QString & to_db, QString & merge_result);	
	bool addNewTblsTodbAction(const QString & out_db, const QString & from_proj, const QString & to_proj, const QStringList & from_tbls, const QStringList & current_tbls, const QStringList & key_lbls, bool merge_add, QString & act_result);
	bool mngExistedTwoDbs(const QString & this_pwds, const QString & out_db);	
	bool mngOwnedSameDbTime(const QString & this_pwds, QString & that_pwds, const QString & out_db);
	bool projsKeyDataesEqualBetweenDbs(const QString & out_db, const QString & proj_time);
	bool projsInfoDataesEqualBetweenDbs(const QString & out_db, const QString & this_proj, const QString & this_time);
	bool projsDailysEqualBetweenDbs(const QString & out_db, const QString & time_constructed, const QString & sel_lbl);
	bool projAssistTblsActionBetweenDbs(const QString & out_db, const QString & this_proj, const QString & that_proj, QString & act_result, bool rep_merge);
	const QString findConflictedProj(const QString & db_time, const QString & this_proj);
	const QString findConflictedMngr(const QString & db_time, const QString & this_pswd);
	QMap<QString, QStringList> db_manageList;
	QMap<QString, QStringList> db_projectList;
	QMap<QString, QStringList> db_departList;
	QHash<QString, QString> projs_conflict;
	QHash<QString, QString> mngs_conflict;	
	QString project_db;
	SpcNum * inspector;
	QSqlDatabase * spc_sqldb;	
};

#endif
