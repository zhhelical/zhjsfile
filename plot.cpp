#include <QtGui>
#include <QtCore>
#include <QtSql>
#include "plot.h"
#include "ptsymbol.h"
#include "spcnum.h"
#include "spcdatabase.h"
#include "dataselectmodel.h"
#include "tablemanagement.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_marker.h"
#include "qwt_text_label.h"
#include "uncontinueaxelmarkers.h"

Plot::Plot(const QwtText & title, SpcDataBase * db, QWidget * parent)
:QwtPlot(parent), all_pts(0), b_left(0.0), b_right(16.0), line(0), db_source(db)
{
	setAttribute(Qt::WA_DeleteOnClose);	
	setPlotShowingVars(title, 1.0);
	setTitle(title);
	plot_title = title.text();	
	QPalette p(palette());
	p.setColor(QPalette::Background, QColor("#008B8B"));
	canvas()->setPalette(p);
	initCtrlLine();
	emptyFileCtrLineShow();
	plotLayout()->setAlignCanvasToScales(true);
}
Plot::~Plot()
{
	if (l_upCtrl)
	{
		delete l_upCtrl;
		delete l_middleCtrl;
		delete l_underCtrl;
	}
	if (l_upMiddleCtrl1)
	{
		delete l_upMiddleCtrl1;
		delete l_upMiddleCtrl2;
		delete l_underMidCtrl1;
		delete l_underMidCtrl2;
		delete l_underMidText;				
	}
	if (line)
		delete line;
	if (!pt_markers.isEmpty())
	{
		for(int i = 0; i < pt_markers.size(); i++)
			delete pt_markers[i];
	}
}

void Plot::setPlotShowingVars(const QwtText & title, double ratio)
{
	if (ratio < 0.5)
		ratio = 0.5;	
	if (!title.text().isEmpty())
	{
		const_cast<QwtText &>(title).setFont(QFont("宋体", 12*ratio));
		setTitle(title);
	}
	axisScaleDraw(QwtPlot::yLeft) -> setTickLength(QwtScaleDiv::MajorTick, 5*ratio);
	axisScaleDraw(QwtPlot::yLeft) -> setTickLength(QwtScaleDiv::MediumTick, 0);
	axisScaleDraw(QwtPlot::yLeft) -> setTickLength(QwtScaleDiv::MinorTick, 0);
	axisScaleDraw(QwtPlot::xBottom) -> setTickLength(QwtScaleDiv::MajorTick, 5*ratio);
	axisScaleDraw(QwtPlot::xBottom) -> setTickLength(QwtScaleDiv::MediumTick, 0);
	axisScaleDraw(QwtPlot::xBottom) -> setTickLength(QwtScaleDiv::MinorTick, 0);
	setAxisFont(QwtPlot::xBottom, QFont("宋体", 8*ratio));
	setAxisFont(QwtPlot::yLeft, QFont("宋体", 8*ratio));	
}

void Plot::resetForMultiPlots(QString & setstr)
{
	QStringList reset_list = setstr.split(",");
	QString find_key = QString("%1").arg(QDateTime::fromString(reset_list.at(0).split(tr("，，。")).at(1)).toTime_t());
	QStringList all_table = db_source->allTables();
	QStringList f_tables = all_table.filter(find_key);		
	if (f_tables.isEmpty())
		return;
	clearAllMemoryPtsDataes();
	initCtrlLine();
	QSqlTableModel db_model(this, db_source->currentConnectionDb()->database(db_source->currentConnectionDb()->connectionName()));
	if (reset_list.at(1).contains("dev"))
	{
		if (title().text() == tr("极差图"))
			setPlotTitle(tr("西格玛图"));
	}		
	else
	{
		if (title().text() == tr("西格玛图"))
			setPlotTitle(tr("极差图"));
	}
	QString reset_name = reset_list.at(1);
	if (reset_list.at(1).contains("avr"))
		reset_name = reset_list.at(1).split("avr").at(0);	
	QString cpk_table = f_tables.filter(tr("，，。cpk")+reset_name).at(0);
	QStringList daily_tbls = f_tables.filter(tr("，，。daily"));
	db_source -> varsModelFromTable(cpk_table, &db_model);
	double show_up = 0.0, show_low = 0.0;
	if (reset_list[1] == "devavr" || reset_list[1] == "rngavr")
	{
		show_up = db_model.data(db_model.index(0, 4)).toDouble();
		show_low = db_model.data(db_model.index(0, 5)).toDouble();	
	}
	else
	{
		show_up = db_model.data(db_model.index(0, 6)).toDouble();
		show_low = db_model.data(db_model.index(0, 7)).toDouble();	
	}	
	if (!daily_tbls.isEmpty())
	{
		all_pts = db_source->lastDataFromTable(daily_tbls.at(0), "groupnum").toInt();
		if (reset_list[1].contains("avr"))
		{
			foreach (QString tbl, daily_tbls)
			{
				QStringList real_back = tbl.split(tr("，，。"));
				if (real_back.back() == "dailyavr")
				{
					initRelatedDataState(tbl);
					initJudgLists(tbl);
					daily_tbls.removeOne(tbl);
					break;
				}				
			}
		}
		else
		{
			if (reset_list[1] == "dev")
			{
				foreach (QString tbl, daily_tbls)
				{
					QStringList real_back = tbl.split(tr("，，。"));
					if (real_back.back() == "dailydev")
					{
						initRelatedDataState(tbl);
						initJudgLists(tbl);
					}				
				}			  				
			}
			else
			{
				foreach (QString tbl, daily_tbls)
				{
					QStringList real_back = tbl.split(tr("，，。"));
					if (real_back.back() == "dailyrng")
					{
						initRelatedDataState(tbl);
						initJudgLists(tbl);
					}				
				}
			}	
		}
	}
	fileCtrLineShow(show_up, show_low);	
	continuedPtsShow(true);
	replot();
}

void Plot::setCurrentPtNumber(int number)
{
	all_pts = number;
}

