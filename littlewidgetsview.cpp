#include <QtGui>
#include <QtCore>
#include <QtDeclarative>
#include <QtSql/QSqlTableModel>
#include "spcdatabase.h"
#include "littlewidgetsview.h"
#include "littlewidgetitem.h"
#include "tablemanagement.h"
#include "tablesgroup.h"
#include "relatedtreeview.h"
#include "dataestableview.h"
#include "engitreeview.h"
#include "plotwidget.h"
#include "matrixwidget.h"
#include "toolsplot.h"
#include "helpcontents.h"

LittleWidgetsView::LittleWidgetsView(QWidget * parent)
:QDeclarativeView(parent), qml_frozen(false), mcol_from(0), big_container(0), drag(0), move_pos(0), agent(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	qmlRegisterType<LittleWidgetItem>("NestedWidget", 1, 0, "LittleWidgetItem");
}

LittleWidgetsView::~LittleWidgetsView()
{}

void LittleWidgetsView::setMainQmlFile(const QString &file)
{
	setSource(QUrl::fromLocalFile(file));
	connect(this, SIGNAL(initRectangle(QVariant, QVariant, QVariant, QVariant)), rootObject(), SLOT(initWholeSize(QVariant, QVariant, QVariant, QVariant)));
	connect(this, SIGNAL(newRectangle(QVariant, QVariant)), rootObject(), SLOT(resetRectangle(QVariant, QVariant)));
}

void LittleWidgetsView::initMarginForParent(const QRect & size)
{
	view_window = size;	
	ensureVisible(view_window, 0, 0);
}

void LittleWidgetsView::initViewMatrix(int h_num, int v_num, QRect & cell_size)
{
	agent = new LittleWidgetItem(qobject_cast<QDeclarativeItem *>(rootObject()->findChild<QObject *>("f_rec")));
	agent -> setNewRect(view_window);
	QGridLayout * l_grid = new QGridLayout;
	l_grid -> setSpacing(5);
	big_container = new QWidget;
	agent -> setWidgetLayoutPt(l_grid, big_container);
	for (int i = 0; i < h_num; i++)
	{
		for (int j = 0; j < v_num; j++)
		{
			MatrixWidget * defaults = agent->generateDefault(cell_size);
			QPair<int, int> index(i, j);
			agent -> setDefaultWidget(index, defaults);
		}
	}
	big_container -> setLayout(l_grid);
	agent -> setWidget(big_container);	
	connect(agent, SIGNAL(matrixChanging()), this, SLOT(resizeAgentSize()));
	connect(agent, SIGNAL(changingPlotInfoes(PlotWidget *)), this, SIGNAL(dealSigFromAgentHidePlot(PlotWidget *)));
	connect(agent, SIGNAL(refreshPlotShowQmlBtns(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)), this, SIGNAL(sendPlotShowToQmlBtns(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)));
	connect(this, SIGNAL(changingPlot(int, QStandardItem *)), agent, SLOT(findPlotToChange(int, QStandardItem *)));
	connect(this, SIGNAL(freezingWidgetsPlots(QVariant)), rootObject(), SLOT(frozenState(QVariant)));
	connect(this, SIGNAL(dragingPt(QVariant, QVariant, QVariant, QVariant)), rootObject(), SLOT(dragPointChanging(QVariant, QVariant, QVariant, QVariant)));
	connect(this, SIGNAL(setCellsNewSize(QVariant, QVariant)), rootObject(), SLOT(resetCellsSize(QVariant, QVariant)));	
	emit initRectangle(view_window.width(), view_window.height(), big_container->width(), big_container->height());	
	show();
}

