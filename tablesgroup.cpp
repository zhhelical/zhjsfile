#include <QtGui>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include "tablesgroup.h"
#include "tablemgmtgroupeditcommand.h"
#include "tablemanagement.h"
#include "dataselectmodel.h"
#include "exportstylegenerator.h"
#include "plotwidget.h"
#include "spcnum.h"
#include "spcdatabase.h"
#include "dialog.h"
#include "mainwindow.h"
#include "printframe.h"

TablesGroup::TablesGroup(QWidget * parent)
:QTableView(parent), edit_state(true), print_pdf(false), pdf_frame(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setStyleSheet("border: 0px");
	setWordWrap(true);
	setTextElideMode(Qt::ElideNone);
	horizontalHeader()->setStyleSheet("QHeaderView{border: none} QHeaderView::section {padding-left: 6px; padding-right: 0px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D4D4D4, stop: 1 #DEDEDE)}");
	verticalHeader()->setStyleSheet("QHeaderView{border: none} QHeaderView::section {padding-left: 6px; padding-right: 0px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D4D4D4, stop: 1 #DEDEDE)}");	
	m_commandStack = new QUndoStack(this);
	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(cellClicked(const QModelIndex &)));
	connect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(resizeTableForNewSizeSetted()));
	connect(verticalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(resizeTableForNewSizeSetted()));
}

TablesGroup::~TablesGroup()
{}

void TablesGroup::setInspectors(SpcNum * cal_val, SpcDataBase * db_val)
{
	algorithm = cal_val;
	inspector = db_val;
}

void TablesGroup::initTable(const QPair<QString, QString> & ctrl_name, QStandardItemModel * curmodel)
{
	using_name = ctrl_name;	
	setModel(curmodel);	
	update();
	resizeTableForNewSizeSetted();	
}

void TablesGroup::setFontSize(bool zoom)
{
	int font_size = zoom_font.pointSize();
	if (zoom)
	{
		font_size += 1;
		zoom_font.setPointSize(font_size);
	}
	else
	{
		font_size -= 1;
		zoom_font.setPointSize(font_size);		
	}
	foreach (QModelIndex index, selectedIndexes())
	{
		if (qobject_cast<QStandardItemModel *>(model())->item(index.row(), index.column()))
			qobject_cast<QStandardItemModel *>(model())->item(index.row(), index.column())->setFont(zoom_font);
	}
}

void TablesGroup::showRelatedRows()
{
	if (rows_hiding.isEmpty())
		return;
	QPair<int, int> hide_back = rows_hiding.back();
	for (int i = hide_back.first; i < hide_back.second+1; i++)
		setRowHidden(i, false);
	rows_hiding.pop_back();
}
	
void TablesGroup::showRelatedColumns()
{
	if (columns_hiding.isEmpty())
		return;
	QPair<int, int> hide_back = columns_hiding.back();
	for (int i = hide_back.first; i < hide_back.second+1; i++)
		setColumnHidden(i, false);
	columns_hiding.pop_back();
}

void TablesGroup::hideRelatedRows()
{
	if (selectedIndexes().isEmpty())
		return;
	if (selectedIndexes().at(selectedIndexes().size()-1).row()-selectedIndexes().at(0).row()+1 == model()->rowCount())
		return;
	QModelIndexList hide_rows = selectedIndexes();
	foreach (QModelIndex index, hide_rows)
	{
		if (!isRowHidden(index.row()))
			setRowHidden(index.row(), true);
	}
	QPair<int, int> hide_now(hide_rows.at(0).row(), hide_rows.at(hide_rows.size()-1).row());
	rows_hiding << hide_now;			
}
	
void TablesGroup::hideRelatedColumns()
{
	if (selectedIndexes().isEmpty())
		return;
	if (selectedIndexes().at(selectedIndexes().size()-1).column()-selectedIndexes().at(0).column()+1 == model()->columnCount())
		return;
	QModelIndexList hide_columns = selectedIndexes();
	foreach (QModelIndex index, hide_columns)
	{
		if (!isColumnHidden(index.column()))
			setColumnHidden(index.column(), true);
	}
	QPair<int, int> hide_now(hide_columns.at(0).column(), hide_columns.at(hide_columns.size()-1).column());
	columns_hiding << hide_now;
}

void TablesGroup::sendSigToToolbar(TableManagement * from_tree, QStandardItem * first_item, QStandardItem * second_item, const QString & order)
{
	emit tablegroupCmd(new TableMgmtGroupEditCmd(from_tree, this, first_item, second_item, order));
}

void TablesGroup::unredoForCellTextEdit(const QPair<QModelIndex, QString> & edit_cell, bool re_un)
{
	QStringList str_list = edit_cell.second.split(tr("，，。"));
	if (re_un)
		model() -> setData(edit_cell.first, str_list[0]);
	else
		model() -> setData(edit_cell.first, str_list[1]);
}

void TablesGroup::unredoForClearContents(const QString & order, const QHash<QModelIndex, QPair<QMap<int, QVariant>, QStandardItem *> > & sel_cells, bool re_un)
{
	QHashIterator<QModelIndex, QPair<QMap<int, QVariant>, QStandardItem *> > i_hash(sel_cells);	
	if (order == tr("清空内容"))
	{
		while (i_hash.hasNext())
		{
			i_hash.next();
			QPair<QMap<int, QVariant>, QStandardItem *> clear_pair = i_hash.value();
			if (re_un)
				model() -> setItemData(i_hash.key(), QMap<int, QVariant>());	
			else
				model() -> setItemData(i_hash.key(), clear_pair.first);		
		}
	}
	else
	{
		while (i_hash.hasNext())
		{
			i_hash.next();
			if (re_un)
			{
				clear_frames << i_hash.key();
				//for clear actived_cells values()
			}	
			else
			{
				clear_frames.removeOne(i_hash.key());
				//for insert actived_cells values()
			}			
		}
		qSort(clear_frames);
		repaint();		
	}
}

void TablesGroup::unredoForCut(const QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & oper_cells, bool re_un)
{
	QHashIterator<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > i_hash(oper_cells);
	while (i_hash.hasNext()) 
	{
		i_hash.next();	
		if (re_un)
		{
			QModelIndex pos_index = i_hash.key().first;
			QMap<int, QVariant> empty = QMap<int, QVariant>();
			model() -> setItemData(pos_index, empty);
		}		
		else
			model() -> setItemData(i_hash.key().first, i_hash.value().first);
	}
}

void TablesGroup::unredoForPaste(const QPair<QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> >, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > > & paste_cells, bool re_un)
{
	QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > new_contents = paste_cells.first;
	QList<QPair<QModelIndex, QString> > new_pairs = new_contents.keys();
	QModelIndexList new_list;
	for (int i = 0; i < new_pairs.size(); i++)
		new_list << new_pairs.at(i).first;
	qSort(new_list);
	QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > old_contents = paste_cells.second;
	QList<QPair<QModelIndex, QString> > old_pairs = old_contents.keys();
	QModelIndexList old_list;
	for (int i = 0; i < old_pairs.size(); i++)
		old_list << old_pairs.at(i).first;
	qSort(old_list);
	QModelIndex from_index = old_list.at(0);
	int d_row = new_list.at(0).row()-from_index.row();
	int d_col = new_list.at(0).column()-from_index.column();
	for (int i = from_index.row(); i < old_list.back().row()+1; i++)
	{
		for (int j = from_index.column(); j < old_list.back().column()+1; j++)
		{
			if (indexWidget(model()->index(i, j)))
				indexWidget(model()->index(i, j)) -> deleteLater();
			if (!model()->itemData(model()->index(i, j)).isEmpty())
			{
				QMap<int, QVariant> empty_map = QMap<int, QVariant>();
				model() -> setItemData(model()->index(i, j), empty_map);
			}
			if (re_un)
			{
				if (new_list.contains(model()->index(i+d_row, j+d_col)))
				{
					QPair<QModelIndex, QString> new_pair;
					for (int m = 0; m < new_pairs.size(); m++)
					{
						if (new_pairs.at(m).first == model()->index(i+d_row, j+d_col))
						{
							new_pair = new_pairs.at(m);
							new_pairs.removeOne(new_pairs.at(m));
							break;
						}
					}
					QModelIndex pos_index = model()->index(i, j);
					QStringList row_col;
					if (new_pair.second.contains(";"))
					{
						QStringList rc_copy = new_pair.second.split(";");
						row_col = rc_copy[0].split(",");
					}
					else	
						row_col = new_pair.second.split(",");
					int row_span = row_col[0].toInt();
					int col_span = row_col[1].toInt();
					setSpan(pos_index.row(), pos_index.column(), row_span, col_span);
					model() -> setItemData(pos_index, new_contents.value(new_pair).first);					
				}
			}
			else
			{
				if (old_list.contains(model()->index(i, j)))
				{
					QPair<QModelIndex, QString> old_pair;
					for (int m = 0; m < old_pairs.size(); m++)
					{
						if (old_pairs.at(m).first == model()->index(i, j))
						{
							old_pair = old_pairs.at(m);
							old_pairs.removeOne(old_pairs.at(m));
							break;
						}
					}
					QModelIndex pos_index = model()->index(i, j);
					QStringList row_col;
					if (old_pair.second.contains(";"))
					{
						QStringList rc_copy = old_pair.second.split(";");
						row_col = rc_copy[0].split(",");
					}
					else	
						row_col = old_pair.second.split(",");
					int row_span = row_col[0].toInt();
					int col_span = row_col[1].toInt();
					setSpan(pos_index.row(), pos_index.column(), row_span, col_span);
					model() -> setItemData(pos_index, old_contents.value(old_pair).first);					
				}
			}
		}
	}
}

void TablesGroup::unredoForInsertRowsCols(const QMap<QModelIndex, QString> & opr_map, const QString & row_col, bool re_un)
{
	if (re_un)
	{
		if (row_col.contains(tr("行")))
			model() -> insertRow(opr_map.keys().at(0).row());
		else
			model() -> insertColumn(opr_map.keys().at(0).column());
	}
	else
	{
		if (row_col.contains(tr("行")))
			model() -> removeRow(opr_map.keys().at(0).row());
		else
			model() -> removeColumn(opr_map.keys().at(0).column());		
	}
	resizeTableForNewSizeSetted();	
}

void TablesGroup::unredoForRemoveRowsCols(const QMap<QModelIndex, QString> & opr_map, const QString & row_col, bool re_un)
{
	QModelIndex rc_index;
	QModelIndex base_index = opr_map.keys().at(0);
	QMapIterator<QModelIndex, QString> i_map(opr_map);
	if (re_un)
	{
		if (row_col.contains(tr("行")))
		{
			while (i_map.hasNext()) 
			{
				i_map.next();
				if (rc_index.row() != i_map.key().row())
				{
					rc_index = i_map.key();
					model() -> removeRow(base_index.row());
				}
			}
		}
		else
		{
			while (i_map.hasNext()) 
			{
				i_map.next();
				if (i_map.key().row() > base_index.row())
					break;
				if (rc_index.column() != i_map.key().column())
				{
					rc_index = i_map.key();
					model() -> removeColumn(base_index.column());
				}
			}
		}
	}
	else
	{
		if (row_col.contains(tr("行")))
		{
			while (i_map.hasNext()) 
			{
				i_map.next();
				if (rc_index.row() != i_map.key().row())
				{
					rc_index = i_map.key();
					model() -> insertRow(i_map.key().row());
				}
				if (!i_map.value().isEmpty())
				{
					QString content = i_map.value();
					QStringList content_list = content.split(tr("；，。"));
					content_list.pop_back();
					QMap<int, QVariant> index_dataes;
					foreach (QString str, content_list)
					{
						QStringList val_list = str.split(tr("，，。"));
						index_dataes.insert(val_list[0].toInt(), val_list[1]);
					}
					model() -> setItemData(i_map.key(), index_dataes);
				}
			}			
		}
		else
		{
			while (i_map.hasNext()) 
			{
				i_map.next();
				if (rc_index.column()!=i_map.key().column() && i_map.key().row()==base_index.row())
				{
					rc_index = i_map.key();
					model() -> insertColumn(i_map.key().column());
				}
				if (!i_map.value().isEmpty())
				{
					QString content = i_map.value();
					QStringList content_list = content.split(tr("；，。"));
					content_list.pop_back();
					QMap<int, QVariant> index_dataes;
					foreach (QString str, content_list)
					{
						QStringList val_list = str.split(tr("，，。"));
						index_dataes.insert(val_list[0].toInt(), val_list[1]);
					}
					model() -> setItemData(i_map.key(), index_dataes);
				}
			}			
		}		
	}
	resizeTableForNewSizeSetted();
}

void TablesGroup::unredoForCellsTextAlign(const QHash<QModelIndex, QPair<QString, QStandardItem *> > & align_hash, bool re_un)
{
	QModelIndexList align_list = align_hash.keys();
	qSort(align_list);
	foreach (QModelIndex index, align_list)
	{
		QPair<QString, QStandardItem *> val_pair = align_hash[index];
		QStringList old_new = val_pair.first.split(tr("，"));
		int align = 0;
		if (re_un)
			align = old_new[1].toInt();
		else
			align = old_new[0].toInt();
		if (align == 1)
			qobject_cast<QStandardItemModel *>(model())->item(index.row(), index.column())->setTextAlignment(Qt::AlignLeft);
		else if (align == 2)
			qobject_cast<QStandardItemModel *>(model())->item(index.row(), index.column())->setTextAlignment(Qt::AlignRight);
		else
			qobject_cast<QStandardItemModel *>(model())->item(index.row(), index.column())->setTextAlignment(Qt::AlignCenter);
	}	
}

