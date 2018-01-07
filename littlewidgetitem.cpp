#include <QtGui>
#include <QtCore>
#include <QtDeclarative>
#include <QtSql/QSqlTableModel>
#include "spcdatabase.h"
#include "spcnum.h"
#include "littlewidgetitem.h"
#include "littlewidgetsview.h"
#include "tablemanagement.h"
#include "dataselectmodel.h"
#include "plotwidget.h"
#include "matrixwidget.h"
#include "mainwindow.h"
#include "dialog.h"

LittleWidgetItem::LittleWidgetItem(QDeclarativeItem * parent)
:QGraphicsProxyWidget(parent), cells_layout(0), widget_container(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setFlag(QGraphicsItem::ItemIsFocusable,true);
}

LittleWidgetItem::~LittleWidgetItem()
{
	if (!plot_widgets.isEmpty())
	{
		QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
		while (p_map.hasNext())
		{
			p_map.next();			
			p_map.value() -> close();
		}
	}
	if (!d_widgets.isEmpty())
	{
		QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
		while (d_map.hasNext())
		{
			d_map.next();			
			d_map.value() -> close();
			d_map.value() -> deleteLater();
		}
	}
	if (!unshow_widgets.isEmpty())
	{
		foreach (PlotWidget * plot, unshow_widgets)
			plot -> deleteLater();		
	}
	if (widget_container)
		widget_container -> deleteLater();
}

void LittleWidgetItem::setWidgetLayoutPt(QGridLayout * w_layout, QWidget * container)
{
	cells_layout = w_layout;
	widget_container = container;
}

void LittleWidgetItem::setDefaultWidget(const QPair<int, int> & pos, MatrixWidget * w_default)
{
	int span = qRound(w_default->rect().width()/w_default->originSize().width());
	cells_layout -> addWidget(w_default, pos.first, pos.second, 1, span);	
	d_widgets.insert(pos, w_default);
}

void LittleWidgetItem::zoomOutPlots()
{
	if (widget_container->width() < showing_rect.width())
		return;
	QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
	while (d_map.hasNext())
	{
		d_map.next();
		qreal d_height = d_map.value()->height()*0.9;
		qreal d_width = d_map.value()->width()*0.9;
		if (d_map.value()->width()/d_map.value()->originSize().width() > 1)
		{
			int span = qRound(d_map.value()->width()/d_map.value()->originSize().width());
			d_width = d_map.value()->originSize().width()*0.9*span+cells_layout->spacing()*(span-1);
		}
		d_map.value() -> setFixedWidth(d_width);
		d_map.value() -> setFixedHeight(d_height);		
		d_map.value() -> resetOrigRectSize(d_map.value()->originSize().width()*0.9, d_height);
		d_map.value() -> show();
	}
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		double p_height = p_map.value()->height()*0.9; 
		double p_width = p_map.value()->width()*0.9;
		if (p_map.value()->width()/p_map.value()->endRect().width() > 1)
		{
			int span = qRound(p_map.value()->width()/p_map.value()->endRect().width());
			p_width = p_map.value()->endRect().width()*span*0.9+cells_layout->spacing()*(span-1);
		}		
		p_map.value() -> setFixedWidth(p_width);
		p_map.value() -> setFixedHeight(p_height);
		p_map.value() -> resetEndRect(p_map.value()->endRect().width()*0.9, p_height);
		p_map.value() -> updateCloseBtnPos();
		p_map.value() -> resetRelatedPlotVars(p_map.value()->width()/showing_rect.width()*0.9);
		p_map.value() -> show();		
	}
	widget_container -> setFixedSize(cells_layout->sizeHint());
}
	
void LittleWidgetItem::zoomInPlots()
{
// resize, update... funcs not change size right now (only setFixed... size)
	QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
	while (d_map.hasNext())
	{
		d_map.next();
		qreal d_height = d_map.value()->height()*1.1;
		qreal d_width = d_map.value()->width()*1.1;		
		if (d_map.value()->width()/d_map.value()->originSize().width() > 1)
		{
			int span = qRound(d_map.value()->width()/d_map.value()->originSize().width());
			d_width = d_map.value()->originSize().width()*1.1*span+cells_layout->spacing()*(span-1);
		}
		d_map.value() -> setFixedWidth(d_width);
		d_map.value() -> setFixedHeight(d_height);		
		d_map.value() -> resetOrigRectSize(d_map.value()->originSize().width()*1.1, d_height);
		d_map.value() -> show();
	}
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		qreal p_height = p_map.value()->height()*1.1;
		qreal p_width = p_map.value()->width()*1.1;
		if (p_map.value()->width()/p_map.value()->endRect().width() > 1)
		{
			int span = qRound(p_map.value()->width()/p_map.value()->endRect().width());
			p_width = p_map.value()->endRect().width()*span*1.1+cells_layout->spacing()*(span-1);
		}
		p_map.value() -> setFixedWidth(p_width);
		p_map.value() -> setFixedHeight(p_height);
		p_map.value() -> resetEndRect(p_map.value()->endRect().width()*1.1, p_height);
		p_map.value() -> updateCloseBtnPos();
		p_map.value() -> resetRelatedPlotVars(p_map.value()->width()/showing_rect.width()*1.1);
		p_map.value() -> show();		
	}
	widget_container -> setFixedSize(cells_layout->sizeHint());
}