void LittleWidgetsView::setViewForRelatedTable(QWidget * w_table)
{
	agent = new LittleWidgetItem(qobject_cast<QDeclarativeItem *>(rootObject()->findChild<QObject *>("f_rec")));
	agent -> setNewRect(view_window);
	setStyleSheet("border: 1px groove gray; border-radius: 4px;");	
	if (qobject_cast<DataesTableView *>(w_table))
	{
		DataesTableView * current_dataes = qobject_cast<DataesTableView *>(w_table);
		current_dataes -> setRightOrigRect(view_window);
		connect(current_dataes, SIGNAL(sizeChanged()), this, SLOT(resizeAgentSize()));				
	}
	else if (qobject_cast<EngiTreeView *>(w_table))
	{
		EngiTreeView * etree_table = qobject_cast<EngiTreeView *>(w_table);
		connect(etree_table, SIGNAL(treeViewSizeChanged()), this, SLOT(resizeAgentSize()));		  
	} 	
	else if (qobject_cast<TablesGroup *>(w_table))
	{
		TablesGroup * current_tablegroup = qobject_cast<TablesGroup *>(w_table);
		connect(current_tablegroup, SIGNAL(groupTableSizeChanged()), this, SLOT(resizeAgentSize()));	
		connect(current_tablegroup, SIGNAL(freezingTable(QVariant)), rootObject(), SLOT(frozenState(QVariant)));			
	} 
	else if (qobject_cast<RelatedTreeView *>(w_table))
	{
		RelatedTreeView * rtree_table = qobject_cast<RelatedTreeView *>(w_table);
		connect(rtree_table, SIGNAL(sizeChanged()), this, SLOT(resizeAgentSize()));	
		connect(rtree_table, SIGNAL(freezingNestCell(QVariant)), rootObject(), SLOT(frozenState(QVariant)));	  
	} 
	else if (qobject_cast<TableManagement *>(w_table))
	{
		TableManagement * tree_table = qobject_cast<TableManagement *>(w_table);
		connect(tree_table, SIGNAL(sizeChanged()), this, SLOT(resizeAgentSize()));	
		connect(tree_table, SIGNAL(freezingNestCell(QVariant)), rootObject(), SLOT(frozenState(QVariant)));		
	}
	agent -> setWidget(w_table);
	if (w_table->objectName()!="plot")
		connect(this, SIGNAL(setCellsNewSize(QVariant, QVariant)), rootObject(), SLOT(resetCellsSize(QVariant, QVariant)));	
	emit initRectangle(view_window.width(), view_window.height(), w_table->width(), w_table->height());
	show();
}

void LittleWidgetsView::setTblManager(TableManagement * tbl_mng)
{
	tbl_guider = tbl_mng;
}

void LittleWidgetsView::setHelpPages(HelpContents * h_pages)
{
	agent = new LittleWidgetItem(qobject_cast<QDeclarativeItem *>(rootObject()->findChild<QObject *>("f_rec")));
	agent -> setNewRect(view_window);
	agent -> setWidget(h_pages);	
	emit initRectangle(view_window.width(), view_window.height(), h_pages->width(), h_pages->height());	
	show();	
}

void LittleWidgetsView::setManualNameFromDB(const QString & db_manual)
{
	stored_manual = db_manual;
}

void LittleWidgetsView::resetQmlFrozenState(bool new_state)
{
	qml_frozen = new_state;
	emit freezingWidgetsPlots(!new_state);
}

void LittleWidgetsView::deletePlotsPicsInDbTbl(const QString & tbl_name, const QString & tbl_contents, SpcDataBase * db)
{
	QStringList first_split = tbl_contents.split(tr("，，&；"));
	first_split.pop_back();
	QString pics_table("pictures");
	foreach (QString item_str, first_split)
	{
		QStringList ev_list = item_str.split(tr("，，～："));
		QString next_back = ev_list.takeLast();
		QStringList old_nums = next_back.split(tr("，，～；"));				
		foreach (QString t_str, old_nums)
		{
			if (!t_str.isEmpty() && !t_str.contains(tr("，！，")) && t_str.contains(tr("视图位置：")))
			{
				QStringList pos_list = t_str.split(tr("，，～，"));
				QStringList chd_list = pos_list.at(0).split(tr("，，～"));
				QString pic_row("nested="+tbl_name+tr("，。，")+chd_list.at(1));
				QString col;
				db -> deleteDataes(pics_table, pic_row, col);					
			}
		}
	}	
}

