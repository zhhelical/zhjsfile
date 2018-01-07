#ifndef NESTEDMODEL_H
#define NESTEDMODEL_H
#include <QtCore>
#include <QtGui>

class RelatedTreeView;
class EngiTreeView;
class EditLongToolsViewer;
class SpcDataBase;
class QSqlTableModel;
class NestedModel : public QStandardItemModel
{
	Q_OBJECT
public:
	NestedModel(QWidget * parent = 0);
	~NestedModel();
	void initModel(SpcDataBase * val, const QPair<QString, QString> & ctrl_name = QPair<QString, QString>());
	void initFreeModel(const QString & free_tbl, SpcDataBase * val);
	void findIndexParentsChidren(const QModelIndex & index, QList<QStandardItem *> & parents);
	void findChildrenNotAll(QStandardItem * parent_item, QList<QStandardItem *> & sibling_items);
	void findAllChildItems(QStandardItem * parent_item, QList<QStandardItem *> & child_items);
	void removeSqltableInhash(const QString & key_str);
	void userInfoIndb(const QPair<QString, QString> & comer, QStringList & infoes);
	void saveFreeConstructionWork(QString & is_save);
	void translatedItemInfoes(QString & transed);	
	void relistQmlViewerStrs(QStandardItem * ref_item, QStringList & cur_list);
	void tmpSavedItemUnshowInfo(QStandardItem * clue_item, const QString & unshow_str, bool do_no);
	void ensureConfilctionForDbsMerging(QStandardItem * db_item);	
	void freshModel();
	bool checkModelWithDbStored();
	bool fillStateCheckForNewComer();
	bool userExistedIndb(const QString & filled_first, const QString & filled_second);
	bool dbMigrationAction(QStandardItem * move_item, QString & from, QString & to);
	bool itemsArchitectureComparation(QStandardItem * base_item, QStandardItem * chk_item);	
	QStandardItem * findChildItem(QStandardItem * parent, QStandardItem * info_item);
	QStandardItem * destParentItem(QStandardItem * child);
	QList<QStandardItem *> & resetDeleteUnredoItems();
	QHash<QStandardItem *, QPair<QStandardItem *, QString> > & originHashItems();
	QHash<QStandardItem *, QString> & unshowHashItems();	
	const QPair<QString, QString> & savedUserName();
	const QPair<QStandardItem *, QStandardItem *> & dbEdittingOriginPair();
	const QHash<QPair<QStandardItem *, QString>, QSqlTableModel *> & listForManualTableNames();
	
private:
	void modelForPowerRelatedTree(int detail);
	void modelForPrjsKeysTree();
	void modelForProductsTree();
	void modelForEngiPropertyTree();
	void modelForDbModifier();
	void modelForHelpView();
	void replyForAppendItem(QStandardItem * parent, const QString & item_text);
	void replyForAppendItemsList(QStandardItem * parent, const QList<QStandardItem *> & child_items);
	void departsManagersMatching(const QString & proj, QStringList & departments, QStringList & managers);
	void furtherInitZeroModel(QStandardItem * dest_depart, const QString & content);
	void empoweredUserStatesForTwoModel(const QString & proj, bool waiting, QStringList & empowereds);
	void arrangeAgainForProjKeyTree(const QStringList & projs, QStandardItem * tree_item);
	void firstOpenFreeModelOndb(const QString & n_tbl);
	void keyDataesTreeSet(QStandardItem * key_root, QString & proj, const QStringList paraes);
	void sortItemsList(QStandardItem * parent_root);
	bool reselectedConflictWithdb(QStandardItem * dest_test);
	bool moveRelatedDbfile(QString & from, QString & to);	
	QStandardItem * findItemFromOrigincondb(const QString & valu);
	RelatedTreeView * tree_view;
	EngiTreeView * engi_view;
	QString cell_oldText;
	QPair<QStandardItem *, QStandardItem *> head_pair;
	QPair<QString, QString> using_name;
	QList<QStandardItem *> del_unredoes;
	QHash<QStandardItem *, QPair<QStandardItem *, QString> > origin_condb;
	QHash<QStandardItem *, QString> noshow_hash;	
	QHash<QPair<QStandardItem *, QString>, QSqlTableModel *> manual_tables;
	SpcDataBase * inspector;
};

#endif