void LittleWidgetItem::setCellSize(int n_width, int n_height)
{
	QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
	while (d_map.hasNext())
	{
		d_map.next();
		if (n_width)
		{
			if (d_map.value()->width() != d_map.value()->originSize().width()*d_map.value()->height()/d_map.value()->originSize().height())
			{
				int span = qFloor(d_map.value()->width()/(d_map.value()->originSize().width()*d_map.value()->height()/d_map.value()->originSize().height()));
				d_map.value() -> setFixedWidth(n_width*span+cells_layout->spacing()*(span-1));
			}
			else			
				d_map.value() -> setFixedWidth(n_width);
			d_map.value() -> resetOrigRectSize(n_width, 0);
		}
		else
		{
			d_map.value() -> setFixedHeight(n_height);
			d_map.value() -> resetOrigRectSize(0, n_height);
		}
		d_map.value() -> show();
	}
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		if (n_width)
		{			
			if (p_map.value()->width() != p_map.value()->endRect().width()*p_map.value()->height()/p_map.value()->endRect().height())
			{
				int span = qFloor(p_map.value()->width()/(p_map.value()->endRect().width()*p_map.value()->height()/p_map.value()->endRect().height()));
				p_map.value() -> setFixedWidth(n_width*span+cells_layout->spacing()*(span-1));
			}
			else			
				p_map.value() -> setFixedWidth(n_width);
			p_map.value() -> resetEndRect(n_width, 0);
		}
		else
		{
			p_map.value() -> setFixedHeight(n_height);	
			p_map.value() -> resetEndRect(0, n_height);
		}		
		p_map.value() -> show();
		p_map.value() -> updateCloseBtnPos();
		p_map.value() -> resetRelatedPlotVars(p_map.value()->width()/showing_rect.width());
	}
	widget_container -> setFixedSize(cells_layout->sizeHint());	
}

void LittleWidgetItem::findPlotsOnRowsColumns(int rows, int columns, QList<PlotWidget *> & push_plots, QHash<QPair<int, int>, int> & push_widgets, QString & over_limit)
{
	QPair<int, int> real_pair = realRowsColsInGridLayout();
	int cur_rows = real_pair.first;
	int cur_columns = real_pair.second;
	if ((rows && rows-1>=cur_rows) || (columns && columns-1>=cur_columns))
		return;
	if (rows)
	{
		for (int i = rows; i < cur_rows+1; i++)
		{
			for (int j = 0; j < cur_columns+1; j++)
			{
				QPair<int, int> find_pos(i, j);
				if (plot_widgets.contains(find_pos))
					push_plots << plot_widgets.value(find_pos);
				if (d_widgets.contains(find_pos))
				{
					int span = qRound(d_widgets.value(find_pos)->rect().width()/d_widgets.value(find_pos)->originSize().width());
					if (span > 1)
						push_widgets[find_pos] = span;
				}			
			}
		}
	}
	else
	{
		for (int i = 0; i < cur_rows+1; i++)
		{
			for (int j = columns; j < cur_columns+1; j++)
			{
				QPair<int, int> find_pos(i, j);
				if (j==columns && !d_widgets.contains(find_pos) && !plot_widgets.contains(find_pos))
				{
					over_limit = tr("单元越界");
					return;
				}
				if (plot_widgets.contains(find_pos))
					push_plots << plot_widgets.value(find_pos);
				if (d_widgets.contains(find_pos))
				{
					int span = qRound(d_widgets.value(find_pos)->rect().width()/d_widgets.value(find_pos)->originSize().width());
					if (span > 1)
						push_widgets[find_pos] = span;
				}	
			}
		}
	}	
}

void LittleWidgetItem::unredoForPlotsMatrix(int old_rc, int new_rc, bool row_col, QHash<QPair<int, int>, int> & unre_widgets, QList<PlotWidget *> & unre_plots)
{
	QPair<int, int> real_pair = realRowsColsInGridLayout();
	int changing_num = 0;
	if (row_col)
		changing_num = real_pair.second;
	else
		changing_num = real_pair.first;
	if (old_rc > new_rc)
		deleteCellsOnGridLayout(old_rc, new_rc, changing_num+1, row_col);
	if (old_rc < new_rc)
		createCellsOnGridLayout(old_rc+1, new_rc+1, changing_num+1, row_col, unre_widgets, unre_plots);
	widget_container -> setFixedSize(cells_layout->sizeHint());
	emit matrixChanging();
}

void LittleWidgetItem::rearrangeForDelDefault(QObject * d_del)//need it?
{
	QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
	while (d_map.hasNext())
	{
		d_map.next();
		if (d_map.value() == d_del)
		{
			d_widgets.remove(d_map.key());
			break;
		}
	}
}

void LittleWidgetItem::horverPlotsCounts(QPair<int, int> & row_col)
{
	row_col.first = cells_layout->rowCount(); 
	row_col.second = cells_layout->columnCount();
}

void LittleWidgetItem::coverShowPlot(PlotWidget * showing, PlotWidget * covered)
{
	QStandardItem * plot_item;
	if (covered)
		plot_item = covered->singlePlotInfo();
	else
		plot_item = showing->singlePlotInfo();	
	QStandardItem * pos_info = plot_item->child(plot_item->rowCount()-1);
	QString info_text = pos_info->text();
	QStringList info_list = info_text.split(tr("："));
	QStringList row_col = info_list[1].split(tr("，"));
	QPair<int, int> pos(row_col[0].toInt()-1, row_col[1].toInt()-1);	
	if (showing && covered)
	{
		covered -> hide();
		int span = qRound(showing->rect().width()/showing->endRect().width());		
		cells_layout -> addWidget(showing, pos.first, pos.second, 1, span);
		showing -> show();		
	}
	else
	{
		QRect base_rect;
		if (!d_widgets.isEmpty())
			base_rect = d_widgets.values().at(0)->originSize();
		else
			base_rect = plot_widgets.values().at(0)->endRect();
		if (showing)
		{
			int span = qRound(showing->rect().width()/showing->endRect().width());			
			if (!showing->exsitedBtn())
			{
				QPushButton * btn_close = new QPushButton(showing);
				showing -> setCloseBtnPtr(btn_close);
				connect(btn_close, SIGNAL(destroyed(QObject *)), this, SLOT(arrangePlotForBtnSig(QObject *)));				
			}
			showing -> setParent(0);	
			showing -> setFixedWidth(base_rect.width()*span+cells_layout->spacing()*(span-1));
			cells_layout -> addWidget(showing, pos.first, pos.second, 1, span);
			showing -> updateCloseBtnPos();
			showing -> show();			
		}
		else 		  
			covered -> hide();
	}
	chkAndCleanNoWidgetItems(pos.first);	
}

void LittleWidgetItem::setNewRect(const QRectF & rect)
{
	showing_rect = rect;
}