void Plot::plotXrangeForSingleMode(QPair<int, int> & range)
{
	range.first = pt_markers.at(0)->xValue();
	range.second = pt_markers.at(pt_markers.size()-1)->xValue();
}

void Plot::setNewValueForCtrlLine(double up, double low)
{
	l_upCtrl -> setYValue(up);
	l_underCtrl -> setYValue(low);
}

void Plot::appendNewMarker()
{
	QwtPlotMarker * tmp_marker = new QwtPlotMarker;
	tmp_marker -> setXValue(relatedDataState.keys().back());
	if (title().text() == tr("均值图"))
	{
		relistPtsValueStacks(three_avrJud.back());
		tmp_marker -> setYValue(three_avrJud.back());
		pt_markers << tmp_marker;
		redefPtsColorForNewAvrPt();
	}	
	else if (title().text() == tr("西格玛图"))
	{
		relistPtsValueStacks(eight_devJud.back());
		tmp_marker -> setYValue(eight_devJud.back());
		pt_markers << tmp_marker;
		redefPtsColorForNewDevPt();
	}			
	else
		redefPtsColorForNewRngPt();
	if (pt_markers.size() > 16)
		setAxisScale(QwtPlot::xBottom, pt_markers.size()-16, pt_markers.size());
	tmp_marker -> attach(this);
	continuedLineShow();	
	replot();
}

void Plot::deleteLastPoint()
{
	if (title().text() == tr("均值图"))
	{
		three_avrJud.pop_back();
		five_avrJud.pop_back();
		six_avrJud.pop_back();
		nine_avrJud.pop_back();
		fourteen_avrJud.pop_back();	
	}
	if (title().text() == tr("西格玛图"))
	{
		eight_devJud.pop_back();
		nine_devJud.pop_back();
		fourteen_devJud.pop_back();
		fifteen_devJud.pop_back();	
	}
//	xaxel_pairs.pop_back();
	relatedDataState.remove(relatedDataState.keys().back());
	judging_list.pop_back();
	if (!judged_list.isEmpty())
		judging_list.push_front(judged_list.takeLast());
	delete pt_markers.back();
	pt_markers.pop_back();
	continuedLineShow();
	replot();
}

void Plot::clearCurrentShowing()
{
	clearAllMemoryPtsDataes();
	initCtrlLine();
	emptyFileCtrLineShow();
	if (title().text() == tr("极差图"))
	{
		QString title(tr("西格玛图"));
		setPlotTitle(title);
	}
	replot();
}