bool LittleWidgetsView::frozenSate()
{
	return qml_frozen;
}

QWidget * LittleWidgetsView::nestedContainer()
{
	return big_container;
}

LittleWidgetItem * LittleWidgetsView::qmlAgent()
{
	return agent;
}

const QString & LittleWidgetsView::dbManualName()//? why
{
	return stored_manual;
}

const QRect & LittleWidgetsView::initRect()
{
	return view_window;
}

void LittleWidgetsView::resetViewWidth(const QVariant & size)
{
	int w_resize = size.toRect().width()-view_window.width();
	int h_resize = size.toRect().height()-view_window.height();
	view_window.adjust(0, 0, w_resize, h_resize);
	setResizeMode(QDeclarativeView::SizeRootObjectToView);
	emit newRectangle(view_window.width(), view_window.height());	
}

void LittleWidgetsView::relatedWorkFromQmlEditor(QString guide, QString action)
{
	if (guide == tr("放大显示") || guide == tr("缩小显示"))
	{
		if (action == "true")
			agent -> zoomInPlots();
		else
			agent -> zoomOutPlots();
		resizeAgentSize();
	}	
	else if (action == tr("修改标题"))
		agent -> relatedPlotTitleEditting();
	else if (action==tr("分区显示") || action==tr("控制线") || action==tr("点图线") || action==tr("点序坐标") || action==tr("时序坐标"))
		agent -> respondPlotShowingContent(action);	
	else if (guide==tr("图宽：") || guide==tr("图高："))
	{
		int size = action.toInt();
		if (!size || action.isEmpty())
			return;
		if (guide==tr("图宽："))
			agent -> setCellSize(size, 0);
		else
			agent -> setCellSize(0, size);
		resizeAgentSize();				
	}
	else if (guide==tr("行数：") || guide==tr("列数："))
	{
		int num = action.toInt();
		if (!num || action.isEmpty())
			return;
		QString matrix(tr("图阵更改"));	
		QList<PlotWidget *> existed_hides;
		QHash<QPair<int, int>, int> store_widgets;
		if (guide==tr("行数："))
		{
			int old_rows = agent->realRowsColsInGridLayout().first;
			if (num-1 == old_rows) 
				return;
			QString out_limit;
			agent -> findPlotsOnRowsColumns(num, 0, existed_hides, store_widgets, out_limit);		
			emit unredoOrderForMatrixChange(this, old_rows, num-1, true, matrix, store_widgets, existed_hides);
		}
		else
		{
			int old_cols = agent->realRowsColsInGridLayout().second; 
			if (num-1 == old_cols) 
				return;
			QString out_limit;
			agent -> findPlotsOnRowsColumns(0, num, existed_hides, store_widgets, out_limit);
			if (out_limit == tr("单元越界"))
			{
				//dialog here
				return;
			}
			emit unredoOrderForMatrixChange(this, old_cols, num-1, false, matrix, store_widgets, existed_hides);
		}					
	}
	else if (guide==tr("列从：") || guide==tr("列到："))
	{
		int click_row = 0;
		bool found = false;
		QMapIterator<QPair<int, int>, MatrixWidget *> d_map(agent->allCurrentDefaults());
		while (d_map.hasNext())
		{
			d_map.next();
			if (d_map.value()->palette().color(QPalette::Background)!=Qt::darkGray)
			{
				found = true;
				click_row = d_map.key().first;
				break;
			}
		}
		if (!found && !agent->allCurrentPlots().isEmpty())
		{
			QMapIterator<QPair<int, int>, PlotWidget *> p_map(agent->allCurrentPlots());
			while (p_map.hasNext())
			{
				p_map.next();
				if (p_map.value()->hasColorPad())
				{
					found = true;
					click_row = p_map.key().first;
					break;
				}
			}
		}
		if (!found)
			return;		
		int num = action.toInt();
		if (!num || action.isEmpty())
			return;
		if (guide==tr("列从："))
			mcol_from = num;
		else
		{
			if (mcol_from==num || mcol_from>num)
				return;
			QHash<QPair<int, int>, int> mws_merging;
			QList<PlotWidget *> ps_merging;
			int end_from = mcol_from-1;
			if (!agent->findPlotsOnMergeRow(&end_from, num-1, click_row, mws_merging, ps_merging))
				return;
			QString col_matrix = tr("列合并")+","+QString("%1").arg(click_row);
			emit unredoOrderForMatrixChange(this, end_from, num-1, true, col_matrix, mws_merging, ps_merging);
			mcol_from = 0;
		}
	}
	else if (guide==tr("拆分列"))
	{
		QStandardItem * found_info1 = new QStandardItem;
		QStandardItem * found_info2 = new QStandardItem;
		if (!agent->findPlotOnSplitRow(found_info1, found_info2))
		{
			delete found_info1;
			delete found_info2;
			return;
		}		
		emit plotPositionChanging(found_info1, found_info2, 0, 0);		
	}
	else if (guide==tr("图背景"))
		agent -> strOrderForPlotFromUsr(guide, action);
	else if (guide==tr("移动图"))
	{
		if (action == "false")
		{
			qml_frozen = true;
			emit sendNewFrozenState(true);
			emit freezingWidgetsPlots(false);
		}
		else
		{
			qml_frozen = false;
			emit sendNewFrozenState(false);
			emit freezingWidgetsPlots(true);
		}
	}
	else if (guide==tr("导出图"))
	{
		if (tbl_guider->currentPartner() != this)
			return;
		agent -> exportPng(this);	
	}
}

