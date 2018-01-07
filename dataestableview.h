#ifndef DATAESTABLEVIEW_H
#define DATAESTABLEVIEW_H
#include <QtGui>
#include <QtCore>

class EditLongToolsViewer;
class EngiTreeView;
class SpcDataBase;
class SpcNum;
class PlotWidget;
class LittleWidgetsView;
class DataesTableView : public QTableView
{
	Q_OBJECT
public:
	DataesTableView(QWidget * parent, int type, SpcDataBase * val = 0, bool edit = true);
	~DataesTableView();
	void initTable(const QString & engi, DataesTableView * partner = 0, const QList<double> & vals_list = QList<double>());
	void setInputor(QPair<QString, QString> & name);
	void setRightOrigRect(const QRect & r_size);
	void setPlotsBackGuide(PlotWidget * plots);
	void setNewNextIndex(const QModelIndex & index);
	void clearRelatedCellsForNextInput();
	void setCalResDataesForDaily();
	void closeNestedQmlViewerOnCell(EditLongToolsViewer * nested);
	void resetEditState(bool state);
	void changeModelForEditting(QStandardItemModel * cpe_model, const QString & lead_time, DataesTableView * guider = 0);
	void resetTblForProjKeysValues(const QString & destProj, const QString & destModel);
	void tmpSaveProjsModels(const QString & saveProj, const QString & testTime);
	void resetCopyingModelPtr(QStandardItemModel * cp_model);
	void deleteTmpModelsPreparation(QStandardItemModel * data_model, DataesTableView * p_view);
	void deleteTmpModels(QStandardItemModel * del_model, DataesTableView * p_view);
	void cancelDeleteModelsAction(QStandardItemModel * del_model);
	void setShiftModel(QStandardItemModel * shift_model, QStandardItemModel * key_model = 0);
	void setCpkProcessPartner(DataesTableView * partner);
	void syncCurrentCursor(QStandardItemModel * proj_model);
	void syncUnredoRelatedCmds(QStandardItemModel * proj_model);
	void findTimestrList(const QList<QPair<QString, QStandardItemModel *> > found_ins, QStringList & findings);	
	void swapModelsForOriginToDb(QStandardItemModel * key_model);
	void setPromotionTreePtr(EngiTreeView * p_tree);
	void initTestCpkDataesModels(QStandardItem * version_item);
	void testCpkDataesFromDb(QStandardItem * sam_item, const QString & v_time);	
	void deletePromotionModel(QStandardItem * tree_item, DataesTableView * p_view);
	void copyModelDataesToOther(QStandardItemModel * from, QStandardItemModel * other, bool cp_all = true);
	void initEditNewConstructionList(QList<QStandardItemModel *> & model_list, DataesTableView * guider = 0);
	void initEditPromotionModelsList(QList<QStandardItemModel *> & model_list, DataesTableView * guider = 0);
	void dealModelsHashForUnreDo(QStandardItemModel * d_model, bool act, QStandardItem * g_item);
	bool tblEdittingState();
	bool checkInputState(QString & chk_result);	
	bool saveProjKeyDataesCheck(bool init_proj, const QString & test_dt);
	bool saveDailySpcDataes();	
	bool cpkInitDataesCalucationCheck();//for not number charactors
	bool dailyDataesCalucationCheck();
	bool pasteModelDataesAction(bool paste_act, QStandardItemModel * guide_model, QString & p_result);
	bool dataesModelExistedInDb(QStandardItemModel * chk_model);
	bool chkAndDeleteEngiDataesInDb();	
	int tableType();
	QStandardItemModel * existedEntirelyModelInHash(const QString & proj, QStandardItemModel * match_model = 0);
	QStandardItemModel * matchTimeKeyModel(const QString & saveTime);	
	QStandardItemModel * matchNewKeysModel(const QString & saveProj, const QString & saveTime);
	QStandardItemModel * shiftedKeyForModelSelection();
	QStandardItem * engiPerformanceItemOrSampleItem();	
	EditLongToolsViewer * qmlViewer();
	PlotWidget * plotsInfomations();
	EngiTreeView * improveTreePtr();
	QUndoStack * commandStack();
	QUndoStack * delModelsStack();
	DataesTableView * nestingUnionView();
	SpcDataBase * backDatabase();
	QString modelConstructTime(const QString & chk_proj, QStandardItemModel * dest_model);	
	const QString & getInfoByModel(QStandardItemModel * dest_model, const QString & by_what);	
	const QModelIndex & curNextIndex();
	const QRect & originShowRect();
	QPair<QString, QString> & projectInputor();
	const QStringList & projsInsertSequence();
	const QHash<QModelIndex, QString> & dailyCalculations();
	QHash<QString, QStandardItemModel *> & tmpDeletionModelsCache();
	const QHash<QStandardItemModel *, QStandardItemModel *> & cursorModels();
	QHash<QString, QList<QPair<QString, QStandardItemModel *> > > & modelHashs();

signals:
	void sizeChanged();
	void resetSelf(bool val);
	void inputted(DataesTableView * thisTable);
	void judgedClickedHeader();
	void saveDataesFailedTodb();
	void changeBtnEnable(const QString & btn_lbl, const QString & toggle_text, bool en);
	void changeStackCommOrder(QUndoStack * comm);
	void deleteTreeProjForPromotion(const QString & proj);

public slots:
	void clearRelatedCellsForReinput();
	void replyForSelectedStrs(int s_num);
	void changeViewStateForNewModel();		

private slots:
	void cellClicked(const QModelIndex & c_index);
	void closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint = QAbstractItemDelegate::NoHint); 
	void clearQmlViewStrsPt(QWidget * clear_pt);
	void clearNestedAnimation();	
	
private:
	void mousePressEvent(QMouseEvent * event);
	void initDifferentDataesModel(QStandardItemModel * n_model, DataesTableView * guider = 0, const QString & proj = QString(), const QList<double> & vals_list = QList<double>());	
	void initStrListViewer(const QModelIndex & pos_index);
	void openNestedQmlViewerOnCell(EditLongToolsViewer * nesting, int end_width, int end_height);
	void canInputtedCells(QList<int> & cells);
	void recalSizeForOpeningNestedViewer(const QModelIndex & index, const QRect & n_rect);
	bool isInputtedCell(const QModelIndex & index);
	int biggerWidthBetweenTextList(const QFontMetrics & fm, const QStringList & strs_list);	
	QStandardItemModel * matchProjTestTimeModel(const QString & saveProj, const QString & testTime);
	QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);	
	bool can_edit;	
	bool input_spcnum;
	int t_type;
	QRect right_origin;
	QString styleSheet_string;
	QString cell_oldText;
	QModelIndex next_index;	
	QPair<QString, QString> inputor;	
	QStringList qml_strlist;
	QStringList projs_sequence;
	QHash<QModelIndex, QString> dataes_indexes;
	QHash<QString, QStandardItemModel *> models_cache;
	QHash<QStandardItemModel *, QStandardItemModel *> pd_cursors;
	QHash<QStandardItemModel *, QUndoStack *> pdd_commands;	
	QHash<QString, QList<QPair<QString, QStandardItemModel *> > > projs_models;
	QStandardItemModel * cursor_model;
	QStandardItemModel * copy_specs;
	QTime * press_time;
	EditLongToolsViewer * multi_viewer;
	DataesTableView * union_viewer;
	EngiTreeView * improve_tree;
	QPropertyAnimation * nested_animation;
	QUndoStack * m_commandStack;
	QUndoStack * model_deleteStack;
	PlotWidget * current_plots;
	SpcDataBase * data_db;
};

#endif