void TablesGroup::unredoForCellsFrame(const QPair<QString, QModelIndexList> & paint_frame, bool reun)// need promotion
{
	if (!paint_frame.first.contains(tr("无框线")))
	{
		if (reun)
		{
			if (!clear_frames.isEmpty())
			{
				clear_backup = clear_frames;
				foreach (QModelIndex index, paint_frame.second)
					clear_frames.removeOne(index);
			}
			actived_cells.insert(paint_frame.first, paint_frame.second);
		}
		else
		{
			actived_cells.remove(paint_frame.first);
			clear_frames = clear_backup;
			qSort(clear_frames);			
		}
	}
	else
	{
		if (reun)
		{
			foreach (QModelIndex index, paint_frame.second)
				clear_frames << index;
			qSort(clear_frames);
		}
		else
		{
			foreach (QModelIndex index, paint_frame.second)
				clear_frames.removeOne(index);
		}	
	}
	repaint();
}

void TablesGroup::unredoColorForCells(const QHash<QModelIndex, QPair<QString, QStandardItem *> > & color_infoes, bool ft_bgd, bool re_un)
{
	QHashIterator<QModelIndex, QPair<QString, QStandardItem *> > i_hash(color_infoes);
	if (re_un)
	{
		while (i_hash.hasNext()) 
		{
			i_hash.next();
			QPair<QString, QStandardItem *> color_text = i_hash.value();
			QStringList new_old = color_text.first.split(tr("，"));
			if (ft_bgd)
				model()->setData(i_hash.key(), QVariant(QColor(new_old[0])), Qt::ForegroundRole);
			else
				model()->setData(i_hash.key(), QVariant(QColor(new_old[0])), Qt::BackgroundRole);
		}			
	}
	else
	{
		while (i_hash.hasNext()) 
		{
			i_hash.next();
			QPair<QString, QStandardItem *> color_text = i_hash.value();
			QStringList new_old = color_text.first.split(tr("，"));
			if (ft_bgd)
				model()->setData(i_hash.key(), QVariant(QColor(new_old[1])), Qt::ForegroundRole);
			else
				model()->setData(i_hash.key(), QVariant(QColor(new_old[1])), Qt::BackgroundRole);
		}
	}	
}

void TablesGroup::unredoSizeForCells(const QHash<QModelIndex, QPair<int, int> > & sizes_hash, bool w_h, bool re_un)
{
	QHashIterator<QModelIndex, QPair<int, int> > i_hash(sizes_hash);
	if (re_un)
	{
		while (i_hash.hasNext()) 
		{
			i_hash.next();
			if (w_h)
				setColumnWidth(i_hash.key().column(), i_hash.value().first);
			else
				setRowHeight(i_hash.key().row(), i_hash.value().first);
		}
	}
	else
	{
		while (i_hash.hasNext()) 
		{
			i_hash.next();
			if (w_h)
				setColumnWidth(i_hash.key().column(), i_hash.value().second);
			else
				setRowHeight(i_hash.key().row(), i_hash.value().second);
		}
	}
	resizeTableForNewSizeSetted();	
}