void LittleWidgetsView::resizeAgentSize()
{
	setResizeMode(QDeclarativeView::SizeRootObjectToView);
	emit setCellsNewSize(agent->widget()->rect().width(), agent->widget()->rect().height());
}

void LittleWidgetsView::dealMimeForPlotMove(PlotWidget * moving, const QPoint & press_pos)
{
	if (!qml_frozen)
		return;
	QMimeData * mimeData = new QMimeData;
	drag = new QDrag(this);
	mimeData->setData("application/s-index", QByteArray::number(0, 0));
	QPixmap pixmap(moving->size());
	pixmap.fill(Qt::transparent);
	moving -> render(&pixmap);
	MatrixWidget * w_temp = 0;
	if (moving->rect().width() > moving->endRect().width())
		w_temp = agent->generateDefault(moving->rect(), moving->endRect());
	else
		w_temp = agent->generateDefault(moving->rect());
	QStandardItem * plot_item = moving->singlePlotInfo();
	QStandardItem * pos_info = plot_item->child(plot_item->rowCount()-1);
	QString info_text = pos_info->text();
	QStringList info_list = info_text.split(tr("："));
	QStringList row_col = info_list.at(1).split(tr("，"));
	QPair<int, int> pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);
	moving -> hide();	
	agent -> setDefaultWidget(pos, w_temp);
	drag->setMimeData(mimeData);
	drag->setPixmap(pixmap);	
	drag->setHotSpot(press_pos);
	drag->exec();	
}

