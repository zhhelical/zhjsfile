#ifndef RELATEDTREEVIEW_H
#define RELATEDTREEVIEW_H
#include <QtGui>
#include <QtCore>

class NestedModel;
class EditLongToolsViewer;
class SpcDataBase;
class QSqlDatabase;
class RelatedTreeView : public QTreeView
{
	Q_OBJECT
public:
	RelatedTreeView(QWidget * parent = 0);
	~RelatedTreeView();
	void initTree(SpcDataBase * db_ptr, const QPair<QString, QString> & ctrl_name, int type);
	void initHelpTree(int type);
	void setInitViewRect(const QRect & rect);
	void setFreeForEdit(bool free);
	void initStrListViewer(NestedModel * transed_model, const QModelIndex & pos_index);
	void closeNestedQmlViewerOnCell(EditLongToolsViewer * nested);	
	void setQmlViewerPtr(EditLongToolsViewer * view_ptr);
	void resetSpecialClickedItem(QStandardItem * s_item);
	void saveFreeConstructionTodb();
	bool dbEditorVerifyPermission(const QPair<QString, QString> & verify_pair, const QSqlDatabase & chk_db, bool constrctor_authgent);
	bool usingNameNull();
	bool checkInputState();
	bool storeDataesTodb();
	bool userIndb();
	bool dbsEdittingAction(QString & act_result);
	int treeType();	
	const QPair<QString, QString> & usingPair();
	EditLongToolsViewer * qmlViewer();
	QStandardItem * specialClickedItem();	
	QUndoStack * commandStack();
	SpcDataBase * databaseSource();
	const QRect & originShowRect();
	QHash<QStandardItem *, QStringList> & itemTextsHash();

signals:
	void sizeChanged();
	void itemEditting(const QModelIndex & index);
	void secondOpenDatabase(const QString & open_db);
	void freezingNestCell(QVariant freeze);

public slots:
	void setEmpowerState(bool set_state);
	void replyForSelectedStrs(int s_num);
	void listenFreezingState(bool freezing);
	void activedItemChanged(QStandardItem * a_item);

private slots:
	void cellClicked(const QModelIndex & index);
	void expandedToResizeQmlView(const QModelIndex & e_index);
	void collapsedToResizeQmlView(const QModelIndex & c_index);
	void clearQmlViewStrsPt(QWidget * clear_pt);
	void clearNestedAnimation();
	void closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint); 
	
private:
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void stringlistMinus(QStringList & tol_list, const QStringList & m_list, bool equal);
	void fillQmlViewerStrs(QStringList & fill_list);
	void openNestedQmlViewerOnCell(EditLongToolsViewer * nesting, int end_width, int end_height);		
	bool judgeToAddNewManagerInfo(const QString & project, QStringList & new_managers);
	bool judgeToRemoveManagerForProj(const QString & project, QStringList & del_managers);
	int biggerWidthBetweenTextList(const QFontMetrics & fm, const QStringList & strs_list, const QString & str = QString());	
	int maxCurrentWidth();
	QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);	
	bool edit_free;
	bool empower_state;
	int t_type;	
	QString cell_oldText;
	QString edit_order;	
	QRect origin_rect;
	QStandardItem * special_clicked;
	QTime * press_time;
	EditLongToolsViewer * cons_viewer;
	QPropertyAnimation * nested_animation;
	QPair<QString, QString> using_name;	
	QHash<QStandardItem *, QStringList> texts_hash;
	QUndoStack * m_commandStack;
	SpcDataBase * inspector;
};

#endif