void TablesGroup::unredoForCellsMerge(const QHash<QModelIndex, QPair<QString, QStandardItem *> > & merge_infoes, const QString & order, bool re_un)//no finished
{
	QModelIndexList merge_list = merge_infoes.keys();
	qSort(merge_list);
	if (re_un)
	{
		if (order == tr("合并单元"))
		{
			QModelIndex base_row;
			QModelIndex base_col;
			int row_span = 0, col_span = 0;
			foreach (QModelIndex index, merge_list)
			{
				QPair<QString, QStandardItem *> str_plot = merge_infoes[index];				
				if (base_row.row()!=index.row() && index.column()==merge_list[0].column())
				{
					base_row = index;
					QStringList span_list = str_plot.first.split(",");
					row_span += span_list[0].toInt();
				}
				if (base_col.column()!=index.column() && index.row()==merge_list[0].row())
				{
					base_col = index;
					QStringList span_list = str_plot.first.split(",");
					col_span += span_list[1].toInt();
				}				
			}
			setSpan(merge_list[0].row(), merge_list[0].column(), row_span, col_span);
			QRect new_rect = visualRect(merge_list[0]);
			QMap<int, QVariant>	cell_data = model()->itemData(merge_list.at(0));
			QPixmap pix_reset = cell_data[1].value<QPixmap>();
			if (!pix_reset.isNull())
			{
				cell_data[1] = pix_reset.scaled(new_rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				model() -> setItemData(merge_list.at(0), cell_data);
			}	
		}
		else
		{
			for (int from_row = merge_list.at(0).row(); from_row < merge_list.back().row()+1; from_row++)
			{
				for (int from_col = merge_list.at(0).column(); from_col < merge_list.back().column()+1; from_col++)
					setSpan(from_row, from_col, 1, 1);
			}			
		}
	}
	else
	{
		foreach (QModelIndex index, merge_list)
		{
			QPair<QString, QStandardItem *> str_plot = merge_infoes[index];
			QStringList span_list = str_plot.first.split(",");
			setSpan(index.row(), index.column(), span_list[0].toInt(), span_list[1].toInt());
			QRect new_rect = visualRect(index);
			QMap<int, QVariant>	cell_data = model()->itemData(index);
			QPixmap pix_reset = cell_data[1].value<QPixmap>();
			if (!pix_reset.isNull())
			{
				cell_data[1] = pix_reset.scaled(new_rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				model() -> setItemData(index, cell_data);
			}			
		}		
	}
}

void TablesGroup::unredoForTreeDataesShow(QStandardItem * info_item, QStandardItem * prim_item, QStandardItemModel * datamodel, QStandardItemModel * &last_model, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & old_hash, bool re_un)
{
	QModelIndex left_index;
	if (last_model)
	{
		QStringList index_pos = prim_item->text().split(",");		
		QStandardItemModel * keep_model = qobject_cast<QStandardItemModel *>(model());
		setModel(last_model);
		left_index = model()->index(index_pos.at(0).toInt(), index_pos.at(1).toInt());
		if (datamodel->data(datamodel->index(0, 0)).toString().contains(tr("，，。")) && datamodel->data(datamodel->index(0, 0)).toString().contains("manualtable") && !datamodel->data(datamodel->index(0, 0)).toString().contains(tr("拷贝完成")))
			attachDbManualTblOnModel(info_item, left_index, datamodel, last_model);
		last_model = keep_model;
		update();
		resizeTableForNewSizeSetted();		
		if (keep_model->rowCount()!=last_model->rowCount() || keep_model->columnCount()!=last_model->columnCount() || (datamodel->data(datamodel->index(0, 0)).toString().contains(tr("，，。")) && datamodel->data(datamodel->index(0, 0)).toString().contains("manualtable")))	
			return;
	}
	else
	{
		QList<QPair<QModelIndex, QString> > pair_list = old_hash.keys();
		QModelIndexList model_list;
		for (int i = 0; i < pair_list.size(); i++)
			model_list << pair_list.at(i).first;
		qSort(model_list);
		left_index = model_list.at(0);
	}
	if (re_un)
		attatchBackupItemDataes(left_index, datamodel);
	else
		dettatchBackupItemDataes(old_hash);
}

void TablesGroup::translationForShowModel(QSqlTableModel * showing_model)//no finished
{
	QModelIndex top_left = model()->index(0, 0);//need promotion for selection model or usr model index dest
	QString filter("rows > 0");
	showing_model -> setFilter(filter);
	showing_model -> removeColumns(showing_model->columnCount()-4, 3);
	showing_model -> removeColumn(0);
	ensureFeildForReplace(top_left, 0, showing_model);
	for (int i = 0; i < showing_model->rowCount(); i++)
	{
		for (int j = 0; j < showing_model->columnCount(); j++)
		{
			QString index_str = showing_model->data(showing_model->index(i, j)).toString();
			QMap<int, QVariant> item_map;
			QStringList frame_lines, span_lists;
			if (!index_str.isEmpty())
				translateCellContent(index_str, item_map, frame_lines, span_lists);
			if (!item_map.isEmpty())
				model() -> setItemData(model()->index(i, j), item_map);
			if (!frame_lines.isEmpty())
			{
				if (!actived_cells.isEmpty())
					actived_cells.clear();
				foreach (QString style, frame_lines)
				{
					QStringList real_lines = style.split("^~");
					QStringList line_style = real_lines[1].split(",");
					QModelIndex last_index = model()->index(line_style[0].toInt(), line_style[1].toInt());
					QModelIndexList insert_list;
					translateIndexesToList(model()->index(j, j), last_index, insert_list);
					actived_cells.insert(real_lines[0], insert_list);
				}
			}
			if (!span_lists.isEmpty())
			{
				int row_span = span_lists[0].toInt();
				int col_span = span_lists[1].toInt();
				if (row_span > 1)
					setSpan(i, j, row_span, 1);
				if (col_span > 1)
					setSpan(i, j, 1, col_span);
			}
		}
	}
	repaint();
	update();
	resizeTableForNewSizeSetted();		
	delete showing_model;
}

void TablesGroup::resetFrameHashForSave(QHash<QString, QModelIndexList> & s_hash)
{
	s_hash = actived_cells;
	QHashIterator<QString, QModelIndexList> i_hash(s_hash);
	while (i_hash.hasNext())
	{
		i_hash.next();
		QModelIndexList h_list = i_hash.value();
		foreach (QModelIndex index, h_list)
		{
			if (clear_frames.contains(index))
			{
				h_list.removeOne(index);
				if (h_list.isEmpty())
					s_hash.remove(i_hash.key());
			}			
		}
		if (!h_list.isEmpty() && h_list!=i_hash.value())
			s_hash.insert(i_hash.key(), h_list);
	}	
}

void TablesGroup::initModelViewFromDB(TableManagement * tree_guide, const QString & view_infoes, const QString & view_contents)
{
	QStringList first_split = view_infoes.split(";");
	QStringList second_split = view_contents.split(tr("，，～；"));
	QStringList rows_split = first_split[0].split(",");
	QStringList cols_split = first_split[1].split(",");
	QStandardItemModel * init_model = new QStandardItemModel(rows_split.size()-1, cols_split.size()-1);
	setModel(init_model);
	for (int i = 0; i < rows_split.size()-1; i++)
	{
		setRowHeight(i, rows_split[i].toInt());
		for (int j = 0; j < cols_split.size()-1; j++)
		{
			if (i == 0)
				setColumnWidth(j, cols_split[j].toInt());
			QString save_pos = "*"+QString("%1").arg(i)+","+QString("%1").arg(j)+"*";
			QStringList existed_list = second_split.filter(save_pos);
			if (!existed_list.isEmpty())
			{
				QStringList detailed_split = existed_list[0].split(tr("，，～，"));
				foreach (QString cell_str, detailed_split)
				{
					if (cell_str.contains("*"))
						continue;
					if (cell_str.contains("$&"))
					{
						QStringList span_split = cell_str.split("$&");					  
						if (span_split[0] != "1" || span_split[1] != "1")
							setSpan(i, j, span_split[0].toInt(), span_split[1].toInt());
					}					
					if (cell_str.contains(tr("，^，")))
					{
						QMap<int, QVariant> c_dataes;
						QStringList detailed_contents = detailed_split[1].split(tr("，^；"));
						foreach (QString m_str, detailed_contents)
						{
							if (m_str.isEmpty())
								break;
							QStringList m_values = m_str.split(tr("，^，"));
							if (m_values.at(1).contains(tr("单元图")))
							{
								QPixmap f_picture = inspector->foundPictureFromDb("pictures", m_values.at(1));
								c_dataes[m_values[0].toInt()] = f_picture;								
							}
							else
							{
								if (m_values[0].toInt() == Qt::FontRole)
								{
									QFont font_role(m_values[1]);
									c_dataes.insert(m_values[0].toInt(), font_role);
								}
								else if (m_values[0].toInt() == Qt::TextAlignmentRole)
									c_dataes.insert(m_values[0].toInt(), m_values[1].toInt());
								else if ((m_values[0].toInt()==Qt::BackgroundRole && !m_values[1].isEmpty()) || m_values[0].toInt()==Qt::ForegroundRole)
								{
									QColor color(m_values[1]);
									QBrush bfg_role(color);
									c_dataes.insert(m_values[0].toInt(), bfg_role);
								}
								else
									c_dataes.insert(m_values[0].toInt(), m_values[1]);
							}
						}						
						model() -> setItemData(model()->index(i, j), c_dataes);
					}				  
				}
			}
		}
	}
	if (tree_guide->treeType() == 2)
		edit_state = false;
	update();
	resizeTableForNewSizeSetted();		
}

void TablesGroup::initFrameHashFromDB(const QString & hash_infoes)
{
	QStringList first_split = hash_infoes.split(tr("）；"));
	foreach (QString f_str, first_split)
	{
		if (f_str.isEmpty())
			break;
		QStringList second_split = f_str.split(tr("，（"));
		QStringList third_split = second_split[1].split(";");
		QModelIndexList index_list;
		foreach (QString t_str, third_split)
		{
			if (t_str.isEmpty())
				break;
			QStringList model_rc = t_str.split(",");
			QModelIndex e_index = model()->index(model_rc[0].toInt(), model_rc[1].toInt());
			index_list << e_index;
		}
		actived_cells.insert(second_split[0], index_list);
	}
	repaint();
}

void TablesGroup::judgeShowOrderForPlot(TableManagement * tree_part, QStandardItem * plot_item, const QString & order)
{
 	QModelIndex pos_index; 
  	if (selectedIndexes().isEmpty())
		pos_index = model()->index(1, 0);
	else
		pos_index = selectedIndexes().at(0);
	QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > need_hash; 
	QString span_info = QString("%1").arg(rowSpan(pos_index.row(), pos_index.column()))+","+QString("%1").arg(columnSpan(pos_index.row(), pos_index.column()));
	QPair<QModelIndex, QString> key_pair(pos_index, span_info);
	QPair<QMap<int, QVariant>, QStandardItem *> old_content;
	old_content.first = model()->itemData(pos_index);	
	old_content.second = 0;	
	need_hash.insert(key_pair, old_content);
	emit tablegroupCmd(new TableMgmtGroupEditCmd(tree_part, this, plot_item, 0, 0, 0, need_hash, order));		
}

void TablesGroup::judgeShowForTreeDataes(TableManagement * tree_zone, QStandardItem * item)
{
	QModelIndex from_index; 
  	if (selectedIndexes().isEmpty())
		from_index = model()->index(1, 0);
	else
		from_index = selectedIndexes().at(0);
	QStandardItemModel * standard = new QStandardItemModel;
	QSqlTableModel m_sql(this, inspector->currentConnectionDb()->database(inspector->currentConnectionDb()->connectionName()));
	QSqlQueryModel m_query(this);
	extractDataFromItem(tree_zone, item, standard, &m_sql, &m_query);
	if (!standard->rowCount() && !m_sql.rowCount() && !m_query.rowCount())
	{
		delete standard;
		return;
	}
	if (m_sql.rowCount())
	{
		if (m_sql.headerData(0, Qt::Horizontal).toString() != "column1")
		{
			QList<QStandardItem *> append_list;
			for (int i = 0; i < m_sql.columnCount(); i++)
			{
				QString h_transed;
				algorithm -> dbSpcNamesTranslation(m_sql.headerData(i, Qt::Horizontal).toString(), h_transed);			  
				QStandardItem * add_item = new QStandardItem(h_transed);
				append_list << add_item;
			}
			standard -> appendRow(append_list);
		}
		for (int i = 0; i < m_sql.rowCount(); i++)
		{
			if (m_sql.data(m_sql.index(i, 1)).isNull())
				continue;
			QList<QStandardItem *> append_list;
			for (int j = 0; j < m_sql.columnCount(); j++)
			{
				QStandardItem * add_item = new QStandardItem;
				add_item -> setText(m_sql.data(m_sql.index(i, j)).toString());
				append_list << add_item;
			}
			standard -> appendRow(append_list);
		}				
	}
	if (m_query.rowCount())
	{
		if (m_query.headerData(0, Qt::Horizontal).toString() != QString("%1").arg(1))
		{
			QList<QStandardItem *> append_list;
			for (int i = 0; i < m_query.columnCount(); i++)
			{
				QString h_transed;
				algorithm -> dbSpcNamesTranslation(m_query.headerData(i, Qt::Horizontal).toString(), h_transed);			  
				QStandardItem * add_item = new QStandardItem(h_transed);
				append_list << add_item;
			}
			standard -> appendRow(append_list);
		} 
		for (int i = 0; i < m_query.rowCount(); i++)
		{
			QList<QStandardItem *> append_list;
			for (int j = 0; j < m_query.columnCount(); j++)
			{
				QStandardItem * add_item = new QStandardItem;
				add_item -> setText(m_query.data(m_query.index(i, j)).toString());
				append_list << add_item;
			}
			standard -> appendRow(append_list);
		}				
	}
	if (standard -> rowCount())
	{
		QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > old_contents;		
		if (!modelContainNewContents(from_index, standard, 0) || (standard->data(standard->index(0, 0)).toString().contains(tr("，，。")) && standard->data(standard->index(0, 0)).toString().contains("manualtable")))
		{
			QStandardItem * pos_item = new QStandardItem(QString("%1").arg(from_index.row())+","+QString("%1").arg(from_index.column()));
			QStandardItemModel * kept_old = new QStandardItemModel(model()->rowCount(), model()->columnCount());
			for (int i = 0; i < model()->rowCount(); i++)
			{
				for (int j = 0; j < model()->columnCount(); j++)
				{
					if (!isIndexHidden(model()->index(i, j)))					
						kept_old -> setItemData(kept_old->index(i, j), model()->itemData(model()->index(i, j)));					
				}
			}
			emit tablegroupCmd(new TableMgmtGroupEditCmd(tree_zone, this, item, pos_item, standard, kept_old, old_contents, tr("添加数据")));
			return;
		}
		int row_offset = standard->rowCount()-1, col_offset = standard->columnCount()-1;
		if (standard->headerData(0, Qt::Horizontal).toString() != QString("%1").arg(1))
			row_offset += 1;
		if (item->hasChildren())
			row_offset += 1;
		if (from_index.row()+row_offset > model()->rowCount()-1)
			row_offset = model()->rowCount()-1-from_index.row();
		if (from_index.column()+col_offset > model()->columnCount()-1)
			row_offset = model()->columnCount()-1-from_index.column();		
		QModelIndex need_botright = model()->index(from_index.row()+row_offset, from_index.column()+col_offset); 
		for (int i = from_index.row(); i < need_botright.row()+1; i++)
		{
			for (int j = from_index.column(); j < need_botright.column()+1; j++)
			{
				if (!isIndexHidden(model()->index(i, j)))
				{
					QString span_info = QString("%1").arg(rowSpan(model()->index(i, j).row(), model()->index(i, j).column()))+","+QString("%1").arg(columnSpan(model()->index(i, j).row(), model()->index(i, j).column()));
					QPair<QModelIndex, QString> key_pair(model()->index(i, j), span_info);
					QPair<QMap<int, QVariant>, QStandardItem *> in_pair;						
					in_pair.first = model()->itemData(model()->index(i, j));						
					old_contents.insert(key_pair, in_pair);
				}
			}
		}
		emit tablegroupCmd(new TableMgmtGroupEditCmd(tree_zone, this, item, 0, standard, 0, old_contents, tr("添加数据")));
	}
	else
		delete standard;
}

void TablesGroup::deletePicturesInDbTbl(const QString & tbl_contents)
{
	QStringList first_split = tbl_contents.split(tr("，，～；"));
	first_split.pop_back();
	foreach (QString content, first_split)
	{
		QStringList detailed_split = content.split(tr("，，～，"));
		foreach (QString cell_str, detailed_split)
		{
			if (cell_str.contains(tr("，^，")))
			{
				QStringList detailed_contents = detailed_split[1].split(tr("，^；"));
				foreach (QString m_str, detailed_contents)
				{
					if (m_str.isEmpty())
						break;
					QStringList m_values = m_str.split(tr("，^，"));
					if (m_values.at(1).contains(tr("单元图")))
					{
						QString pic_row("name="+m_values.at(1));
						QString col;
						inspector -> deleteDataes("pictures", pic_row, col);
					}						
				}
			}
		}
	}	
}

bool TablesGroup::indexHidden(const QModelIndex i_index)
{
	return isIndexHidden(i_index);
}
		
QUndoStack * TablesGroup::commandStack()
{
	return m_commandStack;
}

QPixmap TablesGroup::loadPictureFromImageFolder(DataSelectModel * s_model, QStandardItem * pic_item)
{
	QList<QStandardItem *> pic_line;
	s_model -> findItemsSingleLine(pic_item, pic_line, s_model->item(0)->text());
	pic_line.pop_front();
	pic_line.pop_front();
	QString tol_path = tr("/home/dapang/workstation/image文件");
	foreach (QStandardItem * p_item, pic_line)
		tol_path += "/"+p_item->text();		
	QPixmap pix;
	pix.load(tol_path);
	return pix;
}

void TablesGroup::clearAllShowsInTable()
{
	for (int i = 0; i < model()->rowCount(); i++)
	{
		for (int j = 0; j < model()->columnCount(); j++)
		{
			if (model()->data(model()->index(i, j)).toString().isEmpty())
				continue;
			model() -> setData(model()->index(i, j), "");
		}
	}
}

void TablesGroup::relatedWorkFromQmlEditor(QString guide, QString detail)
{
	if (guide==tr("冻结滑动"))
	{
		if (detail == "false")
			emit freezingTable(false);
		else
			emit freezingTable(true);
	}
	else if (guide == "subslideRepeater")
	{
		if (detail==tr("清空内容") || detail==tr("清空格式") || detail==tr("取消选择"))
		{
			if (detail == tr("取消选择"))
			{
				if (!selectedIndexes().isEmpty())
					selectionModel() -> clear();
			}
			else
			{
				QHash<QModelIndex, QPair<QMap<int, QVariant>, QStandardItem *> > find_cells;
				if (!clearContentsSelection(detail, find_cells))
					return;
				emit tablegroupCmd(new TableMgmtGroupEditCmd(0, this, detail, find_cells));//no finished for tree change
			}
		}
		else if (detail == tr("放大字体"))
			setFontSize(true);
		else if (detail == tr("缩小字体"))
			setFontSize(false);
		else if (detail==tr("复制") || detail==tr("剪切"))
		{
			QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > collections;
			if (!cutCopyCollection(detail, collections))
				return;
			if (detail==tr("剪切"))
			{
				QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > empty_hash = collections;
				empty_hash.clear();
				QPair<QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> >, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > > collection_pair(collections, empty_hash);
				emit tablegroupCmd(new TableMgmtGroupEditCmd(0, this, detail, collection_pair));
			}			
		}
		else if (detail == tr("粘贴"))
		{
			QPair<QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> >, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > > paste_pairs;
			if (!initPasteHash(paste_pairs))
				return;
			emit tablegroupCmd(new TableMgmtGroupEditCmd(0, this, detail, paste_pairs));
		}
		else if (detail==tr("左对齐") || detail==tr("居中") || detail==tr("右对齐"))//no finished for align init
		{
			int align_order = 0;
			if (detail == tr("左对齐"))
				align_order = Qt::AlignLeft;
			else if (detail == tr("居中"))
				align_order = Qt::AlignCenter;
			else
				align_order = Qt::AlignRight;
			QHash<QModelIndex, QPair<QString, QStandardItem *> > hash;
			if (!textAlignInfoesCollection(align_order, hash))
				return;
			emit tablegroupCmd(new TableMgmtGroupEditCmd(this, detail, hash));			
		}
		else if (detail == tr("行表头"))
		{
			if (verticalHeader()->isVisible())
				verticalHeader()->hide();
			else
				verticalHeader()->show();	
		}
		else if (detail == tr("列表头"))
		{
			if (horizontalHeader()->isVisible())
				horizontalHeader()->hide();
			else
				horizontalHeader()->show();
		}
		else if (detail == tr("行显示"))//change it and below 3 for texteditcommand
			showRelatedRows();
		else if (detail == tr("行隐藏"))
			hideRelatedRows();
		else if (detail == tr("列显示"))
			showRelatedColumns();
		else if (detail == tr("列隐藏"))
			hideRelatedColumns();
		else if (detail == tr("合并单元") || detail == tr("取消合并"))
		{
			QHash<QModelIndex, QPair<QString, QStandardItem *> > mergings;
			if ((detail == tr("合并单元") && !mergeCellsInfoCollection(mergings, true)) || (detail == tr("取消合并") && !mergeCellsInfoCollection(mergings, false)))
				return;
			emit tablegroupCmd(new TableMgmtGroupEditCmd(this, detail, mergings));
		}	
		else if (detail == tr("插入行") || detail == tr("删除行") || detail==tr("插入列") || detail==tr("删除列"))
		{
			QMap<QModelIndex, QString> selectings;
			if (!rmInsIndexListCollection(detail, selectings))
				return;
			emit tablegroupCmd(new TableMgmtGroupEditCmd(this, detail, selectings));
		}
		else if (detail==tr("粗匣框线") || detail==tr("外侧框线") || detail==tr("所有框线") || detail==tr("左框线") || detail==tr("右框线") || detail==tr("上框线") || detail==tr("下框线") || detail==tr("无框线"))
		{
			QPair<QString, QModelIndexList> collect_pair;
			collect_pair.first = detail;
			if (!cellsFrameInfoCollection(collect_pair))
				return;
			emit tablegroupCmd(new TableMgmtGroupEditCmd(this, collect_pair.first, collect_pair));
		}
		else if (detail.contains(tr("横向")) || detail.contains(tr("纵向")) || detail == tr("实际尺寸"))
		{
			if (pdf_frame)
				return;
			if (detail != tr("实际尺寸"))
				initPdfPaperDiagram(detail);
			else
				exportPdfTableAction();
		}
	}
	else if (guide == tr("文字") || guide == tr("表格"))
	{
		QHash<QModelIndex, QPair<QString, QStandardItem *> > index_hash;
		if (guide == tr("文字"))
		{
			if (!oldColorCollection(detail, index_hash, true))
				return;
			emit tablegroupCmd(new TableMgmtGroupEditCmd(this, guide, index_hash));
		}
		else
		{
			if (!oldColorCollection(detail, index_hash, false))
				return;
			emit tablegroupCmd(new TableMgmtGroupEditCmd(this, guide, index_hash));
		}
	}
	else if (guide.contains(tr("列宽：")) || guide.contains(tr("行高：")))
	{
		QHash<QModelIndex, QPair<int, int> > index_hash;
		if (guide==tr("列宽：") && !detail.isEmpty())
		{
			int col_size = detail.toInt();
			if (!oldSizesCollection(col_size, 0, index_hash))
				return;
			emit tablegroupCmd(new TableMgmtGroupEditCmd(this, guide, index_hash));
		}
		if (guide==tr("行高：") && !detail.isEmpty())
		{
			int row_size = detail.toInt();
			if (!oldSizesCollection(0, row_size, index_hash))
				return;
			emit tablegroupCmd(new TableMgmtGroupEditCmd(this, guide, index_hash));	
		}			
	}
}

void TablesGroup::exportPdfTableAction()
{
 	DiaLog * save_dialog = new DiaLog;
	MainWindow * mw = qobject_cast<MainWindow *>(inspector->parent()->parent());
	connect(save_dialog, SIGNAL(sendSaveFileName(const QString &)), this, SLOT(receivePdfFileName(const QString &)));
	QString export_file(tr("导出文件"));
	save_dialog -> initSaveStrDefineDialog(mw, inspector, export_file);
	if (save_dialog->exec())
	{
		selectionModel() -> clear();
		paintTableForPrintPdf(true);
		ExportPdfGenerator * exp_pdf = new ExportPdfGenerator(this);
		if (pdf_frame)
		{
			pdf_frame -> hide();			
			exp_pdf -> printTableView(pdf_name, pdf_frame->showedPaperFrame(), pdf_frame);
		}
		else
			exp_pdf -> printTableView(pdf_name, pdf_name, 0);
		paintTableForPrintPdf(false);
	} 	
	emit printed();
}

void TablesGroup::clearPdfFramePtr()
{
	pdf_frame = 0;
}

void TablesGroup::paintEvent(QPaintEvent* event)
{
 	QTableView::paintEvent(event); 
	QPainter painter(viewport());  
	if (print_pdf)
	{
		QPen pen(Qt::white, 1);
		painter.setPen(pen);
		for (int i = 0; i < model()->rowCount(); i++)
		{
			for (int j = 0; j < model()->columnCount(); j++)
			{
				if (!isIndexHidden(model()->index(i, j)))
				{
					QRect rect = visualRect(model()->index(i, j));
					rect.adjust(-1, -1, 1, 1);
					painter.drawRect(rect);
				}
			}
		}
	}
	if(actived_cells.isEmpty())
		return;	
	QHashIterator<QString, QModelIndexList> i_hash(actived_cells);
	while (i_hash.hasNext()) 
	{
		i_hash.next();
		QRect rect = visualRect(i_hash.value().at(0)) | visualRect(i_hash.value().at(i_hash.value().size()-1));
		rect.adjust(-1, -1, 1, 1);
		if (i_hash.key().contains(tr("粗匣框线")))
		{
			QPen pen(Qt::black, 2);
			painter.setPen(pen);
			if (clear_frames.isEmpty())
				painter.drawRect(rect);
			else
			{
				foreach (QModelIndex index, i_hash.value())
				{
					if (!clear_frames.contains(index))
					{
						if (index.row() == i_hash.value().at(0).row())
						{
							QRect cell_rect = visualRect(index); 
							cell_rect.adjust(-1, -1, 0, 0);
							painter.drawLine(cell_rect.topLeft().x(), cell_rect.topLeft().y(), cell_rect.topRight().x(), cell_rect.topRight().y());
						}
						if (index.column() == i_hash.value().at(0).column())
						{
							QRect cell_rect = visualRect(index); 
							cell_rect.adjust(-1, -1, 0, 0);
							painter.drawLine(cell_rect.topLeft().x(), cell_rect.topLeft().y(), cell_rect.bottomLeft().x(), cell_rect.bottomLeft().y());
						}
						if (index.row() == i_hash.value().at(i_hash.value().size()-1).row())
						{
							QRect cell_rect = visualRect(index); 
							cell_rect.adjust(0, 0, 1, 1);
							painter.drawLine(cell_rect.bottomLeft().x(), cell_rect.bottomLeft().y(), cell_rect.bottomRight().x(), cell_rect.bottomRight().y());
						}
						if (index.column() == i_hash.value().at(i_hash.value().size()-1).column())
						{
							QRect cell_rect = visualRect(index); 
							cell_rect.adjust(0, 0, 1, 1);
							painter.drawLine(cell_rect.bottomRight().x(), cell_rect.bottomRight().y(), cell_rect.topRight().x(), cell_rect.topRight().y());
						}						
					}
				}
			}
		}
		if (i_hash.key().contains(tr("外侧框线")))
		{
			QPen pen(Qt::black, 1);
			painter.setPen(pen);
			if (clear_frames.isEmpty())
			{
				rect.adjust(0, 0, -1, -1);
				painter.drawRect(rect);
			}
			else
			{
				foreach (QModelIndex index, i_hash.value())
				{
					if (!clear_frames.contains(index))
					{
						if (index.row() == i_hash.value().at(0).row())
						{
							QRect cell_rect = visualRect(index); 
							cell_rect.adjust(-1, -1, 0, 0);
							painter.drawLine(cell_rect.topLeft().x(), cell_rect.topLeft().y(), cell_rect.topRight().x(), cell_rect.topRight().y());
						}
						if (index.column() == i_hash.value().at(0).column())
						{
							QRect cell_rect = visualRect(index); 
							cell_rect.adjust(-1, -1, 0, 0);
							painter.drawLine(cell_rect.topLeft().x(), cell_rect.topLeft().y(), cell_rect.bottomLeft().x(), cell_rect.bottomLeft().y());
						}
						if (index.row() == i_hash.value().at(i_hash.value().size()-1).row())
						{
							QRect cell_rect = visualRect(index); 
							cell_rect.adjust(0, 0, 1, 1);
							painter.drawLine(cell_rect.bottomLeft().x(), cell_rect.bottomLeft().y(), cell_rect.bottomRight().x(), cell_rect.bottomRight().y());
						}
						if (index.column() == i_hash.value().at(i_hash.value().size()-1).column())
						{
							QRect cell_rect = visualRect(index); 
							cell_rect.adjust(0, 0, 1, 1);
							painter.drawLine(cell_rect.bottomRight().x(), cell_rect.bottomRight().y(), cell_rect.topRight().x(), cell_rect.topRight().y());
						}						
					}
				}
			}
		}
		if (i_hash.key().contains(tr("所有框线")))
		{
			QPen pen(Qt::black, 1);
			painter.setPen(pen);
			drawPartCellsFrameLine(i_hash.value(), clear_frames, painter);
		}
		if (i_hash.key().contains(tr("左框线")))
		{
			QPen pen(Qt::black, 1);
			painter.setPen(pen);
			if (clear_frames.isEmpty())
				painter.drawLine(rect.topLeft().x(), rect.topLeft().y(), rect.topLeft().x(), rect.bottomLeft().y());
			else
			{
				foreach (QModelIndex index, i_hash.value())
				{
					if (!clear_frames.contains(index) && index.column() == i_hash.value().at(0).column())
					{
						QRect cell_rect = visualRect(index); 
						painter.drawLine(cell_rect.topLeft().x(), cell_rect.topLeft().y(), cell_rect.bottomLeft().x(), cell_rect.bottomLeft().y());
					}
				}				
			}
		}
		if (i_hash.key().contains(tr("右框线")))
		{
			QPen pen(Qt::black, 1);
			painter.setPen(pen);
			if (clear_frames.isEmpty())
				painter.drawLine(rect.topRight().x(), rect.topRight().y(), rect.bottomRight().x(), rect.bottomRight().y());
			else
			{
				foreach (QModelIndex index, i_hash.value())
				{
					if (!clear_frames.contains(index) && index.column() == i_hash.value().at(i_hash.value().size()-1).column())
					{
						QRect cell_rect = visualRect(index); 
						painter.drawLine(cell_rect.topRight().x(), cell_rect.topRight().y(), cell_rect.bottomRight().x(), cell_rect.bottomRight().y());
					}
				}				
			}
		}
		if (i_hash.key().contains(tr("上框线")))
		{
			QPen pen(Qt::black, 1);
			painter.setPen(pen);
			if (clear_frames.isEmpty())
				painter.drawLine(rect.topLeft().x(), rect.topLeft().y(), rect.topRight().x(), rect.topRight().y());
			else
			{
				foreach (QModelIndex index, i_hash.value())
				{
					if (!clear_frames.contains(index) && index.row() == i_hash.value().at(0).row())
					{
						QRect cell_rect = visualRect(index); 
						painter.drawLine(cell_rect.topLeft().x(), cell_rect.topLeft().y(), cell_rect.topRight().x(), cell_rect.topRight().y());
					}
				}				
			}
		}
		if (i_hash.key().contains(tr("下框线")))
		{
			QPen pen(Qt::black, 1);
			painter.setPen(pen);
			if (clear_frames.isEmpty())
				painter.drawLine(rect.bottomLeft().x(), rect.bottomLeft().y(), rect.bottomRight().x(), rect.bottomRight().y());
			else
			{
				foreach (QModelIndex index, i_hash.value())
				{
					if (!clear_frames.contains(index) && index.row() == i_hash.value().at(i_hash.value().size()-1).row())
					{
						QRect cell_rect = visualRect(index); 
						painter.drawLine(cell_rect.bottomLeft().x(), cell_rect.bottomLeft().y(), cell_rect.bottomRight().x(), cell_rect.bottomRight().y());
					}
				}				
			}
		}
	}
}

void TablesGroup::cellClicked(const QModelIndex & index)
{
	if (edit_state)
	{
		setCurrentIndex(index);
		edit(index);
	}
	zoom_font = model()->data(index, Qt::FontRole).value<QFont>();
	cell_oldText = model()->data(index).toString();
}

void TablesGroup::closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint)
{
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	QString current_str = model()->data(indexAt(editor->geometry().center())).toString();
	if (cell_oldText != current_str)
	{
		QString new_old = current_str+tr("，，。")+cell_oldText;
		QPair<QModelIndex, QString> edit_pair(indexAt(editor->geometry().topLeft()), new_old);
		emit tablegroupCmd(new TableMgmtGroupEditCmd(this, tr("文字编辑"), edit_pair));
	}
	if (!current_str.isEmpty())
	{
		resizeColumnToContents(indexAt(editor->geometry().center()).column());
		resizeTableForNewSizeSetted();
	}
	QTableView::closeEditor(editor, hint);	
}

void TablesGroup::receivePdfFileName(const QString & defined)
{
 	pdf_name = defined; 
}

void TablesGroup::resizeTableForNewSizeSetted()
{
	setFixedWidth(columnViewportPosition(model()->columnCount()-1)+columnWidth(model()->columnCount()-1)+verticalHeader()->width());
	setFixedHeight(rowViewportPosition(model()->rowCount()-1)+rowHeight(model()->rowCount()-1)+horizontalHeader()->height());
	emit groupTableSizeChanged();		
}

void TablesGroup::extractAloneItemForDataes(TableManagement * tree, QStandardItem * alone_item, QStandardItemModel * extract_model)
{
	DataSelectModel * model_tree = qobject_cast<DataSelectModel *>(tree->model());
	if (tree->treeType() == 7)
	{
		extract_model -> setRowCount(1);
		extract_model -> setColumnCount(1);
		QList<QPair<QStandardItem *, QString> > f_list = model_tree->listForManualTableNames().keys();
		for (int i = 0; i < f_list.size(); i++)
		{
			if (f_list.at(i).first == alone_item)
			{
				extract_model -> setData(extract_model->index(0, 0), f_list.at(i).second);
				return;
			}
		}		
	}
	else if (tree->treeType()==6 && alone_item->parent()->text()==tr("工程信息"))
	{
		extractAloneItemForInfoes(tree, alone_item, extract_model);
		return;
	}
	QString engi = model_tree->findEngiItemText(alone_item);
	QString proj_pos = "name="+engi;
	extract_model -> setRowCount(1);
	extract_model -> setColumnCount(1);
	QString from_tbl("projectskeys");	
	QString f_data;	
	QString var;
	if (alone_item->text()==tr("创建时间") || alone_item->text()==tr("设定时间") || alone_item->text()==tr("测试时间"))
	{
		extract_model -> setData(extract_model->index(0, 0), model_tree->timesSavedHash().value(alone_item));
		extract_model -> insertRow(0);
		extract_model -> setData(extract_model->index(0, 0), alone_item->text());
		extract_model -> insertRow(0);
		extract_model -> setData(extract_model->index(0, 0), engi);	
		return;
	}
	else if (alone_item->text()==tr("创建者") || alone_item->text()==tr("设定者"))
		var = engi+tr("，，。")+"constructor";	
	else if (alone_item->text() == tr("测量单位"))
		var = engi+tr("，，。")+"unit";
	else if (alone_item->text() == tr("测量精度"))
		var = engi+tr("，，。")+"precision";
	else if (alone_item->text() == tr("控制图"))
		var = engi+tr("，，。")+"ctrlplot";
	else if (alone_item->text() == tr("目标上限"))
		var = engi+tr("，，。")+"uplimit";
	else if (alone_item->text() == tr("目标下限"))
		var = engi+tr("，，。")+"lowlimit";
	else if (alone_item->text() == tr("每组样本"))
		var = engi+tr("，，。")+"grpsamles";
	else if (alone_item->text() == tr("样本组数"))
		var = engi+tr("，，。")+"groups";
	else if (alone_item->text() == tr("目标值"))
		var = engi+tr("，，。")+"desnum";
	else if (alone_item->text() == tr("创建时间"))
		var = engi+tr("，，。")+"constructtime";
	else if (alone_item->text() == tr("测量基础数据"))
	{
		var = engi+tr("，，。")+"dataes";
		from_tbl = "cpkdataes";	
	}
	else if (alone_item->text() == tr("总平均值"))
	{
		var = engi+tr("，，。")+"avr";
		from_tbl = "cpkdataes";
	}
	else if (alone_item->text() == tr("测试时间"))
	{
		var = engi+tr("，，。")+"time";
		from_tbl = "cpkdataes";
	}
	else if (alone_item->text() == tr("数据建立者"))
	{
		var = engi+tr("，，。")+"person";
		from_tbl = "cpkdataes";
	}
	else if (alone_item->text() == tr("离散值"))
	{
		var = engi+tr("，，。")+"dev";
		from_tbl = "cpkdev";
	}
	else if (alone_item->text() == tr("标准差") && alone_item->parent()->text() == tr("西格玛"))
	{
		var = engi+tr("，，。")+"dsigma";
		from_tbl = "cpkdev";
	}
	else if (alone_item->text() == tr("CPK值") && alone_item->parent()->text() == tr("西格玛"))
	{
		var = engi+tr("，，。")+"cpk";
		from_tbl = "cpkdev";
	}
	else if (alone_item->text() == tr("均值上控限") && alone_item->parent()->text() == tr("西格玛"))
	{
		var = engi+tr("，，。")+"upavr";
		from_tbl = "cpkdev";
	}
	else if (alone_item->text() == tr("均值下控限") && alone_item->parent()->text() == tr("西格玛"))
	{
		var = engi+tr("，，。")+"lowavr";
		from_tbl = "cpkdev";
	}
	else if (alone_item->text() == tr("西格玛上控限"))
	{
		var = engi+tr("，，。")+"updev";
		from_tbl = "cpkdev";
	}
	else if (alone_item->text() == tr("西格玛下控限"))
	{
		var = engi+tr("，，。")+"lowdev";
		from_tbl = "cpkdev";
	}
	else if (alone_item->text() == tr("西格玛组状态"))
	{
		var = engi+tr("，，。")+"accept";
		from_tbl = "cpkdev";
	}
	else if (alone_item->text() == tr("极差值"))
	{
		var = engi+tr("，，。")+"rng";
		from_tbl = "cpkrng";
	}
	else if (alone_item->text() == tr("标准差") && alone_item->parent()->text() == tr("极差"))
	{
		var = engi+tr("，，。")+"rsigma";
		from_tbl = "cpkrng";
	}
	else if (alone_item->text() == tr("CPK值") && alone_item->parent()->text() == tr("极差"))
	{
		var = engi+tr("，，。")+"cpk";
		from_tbl = "cpkrng";
	}
	else if (alone_item->text() == tr("均值上控限") && alone_item->parent()->text() == tr("极差"))
	{
		var = engi+tr("，，。")+"upavr";
		from_tbl = "cpkrng";
	}
	else if (alone_item->text() == tr("均值下控限") && alone_item->parent()->text() == tr("极差"))
	{
		var = engi+tr("，，。")+"lowavr";
		from_tbl = "cpkrng";
	}
	else if (alone_item->text() == tr("极差上控限"))
	{
		var = engi+tr("，，。")+"uprng";
		from_tbl = "cpkrng";
	}
	else if (alone_item->text() == tr("极差下控限"))
	{
		var = engi+tr("，，。")+"lowrng";
		from_tbl = "cpkrng";
	}
	else if (alone_item->text() == tr("极差组状态"))
	{
		var = engi+tr("，，。")+"accept";
		from_tbl = "cpkrng";
	}
	model_tree -> feedSimpleDataFromTbl(var, from_tbl, extract_model, alone_item);
	extract_model -> insertRow(0);
	extract_model -> setData(extract_model->index(0, 0), alone_item->text());
	extract_model -> insertRow(0);
	extract_model -> setData(extract_model->index(0, 0), engi);	
}
	
void TablesGroup::extractAloneItemForInfoes(TableManagement * tree, QStandardItem * alone_item, QStandardItemModel * extract_model)
{
	if (alone_item->hasChildren() && alone_item->child(0)->text().contains(tr("无")))
		return;
	DataSelectModel * infoes_model = qobject_cast<DataSelectModel *>(tree->model());
	QList<QStandardItem *> in_list;
	infoes_model -> findItemsSingleLine(alone_item, in_list, infoes_model->item(0)->text());
	QString engi;
	QStandardItem * engi_item = 0;
	if (tree->treeType() == 1)
		engi_item = in_list.at(1);
	else
		engi_item = in_list.at(2);
	if (alone_item->text()==tr("管理产品") || alone_item->text()==tr("属性数据"))
	{
		QStringList table_list = inspector->allTables();
		QSqlTableModel db_sql(this, inspector->currentConnectionDb()->database(inspector->currentConnectionDb()->connectionName()));
		QString tbl_lbl;
		if (alone_item->text() == tr("管理产品"))
			tbl_lbl = "product";
		else
			tbl_lbl = "property";		  
		foreach (QString table, table_list)
		{
			if (table.contains(tbl_lbl) && table.contains(engi_item->text()))
				inspector -> varsModelFromTable(table, &db_sql);
		}
		if (tbl_lbl.isEmpty() || !db_sql.rowCount())
		{
			QStandardItem * click_tail = new QStandardItem(tr("无相关内容"));
			alone_item -> appendRow(click_tail);
			return;
		}
		extract_model -> setRowCount(3);
		extract_model -> setColumnCount(db_sql.columnCount()+1);
		extract_model -> setData(extract_model->index(0, 0), tr("工程名称：")+engi_item->text());
		for (int i = 0; i < db_sql.rowCount(); i++)
		{
			for (int j = 0; j < db_sql.columnCount(); j++)
			{
				if (i == 0)
					extract_model -> setData(extract_model->index(i+1, j), db_sql.headerData(j, Qt::Horizontal));
				extract_model -> setData(extract_model->index(i+2, j), db_sql.data(db_sql.index(i, j)));					
			}				
		}			
	}
	else if (alone_item->text()==tr("管理部门") || alone_item->text()==tr("授权代表") || alone_item->text()==tr("待授权者"))
	{
		QMap<QString, QStringList> projs_infoes = inspector->curDBprojectTable();
		QMap<QString, QStringList> mngs_infoes = inspector->curDBmanageTable();
		if (alone_item->text() == tr("管理部门"))
		{
			QHash<QString, QStringList> dprs_hash;
			QStringList mngs = projs_infoes.value(engi_item->text()).at(2).split(";");
			foreach (QString mng, mngs)
			{
				QStringList mng_pwd = mng.split(",");
				dprs_hash[mngs_infoes.value(mng_pwd.at(1)).at(2)] << mngs_infoes.value(mng_pwd.at(1)).at(4)+tr("，，。")+mng_pwd.at(0);
			}
			extract_model -> setColumnCount(3);
			QHashIterator<QString, QStringList> i_hash(dprs_hash);
			while (i_hash.hasNext())
			{
				i_hash.next();
				extract_model -> insertRows(extract_model->rowCount(), i_hash.value().size());
				foreach (QString mng, i_hash.value())
				{
					QStringList de_list = mng.split(tr("，，。"));
					extract_model -> setData(extract_model->index(extract_model->rowCount()-i_hash.value().size()+i_hash.value().indexOf(mng), 0), i_hash.key());
					extract_model -> setData(extract_model->index(extract_model->rowCount()-i_hash.value().size()+i_hash.value().indexOf(mng), 1), de_list.at(0));
					extract_model -> setData(extract_model->index(extract_model->rowCount()-i_hash.value().size()+i_hash.value().indexOf(mng), 2), de_list.at(1));
				}
			}
		}
		else if (alone_item->text()==tr("授权代表") || alone_item->text()==tr("待授权者"))
		{
			QString related_persons;
			QString first_chk;
			if (alone_item->text() == tr("授权代表"))
			{
				related_persons = projs_infoes.value(engi_item->text()).at(3);
				first_chk = tr("无授权代表");
			}
			else
			{
			 	related_persons = projs_infoes.value(engi_item->text()).at(5); 
				first_chk = tr("无待授权者");
			}
			if (related_persons.isEmpty())
			{
				QStandardItem * click_tail = new QStandardItem(first_chk);
				alone_item -> appendRow(click_tail);			  
				return;
			}
			extract_model -> setColumnCount(3);
			QStringList persons_list = related_persons.split(";");
			foreach (QString person, persons_list)
			{
				QStringList detail_person = person.split(",");
				QString depart;
				QString position;
				QString name(detail_person.at(0));
				if (alone_item->text() == tr("授权代表"))
				{
					depart = mngs_infoes.value(detail_person.at(1)).at(2);
					position = mngs_infoes.value(detail_person.at(1)).at(4);			  
				}
				else
				{
					if (detail_person.size() > 2)
					{
						depart = detail_person.at(2);
						position = detail_person.at(3);
					}
					else
					{
						depart = mngs_infoes.value(detail_person.at(1)).at(2);
						position = mngs_infoes.value(detail_person.at(1)).at(4);
					}
				}
				extract_model -> insertRow(extract_model->rowCount());
				extract_model -> setData(extract_model->index(persons_list.indexOf(person), 0), depart);
				extract_model -> setData(extract_model->index(persons_list.indexOf(person), 1), position);
				extract_model -> setData(extract_model->index(persons_list.indexOf(person), 2), name);				
			}
		}
		extract_model -> insertRow(0);
		extract_model -> setData(extract_model->index(0, 0), tr("工程名称：")+engi_item->text());
	}
	else
	{
		QStringList tbls_list = inspector->allTables();
		QString engi_pos("name="+engi_item->text());
		QStringList stamp_list = inspector->dataFromTable("projectskeys", "unit", engi_pos).toString().split(tr("；"));		
		QMap<QString, QStringList> tests_map;
		foreach (QString str_stamp, stamp_list)
		{
			QString stamp_real = QString("%1").arg(QDateTime::fromString(str_stamp.split(tr("，")).at(0)).toTime_t());
			QStringList cpks_list = tbls_list.filter(stamp_real);
			foreach (QString str_cpk, cpks_list)
			{
				QStringList cpks_reals = str_cpk.split(tr("，，。"));
				if (cpks_reals.back().contains("dailyavr"))
				{
					QStringList daily_ctrs;
					inspector -> getDataesFromTable(str_cpk, "state", daily_ctrs);
					int tol_unnormals = 0;
					foreach (QString str_state, daily_ctrs)
					{
						if (str_state.contains("endun"))
							tol_unnormals++;  
					}
					tests_map[QString(tr("版本%1")).arg(stamp_list.indexOf(str_stamp)+1)].push_back(QString(tr("均值异常点%1个")).arg(tol_unnormals));
				}
				else if (cpks_reals.back().contains("daily") && !cpks_reals.back().contains("dataes"))
				{
					QStringList daily_ctrs;
					inspector -> getDataesFromTable(str_cpk, "state", daily_ctrs);
					QString dev_rng;
					if (cpks_reals.back().contains("dev"))
						dev_rng = tr("西格玛");
					else
						dev_rng = tr("极差");
					int tol_unnormals = 0;
					foreach (QString str_state, daily_ctrs)
					{
						if (str_state.contains("endun"))
							tol_unnormals++;  
					}					
					tests_map[QString(tr("版本%1")).arg(stamp_list.indexOf(str_stamp)+1)].push_back(dev_rng+QString(tr("异常点%1个")).arg(tol_unnormals));					
				}
				else if (cpks_reals.back().contains("cpk") && !cpks_reals.back().contains("dataes"))
				{
					QString val_pos("groupnum=1");
					QString cpk_value = inspector->dataFromTable(str_cpk, "cpk", val_pos).toString();
					QString class_res = algorithm->ensureCpkGrade(cpk_value.toDouble());
					if (class_res == "A+")
						class_res += tr("（cpk>1.67）");
					else if (class_res == "A")
						class_res += tr("（1.67≥cpk>1.33）"); 
					else if (class_res == "B")
						class_res += tr("（1.33≥cpk>1.0）");  
					else if (class_res == "C")
						class_res += tr("（1.0≥cpk>0.67）"); 
					else
						class_res += tr("（cpk≤0.67）");
					tests_map[QString(tr("版本%1")).arg(stamp_list.indexOf(str_stamp)+1)].push_front(cpk_value+" "+class_res);
				}				
			}
		}
		extract_model -> setColumnCount(4);
		QMapIterator<QString, QStringList> i_map(tests_map);
		while (i_map.hasNext())
		{
			i_map.next();
			extract_model -> insertRows(extract_model->rowCount(), 1);
			extract_model -> setData(extract_model->index(extract_model->rowCount()-1, 0), i_map.key());
			foreach (QString despr, i_map.value())				
				extract_model -> setData(extract_model->index(extract_model->rowCount()-1, i_map.value().indexOf(despr)+1), despr);
		}		
		extract_model -> insertRow(0);
		extract_model -> setData(extract_model->index(0, 0), tr("工程名称：")+engi_item->text());		
	}
}

void TablesGroup::extractDataFromItem(TableManagement * tree, QStandardItem * engi_item, QStandardItemModel * extract_model1, QSqlTableModel * extract_model2, QSqlQueryModel * extract_model3)
{
	if (!engi_item->hasChildren())
	{
		if (tree->treeType()<1 || tree->treeType()>5)
			extractAloneItemForDataes(tree, engi_item, extract_model1);
		if (tree->treeType() == 1)
			extractAloneItemForInfoes(tree, engi_item, extract_model1);
		return;
	}
	DataSelectModel * model_tree = qobject_cast<DataSelectModel *>(tree->model());
	QStandardItem * tk_item = model_tree->findTimeParentItem(engi_item);
	QString daily_stamp = QString("%1").arg(QDateTime::fromString(model_tree->timesSavedHash().value(tk_item->child(0)), Qt::ISODate).toTime_t());
	QList<QStandardItem *> selected_line;
	model_tree -> findItemsSingleLine(engi_item, selected_line, tr("日常数据"));
	QString daily_find;
	QString daily_lbl;
	if (selected_line.at(1)->text() == tr("周期测量数据"))
		daily_lbl = "dailydataes";
	else if (selected_line.at(1)->text() == tr("均值"))
		daily_lbl = "dailyavr"; 
	else if (selected_line.at(1)->text() == tr("离散值"))
	 	daily_lbl = "dailydev"; 
	else 
	  	daily_lbl = "dailyrng"; 
	model_tree -> findExistedRelatedTable(daily_stamp, daily_lbl, inspector->allTables(), daily_find);
	QList<QStandardItem *> selected_children;
	model_tree -> findAllChildItems(engi_item, selected_children);
	if (engi_item->parent()->text().contains(tr("受控")) || engi_item->parent()->text().contains(tr("异常")))
	{
		QString normal_state;		
		if (engi_item->parent()->text().contains(tr("受控")))
			normal_state = QString("%")+"endnormal"+QString("%");
		else
			normal_state = QString("%")+"endunormal"+QString("%");	  
		inspector -> dataesByRelatedStringFromDb(daily_find, extract_model3, "state", normal_state);
	}
	else
		inspector -> varsModelFromTable(daily_find, extract_model2);
	int from_group = 0, to_group = 0;
	QStringList inputors;		
	QStringList pswds;	
	if (selected_children.size() > 1)
	{
		QStandardItem * from = 0;
		QStandardItem * to = 0;
		QList<QStandardItem *> persons;
		for (int i = selected_children.size()-1; i > 0; i--)
		{
			if (selected_children.at(i)->text().contains(tr("到：")) && !to)
				to = selected_children.at(i);				  
			if (selected_children.at(i)->text().contains(tr("从：")) && !from)
				from = selected_children.at(i);
			if (selected_children.at(i)->text()==tr("记录人员") && !persons.isEmpty())
				model_tree -> findChildrenNotAll(selected_children.at(i), persons);			  
		}	
		if (from)
		{
			if (from->text().contains(tr("时间")))
			{
				QDateTime from_time = QDateTime::fromString(tree->hashSavedRelatedItemInfo().value(from).at(0), Qt::ISODate);
				from_group = inspector->dataFromTableByTime(from_time, daily_find, "groupnum").toInt()-1;  
			}
			else
				from_group = tree->hashSavedRelatedItemInfo().value(from).at(0).toInt()-1;
		}
		if (to)
		{
			if (to->text().contains(tr("时间")))
			{
				QDateTime to_time = QDateTime::fromString(tree->hashSavedRelatedItemInfo().value(to).at(0), Qt::ISODate);
				to_group = inspector->dataFromTableByTime(to_time, daily_find, "groupnum").toInt()-1;	  
			}
			else
				to_group = tree->hashSavedRelatedItemInfo().value(to).at(0).toInt()-1;
		}
		if (!persons.isEmpty())
		{		  
			if (daily_lbl != "dailydataes")
			{
				QString f_daily;
				model_tree -> findExistedRelatedTable(daily_stamp, "dailydataes", inspector->allTables(), f_daily);
				inspector -> getDataesFromTable(f_daily, "person", inputors);
			}
			else
				inspector -> getDataesFromTable(daily_find, "person", inputors);
			foreach (QStandardItem * p_item, persons)
				pswds << tree->hashSavedRelatedItemInfo().value(p_item).at(0);
		}
		
	}
	else
	{
		if (extract_model2->rowCount())
		{
			if (extract_model2->rowCount()>100)
			{
				to_group = extract_model2->rowCount()-1;
				from_group = extract_model2->rowCount()-100;
			}
			else
			{
				to_group = extract_model2->rowCount()-1;
				from_group = 0;			  
			}
		}
		
		if (extract_model3->rowCount())
		{
			if (extract_model3->rowCount()>100)
			{
				to_group = extract_model3->rowCount()-1;
				from_group = extract_model3->rowCount()-100;
			}
			else
			{
				to_group = extract_model3->rowCount()-1;
				from_group = 0;			  
			}
		}		
	}
	if (extract_model2->rowCount())
	{
		if (extract_model2->headerData(0, Qt::Horizontal).toString() != QString("%1").arg(1))
		{
			QList<QStandardItem *> append_list;
			for (int i = 0; i < extract_model2->columnCount(); i++)
			{
				QString h_transed;
				algorithm -> dbSpcNamesTranslation(extract_model2->headerData(i, Qt::Horizontal).toString(), h_transed);			  
				QStandardItem * add_item = new QStandardItem(h_transed);
				append_list << add_item;
			}
			extract_model1 -> appendRow(append_list);
		} 	  
		for (int i = 0; i < extract_model2->rowCount(); i++)
		{
			if (i < from_group)
				continue;
			if (!pswds.isEmpty() && !pswds.contains(inputors.at(i)))
				continue;
			if (to_group && i>to_group)
				break;				
			QList<QStandardItem *> append_list;
			for (int j = 0; j < extract_model2->columnCount(); j++)
			{
				QStandardItem * add_item = 0;
				if (extract_model2->headerData(j, Qt::Horizontal).toString() == "state")
				{
					QString cell_content = extract_model2->data(extract_model2->index(i, j)).toString();
					if (!cell_content.contains("end"))
						cell_content = tr("未定");
					else if (cell_content.contains("endnormal"))
						 cell_content = tr("正常"); 
					else
					{
						QString tran_str;
						algorithm -> dbSpcNamesTranslation(cell_content, tran_str);
						cell_content = tran_str;
					}
					add_item = new QStandardItem(cell_content);
				}
				else if (extract_model2->headerData(j, Qt::Horizontal).toString() == "person")
				{
					QString cell_person = extract_model2->data(extract_model2->index(i, j)).toString();
					QStringList mng_details = inspector->curDBmanageTable().value(cell_person);
					add_item = new QStandardItem(mng_details.at(2)+mng_details.at(4)+mng_details.at(0));
				}
				else
					add_item = new QStandardItem(extract_model2->data(extract_model2->index(i, j)).toString());
				append_list << add_item;
			}
			extract_model1 -> appendRow(append_list);
		}
		extract_model2 -> clear();						
	}
	if (extract_model3->rowCount())
	{
		if (extract_model3->headerData(0, Qt::Horizontal).toString() != QString("%1").arg(1))
		{
			QList<QStandardItem *> append_list;
			for (int i = 0; i < extract_model3->columnCount(); i++)
			{			  
				QString h_transed;
				algorithm -> dbSpcNamesTranslation(extract_model3->headerData(i, Qt::Horizontal).toString(), h_transed);			  
				QStandardItem * add_item = new QStandardItem(h_transed);
				append_list << add_item;
			}
			extract_model1 -> appendRow(append_list);
		} 	  
		for (int i = 0; i < extract_model3->rowCount(); i++)
		{
			if (i < from_group)
				continue;
			if (!pswds.isEmpty() && !pswds.contains(inputors.at(i)))
				continue;
			if (to_group && i>to_group)
				break;				
			QList<QStandardItem *> append_list;
			for (int j = 0; j < extract_model3->columnCount(); j++)
			{
				QStandardItem * add_item = 0;
				if (extract_model3->headerData(j, Qt::Horizontal).toString() == "state")
				{
					QString cell_content = extract_model3->data(extract_model3->index(i, j)).toString();
					if (!cell_content.contains("end"))
						cell_content = tr("未定");
					else if (cell_content.contains("endnormal"))
						 cell_content = tr("正常"); 
					else
					{
						QString tran_str;
						algorithm -> dbSpcNamesTranslation(cell_content, tran_str);
						cell_content = tran_str;
					}
					add_item = new QStandardItem(cell_content);
				}
				else
					add_item = new QStandardItem(extract_model3->data(extract_model3->index(i, j)).toString());				  
				append_list << add_item;
			}
			extract_model1 -> appendRow(append_list);
		}
		extract_model3 -> clear();
	}	
}

void TablesGroup::ensureFeildForReplace(QModelIndex & leftTop, QStandardItemModel * dataes_model1, QSqlTableModel * dataes_model2)
{
	int add_rows = 0, add_columns = 0;
	if (dataes_model1)
	{
		if (dataes_model1->headerData(0, Qt::Horizontal).toString() != QString("%1").arg(1))
			add_rows = dataes_model1->rowCount()+1; 
		else
			add_rows = dataes_model1->rowCount();  
		add_columns = dataes_model1->columnCount();
	}
	if (dataes_model2)
	{
		if (dataes_model2->headerData(0, Qt::Horizontal).toString() == "column1")
			add_rows = dataes_model2->rowCount();
		else
			add_rows = dataes_model2->rowCount()+1; 
		add_columns = dataes_model2->columnCount();
	}
	if (add_rows+leftTop.row()<=model()->rowCount() && add_columns+leftTop.column()<=model()->columnCount())
		return;
	if (add_rows+leftTop.row() > model()->rowCount())
	{
		int n_insert = add_rows+leftTop.row()-model()->rowCount();
		model() -> insertRows(model()->rowCount(), n_insert);
	}
	if (add_columns+leftTop.column() > model()->columnCount())
	{
		int n_insert = add_columns+leftTop.column()-model()->columnCount();
		model() -> insertColumns(model()->columnCount(), n_insert);
	}
	resizeTableForNewSizeSetted();	
}

void TablesGroup::resizeForUnvisualModel(const QModelIndex & leftTop, const QModelIndex & rightBottom, QStandardItemModel * resize_model, QStandardItemModel * other_model1, QSqlTableModel * other_model2)
{
	int empty_rows = rightBottom.row()-leftTop.row()+1;
	int empty_columns = rightBottom.column()-leftTop.column()+1;
	int add_rows = 0, add_columns = 0;
	if (other_model1)
	{
		add_rows = other_model1->rowCount(); 
		add_columns = other_model1->columnCount();
	}
	if (other_model2)
	{
		add_rows = other_model2->rowCount()+1; 
		add_columns = other_model2->columnCount();
	}
	if (add_rows<empty_rows && add_columns<empty_columns)
		return;
	if (add_rows > empty_rows)
	{
		int n_insert = add_rows-empty_rows;
		resize_model -> insertRows(rightBottom.row()+1, n_insert);
	}
	if (add_columns > empty_columns)
	{
		int n_insert = add_columns-empty_columns;
		resize_model -> insertColumns(rightBottom.column()+1, n_insert);
	}	
}

void TablesGroup::attatchBackupItemDataes(QModelIndex & leftTop, QStandardItemModel * dataes)
{
	ensureFeildForReplace(leftTop, dataes);	
	attachDataesFromOtherModel(leftTop, qobject_cast<QStandardItemModel *>(model()), dataes, 0, 0);					
}

void TablesGroup::dettatchBackupItemDataes(const QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & old_dataes)
{
	QList<QPair<QModelIndex, QString> > pair_list = old_dataes.keys();
	QModelIndexList old_cells;
	for (int i = 0; i < pair_list.size(); i++)
		old_cells << pair_list.at(i).first;
	qSort(old_cells);
	foreach (QModelIndex old_index, old_cells)
	{
		QString span_info;
		for (int i = 0; i < pair_list.size(); i++)
		{
			if (pair_list.at(i).first == old_index)
			{
				span_info = pair_list.at(i).second;
				pair_list.removeOne(pair_list.at(i));
				break;
			}
		}
		QPair<QModelIndex, QString> value_pair(old_index, span_info);
		model() -> setItemData(old_index, old_dataes.value(value_pair).first);
	}
}

void TablesGroup::attachDataesFromOtherModel(const QModelIndex & begin, QStandardItemModel * model, QStandardItemModel * s_model, QSqlTableModel * other1, QSqlQueryModel * other2)//need promotion for back 3 model type selection
{
	bool head_copyed = false;
	int rows = 0, columns = 0;
	int begin_row = begin.row(), begin_column = begin.column();
	if (s_model)
	{
		if (s_model->headerData(0, Qt::Horizontal).toString() != QString("%1").arg(1))
			rows = s_model->rowCount()+1; 
		else
			rows = s_model->rowCount();
		columns = s_model->columnCount();
	}
	else if (other1)
	{
		rows = other1->rowCount();
		columns = other1->columnCount();
	}
	else	
	{
		if (other2->headerData(0, Qt::Horizontal).toString() != QString("%1").arg(1))
			rows = other2->rowCount()+1; 
		else
			rows = other2->rowCount(); 
		columns = other2->columnCount();
	}
	if (!rows)
		return;
	QString text;		
	for(int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			if (s_model)
			{
				if (s_model->headerData(j, Qt::Horizontal).toString() != QString("%1").arg(j+1))
				{
					if (!head_copyed)
					{
						if (j==columns-1)
							head_copyed = true;
						algorithm -> dbSpcNamesTranslation(s_model->headerData(j, Qt::Horizontal).toString(), text);
						model -> setData(model->index(i+begin_row, j+begin_column), text);
					}				
					if (j==columns-1)	
					{
						QString ch_str = s_model->data(s_model->index(i, j)).toString();
						QString changed_str = ch_str.split(",").at(0);
						model -> setData(model->index(i+begin_row+1, j+begin_column), changed_str);						
					}
					else			
						model -> setData(model->index(i+begin_row+1, j+begin_column), s_model->data(s_model->index(i, j)));
				}
				else
					model -> setData(model->index(i+begin_row, j+begin_column), s_model->data(s_model->index(i, j)));
			}
			else if (other1)
			{
				if (!head_copyed)
				{
					if (j==columns-1)
						head_copyed = true;
					algorithm -> dbSpcNamesTranslation(other1->headerData(j, Qt::Horizontal).toString(), text);
					if (text==tr("数据") || text=="groupnum")
					{
						begin_column -= 1;
						continue;
					}
					model -> setData(model->index(i+begin_row, j+begin_column), text);
				}
				model -> setData(model->index(i+begin_row+1, j+begin_column), other1->data(other1->index(i, j)));
			}
			else
			{
				if (other2->headerData(j, Qt::Horizontal).toString() != QString("%1").arg(j+1))
				{
					if (!head_copyed)
					{
						if (j==columns-1)
							head_copyed = true;
						algorithm -> dbSpcNamesTranslation(other2->headerData(j, Qt::Horizontal).toString(), text);
						model -> setData(model->index(i+begin_row, j+begin_column), text);
					}				
					if (j==columns-1)	
					{
						QString ch_str = other2->data(other2->index(i, j)).toString();
						QString changed_str = ch_str.split(",").at(0);
						model -> setData(model->index(i+begin_row+1, j+begin_column), changed_str);						
					}
					else			
						model -> setData(model->index(i+begin_row+1, j+begin_column), other2->data(other2->index(i, j)));
				}
				else
					model -> setData(model->index(i+begin_row, j+begin_column), other2->data(other2->index(i, j)));
			}
		}
	}	
}