void LittleWidgetsView::dragMoveEvent(QDragMoveEvent * event)
{
	if (event->mimeData()->hasFormat("application/s-index"))
	{
		QPoint topleft_pos = event->pos()-drag->hotSpot();
		emit dragingPt(topleft_pos.x(), topleft_pos.y(), drag->pixmap().width(), drag->pixmap().height());
		if (!move_pos)
		{
			move_pos = new QLabel(this);
			move_pos -> setAttribute(Qt::WA_DeleteOnClose);
			move_pos -> setStyleSheet("QLabel{color:#43CD80}");
			move_pos -> show();
		}		
		QPointF ptf = mapFromScene(event->pos().x(), event->pos().y());			
		QPair<int, int> which_pair(0, 0);		
		agent -> enterWhichWidget(ptf, which_pair);
		QString str = tr("进入到：行")+QString("%1").arg(which_pair.first+1)+tr("，")+tr("列")+QString("%1").arg(which_pair.second+1);
		move_pos -> setText(str);
		QFontMetrics fm(move_pos->font());
		int width_text = fm.width(move_pos->text());	
		move_pos -> resize(width_text, move_pos->height());
		event -> accept();
	}
}

void LittleWidgetsView::dropEvent(QDropEvent *event)
{
	if (event->mimeData()->hasFormat("application/s-index"))
	{
		QStandardItem * new_pos = new QStandardItem;
		QMapIterator<QPair<int, int>, PlotWidget *> p_map(agent->allCurrentPlots());
		while (p_map.hasNext())
		{
			p_map.next();
			if (p_map.value()->hasColorPad())
			{
				QStringList info_list = move_pos->text().split(tr("，"));
				QStringList row_info = info_list.at(0).split(tr("行"));
				QStringList col_info = info_list.at(1).split(tr("列"));
				QString h_pos = row_info.at(1);
				QString v_pos = col_info.at(1);
				QPair<int, int> mv_to = QPair<int, int>(h_pos.toInt()-1, v_pos.toInt()-1);
				QWidget * mt_widget = qobject_cast<QGridLayout *>(big_container->layout())->itemAtPosition(mv_to.first, mv_to.second)->widget();
				MatrixWidget * mvto_w = qobject_cast<MatrixWidget *>(mt_widget);
				PlotWidget * to_plot = qobject_cast<PlotWidget *>(mt_widget);			
				if ((mvto_w && qRound(p_map.value()->width()/mvto_w->width())!=1) || (to_plot && qRound(p_map.value()->width()/to_plot->width())!=1))
				{
					delete new_pos;
					move_pos -> deleteLater();
					move_pos = 0;
					QStandardItem * plot_item = p_map.value()->singlePlotInfo();
					QStandardItem * pos_info = plot_item->child(plot_item->rowCount()-1);
					QString info_text = pos_info->text();
					QStringList info_list = info_text.split(tr("："));
					QStringList row_col = info_list.at(1).split(tr("，"));
					QPair<int, int> pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);	
					p_map.value() -> show();
					agent->allCurrentDefaults()[pos]->close(); 
					agent->allCurrentDefaults().remove(pos);
					event -> accept();
					return;
				}
				QString pos_text = tr("视图位置：")+h_pos+tr("，")+v_pos;
				new_pos -> setText(pos_text);	
				move_pos -> deleteLater();
				move_pos = 0;
				if (mv_to != p_map.key())	
					emit plotPositionChanging(p_map.value()->singlePlotInfo()->child(p_map.value()->singlePlotInfo()->rowCount()-1), new_pos, p_map.value(), to_plot);
				else
				{
					delete new_pos;
					QStandardItem * plot_item = p_map.value()->singlePlotInfo();
					QStandardItem * pos_info = plot_item->child(plot_item->rowCount()-1);
					QString info_text = pos_info->text();
					QStringList info_list = info_text.split(tr("："));
					QStringList row_col = info_list.at(1).split(tr("，"));
					QPair<int, int> pos(row_col.at(0).toInt()-1, row_col.at(1).toInt()-1);	
					p_map.value() -> show();
					agent->allCurrentDefaults()[pos]->close(); 
					agent->allCurrentDefaults().remove(pos);					
				}				
				break;
			}
		}	
		event -> accept();
	}
}