void LittleWidgetItem::strOrderForPlotFromUsr(const QString & order, const QString & detail)
{
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		if (p_map.value()->hasColorPad())
			p_map.value() -> plotShowDetailChanging(order, detail);
	}
}

void LittleWidgetItem::newPlotwidgetToCoverOld(QStandardItem * cov_item, QPair<PlotWidget *, PlotWidget *> & new_old, QPair<int, int> & cover_pos, SpcNum * back_spc, SpcDataBase * base_db, LittleWidgetsView * p_view)
{
	new_old.second = plot_widgets.value(cover_pos);
	QRect ref_rect = new_old.second->rect();
	QRect base_rect = new_old.second->endRect();
	PlotWidget * new_plot = new PlotWidget(back_spc, base_db);
	connect(new_plot, SIGNAL(enableQmlBtnsForShowContents(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)), this, SIGNAL(refreshPlotShowQmlBtns(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)));
	if (ref_rect.width() != base_rect.width())
		new_plot -> generateSingle(cov_item, 0, ref_rect, p_view, this, 0, base_rect);
	else
		new_plot -> generateSingle(cov_item, 0, base_rect, p_view, this, 0);
	new_plot -> resetRelatedPlotVars(ref_rect.width()/showing_rect.width());
	new_old.first = new_plot;
}

void LittleWidgetItem::enterWhichWidget(const QPointF & enter_pos, QPair<int, int> & which)
{
	QPointF pt_trans(mapFromScene(enter_pos));
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		QRect enter_rect = cells_layout->itemAt(cells_layout->indexOf(p_map.value()))->geometry();		
		if (pt_trans.x() > enter_rect.left()-2.5 && pt_trans.x() < enter_rect.right()+2.5 && pt_trans.y() < enter_rect.bottom()+2.5 && pt_trans.y() > enter_rect.top()-2.5)
		{
			which = p_map.key();
			return;
		}
	}	
	QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
	while (d_map.hasNext())
	{
		d_map.next();
		QRect enter_rect = cells_layout->itemAt(cells_layout->indexOf(d_map.value()))->geometry();
		if (pt_trans.x() > enter_rect.left()-2.5 && pt_trans.x() < enter_rect.right()+2.5 && pt_trans.y() < enter_rect.bottom()+2.5 && pt_trans.y() > enter_rect.top()-2.5)
		{
			which = d_map.key();
			return;
		}
	}	
}

void LittleWidgetItem::unredoMatrixRowMerge(int col_from, int col_to, int row, bool un_redo, const QHash<QPair<int, int>, int> & unre_mws, const QList<PlotWidget *> & unre_plots) 
{
	QRect base_rect;
	QRect show_rect;
	QWidget * cur_widget = findRowColFrontWidget(row, col_from);
	PlotWidget * p_widget = qobject_cast<PlotWidget *>(cur_widget);
	MatrixWidget * m_widget = qobject_cast<MatrixWidget *>(cur_widget);
	if (p_widget)
	{
		base_rect = p_widget->endRect();
		show_rect = p_widget->geometry();
	}
	else
	{
		base_rect = m_widget->originSize();
		show_rect = m_widget->geometry();		
	}
	QPair<int, int> cur_pos = findShowingPos(cur_widget);		
	QList<PlotWidget *> plots_list = unre_plots;	
	if (un_redo)
	{
		if (p_widget)
		{
			p_widget -> setFixedWidth(base_rect.width()*(col_from-cur_pos.second)+cells_layout->spacing()*(col_from-cur_pos.second-1));
			p_widget -> updateCloseBtnPos();
			cells_layout -> addWidget(p_widget, row, cur_pos.second, 1, col_from-cur_pos.second);
		}
		else
		{	  
			m_widget -> setFixedWidth(base_rect.width()*(col_from-cur_pos.second)+cells_layout->spacing()*(col_from-cur_pos.second-1));
			cells_layout -> addWidget(m_widget, row, cur_pos.second, 1, col_from-cur_pos.second);
		}
		foreach (PlotWidget * plot, plots_list)
		{
			QString pos_info = plot->singlePlotInfo()->child(plot->singlePlotInfo()->rowCount()-1)->text();
			QStringList info_list = pos_info.split(tr("："));
			QStringList row_col = info_list[1].split(tr("，"));
			QPair<int, int> pos(row_col[0].toInt()-1, row_col[1].toInt()-1);
			int p_span = qRound(plot->rect().width()/base_rect.width());	
			cells_layout -> addWidget(plot, pos.first, pos.second, 1, p_span);
			plot -> show();
			if (unshow_widgets.contains(plot))
				unshow_widgets.removeOne(plot);			
			plot_widgets.insert(pos, plot);				
		} 
		QHashIterator<QPair<int, int>, int> mws_hash(unre_mws); 
		while (mws_hash.hasNext())
		{
			mws_hash.next();
			QRect n_rect(0, 0, base_rect.width()*mws_hash.value()+cells_layout->spacing()*(mws_hash.value()-1), base_rect.height());
			MatrixWidget * new_mw = generateDefault(n_rect, base_rect);
			cells_layout -> addWidget(new_mw, mws_hash.key().first, mws_hash.key().second, 1, mws_hash.value());
			new_mw -> show();
			d_widgets.insert(mws_hash.key(), new_mw);
		}
		for (int i = col_from; i < col_to+1; i++)
		{
			if (!cells_layout->itemAtPosition(row, i))
			{
				QPair<int, int> pair(row, i);
				MatrixWidget * new_mw = generateDefault(base_rect);
				cells_layout -> addWidget(new_mw, row, i);
				new_mw -> show();
				d_widgets.insert(pair, new_mw);
			}
		}		
	}
	else
	{	
		for (int i = col_from; i < col_to+1; i++)
		{
			QPair<int, int> pair(row, i);		  
			if (d_widgets.contains(pair))
			{
				delete d_widgets.value(pair);
				d_widgets.remove(pair);			
			}
		}		
		foreach (PlotWidget * plot, plots_list)
		{
			QString info_text = plot->singlePlotInfo()->child(plot->singlePlotInfo()->rowCount()-1)->text();
			QStringList info_list = info_text.split(QObject::tr("："));
			QStringList row_col = info_list[1].split(QObject::tr("，"));
			QPair<int, int> pos(row_col[0].toInt()-1, row_col[1].toInt()-1);			  			  
			plot -> hide();
			if (!unshow_widgets.contains(plot))
				unshow_widgets << plot;
			if (plot_widgets.contains(pos))
				plot_widgets.remove(pos);		
		}
		int span = col_to-cur_pos.second+1; 		
		if (p_widget)
		{
			p_widget -> updateCloseBtnPos();
			cells_layout -> addWidget(p_widget, row, cur_pos.second, 1, span);
			p_widget -> setFixedWidth(base_rect.width()*span+cells_layout->spacing()*(span-1));
			p_widget -> updateCloseBtnPos();
		}
		else
		{
			cells_layout -> addWidget(m_widget, row, cur_pos.second, 1, span);
			m_widget -> setFixedWidth(base_rect.width()*span+cells_layout->spacing()*(span-1));			
		}
	}
	chkAndCleanNoWidgetItems(row);	
}