void TablesGroup::changingIndexesMatrics(const QModelIndexList & new_selected, QPair<int, int> & rows_columns)
{
	int rows = new_selected.at(new_selected.size()-1).row()-new_selected.at(0).row()+1;
	int columns = new_selected.at(new_selected.size()-1).column()-new_selected.at(0).column()+1;
	rows_columns.first = columns;
	rows_columns.second = rows;	 
}

void TablesGroup::drawPartCellsFrameLine(const QModelIndexList & draw_list, const QModelIndexList & undraw_list, QPainter & painter)
{
	foreach (QModelIndex cir_index, draw_list)
	{
		if (!undraw_list.contains(cir_index))
		{
			QModelIndex up_index = model()->index(cir_index.row()-1, cir_index.column());
			QModelIndex down_index = model()->index(cir_index.row()+1, cir_index.column());
			QModelIndex left_index = model()->index(cir_index.row(), cir_index.column()-1);
			QModelIndex right_index = model()->index(cir_index.row(), cir_index.column()+1);
			if (!undraw_list.contains(up_index))
				painter.drawLine(visualRect(cir_index).adjusted(-1, -1, 1, 1).topLeft(), visualRect(cir_index).adjusted(-1, -1, 1, 1).topRight());
			if (!undraw_list.contains(left_index))
				painter.drawLine(visualRect(cir_index).adjusted(-1, -1, 1, 1).topLeft(), visualRect(cir_index).adjusted(-1, -1, 1, 1).bottomLeft());
			if (!undraw_list.contains(right_index))	
				painter.drawLine(visualRect(cir_index).adjusted(-1, -1, 1, 1).topRight(), visualRect(cir_index).adjusted(-1, -1, 1, 1).bottomRight());
			if (!undraw_list.contains(down_index))
				painter.drawLine(visualRect(cir_index).adjusted(-1, -1, 1, 1).bottomLeft(), visualRect(cir_index).adjusted(-1, -1, 1, 1).bottomRight());
		}		
	}
}

