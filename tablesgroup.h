#ifndef TABLESGROUP_H
#define TABLESGROUP_H
#include <QtGui>
#include <QtCore>

class QSqlTableModel;
class QSqlQueryModel;
class TableManagement;
class PlotWidget;
class SpcNum;
class SpcDataBase;
class TableMgmtGroupEditCmd;
class PrintFrame;
class DataSelectModel;
class TablesGroup : public QTableView
{
	Q_OBJECT
public:
	TablesGroup(QWidget * parent = 0);
	~TablesGroup();
	void setInspectors(SpcNum * cal_val, SpcDataBase * db_val);
	void initTable(const QPair<QString, QString> & ctrl_name, QStandardItemModel * curmodel);
	void setFontSize(bool zoom);
	void showRelatedRows();
	void showRelatedColumns();
	void hideRelatedRows();
	void hideRelatedColumns();
	void sendSigToToolbar(TableManagement * from_tree, QStandardItem * first_item, QStandardItem * second_item, const QString & order);
	void unredoForCellTextEdit(const QPair<QModelIndex, QString> & edit_cell, bool re_un);
	void unredoForClearContents(const QString & order, const QHash<QModelIndex, QPair<QMap<int, QVariant>, QStandardItem *> > & sel_cells, bool re_un);
	void unredoForCut(const QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & oper_cells, bool re_un);
	void unredoForPaste(const QPair<QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> >, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > > & paste_cells, bool re_un);
	void unredoForInsertRowsCols(const QMap<QModelIndex, QString> & opr_map, const QString & row_col, bool re_un);
	void unredoForRemoveRowsCols(const QMap<QModelIndex, QString> & opr_map, const QString & row_col, bool re_un);
	void unredoForCellsTextAlign(const QHash<QModelIndex, QPair<QString, QStandardItem *> > & align_hash, bool re_un);
	void unredoForCellsFrame(const QPair<QString, QModelIndexList> & paint_frame, bool reun);
	void unredoColorForCells(const QHash<QModelIndex, QPair<QString, QStandardItem *> > & color_infoes, bool ft_bgd, bool re_un);
	void unredoSizeForCells(const QHash<QModelIndex, QPair<int, int> > & sizes_hash, bool w_h, bool re_un);
	void unredoForCellsMerge(const QHash<QModelIndex, QPair<QString, QStandardItem *> > & merge_infoes, const QString & order, bool re_un);
	void unredoForTreeDataesShow(QStandardItem * info_item, QStandardItem * prim_item, QStandardItemModel * datamodel, QStandardItemModel * &last_model, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & old_hash, bool re_un);	
	void translationForShowModel(QSqlTableModel * showing_model);
	void resetFrameHashForSave(QHash<QString, QModelIndexList> & s_hash);
	void initModelViewFromDB(TableManagement * tree_guide, const QString & view_infoes, const QString & view_contents);
	void initFrameHashFromDB(const QString & hash_infoes);
	void judgeShowOrderForPlot(TableManagement * tree_part, QStandardItem * plot_item, const QString & order);	
	void judgeShowForTreeDataes(TableManagement * tree_zone, QStandardItem * item);
	void deletePicturesInDbTbl(const QString & tbl_contents);
	bool indexHidden(const QModelIndex i_index);
	QUndoStack * commandStack();
	QPixmap loadPictureFromImageFolder(DataSelectModel * s_model, QStandardItem * pic_item);	

signals:
	void setReadOnlyCell(bool read);
	void warningInfo(QString & info, bool reback);
	void hideHorizontalHeaderByUser();
	void hideVerticalHeaderByUser();
	void showHorizontalHeaderByUser();
	void showVerticalHeaderByUser();
	void groupTableSizeChanged();
	void freezingTable(QVariant freezing);
	void tablegroupCmd(TableMgmtGroupEditCmd * cmd);
	void printed();

public slots:
	void clearAllShowsInTable();
	void relatedWorkFromQmlEditor(QString guide, QString detail);
	void exportPdfTableAction();
	void clearPdfFramePtr();

protected:
	void paintEvent(QPaintEvent* event);

private slots:
	void cellClicked(const QModelIndex & index);
	void closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint);
	void receivePdfFileName(const QString & defined);
	void resizeTableForNewSizeSetted();	
	