void LittleWidgetItem::unredoMatrixRowSplit(int col_from, int col_to, int row, bool un_redo)
{
	QRect base_rect;
	QPair<int, int> base_pair(row, col_from);
	PlotWidget * plot_split = 0;
	MatrixWidget * mw_split = 0;
	if (d_widgets.contains(base_pair))
	{
		mw_split = d_widgets.value(base_pair);
		base_rect = mw_split->originSize();
	}
	else
	{
		plot_split = plot_widgets.value(base_pair);
		base_rect = plot_split->endRect();
	}
	if (un_redo)
	{
		QList<QLayoutItem *> l_items;
		for (int i = col_from+1; i < col_to+1; i++)
		{
			QPair<int, int> pair(row, i);
			if (d_widgets.contains(pair))
			{
				l_items << cells_layout->takeAt(cells_layout->indexOf(d_widgets.value(pair)));
				d_widgets.value(pair) -> close();
				d_widgets.remove(pair);
			}
		}
		if (plot_split)
		{
			l_items << cells_layout->takeAt(cells_layout->indexOf(plot_split));
			cells_layout -> addWidget(plot_split, row, col_from, 1, col_to-col_from+1);
			plot_split -> setFixedWidth(plot_split->endRect().width()*(col_to-col_from+1)+cells_layout->spacing()*(col_to-col_from));
			plot_split -> updateCloseBtnPos();
		}
		else
		{
			l_items << cells_layout->takeAt(cells_layout->indexOf(mw_split));		  
			cells_layout -> addWidget(mw_split, row, col_from, 1, col_to-col_from+1);
			mw_split -> setFixedWidth(mw_split->originSize().width()*(col_to-col_from+1)+cells_layout->spacing()*(col_to-col_from));			
		}
		foreach (QLayoutItem * d_item, l_items)
			delete d_item;		
	}
	else
	{
		if (plot_split)
		{
			plot_split -> setFixedWidth(base_rect.width());
			plot_split -> updateCloseBtnPos();
			cells_layout -> addWidget(plot_split, row, col_from);			
		}
		else
		{
			mw_split -> setFixedWidth(base_rect.width());
			cells_layout -> addWidget(mw_split, row, col_from);			
		}		
		for (int i = col_from+1; i < col_to+1; i++)
		{
			QPair<int, int> pair(row, i);
			MatrixWidget * new_mw = generateDefault(base_rect);
			cells_layout -> addWidget(new_mw, row, i);
			new_mw -> show();
			d_widgets.insert(pair, new_mw);
		}				
	}			
}

void LittleWidgetItem::unreClearGeneratedPlots(const QList<QStandardItem *> & c_plots, QList<PlotWidget *> & dest_plots, bool un_redo)
{
	if (un_redo)
	{
		if (dest_plots.isEmpty())
		{
			foreach (QStandardItem * item, c_plots)
			{
				QStandardItem * tail_item = item->child(item->rowCount()-1);
				PlotWidget * plot = 0;
				if (tail_item->text().contains(tr("视图位置")))
				{
					QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
					while (p_map.hasNext())
					{
						p_map.next();
						if (p_map.value()->singlePlotInfo() == item)
						{
							plot = p_map.value();
							break;
						}
					}
				}
				if (plot)
				{
					dest_plots << plot;
					coverShowPlot(0, plot);
					QString info_text = tail_item->text();
					QStringList info_list = info_text.split(QObject::tr("："));
					QStringList row_col = info_list[1].split(QObject::tr("，"));
					QPair<int, int> pos(row_col[0].toInt()-1, row_col[1].toInt()-1);	
					plot_widgets.remove(pos);
					MatrixWidget * defaults = 0;
					if (plot->width() > plot->endRect().width())
						defaults = generateDefault(plot->rect(), plot->endRect());
					else
						defaults = generateDefault(plot->rect());
					setDefaultWidget(pos, defaults);									
				}
			}
		}
		else
		{
			foreach (PlotWidget * plot, dest_plots)
			{
				coverShowPlot(0, plot);
				QString info_text = plot->singlePlotInfo()->child(plot->singlePlotInfo()->rowCount()-1)->text();
				QStringList info_list = info_text.split(QObject::tr("："));
				QStringList row_col = info_list[1].split(QObject::tr("，"));
				QPair<int, int> pos(row_col[0].toInt()-1, row_col[1].toInt()-1);	
				plot_widgets.remove(pos);
				MatrixWidget * defaults = 0;
				if (plot->width() > plot->endRect().width())
					defaults = generateDefault(plot->rect(), plot->endRect());
				else
					defaults = generateDefault(plot->rect());
				setDefaultWidget(pos, defaults);					
			}
		}
	}
	else
	{
		foreach (PlotWidget * plot, dest_plots)
		{
			coverShowPlot(plot, 0);
			QString info_text = plot->singlePlotInfo()->child(plot->singlePlotInfo()->rowCount()-1)->text();
			QStringList info_list = info_text.split(QObject::tr("："));
			QStringList row_col = info_list[1].split(QObject::tr("，"));
			QPair<int, int> pos(row_col[0].toInt()-1, row_col[1].toInt()-1);	
			plot_widgets.insert(pos, plot);
			delete d_widgets[pos];
			d_widgets.remove(pos);
		}
	}
}