void TablesGroup::copyCurrentModelToSaveModel(QStandardItemModel * copyed_model)
{
	QStringList frame_keys = actived_cells.keys();
	QList<QRect> span_rects;
	for (int i = 0; i < model()->rowCount(); i++)
	{
		for (int j = 0; j < model()->columnCount(); j++)
		{
			QString save_cell;
			if (!model()->itemData(model()->index(i, j)).isEmpty())
			{
				QMap<int, QVariant> contents_map = model()->itemData(model()->index(i, j));
				QMapIterator<int, QVariant> i_map(contents_map);
				while (i_map.hasNext())
				{
					i_map.next();
					QString con;
					if (i_map.key() == 0)
						con = "0^~";
					else if (i_map.key() == 1)
						con = "1^~";
					else if (i_map.key() == 6)
						con = "6^~";
					else if (i_map.key() == 7)
						con = "7^~";
					else if (i_map.key() == 8)
						con = "8^~";
					else if (i_map.key() == 9)
						con = "9^~";
					else if (i_map.key() == 13)
						con = "13^~";
					else 
						continue;
					con += i_map.value().toString()+"^`^";
					save_cell += con;					
				}
			}
			QString line_style;
			foreach (QString str, frame_keys)
			{
				if (actived_cells.value(str).contains(model()->index(i, j)) && model()->index(i, j)==actived_cells.value(str).at(0))
				{
					if (line_style.isEmpty())
						line_style = "@^^@";
					line_style += "@#"+str;
					line_style += "^~"+QString("%1,%2").arg(actived_cells.value(str).at(actived_cells.value(str).size()-1).row()).arg(actived_cells.value(str).at(actived_cells.value(str).size()-1).column());
				}
			}
			QString span_str;
			if (columnSpan(model()->index(i, j).row(), model()->index(i, j).column())>1 || rowSpan(model()->index(i, j).row(), model()->index(i, j).column())>1)
			{
				QRect index_rect = visualRect(model()->index(i, j));
				if (!span_rects.contains(index_rect))
				{
					if (span_str.isEmpty())
						span_str = "#~~#";
					span_rects << index_rect;
					int row_span = rowSpan(model()->index(i, j).row(), model()->index(i, j).column());
					int col_span = columnSpan(model()->index(i, j).row(), model()->index(i, j).column());
					span_str += QString("%1,%2").arg(row_span).arg(col_span);
				}
			}
			if (!line_style.isEmpty())
				save_cell += line_style;
			if (!span_str.isEmpty())
				save_cell += span_str;			
			copyed_model->setData(copyed_model->index(i, j), save_cell);
		}
	}
}