void Plot::setPlotDeployment(const QString & engi, QStandardItem * deploy_item) 
{
	DataSelectModel * infoes_model = qobject_cast<DataSelectModel *>(deploy_item->model());
	TableManagement * infoes_view = infoes_model->nestedInTreeView();
	QStandardItem * info_item = deploy_item; 
	if (!info_item->parent()->text().contains(tr("图")))
	{
		QList<QStandardItem *> guid_list;
		infoes_model -> findItemsSingleLine(info_item, guid_list, infoes_model->item(0)->text());	
		foreach (QStandardItem * item, guid_list)
		{
			guid_list.removeOne(item);
			if (item->text()==tr("均值图") || item->text()==tr("西格玛图") || item->text()==tr("极差图"))
			{
				guid_list.removeOne(item);
				break;
			}
		}
		info_item = guid_list[0];
	}
	QString sequence(tr("次序"));
	QStandardItem * def_item = infoes_view->lookingForMatchItem(info_item, sequence);
	if (!pt_markers.isEmpty())
		pt_markers.clear();
	QStringList cpk_values = db_source->allTables().filter(engi).filter(tr("，，。cpk"));
	foreach (QString tbl, cpk_values)
	{
		if (tbl.contains(tr("，，。cpkdataes")))
		{
			cpk_values.removeOne(tbl);
			break;
		}
	}
	QString engi_info = cpk_values.at(0);	
	QSqlTableModel db_model(this, db_source->currentConnectionDb()->database(db_source->currentConnectionDb()->connectionName()));
	db_source -> varsModelFromTable(engi_info, &db_model);
	double show_up = 0.0, show_low = 0.0;
	QString d_table;
	if (title().text() == tr("均值图"))
	{
		d_table = "dailyavr";
		show_up = db_model.data(db_model.index(0, 4)).toDouble();
		show_low = db_model.data(db_model.index(0, 5)).toDouble();	
	}
	else
	{
		if (engi_info.contains("cpkdev"))
			d_table = "dailydev";
		else
			d_table = "dailyrng";	
		show_up = db_model.data(db_model.index(0, 6)).toDouble();
		show_low = db_model.data(db_model.index(0, 7)).toDouble();	
	}
	infoes_model -> findExistedRelatedTable(engi, d_table, db_source->allTables(), dest_table);		
	fileCtrLineShow(show_up, show_low);
	QSqlQueryModel showing_model1(this);
	QSqlTableModel * showing_model2 = 0;
	QString daily_group("groupnum");
	QString person(tr("人员"));
	bool person_existed = infoes_view->findMatchBranch(info_item, person);
	if (!person_existed)
	{
		int from_group = 0;
		int to_group = 0;
		if (def_item)
		{
			QStandardItem * def_distance = def_item->child(0);
			if (def_distance->text() == tr("继续组范围"))
			{
				from_group = infoes_view->hashSavedRelatedItemInfo().value(def_distance->child(0)).at(0).toInt();
				if (def_distance->child(1))
					to_group = infoes_view->hashSavedRelatedItemInfo().value(def_distance->child(1)).at(0).toInt();
				if (!to_group)
				{
					QDateTime to_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(def_item->parent()).at(0), Qt::ISODate);
					to_group = db_source->dataFromTableByTime(to_time, dest_table, daily_group).toInt();
				}
			}
			else
			{
				QDateTime from_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(def_distance->child(0)).at(0), Qt::ISODate);
				from_group = db_source->dataFromTableByTime(from_time, dest_table, daily_group).toInt();
				QDateTime to_time;
				if (def_distance->child(1))
					to_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(def_distance->child(1)).at(0), Qt::ISODate);
				if (to_time.isNull())
					to_group = infoes_view->hashSavedRelatedItemInfo().value(def_item->parent()).at(0).toInt();
				else
					to_group = db_source->dataFromTableByTime(to_time, dest_table, daily_group).toInt();
			}
		}
		else
		{
			if (info_item->text().contains(tr("按组序选择")))
			{
				from_group = infoes_view->hashSavedRelatedItemInfo().value(info_item->child(0)->child(0)).at(0).toInt();
				if (info_item->child(0)->child(1))
					to_group = infoes_view->hashSavedRelatedItemInfo().value(info_item->child(0)->child(1)).at(0).toInt();
				if (!to_group)
					to_group = db_source->lastDataFromTable(dest_table, daily_group).toInt();						
			}
			else
			{
				QDateTime from_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(info_item->child(0)->child(0)).at(0), Qt::ISODate);
				from_group = db_source->dataFromTableByTime(from_time, dest_table, daily_group).toInt();
				QDateTime to_time;
				if (info_item->child(0)->child(1))
					to_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(info_item->child(0)->child(1)).at(0), Qt::ISODate);
				if (to_time.isNull())
					to_group = db_source->lastDataFromTable(dest_table, daily_group).toInt();
				else
					to_group = db_source->dataFromTableByTime(to_time, dest_table, daily_group).toInt();				
			}
		}
		db_source -> dataesByGroupsFromDb(from_group, to_group, dest_table, &showing_model1);	
		if (!to_group)
			to_group = showing_model1.data(showing_model1.index(showing_model1.rowCount()-1, 0)).toInt();
		for (int i = 0; i < showing_model1.rowCount(); i++)
		{
			if (i > 16)
			{
				to_group = showing_model1.data(showing_model1.index(i-1, 0)).toInt();
				break;
			}
			QwtPlotMarker * tmp_marker = new QwtPlotMarker;
			double y_val = showing_model1.data(showing_model1.index(i, 1)).toDouble();
			tmp_marker -> setYValue(y_val);
			tmp_marker -> setXValue(i+from_group);
			QString color_info = showing_model1.data(showing_model1.index(i, 2)).toString();
			if (color_info.contains(" endnormal"))
				tmp_marker -> setSymbol(ptColorDef(2));
			else if (color_info.contains("unormal"))
				tmp_marker -> setSymbol(ptColorDef(1));
			else
				tmp_marker -> setSymbol(ptColorDef(0));
			tmp_marker -> attach(this);
			pt_markers << tmp_marker;
		}
		b_left = from_group;
		b_right = to_group;
		all_pts = showing_model1.rowCount();
		continuedLineShow();
		continuedPtsShow(false);
	}
	else
	{
		QString human(tr("链上人员"));
		QStandardItem * person_item;
		QStandardItem * p_human = infoes_view->lookingForMatchItem(deploy_item, human);
		int i = 0;
		if (!p_human)
		{
			while (deploy_item->child(0)->child(i))//?
			{
				if (deploy_item->child(0)->child(i)->foreground() != QBrush(Qt::gray))
				{
					person_item = deploy_item->child(0)->child(i);
					break;
				}
				i++;
			}
		}
		else
		{
			while (p_human->child(i))//?
			{
				if (p_human->child(i)->foreground() != QBrush(Qt::gray))
				{
					person_item = p_human->child(i);
					break;
				}
				i++;
			}
		}
		QStringList groups_list;
		QString person_col("person");
		QString empty;
		db_source -> getDataesFromTable(dest_table, daily_group, groups_list, empty, person_col, infoes_view->hashSavedRelatedItemInfo().value(person_item).at(0));
		if (def_item)
		{
			QStandardItem * def_distance = def_item->child(0);
			QStandardItem * from_item = 0;
			QStandardItem * to_item = 0;
			QString item_detail;
			if (def_distance->text().contains(tr("继续")))
			{
				if (def_distance->text().contains(tr("人员")))
				{
					from_item = def_item->parent()->parent()->child(0);
					to_item = def_item->parent()->parent()->child(1);
					if (def_item->parent()->text().contains(tr("组")))
						item_detail = tr("组");
					else
						item_detail = tr("时间");
				}
				else
				{
					from_item = def_distance->child(0);
					if (def_distance->child(1))
						to_item = def_distance->child(1);
					if (def_distance->child(0)->text().contains(tr("组")))
						item_detail = tr("组");
					else
						item_detail = tr("时间");
				}

			}
			if (def_distance->text().contains(tr("最终")))
			{
				if (def_distance->text().contains(tr("人员")))
				{
					from_item = def_item->parent()->parent()->child(0);
					to_item = def_item->parent()->parent()->child(1);
					if (def_item->parent()->text().contains(tr("组")))
						item_detail = tr("组");
					else
						item_detail = tr("时间");					
				}
				else
				{
					from_item = def_distance->child(0);
					if (def_distance->child(1))
						to_item = def_distance->child(1);
					else
					{
						if (!def_item->parent()->parent()->text().contains(tr("人员")))
							to_item = def_item->parent()->parent()->child(1);
						else
							to_item = def_item->parent()->parent()->parent()->parent();
					}
					if (def_distance->child(0)->text().contains(tr("组")))
						item_detail = tr("组");
					else
						item_detail = tr("时间");					
				}
			}
			if (item_detail == tr("组"))
			{
				int from_group = infoes_view->hashSavedRelatedItemInfo().value(from_item).at(0).toInt();
				int to_group = 0;
				if (!to_item)
					to_group = db_source->lastDataFromTable(dest_table, daily_group).toInt();
				else
				{
					if (to_item->text().contains(tr("组")))
						to_group = infoes_view->hashSavedRelatedItemInfo().value(to_item).at(0).toInt();
					else
					{
						QDateTime to_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(to_item).at(0), Qt::ISODate);
						to_group = db_source->dataFromTableByTime(to_time, dest_table, daily_group).toInt();
					}
				}
				db_source -> dataesByGroupsFromDb(from_group, to_group, dest_table, &showing_model1);						
			}
			else
			{
				QDateTime from_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(from_item).at(0), Qt::ISODate);
				QDateTime to_time;
				QString time_var("time");
				int last_group = db_source->lastDataFromTable(dest_table, daily_group).toInt();
				QString which_grp = daily_group+"="+QString("%1").arg(last_group);
				if (title().text() == tr("均值图"))
				{
					if (!to_item)
						to_time = db_source->dataFromTable(dest_table, time_var, which_grp).toDateTime();
					else
					{
						if (to_item->text().contains(tr("时间")))
							to_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(to_item).at(0), Qt::ISODate);
						else
						{
							int to_group = infoes_view->hashSavedRelatedItemInfo().value(to_item).at(0).toInt();
							QString avr_grp = daily_group+"="+QString("%1").arg(to_group);
							to_time = db_source->dataFromTable(dest_table, time_var, avr_grp).toDateTime();
						}						
					}
					db_source -> dataesByTimeVarFromDb(from_time, to_time, dest_table, &showing_model1);
				}			
				else if (title().text() == tr("西格玛图"))
				{
					if (!to_item)
						to_time = db_source->dataFromTable(dest_table, time_var, which_grp).toDateTime();
					else
					{
						if (to_item->text().contains(tr("时间")))
							to_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(to_item).at(0), Qt::ISODate);
						else
						{
							int to_group = infoes_view->hashSavedRelatedItemInfo().value(to_item).at(0).toInt();
							QString dev_grp = daily_group+"="+QString("%1").arg(to_group);
							to_time = db_source->dataFromTable(dest_table, time_var, dev_grp).toDateTime();
						}
					}
					db_source -> dataesByTimeVarFromDb(from_time, to_time, dest_table, &showing_model1);	
				}		
				else
				{
					if (!to_item)
						to_time = db_source->dataFromTable(dest_table, time_var, which_grp).toDateTime();
					else
					{
						if (to_item->text().contains(tr("时间")))
							to_time = QDateTime::fromString(infoes_view->hashSavedRelatedItemInfo().value(to_item).at(0), Qt::ISODate);
						else
						{
							int to_group = infoes_view->hashSavedRelatedItemInfo().value(to_item).at(0).toInt();
							QString rng_grp = daily_group+"="+QString("%1").arg(to_group);
							to_time = db_source->dataFromTable(dest_table, time_var, rng_grp).toDateTime();
						}
					}
					db_source -> dataesByTimeVarFromDb(from_time, to_time, dest_table, &showing_model1);
				}
			}
		}
		else
		{
			showing_model2 = new QSqlTableModel(this);
			if (title().text() == tr("均值图"))
				db_source -> varsModelFromTable(dest_table, showing_model2);			
			else if (title().text() == tr("西格玛图"))
				db_source -> varsModelFromTable(dest_table, showing_model2);			
			else
				db_source -> varsModelFromTable(dest_table, showing_model2);			
		}
		QList<double> x_markers;
		QList<QwtPlotMarker *> continued_markers;
		if (showing_model2)
		{
			for (int i = 0; i < showing_model2->rowCount(); i++)
			{
				if (x_markers.size() > 15)
					break;
				if (groups_list.contains(showing_model2->data(showing_model2->index(i, 0)).toString()))
				{
					int from_copy = groups_list.indexOf(showing_model2->data(showing_model2->index(i, 0)).toString());
					if (from_copy < groups_list.size()-1) 
						groups_list = groups_list.mid(from_copy+1);
					else 
						groups_list.removeOne(showing_model2->data(showing_model2->index(i, 0)).toString());
					x_markers << showing_model2->data(showing_model2->index(i, 0)).toDouble();
					QwtPlotMarker * tmp_marker = new QwtPlotMarker;
					double y_val = showing_model2->data(showing_model2->index(i, 1)).toDouble();
					tmp_marker -> setYValue(y_val);
					tmp_marker -> setXValue(x_markers.size()-1);
					QString color_info = showing_model2->data(showing_model2->index(i, 2)).toString();
					if (color_info.contains(" endnormal"))
						tmp_marker -> setSymbol(ptColorDef(2));
					else if (color_info.contains("unormal"))
						tmp_marker -> setSymbol(ptColorDef(1));
					else
						tmp_marker -> setSymbol(ptColorDef(0));
					tmp_marker -> attach(this);
					pt_markers << tmp_marker;
					if (continued_markers.size() > 0)
					{
						double x_val1 = x_markers.at(x_markers.size()-2);
						double x_val2 = x_markers.at(x_markers.size()-1);
						if (x_val2-x_val1 == 1)
						{
							continued_markers << tmp_marker;
							if ((groups_list.isEmpty() || x_markers.size()==16) && continued_markers.size()>1)
								uncontinuedLineShow(continued_markers);
						}
						else
						{
							bool drawed = false;
							if (continued_markers.size()>1)
							{
								uncontinuedLineShow(continued_markers);
								drawed = true;
							}
							if (drawed || continued_markers.size()==1)
							{
								continued_markers.clear();
								continued_markers << tmp_marker;
							}							
						}
					}
					else
						continued_markers << tmp_marker;
				}
			}
			all_pts = showing_model2->rowCount();		
		}
		else
		{
			for (int i = 0; i < showing_model1.rowCount(); i++)
			{
				if (x_markers.size() > 15)
					break;
				if (groups_list.contains(showing_model1.data(showing_model1.index(i, 0)).toString()))
				{
					int from_copy = groups_list.indexOf(showing_model1.data(showing_model1.index(i, 0)).toString());
					if (from_copy < groups_list.size()-1) 
						groups_list = groups_list.mid(from_copy+1);
					else 
						groups_list.removeOne(showing_model1.data(showing_model1.index(i, 0)).toString());
					x_markers << showing_model1.data(showing_model1.index(i, 0)).toDouble();
					QwtPlotMarker * tmp_marker = new QwtPlotMarker;
					double y_val = showing_model1.data(showing_model1.index(i, 1)).toDouble();
					tmp_marker -> setYValue(y_val);
					tmp_marker -> setXValue(x_markers.size()-1);
					QString color_info = showing_model1.data(showing_model1.index(i, 2)).toString();
					if (color_info.contains(" endnormal"))
						tmp_marker -> setSymbol(ptColorDef(2));
					else if (color_info.contains("unormal"))
						tmp_marker -> setSymbol(ptColorDef(1));
					else
						tmp_marker -> setSymbol(ptColorDef(0));
					tmp_marker -> attach(this);
					pt_markers << tmp_marker;
					if (continued_markers.size() > 0)
					{
						double x_val1 = x_markers.at(x_markers.size()-2);
						double x_val2 = x_markers.at(x_markers.size()-1);
						if (x_val2-x_val1 == 1)
						{
							continued_markers << tmp_marker;
							if ((groups_list.isEmpty() || x_markers.size()==16) && continued_markers.size()>1)
								uncontinuedLineShow(continued_markers);
						}
						else
						{
							bool drawed = false;
							if (continued_markers.size()>1)
							{
								uncontinuedLineShow(continued_markers);
								drawed = true;
							}
							if (drawed || continued_markers.size()==1)
							{
								continued_markers.clear();
								continued_markers << tmp_marker;
							}							
						}
					}
					else
						continued_markers << tmp_marker;
				}
			}
			all_pts = showing_model1.rowCount();
		}
		UncontinueAxelMarkers * x_uncon = new UncontinueAxelMarkers(this);
		x_uncon -> setRealAxelMarkers(x_markers);
		setAxisScaleDiv(QwtPlot::xBottom, x_uncon->axisesScaleDiv());    
    		setAxisScaleDraw(QwtPlot::xBottom, x_uncon);		
	}
	if (showing_model2)
		delete showing_model2;
	replot();	
}

