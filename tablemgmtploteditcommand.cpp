#include <QtCore>
#include <QtGui>
#include "tablemgmtploteditcommand.h"
#include "tablemanagement.h"
#include "tablesgroup.h"
#include "littlewidgetsview.h"
#include "littlewidgetitem.h"
#include "plotwidget.h"
#include "matrixwidget.h"
#include "dataselectmodel.h"

TableMgmtPlotEditCmd::TableMgmtPlotEditCmd(TableManagement * tree_obj, QStandardItem * selected_item, const QString & order_text, LittleWidgetsView * qml_obj)
: QUndoCommand(), child_row(0), add_pos(0), show_ptr(0), covered_ptr(0)
{
	boss1 = tree_obj;
	parent_item = selected_item;
	orderText = order_text;
	boss2 = qml_obj;		
}

TableMgmtPlotEditCmd::TableMgmtPlotEditCmd(TableManagement * tree_obj, QStandardItem * selected_item, QStandardItem * new_item, const QString & order_text, LittleWidgetsView * qml_obj, PlotWidget * show_plot, PlotWidget * covered_plot)
: QUndoCommand(), child_row(0), add_pos(0)
{
	boss1 = tree_obj;
	parent_item = selected_item;
	child_item = new_item;
	orderText = order_text;
	boss2 = qml_obj;
	show_ptr = show_plot;
	covered_ptr = covered_plot;
}

TableMgmtPlotEditCmd::TableMgmtPlotEditCmd(const QString & order_text, TableManagement * tree_obj, LittleWidgetsView * qml_obj, int old_rc, int new_rc, bool row_col, const QHash<QPair<int, int>, int> & hide_widgets, const QList<PlotWidget *> & hide_plots)
: QUndoCommand(), child_row(0), add_pos(0), show_ptr(0), covered_ptr(0)
{
	rowcol_change = row_col;
	rc_old = old_rc;
	rc_new = new_rc;
	orderText = order_text;
	undo_mwps = hide_widgets;
	undo_plots = hide_plots;
	foreach (PlotWidget * plot, hide_plots)
	{
		QStandardItem * plot_item = plot->singlePlotInfo();
		QPair<QStandardItem *, int> pair_info(plot_item->parent(), plot_item->row());
		inmodel_oldpos.insert(plot_item, pair_info);
	}	
	boss1 = tree_obj;
	boss2 = qml_obj;
}

TableMgmtPlotEditCmd::~TableMgmtPlotEditCmd()
{}