void TablesGroup::translateCellContent(const QString & cell_str, QMap<int, QVariant> & content_map, QStringList & frame_list, QStringList & span_list)
{
	QString trans_cell = cell_str;
	QStringList str_list;
	if (trans_cell.contains("#~~#"))
	{
		str_list = trans_cell.split("#~~#");
		span_list = str_list[1].split(",");
		trans_cell = str_list[0];
	}
	if (trans_cell.contains("@^^@"))
	{
		str_list = trans_cell.split("@^^@");
		QString l_style = str_list[1];
		QStringList line_list = l_style.split("@#");
		line_list.pop_front();
		foreach (QString line, line_list)
			frame_list << line;
		if (!str_list[0].isEmpty())
			trans_cell = str_list[0];
	}
	if (trans_cell.contains("^`^"))
	{
		str_list = trans_cell.split("^`^");
		str_list.pop_back();
		foreach (QString con, str_list)
		{
			QStringList each_con = con.split("^~");
			if (each_con[0]=="1" || each_con[0]=="8" || each_con[0]=="9")	
			{
				QColor color(each_con[1]);
				content_map.insert(each_con[0].toInt(), color);				
			}
			else if (each_con[0] == "6")
			{
				QFont font(each_con[1]);
				content_map.insert(each_con[0].toInt(), font);				
			}
			else if (each_con[0] == "13")
			{
				QStringList size_list = each_con[1].split(",");
				QSize size(size_list[0].toInt(), size_list[1].toInt());
				content_map.insert(each_con[0].toInt(), size);				
			}
			else
			{
				if (!each_con.isEmpty() && each_con.size()==2)
					content_map.insert(each_con[0].toInt(), each_con[1]);
			}
		}
	}	
}