void Plot::setPlotTitle(const QVariant & title)
{
 	QwtText new_title(title.toString()); 
	if (title == tr("还原"))
		setTitle(plot_title);
	else if (!title.toString().isEmpty())
		setTitle(new_title);	
}

void Plot::toggleAreaLinesShowingState()
{
	l_middleCtrl -> setVisible(!l_middleCtrl->isVisible());
	l_upMiddleCtrl1 -> setVisible(!l_upMiddleCtrl1->isVisible());
	l_upMiddleCtrl2 -> setVisible(!l_upMiddleCtrl2->isVisible());
	l_underMidCtrl1 -> setVisible(!l_underMidCtrl1->isVisible());
	l_underMidCtrl2 -> setVisible(!l_underMidCtrl2->isVisible());
	l_underMidText -> setVisible(!l_underMidText->isVisible());
	replot(); 
}

void Plot::toggleCtrlLinesShowingState()
{
 	l_upCtrl -> setVisible(!l_upCtrl->isVisible());
	l_underCtrl -> setVisible(!l_underCtrl->isVisible());
	replot(); 
}

void Plot::showHideDotsLines()
{
	line -> setVisible(!line->isVisible());
	replot();
}

void Plot::toggleXaxisLabels()
{
	UncontinueAxelMarkers * x_draw = static_cast<UncontinueAxelMarkers *>(axisScaleDraw(QwtPlot::xBottom));	
	bool time_xaxis = chkXaxisTimeNumShow();
	if (time_xaxis)
	{
		foreach (QwtPlotMarker * point, pt_markers)
			x_draw -> resetXaxisTicksLbls(dest_table, axisFont(QwtPlot::xBottom), point->xValue(), db_source, false);
	}
	else
	{
		foreach (QwtPlotMarker * point, pt_markers)
			x_draw -> resetXaxisTicksLbls(dest_table, axisFont(QwtPlot::xBottom), point->xValue(), db_source, true);
	}
	updateAxes();
}
	