void LittleWidgetItem::unreClearGeneratedPlot(QStandardItem * c_plot, PlotWidget * &dest_plot, bool un_redo)
{
 	int tail = c_plot->rowCount()-1; 
	if (un_redo)
	{	  
		if (!dest_plot)
		{
			if (c_plot->child(tail)->text().contains(tr("视图位置")))
			{
				QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
				while (p_map.hasNext())
				{
					p_map.next();
					if (p_map.value()->singlePlotInfo() == c_plot)
						dest_plot = p_map.value();
				}
			}
		}
		if (dest_plot)
		{
			coverShowPlot(0, dest_plot);
			QString info_text = c_plot->child(tail)->text();
			QStringList info_list = info_text.split(QObject::tr("："));
			QStringList row_col = info_list[1].split(QObject::tr("，"));
			QPair<int, int> pos(row_col[0].toInt()-1, row_col[1].toInt()-1);	
			plot_widgets.remove(pos);			
			int p_span = qRound(dest_plot->rect().width()/dest_plot->endRect().width());
			MatrixWidget * m_default = 0;
			if (p_span > 1)
				m_default = generateDefault(dest_plot->rect(), dest_plot->endRect());
			else
				m_default = generateDefault(dest_plot->rect());
			setDefaultWidget(pos, m_default);
		}
	}
	else
	{
		if (dest_plot)
		{
			QString info_text = c_plot->child(tail)->text();
			QStringList info_list = info_text.split(QObject::tr("："));
			QStringList row_col = info_list[1].split(QObject::tr("，"));
			QPair<int, int> pos(row_col[0].toInt()-1, row_col[1].toInt()-1);	
			delete d_widgets.value(pos);
			d_widgets.remove(pos);
			coverShowPlot(dest_plot, 0);
			plot_widgets.insert(pos, dest_plot);
		}
	}
}

void LittleWidgetItem::checkSameColorInMatrix(QWidget * sender)
{
	QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
	while (d_map.hasNext())
	{
		d_map.next();
		if (d_map.value()->palette().color(QPalette::Background)!=Qt::darkGray && d_map.value()!=sender)
		{
			const_cast<QPalette &>(d_map.value()->palette()).setColor(QPalette::Background, Qt::darkGray);
			d_map.value() -> clearPosLabel();
			break;
		}
	}
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		if (p_map.value()->hasColorPad() && p_map.value()!=sender)
		{
			p_map.value()->removeCoverPad();
			break;
		}
	}
	update();	
}

void LittleWidgetItem::exportPng(LittleWidgetsView * view)
{
	DiaLog * save_dialog = new DiaLog;
	MainWindow * mw = qobject_cast<MainWindow *>(view->parent());
	connect(save_dialog, SIGNAL(sendSaveFileName(const QString &)), this, SLOT(receivePdfFileName(const QString &)));
	save_dialog -> initSaveStrDefineDialog(mw, mw->curBackGuider(), tr("导出图片"));
	if (!save_dialog->exec())
		return; 
	QString defed_name(title+".png");
	QMapIterator<QPair<int, int>, PlotWidget *> p_map0(plot_widgets);
	while (p_map0.hasNext())
	{
		p_map0.next();				
		if (p_map0.value()->hasColorPad())
		{
			p_map0.value()->coverWidgetPtr()->hide();
			p_map0.value()->exsitedBtn()->hide();
			QPixmap pixmap(p_map0.value()->size());
			p_map0.value() -> render(&pixmap);
			pixmap.save(defed_name);
			p_map0.value()->coverWidgetPtr()->show();
			p_map0.value()->exsitedBtn()->show();
			QDir test1_dir(tr("/home/dapang/workstation/spc-tablet/")+defed_name);
			test1_dir.rename(test1_dir.path(), tr("/home/dapang/workstation/image文件/")+defed_name);				
			return;
		}
	}
	QMapIterator<QPair<int, int>, PlotWidget *> p_map1(plot_widgets);
	while (p_map1.hasNext())
	{
		p_map1.next();				
		p_map1.value()->exsitedBtn()->hide();  
	}
	QPixmap * w_pixmap = new QPixmap(widget_container->size());
	widget_container -> render(w_pixmap);
	w_pixmap -> save(defed_name);	
	if (!w_pixmap->paintEngine()->isActive())
	{
		delete w_pixmap;
		QMapIterator<QPair<int, int>, PlotWidget *> p_map2(plot_widgets);
		while (p_map2.hasNext())
		{
			p_map2.next();
			p_map2.value()->exsitedBtn()->show();
		}
	}
	QDir test0_dir(tr("/home/dapang/workstation/spc-tablet/")+defed_name);
	test0_dir.rename(test0_dir.path(), tr("/home/dapang/workstation/image文件/")+defed_name);	
}

