#ifndef ENGILISTVIEW_H
#define ENGILISTVIEW_H
#include<QtGui>
#include<QtCore>

class SpcDataBase;
class DataesTableView;
class NestedModel;
class EngiTreeView : public QTreeView
{
	Q_OBJECT
public:
	EngiTreeView(QWidget * parent = 0);
	~EngiTreeView();
	void setInspector(SpcDataBase * val);
	void initTreeInformation(QPair<QString, QString> & manager, int type);
	void setInitViewRect(const QRect & rect);
	void columnWidthMatchText();
	void findIndexParents(const QModelIndex & index, QList<QStandardItem *> & parents);
	void findAllChildItems(QStandardItem * parent_item, QList<QStandardItem *> & child_items);
	void findChildrenNotAll(QStandardItem * parent_item, QList<QStandardItem *> & sibling_items);
	void addItemForNewTest(QStandardItem * engi_item, DataesTableView * dest_tbl); 
	void addItemForNewSamp(DataesTableView * dest_tbl, DataesTableView * din_tbl, DataesTableView * key_tbl);
	void defaultSettedModelTransaction();
	void initModelsForRelatedView(DataesTableView * dest_tbl, DataesTableView * dest_partner = 0);	
	bool saveProjsPromotions(QList<QStandardItem *> & save_items, DataesTableView * din_tbl, DataesTableView * cal_tbl, DataesTableView * key_tbl);
	QStandardItem * currentEngiItem();
	const QRect & originShowRect();	
	QList<QStandardItem *> & deletionItemsList();
	const QHash<QStandardItem *, QStandardItemModel *> & openningItemsModels();

signals:
	void treeViewSizeChanged();
	
private slots:
	void resizeRow(const QModelIndex & index);
	void recalculateSize(const QModelIndex & r_index);
	void relatedEngiClickedInFileView(const QModelIndex & index); 
	void deleteProjByPromotion(const QString & del_proj);
	
private:
	void ownerEngiesAttributions(QStringList & projs);
	void multiRootsTree(QHash<QString, QStringList> & roots);
	void appendEngiesAttribution(const QStringList & projs, QStandardItem * root_item);
	void keyDataesTreeSet(QStandardItem * key_root, QString & proj, const QStringList paraes);
	void samplesDataesTreeSet(QStandardItem * sams_root, QString & proj, const QStringList tbls);	
	void sortItemsList(QStandardItem * parent_root);
	void copyModelItems(QStandardItemModel * from, QStandardItemModel * to);
	int maxCurrentWidth();
	int tree_type;
	QStandardItem * open_selection;
	QStandardItemModel * bk_model;
	QRect orig_rect;
	QPair<QString, QString> user_info;
	QList<QStandardItem *> del_items;
	QHash<QStandardItem *, QStandardItemModel *> open_models;
	SpcDataBase * base_db;	

};
      
#endif
