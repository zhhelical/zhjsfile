#ifndef TABLEMANAGEMENT_H
#define TABLEMANAGEMENT_H
#include <QtGui>
#include <QtCore>

class DataSelectModel;
class EditLongToolsViewer;
class LittleWidgetsView;
class TableMgmtPlotEditCmd;
class TableMgmtGroupEditCmd;
class AloneTableEditCmd;
class PlotWidget;
class SpcNum;
class SpcDataBase;
class QSqlTableModel;
class TableManagement : public QTreeView
{
	Q_OBJECT
public:
	TableManagement(QWidget * parent = 0);
	~TableManagement();
	void initManageTree(const QPair<QString, QString> & ctrl_name, SpcNum * spc_guider, SpcDataBase * dataes_source, int type, LittleWidgetsView * related_viewer = 0, EditLongToolsViewer * guide_bar = 0);
	void setInitViewRect(const QRect & rect);
	void setDelegateCell(QWidget * delegate);
	void sendSignalToRelated(int type, QStandardItem * info);
	void removeItemsInHash(const QList<QStandardItem *> & items);
	void rearrangeItemsForDelete(QList<QStandardItem *> & items);
	void unredoItemsForMatrixChange(const QList<PlotWidget *> & hide_plots, const QHash<QStandardItem *, QPair<QStandardItem *, int> > & on_old, bool re_un);
	void itemsInfoesTransToDB(QStandardItem * parent_item, QString & trans_plots, QString & trans_hash, QString & trans_names);
	void feedbackForManualsName(QString & t_name);
	void actionForOtherDelManualWork(const QString & del_name);
	void removeItemChildrenInHash(QStandardItem * parent_item, QHash<QStandardItem *, QStringList> & bk_hashes);
	void reinsertItemChildrenInHash(const QHash<QStandardItem *, QStringList> & bk_hashes);
	void setTypeTwoEditting(bool edit_define);
	void unredoGeneratePlotForTmtbl(QStandardItem * plot, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & ur_hash, bool act);
	void unredoFolderPictureForTmtbl(QStandardItem * plot, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & ur_hash, bool act);
	void mBackItemsTransFromDB(QStandardItem * c_item, const QString & items, const QHash<QString, QStringList> & db_hash, const QHash<QString, QStringList> & db_names);	
	bool findMatchBranch(QStandardItem * dest_item, const QString & find_text);
	bool considerSaveNewOrOldManuals(QString & save_res);
	bool unredoForRenameDbEngi(const QString & oldname, const QString & newname);	
	bool backupRelatedDbForItems();
	bool selectionForDbBackupFault();	
	int relationshipBetweenItems(QStandardItem * one, QStandardItem * other);
	int treeType();
	int biggerWidthBetweenTextList(const QFontMetrics & fm, const QStringList & list, const QString & str = QString());//maybe move to private	
	QStandardItem * lookingForMatchItem(QStandardItem * cause_item, const QString & clue_str);
	DataSelectModel * currentModel();
	QUndoStack * commandStack();
	EditLongToolsViewer * delegateViewer();
	LittleWidgetsView * currentPartner();
	const QRect & originShowRect();
	QList<QStandardItem *> & keepDeletingItems();
	QMap<int, QPair<QStandardItem *, LittleWidgetsView *> > & backupViewerHash();
	QHash<QStandardItem *, QList<QStandardItem *> > & backupManualFileItems();
	const QHash<QStandardItem *, QStringList> & hashSavedRelatedItemInfo();

signals:
	void currentRedoToNone(bool redo);
	void currentUndoToNone(bool undo);
	void itemEditting(const QModelIndex & index);
	void warningInfo(QString & info, bool reback);
	void finishedDropAction();
	void plotFeildToBeChanged(int type, QStandardItem * which);
	void newPlotEditCommand(TableMgmtPlotEditCmd * com);
	void sizeChanged();
	void timeDefineTitle(QVariant str);
	void freezingNestCell(QVariant freeze);
	void showSavedManualPlots(TableManagement * sending, LittleWidgetsView * new_viewer, bool direct);
	void saveProgressforwarded(int working);

public slots:
	void clearShowDataes();
	void relatedStyleTreeActions(QString style, bool action);
	void replyForSelectedStrData(int data);
	void listenFreezingState(bool state);
	void resetDatelist(int year, int month);
	void endSelectionForDate(int year, int month, int day);
	void viewerRealWidthReset(int real_val);
	void endSelectionForNumber(QString data);
	void closeNestedWidget();
	void clearLongToolPt(QWidget * pt);
	void activedItemChanged(QStandardItem * a_item);	
	
private slots:
	void cellClicked(const QModelIndex & d_index);
	void clearNestedAnimation();
	void recalculateSize(const QModelIndex & e_index);
	void sendsigOrderFromPartner(PlotWidget * hide);
	void closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint); 
	void syncPartnersFrozenState(bool state);
	