void LittleWidgetItem::resetMatrixForSavePlots(TableManagement * tree_info, QSqlTableModel * plots_model, LittleWidgetsView * p_view, SpcNum * spc, SpcDataBase * db)
{
	QString matrix_infoes = plots_model->data(plots_model->index(0, 0)).toString();
	QStringList matrix_text = matrix_infoes.split(tr("；"));
	QStringList matrix_detail = matrix_text[1].split(";");
	QString rect_str = matrix_detail.takeFirst();
	QStringList rect_info = rect_str.split(",");
	int base_width = rect_info[0].toInt();
	int base_height = rect_info[1].toInt();
	QRect rect_base(0, 0, base_width, base_height);
	QString projs_str = plots_model->data(plots_model->index(0, 1)).toString();
	QStringList projs_list = projs_str.split(tr("，，&；"));
	projs_list.pop_back();
	DataSelectModel * t_model = qobject_cast<DataSelectModel *>(tree_info->model());
	foreach (QString e_str, projs_list)
	{
		QStringList orig_pojs = e_str.split(tr("，，～："));
		QStringList old_nums = orig_pojs.back().split(tr("，，～；"));
		QStringList f_items = old_nums.filter(tr("视图位置："));
		QStringList pos_items = f_items.back().split(tr("，，～，"));
		QStringList pos_item = pos_items.at(0).split(tr("，，～")); 
		QList<QStandardItem *> t_item = t_model->findItems(pos_item.back(), Qt::MatchRecursive);	
		PlotWidget * new_plot = new PlotWidget(spc, db);
		connect(new_plot, SIGNAL(enableQmlBtnsForShowContents(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)), this, SIGNAL(refreshPlotShowQmlBtns(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)));
		QStringList pos_list = t_item.at(0)->text().split(tr("，"));
		QStringList row_list = pos_list[0].split(tr("视图位置："));
		QPair<int, int> plot_key(row_list[1].toInt()-1, pos_list[1].toInt()-1);	
		QString temp_str = QString("%1").arg(plot_key.first)+","+QString("%1").arg(plot_key.second);
		bool found = false;
		foreach (QString str, matrix_detail)
		{
			if (str.contains(temp_str))
			{
				temp_str = str;
				found = true;
				break;
			}
		}
		if (found) 
		{
			QStringList pos_details = temp_str.split(",");
			int p_col = pos_details[1].toInt();
			int span = pos_details[2].toInt();
			new_plot -> generateSingle(t_item.at(0)->parent(), t_item.at(0), QRect(0, 0, base_width*span+cells_layout->spacing()*(span+p_col-1), base_height), p_view, this, 0, rect_base);
			for (int i = plot_key.second+1; i < plot_key.second+span; i++)
			{
				QPair<int, int> w_rm(plot_key.first, i);	
				delete d_widgets.value(w_rm);
				d_widgets.remove(w_rm);
			}
		}
		else
			new_plot -> generateSingle(t_item.at(0)->parent(), t_item.at(0), rect_base, p_view, this, 0);
		delete d_widgets.value(plot_key);
		d_widgets.remove(plot_key);
		plot_widgets.insert(plot_key, new_plot);
		new_plot -> resetRelatedPlotVars(rect_info[0].toInt()/showing_rect.width());		
		coverShowPlot(new_plot, 0);							
	}
}

void LittleWidgetItem::relatedTmpUnshowWidgetPos(PlotWidget * f_widget, QPair<int, int> & f_pos)
{
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		if (p_map.value() == f_widget)
			f_pos = p_map.key();
	}
}

void LittleWidgetItem::relatedPlotTitleEditting()
{
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		if (p_map.value()->hasColorPad())
		{
			p_map.value() -> actionForNestedorTitle();
			break;			
		}
	}
}

void LittleWidgetItem::respondPlotShowingContent(const QString & s_order)
{
 	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		if (p_map.value()->hasColorPad())
		{
			p_map.value() -> nestedPlotContentAction(s_order); 
			break;
		}
	} 
}

bool LittleWidgetItem::findAdjacentBackPosPair(QWidget * front, QPair<int, int> & f_pos)
{
	if (cells_layout->indexOf(front) == cells_layout->count())
		return false;
	QPair<int, int> cur_pair = findShowingPos(front);	  
	if (d_widgets.contains(cur_pair))
	{
		QList<QPair<int, int> > pair_list = d_widgets.keys();
		if (pair_list.indexOf(cur_pair) == pair_list.size()-1)
		{
			for (int i = 0; i < plot_widgets.keys().size(); i++)
			{
				if (plot_widgets.keys().at(i) > cur_pair)
				{
					f_pos = plot_widgets.keys().at(i);
					return true;
				}
			}		  
		}
		else
		{
			QPair<int, int> b_pair = pair_list.at(pair_list.indexOf(cur_pair)+1);
			for (int i = 0; i < plot_widgets.keys().size(); i++)
			{
				if (plot_widgets.keys().at(i) > cur_pair && plot_widgets.keys().at(i) < b_pair)
				{
					f_pos = plot_widgets.keys().at(i);
					return true;
				}
			}
			f_pos = b_pair;
		}
	}
	else
	{
		QList<QPair<int, int> > pair_list = plot_widgets.keys();	  
		if (pair_list.indexOf(cur_pair) == pair_list.size()-1)
		{
			for (int i = 0; i < d_widgets.keys().size(); i++)
			{
				if (d_widgets.keys().at(i) > cur_pair)
				{
					f_pos = d_widgets.keys().at(i);
					return true;
				}
			}		  
		}	
		else
		{
			QPair<int, int> b_pair = plot_widgets.keys().at(plot_widgets.keys().indexOf(cur_pair)+1);
			for (int i = 0; i < d_widgets.keys().size(); i++)
			{
				if (d_widgets.keys().at(i) > cur_pair && d_widgets.keys().at(i) < b_pair)
				{
					f_pos = d_widgets.keys().at(i);
					return true;
				}
			}
			f_pos = b_pair;			
		}
	}
	return true;
}

bool LittleWidgetItem::findPlotsOnMergeRow(int * col_from, int col_to, int row, QHash<QPair<int, int>, int> & merge_matrixes, QList<PlotWidget *> & merge_plots)
{
 	if (*col_from > col_to)
		return false; 
	QWidget * mw = cells_layout->itemAtPosition(row, *col_from)->widget();
	QPair<int, int> back_pos;
	if (!findAdjacentBackPosPair(mw, back_pos))
		return false;
	*col_from = back_pos.second;	
	for (int i = *col_from; i < col_to+1; i++)
	{
		QPair<int, int> pair(row, i);
		if (d_widgets.contains(pair))
		{
			int mw_span = qRound(d_widgets.value(pair)->rect().width()/d_widgets.value(pair)->originSize().width());
			if (mw_span > 1)
				merge_matrixes[pair] = mw_span;
		}
		if (plot_widgets.contains(pair))
			merge_plots << plot_widgets.value(pair);
	}
	return true;
}