bool Plot::chkAreaLinesShowingState()
{
	return l_middleCtrl->isVisible();
}
	
bool Plot::chkCtrlLinesShowingState()
{
 	return l_upCtrl->isVisible(); 
}
	
bool Plot::chkDotsLinesShowingState()
{
 	return line->isVisible();  
}

bool Plot::chkXaxisTimeNumShow()
{
 	UncontinueAxelMarkers * x_draw = static_cast<UncontinueAxelMarkers *>(axisScaleDraw(QwtPlot::xBottom));
	QwtText find_txt = x_draw->chkTickLabel(axisFont(QwtPlot::xBottom), pt_markers.at(0)->xValue());
	bool r_bool = false;
	QDateTime find_time = QDateTime::fromString(find_txt.text(), Qt::ISODate); 
	if (find_time.isValid())
		r_bool = true;
	return r_bool;
}

double Plot::relatedUpCtrlLimit()
{
	return l_upCtrl->yValue();
}
	
double Plot::relatedLowCtrlLimit()
{
	return l_underCtrl->yValue();
}
	
QwtPlotMarker * Plot::ctrlLineShowPtr()
{
	return l_upCtrl;
}
	
QwtPlotMarker * Plot::areaLineShowPtr()
{
	return l_middleCtrl;  
}
	
QwtPlotCurve * Plot::dotsLineShowPtr()
{
	return line; 
}

const QString & Plot::relatedPtJudgedState(int num)
{
	return relatedDataState[num];
}

QList<double> & Plot::ptJudgeStack(int type)
{
	if (type == 3)
		return three_avrJud;
	else if (type == 5)
		return five_avrJud;
	else if (type == 6)
		return six_avrJud;
	else if (type == 8)
		return eight_devJud;
	else if (type == 90)
		return nine_avrJud;
	else if (type == 91)
		return nine_devJud;
	else if (type == 140)
		return fourteen_avrJud;
	else if (type == 141)
		return fourteen_devJud;
	else if (type == 15)
		return fifteen_devJud;
	return judged_list;
}

QMap<int, QString> & Plot::relatedPtJudgedState()
{
	return relatedDataState;
}