private:
	void mousePressEvent(QMouseEvent * event);
	void longPressForDataesModel(QStandardItem * click_item, DataSelectModel * dataes_model);
	void longPressForInfoesModel(QStandardItem * click_item);
	void longPressForBackupModel(QStandardItem * click_item, DataSelectModel * bk_model);
	void longPressForPlotsModel(QStandardItem * click_item);
	void longPressForDataInfoModel(QStandardItem * click_item, DataSelectModel * di_model);
	void longPressForTotalModel(QStandardItem * click_item, DataSelectModel * total_model);
	void longPressForDailyItems(QStandardItem * daily_item);
	void replyQmlStrDataForDmTree(QStandardItem * for_item, int str_pos, const QString & reply_str);
	void replyQmlStrDataForBkTree(QStandardItem * for_item, DataSelectModel * bk_reply, QString & reply_str, QString & is_return);
	void replyQmlStrDataForPmTree(QStandardItem * for_item, int str_pos, const QString & reply_str);
	void replyQmlStrDataForTmTree(QStandardItem * for_item, int str_pos, const QString & reply_str);
	void threeFactoresProcess(QStandardItem * factor_item, int data_pos = -1, const QString & reply_str = QString());	
	void initDelegateStrViewer(QStandardItem * in_item, const QStringList & specials = QStringList());
	void initDelegateNumberViewer(QStandardItem * in_item);
	void initCalandarViewer(QStandardItem * in_item);
	void setAllDatesList(QStandardItem * mark_item);
	void ensureRecordersForViewer(QStandardItem * r_item);
	void redefineRelatedItemHash(QStandardItem * item_key);
	void redefineDatesListForNewSelection(const QString & base_time, const QStringList & all_time, bool type, QStringList & def_time);
	void redefineStrlistByCtrlItem(QStandardItem * ctrl_item, QStringList & strlist);
	void defineNextSelectString(QStandardItem * ctrl_item, QString & next_str);
	void actionForManualDataesShow(QStandardItem * manual_item, const QString & t_name, QSqlTableModel * db_model);
	void mBackStringsTransFromDB(const QString & hash, const QString & names, QHash<QString, QStringList> & db_hash, QHash<QString, QStringList> & db_names);
	void treeProjsFresh(QStandardItem * new_item, QStandardItem * old_item);
	void createPartnerForClickedItem(QStandardItem * c_item, QSqlTableModel * sql_model, const QString & db_name);
	void setPartnerListForQmlTrans(LittleWidgetsView * new_partner);
	void edittingEvironmentTrans(QStandardItem * transed_item, QStandardItem * transing_item);
	void manualDBnameByItem(QStandardItem * key_item, QString & dest_name);
	void recalSizeForOpeningNestedViewer(QStandardItem * pos_item, const QRect & n_rect);
	void openNestedWidgetInBranch(EditLongToolsViewer * nesting, int end_width, int end_height);
	void closeNestedWidgetInBranch(EditLongToolsViewer * nested);	
	bool plotExistedInViewer(const QModelIndex & check_index);
	bool curIndexMatchForMoveToViewer(const QModelIndex & match_index);
	bool compareShowingClickingItem(QStandardItem * key_item);
	bool newDbGenerationForItem(QStandardItem * bk_item, const QDateTime & db_time);
	int maxCurrentWidth();
	QStandardItem * showingItemFind();
	QStandardItem * existedStrOnItemlist(const QString & chk_str, QList<QStandardItem *> & on_list);
	LittleWidgetsView * containsPartnerGuideItem(QStandardItem * key_item);	
	QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);	
	bool editting;
	int t_type;
	QRect origin_rect;
	QString cell_oldText;
	QString edit_order;	
	QStringList year_list;
	QStringList month_list;
	QStringList days_list;
	QStringList group_numbers;
	QList<QStandardItem *> delete_items;
	QMap<int, QPair<QStandardItem *, LittleWidgetsView *> > bk_viewers; 
	QHash<QStandardItem *, QList<QStandardItem *> > db_items;
	QHash<QStandardItem *, QStringList> names_list;
	QHash<QStandardItem *, QStringList> texts_hash;
	QHash<QStandardItem *, QHash<QStandardItem *, QStringList> > bk_names;
	QHash<QStandardItem *, QHash<QStandardItem *, QStringList> > bk_texts;
	QHash<QString, QStringList> tmp_mngs;
	QTime * press_time;
	QPropertyAnimation * nested_animation;
	EditLongToolsViewer * strs_viewer;
	LittleWidgetsView * partner;
	QUndoStack * m_commandStack;
	SpcNum * back_calculator;
	SpcDataBase * source;
	EditLongToolsViewer * tools_bar;
};

#endif