bool LittleWidgetItem::findPlotOnSplitRow(QStandardItem * span_info, QStandardItem * row_info)
{	
	QPair<int, int> found_pair;
	bool found = false;
	QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
	while (d_map.hasNext())
	{
		d_map.next();
		if (d_map.value()->palette().color(QPalette::Background) != Qt::darkGray)
		{
			found_pair = d_map.key();
			found = true;
			break;
		}
	}
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		if (p_map.value()->hasColorPad())
		{
			found_pair = p_map.key();
			found = true;
			break;
		}
	}
	if (found)
	{	
		int span = 0;
		QWidget * which_widget = d_widgets.value(found_pair);
		if (which_widget)
			span = qRound(d_widgets.value(found_pair)->width()/d_widgets.value(found_pair)->originSize().width());
		else
			span = qRound(plot_widgets.value(found_pair)->width()/plot_widgets.value(found_pair)->endRect().width());
		if (span == 1)
			return false;
		span_info -> setColumnCount(span);
		row_info -> setRowCount(found_pair.first);
		row_info -> setColumnCount(found_pair.second);
	}
	return found;
}

bool LittleWidgetItem::coveredPlotExisted(QPair<int, int> & cover_pos)
{
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();			
		if (p_map.value()->hasColorPad())
		{
			cover_pos = p_map.key();
			return true;
		}
	}
	return false;	
}

QStandardItem * LittleWidgetItem::destPlotDetailInformation(const QPair<int, int> & pos)
{
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		if (p_map.key() == pos)
			return p_map.value()->singlePlotInfo();
	}
	return 0;	
}

MatrixWidget * LittleWidgetItem::generateDefault(const QRect & size, const QRect & origin_size)
{
	MatrixWidget * default_widget = 0;
	if (!origin_size.isNull())
	{
		default_widget = new MatrixWidget(origin_size, this);
		default_widget -> setFixedWidth(size.width());
		default_widget -> setFixedHeight(size.height());
	}
	else
		default_widget = new MatrixWidget(size, this);
//	connect(default_widget, SIGNAL(destroyed(QObject *)), this, SLOT(rearrangeForDelDefault(QObject *)));
	return default_widget;	
}

PlotWidget * LittleWidgetItem::newPlotwidgetPlaceMw(QStandardItem * with_item, QPair<int, int> & place_pos, SpcNum * back_spc, SpcDataBase * base_db, LittleWidgetsView * p_view)
{
	PlotWidget * new_plot = 0;
	bool selmw_found = false;
	QRect base_rect;
	QRect cur_rect; 
	QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
	while (d_map.hasNext())
	{
		d_map.next();
		if (d_map.value()->palette().color(QPalette::Background)!=Qt::darkGray)
		{
			place_pos = d_map.key();
			base_rect = d_map.value()->originSize();
			cur_rect = d_map.value()->rect();
			selmw_found = true;
			break;
		}
	}
	if (!selmw_found)
	{
		place_pos = d_widgets.keys().at(0);
		base_rect = d_widgets.value(place_pos)->originSize();
		cur_rect = d_widgets.value(place_pos)->rect();
	}
	new_plot = new PlotWidget(back_spc, base_db);
	connect(new_plot, SIGNAL(enableQmlBtnsForShowContents(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)), this, SIGNAL(refreshPlotShowQmlBtns(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)));
	if (cur_rect.width() != base_rect.width())
		new_plot -> generateSingle(with_item, 0, cur_rect, p_view, this, 0, base_rect);
	else
		new_plot -> generateSingle(with_item, 0, cur_rect, p_view, this, 0);
	new_plot -> resetRelatedPlotVars(cur_rect.width()/showing_rect.width());
	return new_plot;
}

QPair<int, int> LittleWidgetItem::realRowsColsInGridLayout()
{
	QPair<int, int> found_pair;
	QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
	while (d_map.hasNext())
	{
		d_map.next();
		if (d_map.key() > found_pair)
			found_pair = d_map.key();
	}
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		if (p_map.key() > found_pair)
			found_pair = p_map.key();
	}
	return found_pair;
}

QList<PlotWidget *> & LittleWidgetItem::unshowPlotsMatrixWidgets()
{
	return unshow_widgets;
}

QMap<QPair<int, int>, MatrixWidget *> & LittleWidgetItem::allCurrentDefaults()
{
	return d_widgets;
}

QMap<QPair<int, int>, PlotWidget *> & LittleWidgetItem::allCurrentPlots()
{
	return plot_widgets;
}

void LittleWidgetItem::arrangePlotForBtnSig(QObject * deled)
{
	PlotWidget * hide_plot = qobject_cast<PlotWidget *>(deled->parent());
	if (!hide_plot || hide_plot->isHidden())
		return;
	hide_plot -> setCloseBtnPtr(0);
	QStandardItem * plot_item = hide_plot->singlePlotInfo();
	QStandardItem * pos_info = plot_item->child(plot_item->rowCount()-1);
	QString info_text = pos_info->text();
	QStringList info_list = info_text.split(tr("："));
	QStringList row_col = info_list[1].split(tr("，"));
	QPair<int, int> pos(row_col[0].toInt()-1, row_col[1].toInt()-1);
	plot_widgets.remove(pos);
	emit changingPlotInfoes(hide_plot);
}

void LittleWidgetItem::findPlotToChange(int type, QStandardItem * find_plot)
{
	QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
	while (p_map.hasNext())
	{
		p_map.next();
		if (p_map.value()->singlePlotInfo() == find_plot)
		{
			p_map.value() -> respondChangingAct(type);
			break;
		}
	}	
}

void LittleWidgetItem::receivePdfFileName(const QString & df_name)
{
	title = df_name;
}