void Plot::setCanvasColor(const QString & color)
{
	if (color == "darkGreen")
		canvas()->setStyleSheet("background-color: #008B8B");
	else if (color == "SteelBlue")
		canvas()->setStyleSheet("background-color: #4682B4");
	else
		canvas()->setStyleSheet("background-color: #708090");
	replot();
}

void Plot::initCtrlLine()
{
	l_upCtrl = new QwtPlotMarker();
	l_middleCtrl = new QwtPlotMarker();
	l_underCtrl = new QwtPlotMarker();
	l_upMiddleCtrl1 = new QwtPlotMarker();
	l_upMiddleCtrl2 = new QwtPlotMarker();
	l_underMidCtrl1 = new QwtPlotMarker();
	l_underMidCtrl2 = new QwtPlotMarker();
	l_underMidText = new QwtPlotMarker();
}

void Plot::clearAllMemoryPtsDataes()
{
	if (!pt_markers.isEmpty())
	{
		for(int i = 0; i < pt_markers.size(); i++)
			delete pt_markers[i];
		pt_markers.clear();
	}
	if (l_upCtrl)
	{
		delete l_upCtrl;
		l_upCtrl = 0;
		delete l_middleCtrl;
		l_middleCtrl = 0;
		delete l_underCtrl;
		l_underCtrl = 0;
	}
	if (l_upMiddleCtrl1)
	{
		delete l_upMiddleCtrl1;
		l_upMiddleCtrl1 = 0;
		delete l_upMiddleCtrl2;
		l_upMiddleCtrl2 = 0;
		delete l_underMidCtrl1;
		l_underMidCtrl1 = 0;
		delete l_underMidCtrl2;
		l_underMidCtrl2 = 0;
		delete l_underMidText;
		l_underMidText = 0;				
	}
	if (line)
	{
		delete line;
		line = 0;
	}
	three_avrJud.clear();
	five_avrJud.clear();
	six_avrJud.clear();
	eight_devJud.clear();
	nine_avrJud.clear();
	nine_devJud.clear();
	fourteen_avrJud.clear();
	fourteen_devJud.clear();
	fifteen_devJud.clear();
	xaxel_pairs.clear();
	relatedDataState.clear();
	judging_list.clear();
	judged_list.clear();
}

void Plot::fileCtrLineShow(double up, double low)
{
	double mid = (low+up)/2;
	double unit = (up-low)/8;
	double m_unit = (up-low)/6;
	l_upCtrl->setValue(0.0, up);
	l_upCtrl->setLineStyle(QwtPlotMarker::HLine);
	l_upCtrl->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
	l_upCtrl->setLinePen(QPen(Qt::yellow, 0, Qt::DashDotLine));
	QString up_label = QString("UCL=%1").arg(up);
	QwtText up_text(up_label);
	up_text.setFont(QFont("UTF-8",10, QFont::Bold));
	up_text.setColor(QColor(200,150,0));
	l_upCtrl->setLabel(up_text);
	l_upCtrl->attach(this);
	l_upMiddleCtrl1->setValue(0.0, up-m_unit);
	l_upMiddleCtrl1->setLineStyle(QwtPlotMarker::HLine);
	l_upMiddleCtrl1->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
	l_upMiddleCtrl1->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
	QString upMiddle_label1 = "A";
	QwtText upMiddle_text1(upMiddle_label1);
	upMiddle_text1.setFont(QFont("UTF-8",10, QFont::Bold));
	upMiddle_text1.setColor(QColor(200,150,0));
	l_upMiddleCtrl1->setLabel(upMiddle_text1);
	l_upMiddleCtrl1->attach(this);
	l_upMiddleCtrl2->setValue(0.0, up-2*m_unit);
	l_upMiddleCtrl2->setLineStyle(QwtPlotMarker::HLine);
	l_upMiddleCtrl2->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
	l_upMiddleCtrl2->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
	QString upMiddle_label2 = "B";
	QwtText upMiddle_text2(upMiddle_label2);
	upMiddle_text2.setFont(QFont("UTF-8",10, QFont::Bold));
	upMiddle_text2.setColor(QColor(200,150,0));
	l_upMiddleCtrl2->setLabel(upMiddle_text2);
	l_upMiddleCtrl2->attach(this);
	l_middleCtrl->setValue(0.0, mid);
	l_middleCtrl->setLineStyle(QwtPlotMarker::HLine);
	l_middleCtrl->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
	l_middleCtrl->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
	QString middle_label = "C";
	QwtText middle_text(middle_label);
	middle_text.setFont(QFont("UTF-8",10, QFont::Bold));
	middle_text.setColor(QColor(200,150,0));
	l_middleCtrl->setLabel(middle_text);
	l_middleCtrl->attach(this);
	l_underMidCtrl1->setValue(0.0, mid-m_unit);
	l_underMidCtrl1->setLineStyle(QwtPlotMarker::HLine);
	l_underMidCtrl1->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
	l_underMidCtrl1->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
	QString underMid_label1 = "C";
	QwtText underMid_text1(underMid_label1);
	underMid_text1.setFont(QFont("UTF-8",10, QFont::Bold));
	underMid_text1.setColor(QColor(200,150,0));
	l_underMidCtrl1->setLabel(underMid_text1);
	l_underMidCtrl1->attach(this);
	l_underMidCtrl2->setValue(0.0, mid-2*m_unit);
	l_underMidCtrl2->setLineStyle(QwtPlotMarker::HLine);
	l_underMidCtrl2->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
	l_underMidCtrl2->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
	QString underMid_label2 = "B";
	QwtText underMid_text2(underMid_label2);
	underMid_text2.setFont(QFont("UTF-8",10, QFont::Bold));
	underMid_text2.setColor(QColor(200,150,0));
	l_underMidCtrl2->setLabel(underMid_text2);
	l_underMidCtrl2->attach(this);
	l_underMidText->setValue(0.0, low);
	l_underMidText->setLineStyle(QwtPlotMarker::HLine);
	l_underMidText->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
	l_underMidText->setLinePen(QPen(Qt::yellow, 0, Qt::DashDotLine));
	QString underMid_label3 = "A";
	QwtText underMid_text3(underMid_label3);
	underMid_text3.setFont(QFont("UTF-8",10, QFont::Bold));
	underMid_text3.setColor(QColor(200,150,0));
	l_underMidText->setLabel(underMid_text3);
	l_underMidText->attach(this);
	l_underCtrl->setValue(0.0, low);
	l_underCtrl->setLineStyle(QwtPlotMarker::HLine);
	l_underCtrl->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
	l_underCtrl->setLinePen(QPen(Qt::yellow, 0, Qt::DashDotLine));
	QString under_label = QString("LCL=%1").arg(low);
	QwtText under_text(under_label);
	under_text.setFont(QFont("UTF-8",10, QFont::Bold));
	under_text.setColor(QColor(200,150,0));
	l_underCtrl->setLabel(under_text);
	l_underCtrl->attach(this);
	setAxisScale(QwtPlot::yLeft, low-unit, up+unit);	
}