void TableMgmtPlotEditCmd::redo()
{
	if (orderText == QObject::tr("替换"))
	{		
		if (!child_row)
			child_row = child_item->row();			
		boss1 -> setExpanded(parent_item->index(), false);		
		parent_item->takeRow(child_row);
		QString info_text = child_item->child(child_item->rowCount()-1)->text();
		QStringList info_list = info_text.split(QObject::tr("："));
		QStringList row_col = info_list.at(1).split(QObject::tr("，"));
		QPair<int, int> pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);	
		MatrixWidget * defaults = 0;
		if (covered_ptr->width() > covered_ptr->endRect().width())
			defaults = boss2->qmlAgent()->generateDefault(covered_ptr->rect(), covered_ptr->endRect());
		else
			defaults = boss2->qmlAgent()->generateDefault(covered_ptr->rect());		
		boss2->qmlAgent()->coverShowPlot(show_ptr, covered_ptr);	
		boss1 -> setExpanded(parent_item->index(), true);		
		if (!boss1->keepDeletingItems().contains(child_item))
			boss1->keepDeletingItems() << child_item;
		boss2->qmlAgent()->setDefaultWidget(pos, defaults);		
		boss2->qmlAgent()->allCurrentPlots().remove(pos);
		if (!boss2->qmlAgent()->unshowPlotsMatrixWidgets().contains(covered_ptr))
			boss2->qmlAgent()->unshowPlotsMatrixWidgets() << covered_ptr;							
	}
	else if (orderText == QObject::tr("清除选择"))
	{
		if (undo_items.isEmpty())
		{
			DataSelectModel * t_model = qobject_cast<DataSelectModel *>(parent_item->model());
			t_model -> findChildrenNotAll(parent_item, undo_items);		  
		}
		boss1 -> setExpanded(parent_item->index(), false);
		foreach (QStandardItem * item, undo_items)
		{
			parent_item->takeRow(item->row());
			if (!boss1->keepDeletingItems().contains(item))
				boss1->keepDeletingItems() << item;
		}	
		boss1 -> setExpanded(parent_item->index(), true);
		boss2->qmlAgent()->unreClearGeneratedPlots(undo_items, undo_plots, true);
		foreach (PlotWidget * plot, undo_plots)
		{
			if (!boss2->qmlAgent()->unshowPlotsMatrixWidgets().contains(plot))
				boss2->qmlAgent()->unshowPlotsMatrixWidgets() << plot;				
		}
	}
	else if (orderText == QObject::tr("重新选择"))
	{
		if (undo_items.isEmpty())
		{
			DataSelectModel * t_model = qobject_cast<DataSelectModel *>(parent_item->model());
			t_model -> findChildrenNotAll(parent_item->child(0), undo_items);
		}
		boss1 -> setExpanded(parent_item->child(0)->index(), false);
		boss1 -> removeItemChildrenInHash(parent_item, names_texts);
		boss2->qmlAgent()->unreClearGeneratedPlot(parent_item, covered_ptr, true);		
		foreach (QStandardItem * item, undo_items)
		{
			parent_item->child(0)->takeRow(item->row());
			if (!boss1->keepDeletingItems().contains(item))
				boss1->keepDeletingItems() << item;
		}		
		if (covered_ptr && !boss2->qmlAgent()->unshowPlotsMatrixWidgets().contains(covered_ptr))
			boss2->qmlAgent()->unshowPlotsMatrixWidgets() << covered_ptr;					
//		child_item = parent_item->takeRow(1).at(0);
		boss1 -> setExpanded(parent_item->child(0)->index(), true);	
	}
	else if (orderText == QObject::tr("图阵更改"))
	{
		boss2->qmlAgent()->unredoForPlotsMatrix(rc_old, rc_new, rowcol_change, undo_mwps, undo_plots);		
		if (boss1 && !undo_plots.isEmpty())
			boss1 -> unredoItemsForMatrixChange(undo_plots, inmodel_oldpos, true);
	}
	else if (orderText.contains(QObject::tr("移动位置")))
	{
		if (!add_pos && covered_ptr)
			add_pos = covered_ptr->singlePlotInfo()->parent();
		QString info_text = parent_item->text();
		QStringList info_list = info_text.split(QObject::tr("："));
		QStringList row_col = info_list.at(1).split(QObject::tr("，"));
		QPair<int, int> pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);
		parent_item -> setText(child_item->text());		  
		boss2->qmlAgent()->coverShowPlot(show_ptr, covered_ptr);	
		if (!boss2->qmlAgent()->allCurrentDefaults().contains(pos))
		{
			MatrixWidget * defaults = 0;
			if (show_ptr->width() > show_ptr->endRect().width())
				defaults = boss2->qmlAgent()->generateDefault(show_ptr->rect(), show_ptr->endRect());
			else
				defaults = boss2->qmlAgent()->generateDefault(show_ptr->rect());
			boss2->qmlAgent()->setDefaultWidget(pos, defaults);
		}		
		boss2->qmlAgent()->allCurrentPlots().remove(pos);		
		info_list = child_item->text().split(QObject::tr("："));
		row_col = info_list.at(1).split(QObject::tr("，"));
		QPair<int, int> r_pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);		
		if (add_pos)
		{
			add_pos -> takeRow(covered_ptr->singlePlotInfo()->row());					
			if (!boss2->qmlAgent()->unshowPlotsMatrixWidgets().contains(covered_ptr))
				boss2->qmlAgent()->unshowPlotsMatrixWidgets() << covered_ptr;	
		}
		else
		{
			delete boss2->qmlAgent()->allCurrentDefaults().value(r_pos);
			boss2->qmlAgent()->allCurrentDefaults().remove(r_pos);				
		}
		boss2->qmlAgent()->allCurrentPlots()[r_pos] = show_ptr;
		child_item -> setText(info_text);		
		if (!boss1->keepDeletingItems().contains(child_item))
			boss1->keepDeletingItems() << child_item;			
	}
	else if (orderText.contains(QObject::tr("列合并")))
	{
		QStringList find_list = orderText.split(",");
		boss2->qmlAgent()->unredoMatrixRowMerge(rc_old, rc_new, find_list.at(1).toInt(), false, undo_mwps, undo_plots);
		if (boss1 && !undo_plots.isEmpty())
			boss1 -> unredoItemsForMatrixChange(undo_plots, inmodel_oldpos, true);
	}
	else if (orderText.contains(QObject::tr("拆分列")))
	{
		int off_to = parent_item->columnCount();
		int from = child_item->columnCount();
		if (!boss1->keepDeletingItems().contains(parent_item))
			boss1->keepDeletingItems() << parent_item;	
		if (!boss1->keepDeletingItems().contains(child_item))
			boss1->keepDeletingItems() << child_item;		
		int to = from+off_to-1;
		int row = child_item->rowCount();
		boss2->qmlAgent()->unredoMatrixRowSplit(from, to, row, false);		
	}			
	else
	{
		boss1 -> setExpanded(parent_item->index(), false);
		parent_item -> appendRow(child_item);
		child_row = child_item->row();
		if (orderText.contains(QObject::tr("生成图")))
		{
			QString info_text = child_item->text();
			QStringList info_list = info_text.split(QObject::tr("："));
			QStringList row_col = info_list.at(1).split(QObject::tr("，"));
			QPair<int, int> pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);	
			boss2->qmlAgent()->coverShowPlot(show_ptr, covered_ptr);			
			if (covered_ptr)
			{
				covered_ptr -> hide();		
				if (!boss2->qmlAgent()->unshowPlotsMatrixWidgets().contains(covered_ptr))
					boss2->qmlAgent()->unshowPlotsMatrixWidgets() << covered_ptr;
			}
			else
			{
				delete boss2->qmlAgent()->allCurrentDefaults()[pos];
				boss2->qmlAgent()->allCurrentDefaults().remove(pos);
			}		  
			boss2->qmlAgent()->allCurrentPlots()[pos] = show_ptr;			
		}		
		boss1 -> setExpanded(parent_item->index(), true);
		if (boss1->keepDeletingItems().contains(child_item))
			boss1->keepDeletingItems().removeOne(child_item);			
	}
}