private:
	void extractAloneItemForDataes(TableManagement * tree, QStandardItem * alone_item, QStandardItemModel * extract_model);
	void extractAloneItemForInfoes(TableManagement * tree, QStandardItem * alone_item, QStandardItemModel * extract_model);
	void extractDataFromItem(TableManagement * tree, QStandardItem * engi_item, QStandardItemModel * extract_model1, QSqlTableModel * extract_model2, QSqlQueryModel * extract_model3);
	void ensureFeildForReplace(QModelIndex & leftTop, QStandardItemModel * dataes_model1, QSqlTableModel * dataes_model2 = 0);
	void resizeForUnvisualModel(const QModelIndex & leftTop, const QModelIndex & rightBottom, QStandardItemModel * resize_model, QStandardItemModel * other_model1, QSqlTableModel * other_model2);
	void attatchBackupItemDataes(QModelIndex & leftTop, QStandardItemModel * dataes);
	void dettatchBackupItemDataes(const QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & old_dataes);
	void attachDataesFromOtherModel(const QModelIndex & begin, QStandardItemModel * model, QStandardItemModel * s_model, QSqlTableModel * other1, QSqlQueryModel * other2);
	void changingIndexesMatrics(const QModelIndexList & new_selected, QPair<int, int> & rows_columns);
	void drawPartCellsFrameLine(const QModelIndexList & draw_list, const QModelIndexList & undraw_list, QPainter & painter);
	void copyCurrentModelToSaveModel(QStandardItemModel * copyed_model);
	void translateCellContent(const QString & cell_str, QMap<int, QVariant> & content_map, QStringList & frame_list, QStringList & span_list);
	void translateIndexesToList(const QModelIndex & leftTop, const QModelIndex & rightBottom, QModelIndexList & dest_list);
	void checkEmptyModelArea(const QModelIndex & leftTop, const QModelIndex & rightBottom, QModelIndexList & dest_list);
	void attachDbManualTblOnModel(QStandardItem * info_item, QModelIndex from_index, QStandardItemModel * copyed_model, QStandardItemModel * copy_model);
	void paintTableForPrintPdf(bool painting);
	void initPdfPaperDiagram(const QString & type);
	bool clearContentsSelection(const QString & order, QHash<QModelIndex, QPair<QMap<int, QVariant>, QStandardItem *> > & cells);
	bool cutCopyCollection(const QString & hint, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & oper_hash);
	bool initPasteHash(QPair<QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> >, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > > & initting);
	bool cellsFrameInfoCollection(QPair<QString, QModelIndexList> & collection_pair);
	bool textAlignInfoesCollection(int flag, QHash<QModelIndex, QPair<QString, QStandardItem *> > & align_hash);
	bool mergeCellsInfoCollection(QHash<QModelIndex, QPair<QString, QStandardItem *> > & merge_infoes, bool merge_split);
	bool rmInsIndexListCollection(const QString & key_text, QMap<QModelIndex, QString> & sel_hash);
	bool modelContainNewContents(const QModelIndex & leftTop, QStandardItemModel * dataes_model1, QSqlTableModel * dataes_model2);
	bool oldColorCollection(const QString & new_color, QHash<QModelIndex, QPair<QString, QStandardItem *> > & color_hash, bool ft_bgd);
	bool oldSizesCollection(int new_width, int new_height, QHash<QModelIndex, QPair<int, int> > & info_hash);
	bool isEqualValueInHash(const QString & key_text, const QModelIndexList & value_list);
	QStandardItem * findPosItemForNewPlot(QStandardItem * parent, const QModelIndex & pos_index);
	bool edit_state;
	bool print_pdf;
	int currow_tail;
	QFont zoom_font;
	QString cell_oldText;
	QString pdf_name;
	QModelIndexList clear_frames;
	QModelIndexList clear_backup;	
	QList<QPair<int, int> > rows_hiding;
	QList<QPair<int, int> > columns_hiding;
	QPair<QString, QString> using_name;
	QHash<QString, QModelIndexList> actived_cells;
	QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > content_hashes;
	QMap<QString, QStandardItemModel *> selected_models;
	QUndoStack * m_commandStack;//need it for cpk initial dataes edit
	PrintFrame * pdf_frame;
	SpcNum * algorithm;
	SpcDataBase * inspector;
};

#endif