void Plot::emptyFileCtrLineShow()
{
	l_upCtrl->setValue(0.0, 10.0);
	l_upCtrl->setLineStyle(QwtPlotMarker::HLine);
	l_upCtrl->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
	l_upCtrl->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
	QString up_label = "UCL=10";
	QwtText up_text(up_label);
	up_text.setFont(QFont("UTF-8",10, QFont::Bold));
	up_text.setColor(QColor(200,150,0));
	l_upCtrl->setLabel(up_text);
	l_upCtrl->attach(this);
	if (!l_middleCtrl->label().isEmpty())
		l_middleCtrl->setLabel(tr(""));
	l_middleCtrl->setValue(0.0, 5.5);
	l_middleCtrl->setLineStyle(QwtPlotMarker::HLine);
	l_middleCtrl->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
	l_middleCtrl->attach(this);
	l_underCtrl->setValue(0.0, 1.0);
	l_underCtrl->setLineStyle(QwtPlotMarker::HLine);
	l_underCtrl->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
	l_underCtrl->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
	QString under_label = "LCL=1.0";
	QwtText under_text(under_label);
	under_text.setFont(QFont("UTF-8",10, QFont::Bold));
	under_text.setColor(QColor(200,150,0));
	l_underCtrl->setLabel(under_text);
	l_underCtrl->attach(this);
	setAxisScale(QwtPlot::yLeft, 0.0, 11.0);
	setAxisScale(QwtPlot::xBottom, b_left, b_right);	
}

void Plot::continuedPtsShow(bool multi)
{
	if (all_pts == 0)
		return;
	if (multi)
	{
		if (all_pts > 16)
		{
			b_left = all_pts-16;
			b_right = all_pts;
		}
		else
		{
			b_left = 0;
			b_right = 16;
		}
		QList<double> ys;
		if (title().text() != tr("极差图"))
		{
			ys = judged_list;
			ys += judging_list;
		}
		else
			ys = judging_list;
		for (int i = all_pts-ys.size(); i < all_pts; i++)
		{
			QwtPlotMarker * tmp_marker = new QwtPlotMarker;
			tmp_marker -> setYValue(ys.at(i));
			tmp_marker -> setXValue(i+1);
			QString color_info = relatedDataState[all_pts-ys.size()+i+1];			
			if (color_info.contains(" endnormal"))
				tmp_marker -> setSymbol(ptColorDef(2));
			else if (color_info.contains("unormal"))
				tmp_marker -> setSymbol(ptColorDef(1));
			else
				tmp_marker -> setSymbol(ptColorDef(0));
			tmp_marker -> attach(this);
			pt_markers << tmp_marker;
		}
		continuedLineShow();
	}
	setAxisScale(QwtPlot::xBottom, b_left, b_right);
}

void Plot::continuedLineShow()
{
	if (line)
	{
		delete line;
		line = NULL;
	}
	line = new QwtPlotCurve();
	QVector<double> xDataes;
	QVector<double> yDataes;
	for (int i = 0; i < pt_markers.size(); i++)
	{
		xDataes << pt_markers.at(i)->xValue();
		yDataes << pt_markers.at(i)->yValue();
	}
	line -> setSamples(xDataes, yDataes);
	line -> attach(this);	
}

void Plot::uncontinuedLineShow(const QList<QwtPlotMarker *> & plot_markers)
{
	line = new QwtPlotCurve();
	QVector<double> xDataes;
	QVector<double> yDataes;
	for (int i = 0; i < plot_markers.size(); i++)
	{
		xDataes << plot_markers.at(i)->xValue();
		yDataes << plot_markers.at(i)->yValue();
	}
	line -> setSamples(xDataes, yDataes);
	line -> attach(this);
}

void Plot::initJudgLists(const QString & description)
{
	if (all_pts <= 100)
	{
		if (title().text() == tr("均值图"))
		{
			QString varavr = "avr";
			QStringList curtol_avr;
			db_source -> getDataesFromTable(description, varavr, curtol_avr);
			QList<double> trans_avr;
			for (int j = 0; j < curtol_avr.size(); j++)
				trans_avr << curtol_avr.at(j).toDouble();
			if (trans_avr.size() > 13)
			{
				for (int i = 0; i < 13; i++)
					judging_list.push_front(trans_avr.takeAt(trans_avr.back()));
				if (!trans_avr.isEmpty())
					judged_list = trans_avr;
			}
			else
				judging_list = trans_avr;
			fourteen_avrJud = judging_list;
			nine_avrJud = judging_list;
			six_avrJud = judging_list;
			five_avrJud = judging_list;
			three_avrJud = judging_list;
			for (int i = 2; i < 13; i++)
			{
				if (judging_list.size()-i-1 >= 0)
				{
					three_avrJud.pop_front();
					if (i > 3)
						five_avrJud.pop_front();
					if (i > 4)	
						six_avrJud.pop_front();
					if (i > 10)
						nine_avrJud.pop_front();
				}
				else
					break;
			}
		}
		else if (title().text() == tr("西格玛图"))
		{
			QString vardev = "dev";
			QStringList curtol_dev;
			db_source -> getDataesFromTable(description, vardev, curtol_dev);
			QList<double> trans_dev;
			trans_dev.clear();
			for (int j = 0; j < curtol_dev.size(); j++)
				trans_dev << curtol_dev.at(j).toDouble();
			if (trans_dev.size() > 14)
			{
				for (int i = 0; i < 14; i++)
					judging_list.push_front(trans_dev.takeAt(trans_dev.back()));
				if (!trans_dev.isEmpty())
					judged_list = trans_dev;
			}
			else
				judging_list = trans_dev;
			fifteen_devJud = judging_list;
			fourteen_devJud = judging_list;
			nine_devJud = judging_list;
			eight_devJud = judging_list;
			for (int i = 7; i < 14; i++)
			{
				if (judging_list.size()-i-1 >= 0)
				{
					eight_devJud.pop_front();
					if (i > 7)
						nine_devJud.pop_front();
					if (i > 12)	
						fourteen_devJud.pop_front();
				}
				else
					break;
			}		
		}
		else
		{
			QString varrng = "rng";
			QStringList curtol_rng;
			db_source->getDataesFromTable(description, varrng, curtol_rng);
			for (int i = 0; i < curtol_rng.size(); i++)
				judged_list << curtol_rng.at(i).toDouble();			
		}	
	}	
}

