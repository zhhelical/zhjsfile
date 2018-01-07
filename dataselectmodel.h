#ifndef DATASELECTMODEL_H
#define DATASELECTMODEL_H
#include <QtCore>
#include <QtGui>

class TableManagement;
class SpcDataBase;
class EditLongToolsViewer;
class QSqlTableModel;
class QSqlDatabase;
class DataSelectModel : public QStandardItemModel
{
	Q_OBJECT
public:
	DataSelectModel(TableManagement * parent = 0);
	~DataSelectModel();
	void initModel(const QPair<QString, QString> & ctrl_name, SpcDataBase * val);
	void findItemsSingleLine(QStandardItem * hint_item, QList<QStandardItem *> & i_line, const QString & head_str);
	void findChildrenNotAll(QStandardItem * parent_item, QList<QStandardItem *> & sibling_items);
	void findAllChildItems(QStandardItem * parent_item, QList<QStandardItem *> & child_items);
	void removeSqltableInhash(const QString & key_str);
	void findExistedDbsBackupTo(QStandardItem * select_item, QStringList & dbs);
	void projBackupJudgeMngs(QStandardItem * bk_item, QStandardItem * n_db);
	void feedSimpleDataFromTbl(const QString & lbl_feed, const QString & feed_source, QStandardItemModel * set_model, QStandardItem * hint_item);
	void findExistedRelatedTable(const QString & proj_version, const QString & v_name, const QStringList & all_tbls, QString & dest_tbl);	
	QStandardItem * findTimeParentItem(QStandardItem * c_item);	
	QStandardItem * findChildItem(QStandardItem * parent, QStandardItem * info_item);	
	TableManagement * nestedInTreeView();
	QString findEngiItemText(QStandardItem * info_item);
	const QPair<QString, QString> & savedUserName();
	const QHash<QStandardItem *, QString> & timesSavedHash();
	const QHash<QPair<QStandardItem *, QString>, QSqlTableModel *> & listForManualTableNames();
	QHash<QPair<QStandardItem *, QStandardItem *>, QStringList> & bkdbUsrsInformation();
	
private:
	void modelForSpcDataesSelect();
	void modelForSpcInfoesSelect();
	void modelForSpcDbTblBackup();
	void modelForSpcPlotsSelect(int detail);
	void modelForSpcTotalSelect(int detail);
	void treeArrangementForEngiDataes(QStandardItem * root_item, const QStringList & engis_list);
	void treeArrangementForEngiInfoes(QStandardItem * r_item);
	void treeArrangementForEngiPlots(QStandardItem * root_item, const QStringList & engis_list);
	void manualItemOnTreeAction(QStandardItem * branch, const QStringList & db_manuals);
	void initManualTableModelOnTree(QStandardItem * branch, const QString & db_manual);
	void engiVersionDistribution(QStandardItem * engi_item, const QString & proj_name, const QStringList & db_tbls);
	void engiSamplesDistribution(QStandardItem * engi_version, const QString & version_time, const QStringList & db_tbls);
	void engiVersionDistriForPlots(QStandardItem * engi_branch, const QString & proj_name, const QStringList & db_tbls, QList<QStandardItem *> & copy_items);
	void engiSamplesDistriForPlots(QStandardItem * engi_root, QStandardItem * engi_version, const QString & version_type, const QStringList & db_tbls, QList<QStandardItem *> & copy_items);
	void findManualTblsFromDb(const QString & m_lbl, const QString & m_noter, const QStringList & db_tbls, QStringList & find_tbls);
	void completePlotsManualTblTree(QStandardItem * manual_item, QSqlTableModel * m_sql);
	void recurseAppendDirsInFolder(QStandardItem * dir_item, const QString & n_dir);
	void sortItemsList(QStandardItem * parent_root);
	bool isCurrentIndexContainEngi(const QModelIndex & index, const QString & engi);
	TableManagement * tree_view;
	QString cell_oldText;
	QPair<QString, QString> using_name;
	QHash<QStandardItem *, QString> key_times;
	QHash<QPair<QStandardItem *, QStandardItem *>, QStringList> usr_corruptions;
	QHash<QPair<QStandardItem *, QString>, QSqlTableModel *> manual_tables;
	SpcDataBase * inspector;
};

#endif