void TablesGroup::translateIndexesToList(const QModelIndex & leftTop, const QModelIndex & rightBottom, QModelIndexList & dest_list)
{
	dest_list << leftTop;
	for (int i = leftTop.column(); i < rightBottom.column()+1; i++)
	{
		for (int j = leftTop.row(); j < rightBottom.row()+1; j++)
		{
			if (i == leftTop.row() && j == leftTop.column())
				continue;
			QModelIndex i_index = model()->index(j, i);
			dest_list << i_index;
		}
	}	
}

void TablesGroup::checkEmptyModelArea(const QModelIndex & leftTop, const QModelIndex & rightBottom, QModelIndexList & dest_list)
{
	int empty_row = 0, empty_col = 0;
	for (int i = leftTop.row(); i < rightBottom.row()+1; i++)
	{
		for (int j = leftTop.column(); j < rightBottom.column()+1; j++)
		{
			if (model()->data(model()->index(i, j)).toString().isEmpty())
				dest_list << model()->index(i, j);
			else
			{
				if (!empty_row)
					empty_row = i;
				if (!empty_col)
					empty_col = j;					
			}
		}
	}
	if (!empty_row && !empty_col)
		return;
	if (empty_row)
		empty_row -= 1;
	if (empty_col)
		empty_col -= 1;
	QModelIndex nested_index = model()->index(empty_row, empty_col);
	if (!dest_list.isEmpty())
	{
		foreach (QModelIndex f_index, dest_list)
		{
			if (nested_index < f_index)
				dest_list.removeOne(f_index);
		}
	}
}

void TablesGroup::attachDbManualTblOnModel(QStandardItem * info_item, QModelIndex from_index, QStandardItemModel * copyed_model, QStandardItemModel * copy_model)
{
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(info_item->model());
	QString tbl_name(copyed_model->data(copyed_model->index(0, 0)).toString()); 
	QPair<QStandardItem *, QString> key_pair(info_item, tbl_name);
	QSqlTableModel * sql_model = t_model->listForManualTableNames().value(key_pair);
	QString all_contents = sql_model->data(sql_model->index(0, 0)).toString();
  	QString all_rcs = sql_model->data(sql_model->index(0, 1)).toString();
	QString all_frames = sql_model->data(sql_model->index(0, 2)).toString();
	QStringList first_split = all_rcs.split(";");
	QStringList second_split = all_contents.split(tr("，，～；"));
	QStringList rows_split = first_split[0].split(",");
	if (from_index.row()+rows_split.size()-1 > copy_model->rowCount())
		copy_model -> insertRows(copy_model->rowCount(), from_index.row()+rows_split.size()-1-copy_model->rowCount());
	QStringList cols_split = first_split[1].split(",");	
	if (from_index.column()+cols_split.size()-1 > copy_model->columnCount())
		copy_model -> insertColumns(copy_model->columnCount(), from_index.column()+cols_split.size()-1-copy_model->columnCount());	
	for (int i = 0; i < rows_split.size()-1; i++)
	{
		setRowHeight(from_index.row()+i, rows_split[i].toInt());
		for (int j = 0; j < cols_split.size()-1; j++)
		{
			if (i == 0)
				setColumnWidth(from_index.column()+j, cols_split[j].toInt());
			QString save_pos = "*"+QString("%1").arg(i)+","+QString("%1").arg(j)+"*";
			QStringList existed_list = second_split.filter(save_pos);
			if (!existed_list.isEmpty())
			{
				QStringList detailed_split = existed_list[0].split(tr("，，～，"));
				foreach (QString cell_str, detailed_split)
				{
					if (cell_str.contains("*"))
						continue;
					if (cell_str.contains("$&"))
					{
						QStringList span_split = cell_str.split("$&");					  
						if (span_split[0] != "1" || span_split[1] != "1")
							setSpan(from_index.row()+i, from_index.column()+j, span_split[0].toInt(), span_split[1].toInt());
					}					
					if (cell_str.contains(tr("，^，")))
					{
						QMap<int, QVariant> c_dataes;
						QStringList detailed_contents = detailed_split[1].split(tr("，^；"));
						foreach (QString m_str, detailed_contents)
						{
							if (m_str.isEmpty())
								break;
							QStringList m_values = m_str.split(tr("，^，"));
							if (m_values.at(1).contains(tr("单元图")))
							{
								QPixmap f_picture = inspector->foundPictureFromDb("pictures", m_values.at(1));
								c_dataes[m_values[0].toInt()] = f_picture;								
							}
							else
							{
								if (m_values[0].toInt() == Qt::FontRole)
								{
									QFont font_role(m_values[1]);
									c_dataes.insert(m_values[0].toInt(), font_role);
								}
								else if (m_values[0].toInt() == Qt::TextAlignmentRole)
									c_dataes.insert(m_values[0].toInt(), m_values[1].toInt());
								else if ((m_values[0].toInt()==Qt::BackgroundRole && !m_values[1].isEmpty()) || m_values[0].toInt()==Qt::ForegroundRole)
								{
									QColor color(m_values[1]);
									QBrush bfg_role(color);
									c_dataes.insert(m_values[0].toInt(), bfg_role);
								}
								else
									c_dataes.insert(m_values[0].toInt(), m_values[1]);								
							}
						}						
						model() -> setItemData(model()->index(from_index.row()+i, from_index.column()+j), c_dataes);
					}				  
				}
			}
		}
	}
	QStringList frame_split = all_frames.split(tr("）；"));
	foreach (QString f_str, frame_split)
	{
		if (f_str.isEmpty())
			break;
		QStringList next_split = f_str.split(tr("，（"));
		QStringList third_split = next_split[1].split(";");
		QModelIndexList index_list;
		foreach (QString t_str, third_split)
		{
			if (t_str.isEmpty())
				break;
			QStringList model_rc = t_str.split(",");
			QModelIndex e_index = model()->index(from_index.row()+model_rc[0].toInt(), from_index.column()+model_rc[1].toInt());
			index_list << e_index;
		}
		actived_cells.insert(next_split[0], index_list);
	}
	repaint();	
	copyed_model -> setData(copyed_model->index(0, 0), copyed_model->data(copyed_model->index(0, 0)).toString()+tr("拷贝完成"));
}