void LittleWidgetItem::createCellsOnGridLayout(int create_from, int create_to, int create_num, bool h_v, QHash<QPair<int, int>, int> & red_widgets, QList<PlotWidget *> & red_plots)
{
	QHash<QPair<int, int>, int> take_widgets = red_widgets;
	QList<PlotWidget *> take_plots = red_plots;
	QPair<int, int> temp_rv;
	temp_rv.first = 0;
	temp_rv.second = 0;
	QRect base_rect;
	QRect add_size;
	if (d_widgets.contains(temp_rv))
	{
		base_rect = d_widgets[temp_rv]->originSize();
		add_size = d_widgets[temp_rv]->rect();
	}
	else
	{
		base_rect = plot_widgets[temp_rv]->endRect();
		add_size = plot_widgets[temp_rv]->rect();	
	}
	if (h_v)
	{
		for (int i = create_from; i < create_to; i++)
		{
			for (int j = 0; j < create_num; j++)
			{
				temp_rv.first = i;
				temp_rv.second = j;
				MatrixWidget * new_mw = generateDefault(add_size, base_rect);
				cells_layout -> addWidget(new_mw, temp_rv.first, temp_rv.second);
				new_mw -> show();
				d_widgets.insert(temp_rv, new_mw);
			}
		}
		
	}
	else
	{
		for (int i = 0; i < create_num; i++)
		{
			for (int j = create_from; j < create_to; j++)
			{
				temp_rv.first = i;
				temp_rv.second = j;
				MatrixWidget * new_mw = generateDefault(add_size, base_rect);
				cells_layout -> addWidget(new_mw, temp_rv.first, temp_rv.second);
				new_mw -> show();
				d_widgets.insert(temp_rv, new_mw);
			}
		}
	}
	foreach (PlotWidget * plot, take_plots)
	{
		QString info_text = plot->singlePlotInfo()->child(plot->singlePlotInfo()->rowCount()-1)->text();
		QStringList info_list = info_text.split(QObject::tr("："));
		QStringList row_col = info_list[1].split(QObject::tr("，"));
		QPair<int, int> pos(row_col[0].toInt()-1, row_col[1].toInt()-1);				
		int span = qRound(plot->width()/plot->endRect().width());
		if (plot->endRect() != base_rect)
		{
			plot -> resetEndRect(base_rect.width(), base_rect.height());
			plot -> setFixedWidth(base_rect.width()*span+cells_layout->spacing()*(span-1));
			plot -> setFixedHeight(base_rect.height());
			plot -> updateCloseBtnPos();
		}
		for (int i = pos.second; i < pos.second+span; i++)
		{
			QPair<int, int> pair(pos.first, i);		  
			if (d_widgets.contains(pair))
			{
				delete d_widgets.value(pair);
				d_widgets.remove(pair);						
			}
		}
		cells_layout -> addWidget(plot, pos.first, pos.second, 1, span);
		plot -> show();
		plot_widgets.insert(pos, plot);
	}
	QHashIterator<QPair<int, int>, int> mw_hash(red_widgets);
	while (mw_hash.hasNext())
	{
		mw_hash.next();
		QPair<int, int> pos(mw_hash.key().first, mw_hash.key().second);
		for (int i = pos.second; i < pos.second+mw_hash.value(); i++)
		{
			QPair<int, int> pair(pos.first, i);		  
			if (d_widgets.contains(pair))
			{
				delete d_widgets.value(pair);
				d_widgets.remove(pair);						
			}			  
		}
		QRect span_rect(0, 0, base_rect.width()*mw_hash.value()+cells_layout->spacing()*(mw_hash.value()-1), base_rect.height());
		MatrixWidget * new_mw = generateDefault(span_rect, base_rect);
		cells_layout -> addWidget(new_mw, pos.first, pos.second, 1, mw_hash.value());
		new_mw -> show();
		d_widgets.insert(pos, new_mw);			
	}	
}
	
void LittleWidgetItem::deleteCellsOnGridLayout(int delete_from, int delete_to, int delete_num, bool h_v)
{
	QPair<int, int> temp_rv;
	QPair<int, int> match_rv;
	if (h_v)
	{
		match_rv.first = delete_to;
		match_rv.second = 0;
	}
	else
		match_rv.second = delete_to;
	for (int i = delete_from; i > delete_to; i--)
	{
		for (int j = 0; j < delete_num; j++)
		{
			if (h_v)
			{
				temp_rv.first = i;
				temp_rv.second = j;
			}
			else
			{
				match_rv.first = j;
				temp_rv.first = j;
				temp_rv.second = i;
			}
			if (temp_rv >= match_rv)
			{
				if (d_widgets.contains(temp_rv))
				{
					delete d_widgets.value(temp_rv);
					d_widgets.remove(temp_rv);
				}
				if (plot_widgets.contains(temp_rv))
				{
					QLayoutItem * item = cells_layout->takeAt(cells_layout->indexOf(plot_widgets.value(temp_rv)));
					plot_widgets.value(temp_rv)->hide();
					plot_widgets.remove(temp_rv);
					delete item;				
				}
			}
		}
	}
}

void LittleWidgetItem::chkAndCleanNoWidgetItems(int row)
{
	for (int i = 0; i < cells_layout->columnCount(); i++)
	{
		QLayoutItem * f_item = cells_layout->itemAtPosition(row, i);		
		if (f_item && (!f_item->widget() || f_item->widget()->isHidden()))
		{
			if (f_item->widget()) 
				cells_layout->takeAt(cells_layout->indexOf(f_item->widget()));
			delete f_item;
		}
	}
}

const QPair<int, int> & LittleWidgetItem::findShowingPos(QWidget * finding)
{
	const QPair<int, int> & find_pos = QPair<int, int>();
	if (qobject_cast<MatrixWidget *>(finding))
	{
		QMapIterator<QPair<int, int>, MatrixWidget *> d_map(d_widgets);
		while (d_map.hasNext())
		{
			d_map.next();			
			if (d_map.value() == finding)
				return d_map.key();
		}		
	}
	else
	{
		QMapIterator<QPair<int, int>, PlotWidget *> p_map(plot_widgets);
		while (p_map.hasNext())
		{
			p_map.next();			
			if (p_map.value() == finding)
				return p_map.key();
		}
	}
	return find_pos;
}

QWidget * LittleWidgetItem::findRowColFrontWidget(int row, int col)
{
	int match_col = col;
	QLayoutItem * f_item = 0;
	match_col -= 1;	
	while (!f_item)
	{
		f_item = cells_layout->itemAtPosition(row, match_col);
		match_col --;		
	}
	return f_item->widget(); 
}