void Plot::initRelatedDataState(const QString & description)
{
	QString table = description;
	QString var = "state";
	if (all_pts <= 100)
	{
		QStringList r_states;
		db_source -> getDataesFromTable(table, var, r_states);
		for (int i = 0; i < r_states.size(); i++)
			relatedDataState[i+1] = r_states.at(i);		
	}
	else
	{
		QStringList r_states = db_source->desVarDataesFromTable(table, var, all_pts-100, all_pts);
		for (int i = 0; i < r_states.size(); i++)
			relatedDataState[all_pts-100+i] = r_states.at(i);		
	}
}

void Plot::redefPtsColorForNewAvrPt()
{
	int m = pt_markers.size();
	int total_back = relatedDataState.keys().back();
	QString new_state = relatedDataState[total_back];
	if (total_back > 2)
	{
		if (new_state.contains("unormal"))
			pt_markers[m-1] -> setSymbol(ptColorDef(1));
		else
			pt_markers[m-1] -> setSymbol(ptColorDef(0));
		if (total_back > 13)
		{
			QString fourteen_state = relatedDataState[total_back-13];
			if (fourteen_state.contains(" endnormal"))
				pt_markers[m-14] -> setSymbol(ptColorDef(2));
		}
		int head;
		if (m < 14)
			head = m;
		else
			head = 14;						
		for (int j = m-head; j < m; j++)
		{
			QString end_state = relatedDataState[total_back-m+j+1];
			PtSymbol * sym = (PtSymbol *)pt_markers[j]->symbol();
			QString endState = sym->imagePath();
			if (endState.contains("mid") && end_state.contains("unormal"))
				pt_markers[j] -> setSymbol(ptColorDef(1));	
		}
	}
	else
	{
		if (new_state.contains("unormal"))
			pt_markers[m-1] -> setSymbol(ptColorDef(1));
		else
			pt_markers[m-1] -> setSymbol(ptColorDef(0));
	}
}
	
void Plot::redefPtsColorForNewDevPt()
{
	int m = pt_markers.size();
	int total_back = relatedDataState.keys().back();
	QString new_state = relatedDataState[total_back];
	if (total_back > 7)
	{
		if (new_state.contains("unormal"))
			pt_markers[m-1] -> setSymbol(ptColorDef(1));
		else
			pt_markers[m-1] -> setSymbol(ptColorDef(0));
		if (total_back > 14)
		{
			QString fifteen_state = relatedDataState[total_back-14];
			if (fifteen_state.contains(" endnormal"))
				pt_markers[m-15] -> setSymbol(ptColorDef(2));
		}
		int head;
		if (m < 15)
			head = m;
		else
			head = 15;						
		for (int j = m-head; j < m; j++)
		{
			QString end_state = relatedDataState[total_back-m+j+1];
			PtSymbol * sym = (PtSymbol *)pt_markers[j]->symbol();
			QString endState = sym->imagePath();
			if (endState.contains("mid") && end_state.contains("unormal"))
				pt_markers[j] -> setSymbol(ptColorDef(1));	
		}
	}
	else
	{
		if (new_state.contains("unormal"))
			pt_markers[m-1] -> setSymbol(ptColorDef(1));
		else
			pt_markers[m-1] -> setSymbol(ptColorDef(0));	
	}
}
	
void Plot::redefPtsColorForNewRngPt()
{}

void Plot::relistPtsValueStacks(double new_val)
{
	judging_list << new_val;
	if (title().text() == tr("均值图") && judging_list.size()>13)
		judged_list.push_back(judging_list.takeFirst());
	if (title().text() == tr("西格玛图") && judging_list.size()>14)
		judged_list.push_back(judging_list.takeFirst());
}

PtSymbol * Plot::ptColorDef(int type)
{
	if (type == 0)
	{
		PtSymbol * mid_sym = new PtSymbol(":/images/mid.png");
		return mid_sym;
	}
	else if (type == 1)
	{
		PtSymbol * red_sym = new PtSymbol(":/images/red.png");
		return red_sym;		
	}
	else
	{
		PtSymbol * green_sym = new PtSymbol(":/images/green.png");
		return green_sym;
	}	
}

/*QwtScaleDraw Plot::defineXaxisScaleDraw(const QList<double> & markers) const
{
	QList<double> ticks[QwtScaleDiv::NTickTypes];
	QList<double> &majorTicks = ticks[QwtScaleDiv::MajorTick];
	foreach (double marker, markers)
		majorTicks += marker;
	QwtScaleDiv scaleDiv(majorTicks.first(), majorTicks.last(), ticks);
	return scaleDiv;	
}

void Plot::scrollLeftAxis(double value)
{
	setAxisScale(yLeft, value, value + 100.0);
	replot();
}*/