void TableMgmtPlotEditCmd::undo()
{
	if (orderText == QObject::tr("替换"))
	{
		boss1 -> setExpanded(parent_item->index(), false);	  
		boss2->qmlAgent()->coverShowPlot(covered_ptr, show_ptr);		
		parent_item -> insertRow(child_row, child_item);
		boss1 -> setExpanded(parent_item->index(), true);		
		if (boss1->keepDeletingItems().contains(child_item))
			boss1->keepDeletingItems().removeOne(child_item);
		QString info_text = child_item->child(child_item->rowCount()-1)->text();
		QStringList info_list = info_text.split(QObject::tr("："));
		QStringList row_col = info_list.at(1).split(QObject::tr("，"));
		QPair<int, int> pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);	
		delete boss2->qmlAgent()->allCurrentDefaults().value(pos);
		boss2->qmlAgent()->allCurrentDefaults().remove(pos);			
		boss2->qmlAgent()->allCurrentPlots().insert(pos, covered_ptr);
		if (boss2->qmlAgent()->unshowPlotsMatrixWidgets().contains(covered_ptr))
			boss2->qmlAgent()->unshowPlotsMatrixWidgets().removeOne(covered_ptr);		
	}
	else if (orderText == QObject::tr("清除选择"))
	{
		boss1 -> setExpanded(parent_item->index(), false);
		foreach (QStandardItem * item, undo_items)
		{
			parent_item->appendRow(item);
			if (boss1->keepDeletingItems().contains(item))
				boss1->keepDeletingItems().removeOne(item);			
		}
		boss1 -> setExpanded(parent_item->index(), true);
		boss2->qmlAgent()->unreClearGeneratedPlots(undo_items, undo_plots, false);
		foreach (PlotWidget * plot, undo_plots)
		{
			if (boss2->qmlAgent()->unshowPlotsMatrixWidgets().contains(plot))
				boss2->qmlAgent()->unshowPlotsMatrixWidgets().removeOne(plot);				
		}		
	}
	else if (orderText == QObject::tr("重新选择"))
	{
		boss1 -> setExpanded(parent_item->child(0)->index(), false);
		foreach (QStandardItem * item, undo_items)
		{
			parent_item->child(0)->appendRow(item);
			if (boss1->keepDeletingItems().contains(item))
				boss1->keepDeletingItems().removeOne(item);			
		}
		boss1 -> setExpanded(parent_item->child(0)->index(), true);		
		boss1 -> reinsertItemChildrenInHash(names_texts);
		if (covered_ptr)
		{
			boss2->qmlAgent()->unreClearGeneratedPlot(parent_item, covered_ptr, false);
			if (boss2->qmlAgent()->unshowPlotsMatrixWidgets().contains(covered_ptr))
				boss2->qmlAgent()->unshowPlotsMatrixWidgets().removeOne(covered_ptr);					
		}
	}
	else if (orderText == QObject::tr("图阵更改"))
	{
		boss2->qmlAgent()->unredoForPlotsMatrix(rc_new, rc_old, rowcol_change, undo_mwps, undo_plots);
		if (boss1 && !undo_plots.isEmpty())
			boss1 -> unredoItemsForMatrixChange(undo_plots, inmodel_oldpos, false);
	}
	else if (orderText.contains(QObject::tr("移动位置")))
	{
		QString info_text = child_item->text();
		child_item -> setText(parent_item->text());			
		parent_item -> setText(info_text);
		if (add_pos)
			add_pos -> appendRow(covered_ptr->singlePlotInfo());
		boss2->qmlAgent()->coverShowPlot(show_ptr, 0);			
		QStringList info_list = info_text.split(QObject::tr("："));
		QStringList row_col = info_list.at(1).split(QObject::tr("，"));
		QPair<int, int> pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);	
		boss2->qmlAgent()->allCurrentPlots()[pos] = show_ptr;	
		delete boss2->qmlAgent()->allCurrentDefaults().value(pos);
		boss2->qmlAgent()->allCurrentDefaults().remove(pos);		
		info_text = child_item->text();
		info_list = info_text.split(QObject::tr("："));
		row_col = info_list.at(1).split(QObject::tr("，"));	
		QPair<int, int> re_pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);		
		if (covered_ptr)
		{
			boss2->qmlAgent()->coverShowPlot(covered_ptr, 0);
			boss2->qmlAgent()->allCurrentPlots().insert(re_pos, covered_ptr);	
		}
		else
		{
			MatrixWidget * defaults = 0;
			if (show_ptr->width() > show_ptr->endRect().width())
				defaults = boss2->qmlAgent()->generateDefault(show_ptr->rect(), show_ptr->endRect());
			else
				defaults = boss2->qmlAgent()->generateDefault(show_ptr->rect());
			boss2->qmlAgent()->setDefaultWidget(re_pos, defaults);			
			boss2->qmlAgent()->allCurrentPlots().remove(re_pos);			
		}		
		if (boss2->qmlAgent()->unshowPlotsMatrixWidgets().contains(covered_ptr))
			boss2->qmlAgent()->unshowPlotsMatrixWidgets().removeOne(covered_ptr);
		if (boss1->keepDeletingItems().contains(child_item) && covered_ptr)
			boss1->keepDeletingItems().removeOne(child_item);			
	}
	else if (orderText.contains(QObject::tr("列合并")))
	{
		QStringList find_list = orderText.split(",");
		boss2->qmlAgent()->unredoMatrixRowMerge(rc_old, rc_new, find_list.at(1).toInt(), true, undo_mwps, undo_plots);
		if (boss1 && !undo_plots.isEmpty())
			boss1 -> unredoItemsForMatrixChange(undo_plots, inmodel_oldpos, false);
	}
	else if (orderText.contains(QObject::tr("拆分列")))
	{
		int off_to = parent_item->columnCount();
		int from = child_item->columnCount();
		int to = from+off_to-1;
		int row = child_item->rowCount();	
		boss2->qmlAgent()->unredoMatrixRowSplit(from, to, row, true);
	}	
	else
	{
		boss1 -> setExpanded(parent_item->index(), false);		
		if (orderText.contains(QObject::tr("生成图")))
		{
			boss2->qmlAgent()->coverShowPlot(covered_ptr, show_ptr);
			QString info_text = child_item->text();
			QStringList info_list = info_text.split(QObject::tr("："));
			QStringList row_col = info_list.at(1).split(QObject::tr("，"));
			QPair<int, int> pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);			
			if (covered_ptr)
			{
				boss2->qmlAgent()->coverShowPlot(covered_ptr, 0);	
				boss2->qmlAgent()->allCurrentPlots()[pos] = covered_ptr;				
				if (boss2->qmlAgent()->unshowPlotsMatrixWidgets().contains(covered_ptr))
					boss2->qmlAgent()->unshowPlotsMatrixWidgets().removeOne(covered_ptr);
			}		
			else
			{			
				boss2->qmlAgent()->allCurrentPlots().remove(pos);
				MatrixWidget * defaults = 0;
				if (show_ptr->rect().width() > show_ptr->endRect().width())
					defaults = boss2->qmlAgent()->generateDefault(show_ptr->rect(), show_ptr->endRect());
				else
					defaults = boss2->qmlAgent()->generateDefault(show_ptr->rect());
				boss2->qmlAgent()->setDefaultWidget(pos, defaults);			
			}	
		}
		parent_item->takeRow(child_row);
		if (!boss1->keepDeletingItems().contains(child_item))
			boss1->keepDeletingItems() << child_item;		
		boss1 -> setExpanded(parent_item->index(), true);	
	}	
}