void TablesGroup::paintTableForPrintPdf(bool painting)
{
	print_pdf = painting;
	repaint();
}

void TablesGroup::initPdfPaperDiagram(const QString & type)
{
	pdf_frame = new PrintFrame(this);
	pdf_frame -> initPaperFrame(type); 
}

bool TablesGroup::clearContentsSelection(const QString & order, QHash<QModelIndex, QPair<QMap<int, QVariant>, QStandardItem *> > & cells)
{
	if (selectedIndexes().isEmpty())
		return false;
	if (order == tr("清空内容"))
	{
		foreach (QModelIndex index, selectedIndexes())
		{
			QPair<QMap<int, QVariant>, QStandardItem *> contents_pair;
			contents_pair.first = model()->itemData(index);
			if (indexWidget(index))
				contents_pair.second = qobject_cast<PlotWidget *>(indexWidget(index))->singlePlotInfo();
			cells.insert(index, contents_pair);
		}		
	}
	else
	{
		QModelIndexList match_list;
		QHashIterator<QString, QModelIndexList> i_hash(actived_cells);
		while (i_hash.hasNext()) 
		{
			i_hash.next();
			match_list += i_hash.value();
		}
		foreach (QModelIndex index, selectedIndexes())
		{
			if (match_list.contains(index))
			{
				QPair<QMap<int, QVariant>, QStandardItem *> contents_pair(QMap<int, QVariant>(), 0);
				cells.insert(index, contents_pair);
			}
		}		
	}
	return true;	
}

bool TablesGroup::cutCopyCollection(const QString & hint, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > & oper_hash)
{
	if (selectedIndexes().isEmpty())
		return false;
	foreach (QModelIndex index, selectedIndexes())
	{
		QString span_info = QString("%1").arg(rowSpan(index.row(), index.column()))+","+QString("%1").arg(columnSpan(index.row(), index.column()));
		if (hint == tr("复制"))
			span_info += ";"+hint;
		QPair<QModelIndex, QString> key_pair(index, span_info);
		QPair<QMap<int, QVariant>, QStandardItem *> in_pair;		
		if (qobject_cast<PlotWidget *>(indexWidget(index)))
			in_pair.second = qobject_cast<PlotWidget *>(indexWidget(index))->singlePlotInfo();
		else
			in_pair.first = model()->itemData(index);		
		oper_hash.insert(key_pair, in_pair);
	}
	content_hashes = oper_hash;		
	return true;	
}

bool TablesGroup::initPasteHash(QPair<QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> >, QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > > & initting)
{
	if (selectedIndexes().isEmpty() || content_hashes.isEmpty())
		return false;
	QList<QPair<QModelIndex, QString> > pair_list = content_hashes.keys();
	QModelIndexList pasting;
	for (int i = 0; i < pair_list.size(); i++)
		pasting << pair_list.at(i).first;
	qSort(pasting);
	QModelIndex from_index = selectedIndexes().at(0);
	int d_row = pasting.at(0).row()- from_index.row();
	int d_col = pasting.at(0).column()- from_index.column();
	QModelIndex to_index = model()->index(pasting.back().row()-d_row, pasting.back().column()-d_col);
	QHash<QPair<QModelIndex, QString>, QPair<QMap<int, QVariant>, QStandardItem *> > old_hash;
	for (int i = from_index.row(); i < to_index.row()+1; i++)
	{
		for (int j = from_index.column(); j < to_index.column()+1; j++)
		{
			QModelIndex valid_index = model()->index(i, j);
			if (!isIndexHidden(valid_index))
			{
				QString span_info = QString("%1").arg(rowSpan(valid_index.row(), valid_index.column()))+","+QString("%1").arg(columnSpan(valid_index.row(), valid_index.column()));
				QPair<QModelIndex, QString> key_pair(valid_index, span_info);
				QPair<QMap<int, QVariant>, QStandardItem *> in_pair;		
				if (qobject_cast<PlotWidget *>(indexWidget(valid_index)))
					in_pair.second = qobject_cast<PlotWidget *>(indexWidget(valid_index))->singlePlotInfo();
				else
					in_pair.first = model()->itemData(valid_index);
				old_hash.insert(key_pair, in_pair);				
			}
		}
	}
	initting.first = content_hashes;
	initting.second = old_hash;
	return true;
}

bool TablesGroup::cellsFrameInfoCollection(QPair<QString, QModelIndexList> & collection_pair)
{
	if (selectedIndexes().isEmpty() || isEqualValueInHash(collection_pair.first, selectedIndexes()))
		return false;
	collection_pair.first += tr("，")+QString("%1").arg(actived_cells.size()+1);
	collection_pair.second = selectedIndexes();
	actived_cells.insert(collection_pair.first, selectedIndexes());
	return true;	
}

bool TablesGroup::textAlignInfoesCollection(int flag, QHash<QModelIndex, QPair<QString, QStandardItem *> > & align_hash)
{
	if (selectedIndexes().isEmpty())
		return false;
	int i = 0;
	foreach (QModelIndex index, selectedIndexes())
	{
		if (!model()->data(index).isNull()) 
		{
			int item_align = qobject_cast<QStandardItemModel *>(model())->item(index.row(), index.column())->textAlignment();
			if (item_align != flag)
			{
				QString bf_flags = QString("%1").arg(item_align)+tr("，")+QString("%1").arg(flag);
				QPair<QString, QStandardItem *> pair(bf_flags, 0);
				align_hash.insert(index, pair);
			}
			else
				i++;
		}
		else
			i++;
	}
	if (selectedIndexes().size() == i)
		return false;
	return true;			
}

bool TablesGroup::mergeCellsInfoCollection(QHash<QModelIndex, QPair<QString, QStandardItem *> > & merge_infoes, bool merge_split)
{
	if (selectedIndexes().isEmpty())
		return false;
	if (merge_split)
	{
		if (selectedIndexes().size()==1)
			return false;
	}
	else
	{
		QModelIndex single = selectedIndexes().at(0);
		if (selectedIndexes().size()==1 && rowSpan(single.row(), single.column())==1 && columnSpan(single.row(), single.column())==1)
			return false;				
	}
	foreach (QModelIndex index, selectedIndexes())
	{
		QString span_info = QString("%1").arg(rowSpan(index.row(), index.column()))+","+QString("%1").arg(columnSpan(index.row(), index.column()));
		QPair<QString, QStandardItem *> str_plot;
		str_plot.first = span_info;
		if (indexWidget(index))
		{
			QWidget * t_widget = indexWidget(index);
			if (qobject_cast<PlotWidget *>(t_widget))
			{
				QStandardItem * item = qobject_cast<PlotWidget *>(t_widget)->singlePlotInfo();
				str_plot.second = item;			  
			}
			else
				str_plot.first += "Label";
		}
		merge_infoes.insert(index, str_plot);
	}
	return true;	
}

bool TablesGroup::rmInsIndexListCollection(const QString & key_text, QMap<QModelIndex, QString> & sel_map)
{
	if (selectedIndexes().isEmpty())
		return false;
	QModelIndexList selected_list;
	if (key_text == tr("插入行"))
		sel_map.insert(selectedIndexes().at(0), QString());
	else if (key_text == tr("插入列"))
		sel_map.insert(selectedIndexes().at(0), QString());
	else if (key_text == tr("删除行"))
	{
		QModelIndex match_index;
		foreach (QModelIndex index, selectedIndexes())
		{
			if (match_index.row() != index.row())
			{
				match_index = index;
				selected_list << index;
			}
		}
		for (int i = selected_list.at(0).row(); i < selected_list.back().row()+1; i++)
		{
			for (int j = 0; j < model()->columnCount(); j++)
			{
				QMap<int, QVariant> item_index = model()->itemData(model()->index(i, j));
				QString map_info;
				QMapIterator<int, QVariant> i_map(item_index);
				while (i_map.hasNext()) 
				{
					i_map.next();
					map_info += QString("%1").arg(i_map.key())+tr("，，。")+i_map.value().toString()+tr("；，。");
				}
				sel_map.insert(model()->index(i, j), map_info);				
			}
		}
	}
	else
	{
		QModelIndex match_index;
		foreach (QModelIndex index, selectedIndexes())
		{
			if (match_index.column() != index.column())
			{
				match_index = index;
				selected_list << index;
			}
		} 
		for (int i = selected_list.at(0).column(); i < selected_list.back().column()+1; i++)
		{
			for (int j = 0; j < model()->rowCount(); j++)
			{
				QMap<int, QVariant> item_index = model()->itemData(model()->index(j, i));
				QString map_info;
				QMapIterator<int, QVariant> i_map(item_index);
				while (i_map.hasNext()) 
				{
					i_map.next();
					map_info += QString("%1").arg(i_map.key())+tr("，，。")+i_map.value().toString()+tr("；，。");
				}
				sel_map.insert(model()->index(j, i), map_info);				
			}
		}
	}
	return true;	
}

bool TablesGroup::modelContainNewContents(const QModelIndex & leftTop, QStandardItemModel * dataes_model1, QSqlTableModel * dataes_model2)
{
  	int add_rows = 0, add_columns = 0;
	if (dataes_model1)
	{
		if (dataes_model1->headerData(0, Qt::Horizontal).toString() != QString("%1").arg(1))
			add_rows = dataes_model1->rowCount()+1; 
		else
			add_rows = dataes_model1->rowCount();  
		add_columns = dataes_model1->columnCount();
	}
	if (dataes_model2)
	{
		if (dataes_model2->headerData(0, Qt::Horizontal).toString() == "column1")
			add_rows = dataes_model2->rowCount();
		else
			add_rows = dataes_model2->rowCount()+1; 
		add_columns = dataes_model2->columnCount();
	}
	if (add_rows+leftTop.row()>model()->rowCount() || add_columns+leftTop.column()>model()->columnCount())
		return false;
	return true;
}

bool TablesGroup::oldColorCollection(const QString & new_color, QHash<QModelIndex, QPair<QString, QStandardItem *> > & color_hash, bool ft_bgd)
{
	QModelIndexList selected_list = selectedIndexes();
	if (selected_list.isEmpty())
		return false;
	bool found = false;	
	if (ft_bgd)	
	{
		foreach (QModelIndex index, selected_list)
		{
			QMap<int, QVariant> item_index = model()->itemData(index);
			if (item_index.value(Qt::ForegroundRole)!=QVariant(QColor(new_color)))
			{
				QString c_str = new_color+tr("，")+item_index.value(Qt::ForegroundRole).toString();
				QPair<QString, QStandardItem *> color_item(c_str, 0);
				color_hash.insert(index, color_item);
				found = true;
			}
		}		
	}
	else
	{
		foreach (QModelIndex index, selected_list)
		{
			QMap<int, QVariant> item_index = model()->itemData(index);
			if (item_index.value(Qt::BackgroundColorRole)!=QVariant(QColor(new_color)))
			{
				QString c_str;
				if (item_index.value(Qt::BackgroundColorRole).toString().isEmpty())
					c_str = new_color+tr("，")+"White";
				else
					c_str = new_color+tr("，")+item_index.value(Qt::BackgroundColorRole).toString();
				QPair<QString, QStandardItem *> color_item(c_str, 0);
				color_hash.insert(index, color_item);
				found = true;
			}
		}
	}
	return found;
}

bool TablesGroup::oldSizesCollection(int new_width, int new_height, QHash<QModelIndex, QPair<int, int> > & info_hash)
{
	QModelIndexList selected_list = selectedIndexes();
	if (selected_list.isEmpty())
		return false;
	QModelIndex match_index;
	bool found = false;
	if (new_width)
	{
		foreach (QModelIndex index, selected_list)
		{
			if (match_index.column()!=index.column() && columnWidth(index.column())!=new_width)
			{
				match_index = index;
				QPair<int, int> value_pair(new_width, columnWidth(index.column()));
				info_hash.insert(index, value_pair);
				found = true;
			}
		}		
	}
	else
	{
		foreach (QModelIndex index, selected_list)
		{
			if (match_index.row()!=index.row() && rowHeight(index.row())!=new_height)
			{
				match_index = index;
				QPair<int, int> value_pair(new_height, rowHeight(index.row()));
				info_hash.insert(index, value_pair);
				found = true;
			}
		}
	}
	return found;
}

bool TablesGroup::isEqualValueInHash(const QString & key_text, const QModelIndexList & value_list)
{
	if (actived_cells.isEmpty())
		return false;
	QHashIterator<QString, QModelIndexList> i_hash(actived_cells);
	while (i_hash.hasNext()) 
	{
		i_hash.next();
		if (i_hash.key().contains(key_text) && i_hash.value()==value_list)
			return true;
	}
	return false;	
}

QStandardItem * TablesGroup::findPosItemForNewPlot(QStandardItem * parent, const QModelIndex & pos_index)
{
	QString h_pos(QString("%1").arg(pos_index.row()+1));
	QString v_pos(QString("%1").arg(pos_index.column()+1));
	QString pos_str = tr("视图位置：行")+h_pos+tr("，")+tr("列")+v_pos;
	QStandardItem * item_child = 0;
	int i = 0;
	while (parent->child(i))
	{
		if (parent->child(i)->text() == pos_str)
		{
			item_child = parent->child(i);
			break;
		}
		i++;
	}
	return item_child;
}
