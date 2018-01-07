#include <QtGui>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>
#include "dataestableview.h"
#include "dataestableeditcommand.h"
#include "editlongtoolsviewer.h"
#include "engitreeview.h"
#include "dialog.h"
#include "plotwidget.h"
#include "spcdatabase.h"
#include "spcnum.h"
#include "littlewidgetsview.h"
#include "littlewidgetitem.h"
#include "mainwindow.h"

DataesTableView::DataesTableView(QWidget * parent, int type, SpcDataBase * val, bool edit)
:QTableView(parent), input_spcnum(false), cursor_model(0), copy_specs(0), press_time(0), multi_viewer(0), union_viewer(0), improve_tree(0), nested_animation(0), model_deleteStack(0), current_plots(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setTextElideMode(Qt::ElideNone);
	setStyleSheet("font-family: Microsoft YaHei; font-size:18px; border: 0px groove gray; border-radius: 4px;");	
	data_db = val;
	t_type = type; 	
	can_edit = edit;
	m_commandStack = new QUndoStack(this);
	m_commandStack -> setObjectName("inputComm");
	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(cellClicked(const QModelIndex &)));	
}

DataesTableView::~DataesTableView()
{
	if (press_time)
		delete press_time;
}

void DataesTableView::initTable(const QString & engi, DataesTableView * partner, const QList<double> & vals_list)
{
	QString thisEngi =  tr("departments");
	QTableView * view = new QTableView;
	QSqlTableModel * q_model = new QSqlTableModel(0, data_db->currentConnectionDb()->database(data_db->currentConnectionDb()->connectionName()));
	data_db->varsModelFromTable(thisEngi, q_model);
	view->setModel(q_model);
	view->show();
	QStandardItemModel * init_model = new QStandardItemModel(this);	
	if (t_type != 4)
		setModel(init_model);	
	connect(init_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(changeViewStateForNewModel()));	
	if (t_type != 0)
		horizontalHeader() -> close();
	if (partner && vals_list.isEmpty())
		initDifferentDataesModel(init_model, partner);
	else if (partner && !vals_list.isEmpty())
		initDifferentDataesModel(init_model, partner, engi, vals_list);
	else
		initDifferentDataesModel(init_model, 0, engi);	
	if (t_type==0 || t_type==3 || t_type==4)
	{
		QString manual_time;
		QPair<QString, QStandardItemModel *> manual_pair(manual_time, init_model);
		if (!projs_models.contains(tr("草稿工程，，。")))
			projs_models[tr("草稿工程，，。")] << manual_pair;
		if (t_type==3 || (improve_tree && !model_deleteStack))
		{
			model_deleteStack = new QUndoStack(this);
			model_deleteStack -> setObjectName("delComm");
		}
	}
	changeViewStateForNewModel();
	if (!horizontalHeader()->isHidden())
		horizontalHeader() -> setStyleSheet(QString("QHeaderView{border: none} QHeaderView::section{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop:0 #bec1d2, stop: 0.4 #717990, stop: 0.5 #5c637d, stop:1 #68778e); border-width: 1px; border-color: #339; border-style: solid; border-radius: 3; padding: 3px; font-size: 14px; padding-left: 2px; padding-right: 2px; min-width: %1px; min-height: 13px; max-height: 13px;}").arg(horizontalHeader()->width()));
	verticalHeader() -> setStyleSheet(QString("QHeaderView{border: none} QHeaderView{background-color: white} QHeaderView::section{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop:0 #bec1d2, stop: 0.4 #717990, stop: 0.5 #5c637d, stop:1 #68778e); border-width: 1px; border-color: #339; border-style: solid; border-radius: 3; padding: 3px; font-size: 14px; padding-left: 2px; padding-right: 2px; min-width: %1px; max-width: %2px; min-height: 13px; max-height: 13px;}").arg(verticalHeader()->width()).arg(verticalHeader()->width()));	
}

void DataesTableView::setInputor(QPair<QString, QString> & name)
{
	inputor = name;
}

void DataesTableView::setRightOrigRect(const QRect & r_size)
{
	right_origin = r_size;
	if (height() > right_origin.height())
		emit sizeChanged();
}

void DataesTableView::setPlotsBackGuide(PlotWidget * plots)
{
	current_plots = plots;
}

void DataesTableView::setNewNextIndex(const QModelIndex & index)
{
	next_index = index;
}

void DataesTableView::clearRelatedCellsForNextInput()
{
	if (t_type == 0)
	{
		for (int i = 0; i < model()->rowCount(); i++)
		{
			for (int j = 0; j < model()->columnCount(); j++)
				model() -> setData(model()->index(i, j), "");
		}
	}
	else if (t_type == 2)
	{
		int new_group = model()->data(model()->index(0, 0)).toInt()+1;
		model() -> setData(model()->index(0, 0), new_group);
		for (int i = 1; i < model()->rowCount(); i++)
		{
			if (i == model()->rowCount()-5)
				continue;
			model() -> setData(model()->index(i, 0), "");
		}
	}
	else if (t_type == 3)
	{
		for (int i = 0; i < model()->rowCount(); i++)
			model() -> setData(model()->index(i, 0), "");
	}
	else
		return;
}

void DataesTableView::setCalResDataesForDaily()
{
	dataes_indexes.clear();
 	SpcNum * calculator = qobject_cast<SpcNum *>(data_db->parent()); 
	QString cell_fill;
	calculator -> dbSpcNamesTranslation(current_plots->relatedJudgedStateMap(0).value(current_plots->relatedJudgedStateMap(0).keys().back()), cell_fill);
	dataes_indexes[model()->index(model()->rowCount()-1, 0)] = current_plots->relatedJudgedStateMap(0).value(current_plots->relatedJudgedStateMap(0).keys().back());
	model()->setData(model()->index(model()->rowCount()-1, 0), cell_fill);
	calculator -> dbSpcNamesTranslation(current_plots->relatedJudgedStateMap(1).value(current_plots->relatedJudgedStateMap(1).keys().back()), cell_fill);
	dataes_indexes[model()->index(model()->rowCount()-3, 0)] = current_plots->relatedJudgedStateMap(1).value(current_plots->relatedJudgedStateMap(1).keys().back());
	model()->setData(model()->index(model()->rowCount()-3, 0), cell_fill);	
}

void DataesTableView::closeNestedQmlViewerOnCell(EditLongToolsViewer * nested)
{
	nested_animation = new QPropertyAnimation(this);
	connect(nested_animation, SIGNAL(valueChanged(const QVariant &)), nested, SLOT(rectangleAnimating(const QVariant &)));
	connect(nested_animation, SIGNAL(finished()), nested, SLOT(killSelf()));
	connect(nested_animation, SIGNAL(finished()), this, SLOT(clearNestedAnimation()));
	nested_animation -> setTargetObject(nested);
	nested_animation -> setPropertyName("geometry");
	nested_animation->setDuration(500);
	QRect nested_startRect(QRect(nested->frameGeometry().topLeft().x(), nested->frameGeometry().topLeft().y(), nested->width(), 100));
	QRect nested_endRect(QRect(nested->frameGeometry().topLeft().x(), nested->frameGeometry().topLeft().y(), 0, 0));
	nested_animation->setStartValue(nested_startRect);
	nested_animation->setEndValue(nested_endRect);
	nested_animation -> start();
}

void DataesTableView::resetEditState(bool state)
{
	can_edit = state;
	if (!can_edit)
		setEditTriggers(QAbstractItemView::NoEditTriggers);
	if (improve_tree)
	{
		if (can_edit)
			emit changeStackCommOrder(m_commandStack);
		else
			emit changeStackCommOrder(model_deleteStack);
	}
}

void DataesTableView::changeModelForEditting(QStandardItemModel * cpe_model, const QString & lead_time, DataesTableView * guider)
{
	QStandardItemModel * c_model = projs_models.value(tr("草稿工程，，。")).at(0).second;	
	if (guider)
	{
		c_model -> clear();	  
		initDifferentDataesModel(c_model, guider);		
		next_index = c_model->index(0, 0);		
		setModel(c_model);		
	}
	else
	{
		if (c_model == cpe_model)
			return;
		QString g_engi = cpe_model->data(cpe_model->index(0, 0)).toString();		
		c_model -> clear();
		c_model -> setRowCount(cpe_model->rowCount());
		c_model -> setColumnCount(cpe_model->columnCount());
		copyModelDataesToOther(cpe_model, c_model);	
		setModel(c_model);
	}
	projs_models[tr("草稿工程，，。")][0].first = lead_time;
	changeViewStateForNewModel();
}

void DataesTableView::resetTblForProjKeysValues(const QString & destProj, const QString & destModel)
{
	QList<QPair<QString, QStandardItemModel *> > chk_list = projs_models[destProj];
	for (int i = 0; i < chk_list.size(); i++)
	{
		if (chk_list.at(i).first == destModel)
		{
			setModel(chk_list.at(i).second);
			break;
		}
	}
}

void DataesTableView::tmpSaveProjsModels(const QString & saveProj, const QString & testTime)
{
	QList<QPair<QString, QStandardItemModel *> > chk_list = projs_models.value(saveProj); 
	for (int i = 0; i < chk_list.size(); i++)
	{
		if (chk_list.at(i).first.contains(testTime))
			return;
	}
	QStandardItemModel * save_model = new QStandardItemModel(this);
	copyModelDataesToOther(qobject_cast<QStandardItemModel *>(model()), save_model);
	if ((t_type==4 || t_type==0) && projs_models.value(tr("草稿工程，，。")).at(0).second == model())
		projs_models[tr("草稿工程，，。")][0].first = "";
	if (t_type == 3)
		projs_sequence << testTime;
	QVariant null_cell;
	for (int i = 0; i < model()->rowCount(); i++)
	{
		for (int j = 0; j < model()->columnCount(); j++)
			model() -> setData(model()->index(i, j), null_cell);
	}
	setModel(save_model);	
	QPair<QString, QStandardItemModel *> t_dpair(testTime, save_model);
	projs_models[saveProj] << t_dpair;
}

void DataesTableView::resetCopyingModelPtr(QStandardItemModel * cp_model)
{
	copy_specs = cp_model;
}

void DataesTableView::deleteTmpModelsPreparation(QStandardItemModel * data_model, DataesTableView * p_view)
{	
	model_deleteStack -> push(new DataesTableEditCmd(this, data_model, tr("删除，，。"), p_view)); 
}

void DataesTableView::deleteTmpModels(QStandardItemModel * del_model, DataesTableView * p_view)
{
	QList<QStandardItemModel *> models_list;
	initEditNewConstructionList(models_list, p_view);
	QString m_time = getInfoByModel(del_model, tr("时间"));
	QPair<QString, QStandardItemModel *> m_pair(m_time, del_model);
	QString cache_head = m_time;
	if (p_view)
	{
		QString mk_time = getInfoByModel(del_model, "");
		cache_head = mk_time+";"+m_time;		
	}	
	QHashIterator<QString, QList<QPair<QString, QStandardItemModel *> > > i_hash(projs_models);
	while (i_hash.hasNext())
	{
		i_hash.next();
		if (i_hash.value().contains(m_pair))
		{
			projs_models[i_hash.key()].removeOne(m_pair);
			if (projs_models.value(i_hash.key()).isEmpty())
				projs_models.remove(i_hash.key());
			break;
		}
	}
	models_cache.insert(cache_head, del_model);
	if (!p_view && union_viewer->modelHashs().contains(m_time))
	{
		QList<QPair<QString, QStandardItemModel *> > model_list = union_viewer->modelHashs().value(m_time);
		union_viewer->modelHashs().remove(m_time);
		for (int i = 0; i < model_list.size(); i++)
		{
			QPair<QString, QStandardItemModel *> rm_pair = model_list.at(i);
			QString keep_times = m_time+";"+rm_pair.first;
			union_viewer->tmpDeletionModelsCache().insert(keep_times, rm_pair.second);
		}
	}
	int last_pos = models_list.indexOf(del_model);
	QStandardItemModel * m_next = 0;	
	if (last_pos == models_list.size()-1)
		m_next = models_list.at(0);
	else
		m_next = models_list.at(last_pos+1);
	setModel(m_next);
	if (t_type == 0)
		changeViewStateForNewModel();
	if (m_next == projs_models.value(tr("草稿工程，，。")).at(0).second)	
	{
		QString del_btn;
		if (!p_view)
		{									
			QStandardItemModel * manual_dmodel = union_viewer->modelHashs().value(tr("草稿工程，，。")).at(0).second;
			for (int i = 0; i < manual_dmodel->rowCount(); i++)
			{
				for (int j = 0; j < manual_dmodel->columnCount(); j++)
					manual_dmodel->setData(manual_dmodel->index(i, j), "");
			}
			union_viewer -> setModel(manual_dmodel);
			del_btn = tr("删除新建");			
		}
		else		
			del_btn = tr("删除样本");
		can_edit = true;		
		emit changeBtnEnable(tr("保存当前"), tr("使能"), false);		
		emit changeBtnEnable(del_btn, tr("使能"), false);		
	}
	else
	{
		can_edit = false;
		QString del_btn;
		if (!p_view)
		{
			QString km_time = getInfoByModel(m_next, tr("时间"));
			QStandardItemModel * manual_dmodel = 0;
			if (union_viewer->modelHashs().contains(km_time))
				manual_dmodel = union_viewer->modelHashs().value(km_time).at(0).second;
			else
			{
				manual_dmodel = union_viewer->modelHashs().value(tr("草稿工程，，。")).at(0).second;
				for (int i = 0; i < manual_dmodel->rowCount(); i++)
				{
					for (int j = 0; j < manual_dmodel->columnCount(); j++)
						manual_dmodel->setData(manual_dmodel->index(i, j), "");
				}
			}
			union_viewer -> setModel(manual_dmodel);
			del_btn = tr("删除新建");			
		}
		else
			del_btn = tr("删除样本");
		emit changeBtnEnable(del_btn, tr("使能"), true);			
		if (!dataesModelExistedInDb(m_next))
			emit changeBtnEnable(tr("保存当前"), tr("使能"), true);
		else
			emit changeBtnEnable(tr("保存当前"), tr("使能"), false);  
	}
}

void DataesTableView::cancelDeleteModelsAction(QStandardItemModel * del_model)
{	
	if (can_edit)
		can_edit = false;
	QPair<QString, QStandardItemModel *> del_pair;
	QHashIterator<QString, QStandardItemModel *> c_hash(models_cache);
	while (c_hash.hasNext())
	{
		c_hash.next();
		if (c_hash.value() == del_model)
		{
			del_pair.first = c_hash.key();
			del_pair.second = del_model;
			models_cache.remove(c_hash.key());
			break;
		}
	}
	QString del_btn;
	SpcNum * calculator = qobject_cast<SpcNum *>(data_db->parent());
	if (del_pair.first.contains(";"))
	{
		QStringList cache_heads = del_pair.first.split(";");
		del_pair.first = cache_heads.at(1);
		if (projs_models.contains(cache_heads.at(0)))
		{
			QStringList t_list;
			for (int i = 0; i < projs_models.value(cache_heads.at(0)).size(); i++)
				t_list << projs_models.value(cache_heads.at(0)).at(i).first;
			int ins_pos = calculator->findTimestrPosInStringlist(t_list, cache_heads.at(1));
			projs_models[cache_heads.at(0)].insert(ins_pos, del_pair);
		}
		else
			projs_models[cache_heads.at(0)] << del_pair;
		del_btn = tr("删除样本");
	}
	else
	{
		QString proj_name = del_model->data(del_model->index(0, 0)).toString();
		if (projs_models.contains(proj_name))
		{
			QStringList t_list;
			for (int i = 0; i < projs_models.value(proj_name).size(); i++)
				t_list << projs_models.value(proj_name).at(i).first;
			int ins_pos = calculator->findTimestrPosInStringlist(t_list, del_pair.first);
			projs_models[proj_name].insert(ins_pos, del_pair);
		}
		else
			projs_models[proj_name] << del_pair;
		QStringList ins_list;
		QHashIterator<QString, QStandardItemModel *> union_hash(union_viewer->tmpDeletionModelsCache());
		while (union_hash.hasNext())
		{
			union_hash.next();
			if (union_hash.key().contains(del_pair.first))
			{
				QStringList t_list = union_hash.key().split(";");
				QPair<QString, QStandardItemModel *> ins_pair(t_list.at(1), union_hash.value());
				int ins_pos = calculator->findTimestrPosInStringlist(ins_list, t_list.at(1));
				ins_list.insert(ins_pos, t_list.at(1));
				union_viewer->modelHashs()[del_pair.first].insert(ins_pos, ins_pair);
			}
		}
		if (!union_viewer->modelHashs().value(del_pair.first).isEmpty())
			union_viewer -> setModel(union_viewer->modelHashs().value(del_pair.first).at(0).second);
		del_btn = tr("删除新建");
	}	
	setModel(del_model);
	emit changeBtnEnable(del_btn, tr("使能"), true);
	emit changeBtnEnable(tr("保存当前"), tr("使能"), false); 
}

void DataesTableView::setShiftModel(QStandardItemModel * shift_model, QStandardItemModel * key_model)
{
	if (key_model)
	{
		  if (!pd_cursors.contains(key_model))
			  pd_cursors.insert(key_model, shift_model);
		  else
			  pd_cursors[key_model] = shift_model;
	}
	cursor_model = shift_model;
}

void DataesTableView::setCpkProcessPartner(DataesTableView * partner)
{
	if (!union_viewer)
		union_viewer = partner;
}

void DataesTableView::syncCurrentCursor(QStandardItemModel * proj_model)
{
	if (!pd_cursors.contains(proj_model))
		cursor_model = 0;
	else
		cursor_model = pd_cursors.value(proj_model);	
}

void DataesTableView::syncUnredoRelatedCmds(QStandardItemModel * proj_model)
{
 	if (!pdd_commands.contains(proj_model))
	{		
		model_deleteStack = new QUndoStack(this);
		model_deleteStack -> setObjectName("delComm");
		pdd_commands.insert(proj_model, model_deleteStack);
	}
	else
		model_deleteStack = pdd_commands.value(proj_model);	  
}

void DataesTableView::findTimestrList(const QList<QPair<QString, QStandardItemModel *> > found_ins, QStringList & findings)
{
	for (int i = 0; i < found_ins.size(); i++)
	{
		QStringList split_strs = found_ins.at(i).first.split(";");
		if (t_type == 3)
			findings << split_strs[0];
		else if (t_type==0 || t_type==1)
			findings << split_strs[1];  
	}
}

void DataesTableView::swapModelsForOriginToDb(QStandardItemModel * key_model)
{
	QList<QPair<QString, QStandardItemModel *> > swap_list = projs_models.value(key_model->data(key_model->index(0, 0)).toString());
	QPair<QString, QStandardItemModel *> first_pair = swap_list.at(0);
	int key_pos = 0;
	for (int i = 0; i < swap_list.size(); i++)
	{
		if (swap_list.at(i).second == key_model)
		{
			key_pos = i;
			break;
		}
	}
	QPair<QString, QStandardItemModel *> swap_pair = swap_list.at(key_pos);
	QList<QPair<QString, QStandardItemModel *> > tmp_list = union_viewer->modelHashs().value(first_pair.first);
	union_viewer->modelHashs()[first_pair.first] = union_viewer->modelHashs().value(swap_pair.first);
	union_viewer->modelHashs()[swap_pair.first] = tmp_list;		
	QStandardItemModel * tmp_model = first_pair.second;
	projs_models[key_model->data(key_model->index(0, 0)).toString()][0].second = key_model;
	projs_models[key_model->data(key_model->index(0, 0)).toString()][key_pos].second = tmp_model;
}

void DataesTableView::setPromotionTreePtr(EngiTreeView * p_tree)
{
	improve_tree = p_tree;
}

void DataesTableView::initTestCpkDataesModels(QStandardItem * engi_item)
{
	QList<QStandardItem *> version_list;
	improve_tree -> findChildrenNotAll(engi_item, version_list);
	version_list.pop_front();
	foreach (QStandardItem * version_item, version_list)
	{
		QList<QStandardItem *> sam_list;
		improve_tree -> findChildrenNotAll(version_item->child(1), sam_list);
		foreach (QStandardItem * s_item, sam_list)
			testCpkDataesFromDb(s_item, version_item->child(0)->child(0)->child(0)->text());
		if (!sam_list.isEmpty() && version_list.indexOf(version_item)==0)
		{
			QStandardItemModel * init_model = projs_models.value(version_item->child(0)->child(0)->child(0)->text()).at(0).second;
			setModel(init_model);
			changeViewStateForNewModel();
		}
	}
}

void DataesTableView::testCpkDataesFromDb(QStandardItem * sam_item, const QString & v_time)
{	
	QStandardItemModel * cpktest_model = new QStandardItemModel(this); 
	QString test_time(sam_item->child(0)->child(0)->text());
	QStringList db_tbls = data_db->allTables();
	QString version_time = QString("%1").arg(QDateTime::fromString(v_time).toTime_t());	
	QString proj_name;
	foreach (QString tbl, db_tbls)
	{
		if (tbl.contains(version_time) && tbl.contains(tr("，，。cpkdataes")))
		{
			proj_name = tbl;
			break;
		}
	}
	proj_name += tr("，。，")+QDateTime::fromString(test_time).toString(Qt::ISODate);
	data_db->spcCalculator()->cpkInitModelFromDb(0, proj_name, cpktest_model);
	cpktest_model -> removeRow(0);
	for (int i = 0; i < cpktest_model->rowCount(); i++)
	{
		cpktest_model -> setHeaderData(i, Qt::Vertical, cpktest_model->data(cpktest_model->index(i, 0)));
		for (int j = 0; j < cpktest_model->columnCount(); j++)
		{
			if (i == 0)
				cpktest_model -> setHeaderData(j, Qt::Horizontal, cpktest_model->data(cpktest_model->index(0, j)));			
		}
	}
	cpktest_model -> removeRow(0);	
	cpktest_model -> removeColumn(0);
	QPair<QString, QStandardItemModel *> sam_pair(test_time, cpktest_model);
	projs_models[v_time] << sam_pair;
}

void DataesTableView::deletePromotionModel(QStandardItem * tree_item, DataesTableView * p_view)
{
	model_deleteStack -> push(new DataesTableEditCmd(this, tree_item, tr("删除，，。"), p_view));
}

void DataesTableView::copyModelDataesToOther(QStandardItemModel * from, QStandardItemModel * other, bool cp_all)
{
	if (!other->rowCount())
	{
		other -> setRowCount(from->rowCount());
		other -> setColumnCount(from->columnCount());
	}
	for (int i = 0; i < from->rowCount(); i++)
	{
		if (from->headerData(i, Qt::Vertical) != other->headerData(i, Qt::Vertical))
			other -> setHeaderData(i, Qt::Vertical, from->headerData(i, Qt::Vertical));	 
		for (int j = 0; j < from->columnCount(); j++)
		{
			if (horizontalHeader() && i==0 && from->headerData(j, Qt::Horizontal)!=other->headerData(j, Qt::Horizontal))
				other -> setHeaderData(j, Qt::Horizontal, from->headerData(j, Qt::Horizontal));
			if (from->data(from->index(i, j)) == other->data(other->index(i, j)) || !cp_all)
				continue;
			else
				other -> setData(other->index(i, j), from->data(from->index(i, j)));
		}
	}
}

void DataesTableView::initEditNewConstructionList(QList<QStandardItemModel *> & model_list, DataesTableView * guider)
{
	QStandardItemModel * manual_model = projs_models.value(tr("草稿工程，，。")).at(0).second;
	QString manual_for = projs_models.value(tr("草稿工程，，。")).at(0).first;
	if (!guider)
	{
		SpcNum * calculator = qobject_cast<SpcNum *>(data_db->parent());
		QStringList projs;		
		QStringList times_list;
		QHashIterator<QString, QList<QPair<QString, QStandardItemModel *> > > i_hash(projs_models);
		while (i_hash.hasNext())
		{
			i_hash.next();
			if (i_hash.key() == tr("草稿工程，，。"))
				continue;
			int pos = calculator->findTimestrPosInStringlist(times_list, i_hash.value().at(0).first);
			times_list.insert(pos, i_hash.value().at(0).first);
			projs.insert(pos, i_hash.key());
		}
		foreach (QString proj, projs)
		{
			for (int i = 0; i < projs_models.value(proj).size(); i++)
				model_list << projs_models.value(proj).at(i).second;		
		}
	}
	else
	{
		QString engi_key = guider->getInfoByModel(qobject_cast<QStandardItemModel *>(guider->model()), tr("时间"));
		for (int i = 0; i < projs_models.value(engi_key).size(); i++)
			model_list << projs_models.value(engi_key).at(i).second;
		if (!model_list.isEmpty())
		{
			QStandardItemModel * model_from = projs_models.value(engi_key).at(0).second;
			copyModelDataesToOther(model_from, manual_model, false);
			projs_models[tr("草稿工程，，。")][0].first = engi_key;
			next_index = manual_model->index(0, 0);
		}
		else
		{
			manual_model -> clear();
			initDifferentDataesModel(manual_model, guider);
		}
	}
	model_list << manual_model;	
}
	
void DataesTableView::initEditPromotionModelsList(QList<QStandardItemModel *> & model_list, DataesTableView * guider)
{
	QStandardItemModel * manual_model = projs_models.value(tr("草稿工程，，。")).at(0).second;
	QString manual_for = projs_models.value(tr("草稿工程，，。")).at(0).first;
	QString engi_key;
	if (t_type == 4)
		engi_key = model()->data(model()->index(0, 0)).toString();
	else
		engi_key = guider->getInfoByModel(qobject_cast<QStandardItemModel *>(guider->model()), tr("时间"));
	for (int i = 0; i < projs_models.value(engi_key).size(); i++)
		model_list << projs_models.value(engi_key).at(i).second;	
	if (manual_for==engi_key || !projs_models.contains(engi_key))
	{ 
		if (guider && inputor.second!=guider->model()->data(guider->model()->index(10, 0)).toString())
			return;
		model_list << manual_model;		
	}
}

void DataesTableView::dealModelsHashForUnreDo(QStandardItemModel * d_model, bool act, QStandardItem * g_item)
{
	if (act)
	{
		QString m_time = getInfoByModel(d_model, tr("时间"));
		QPair<QString, QStandardItemModel *> m_pair(m_time, d_model);
		QHashIterator<QString, QList<QPair<QString, QStandardItemModel *> > > i_hash(projs_models);
		while (i_hash.hasNext())
		{
			i_hash.next();
			if (i_hash.value().contains(m_pair))
			{
				projs_models[i_hash.key()].removeOne(m_pair);
				if (projs_models.value(i_hash.key()).isEmpty())
					projs_models.remove(i_hash.key());
				break;
			}
		}
		QString cache_head = m_time;
		if (t_type == 0)
			cache_head = g_item->parent()->parent()->child(0)->child(0)->child(0)->text()+";"+m_time;
		models_cache.insert(cache_head, d_model);
		if (t_type==4 && union_viewer->modelHashs().contains(m_time))
		{
			QList<QPair<QString, QStandardItemModel *> > model_list = union_viewer->modelHashs().value(m_time);
			union_viewer->modelHashs().remove(m_time);
			for (int i = 0; i < model_list.size(); i++)
			{
				QPair<QString, QStandardItemModel *> rm_pair = model_list.at(i);
				QString keep_times = m_time+";"+rm_pair.first;
				union_viewer->tmpDeletionModelsCache().insert(keep_times, rm_pair.second);
			}
		}
	}
	else
	{
		QPair<QString, QStandardItemModel *> m_pair;
		SpcNum * calculator = qobject_cast<SpcNum *>(data_db->parent());		
		QHashIterator<QString, QStandardItemModel *> i_hash(models_cache);
		while (i_hash.hasNext())
		{
			i_hash.next();
			if (i_hash.value() == d_model)
			{
				m_pair.first = i_hash.key();
				m_pair.second = d_model;
				models_cache.remove(i_hash.key());
				break;
			}
		}
		if (m_pair.first.contains(";"))
		{
			QStringList cache_heads = m_pair.first.split(";");
			m_pair.first = cache_heads.at(1);
			if (projs_models.contains(cache_heads.at(0)))
			{
				QStringList t_list;
				for (int i = 0; i < projs_models.value(cache_heads.at(0)).size(); i++)
					t_list << projs_models.value(cache_heads.at(0)).at(i).first;
				int ins_pos = calculator->findTimestrPosInStringlist(t_list, cache_heads.at(1));
				projs_models[cache_heads.at(0)].insert(ins_pos, m_pair);
			}
			else
				projs_models[cache_heads.at(0)] << m_pair;
		}
		else
		{
			if (projs_models.contains(g_item->parent()->text()))
			{
				QStringList t_list;
				for (int i = 0; i < projs_models.value(g_item->parent()->text()).size(); i++)
					t_list << projs_models.value(g_item->parent()->text()).at(i).first;
				int ins_pos = calculator->findTimestrPosInStringlist(t_list, m_pair.first);
				projs_models[g_item->parent()->text()].insert(ins_pos, m_pair);
			}
			else
				projs_models[g_item->parent()->text()] << m_pair;
			QStringList ins_list;
			QHashIterator<QString, QStandardItemModel *> union_hash(union_viewer->tmpDeletionModelsCache());
			while (union_hash.hasNext())
			{
				union_hash.next();
				if (union_hash.key().contains(m_pair.first))
				{
					QStringList t_list = union_hash.key().split(";");
					QPair<QString, QStandardItemModel *> ins_pair(t_list.at(1), union_hash.value());
					int ins_pos = calculator->findTimestrPosInStringlist(ins_list, t_list.at(1));
					ins_list.insert(ins_pos, t_list.at(1));
					union_viewer->modelHashs()[m_pair.first].insert(ins_pos, ins_pair);
				}
			}
		}
	}
}

bool DataesTableView::tblEdittingState()
{
	return can_edit;
}

bool DataesTableView::checkInputState(QString & chk_result)
{
	if (!selectedIndexes().isEmpty() && indexWidget(selectedIndexes().at(0)))
	{
		commitData(indexWidget(selectedIndexes().at(0)));
		closeEditor(indexWidget(selectedIndexes().at(0))); 
	}
	SpcNum * calculation = qobject_cast<SpcNum *>(data_db->parent());
	if (t_type == 0)
	{
		for (int i = 0; i < model()->rowCount(); i++)
		{
			for (int j = 0; j < model()->columnCount(); j++)
			{
				QString save_string = model()->data(model()->index(i, j)).toString();				
				if (save_string.isEmpty() || !calculation->inputNumbersJudging(save_string))
				{
					if (save_string.isEmpty())
					{
						chk_result += QString("%1").arg(i)+","+QString("%1").arg(j)+","+"empty";
						return false;
					}
					if (!calculation->inputNumbersJudging(save_string))
					{
						if (chk_result.isEmpty())
							chk_result = QString("%1").arg(i)+","+QString("%1").arg(j)+","+"no_number";
						else
							chk_result += ";"+QString("%1").arg(i)+","+QString("%1").arg(j)+","+"no_number";
					}
				}
			}
		}
	}
	else if (t_type == 2)
	{
		QList<int> loop_cells;
		canInputtedCells(loop_cells);
		int n = loop_cells.size();
		for (int i = 0; i < n; i++)
		{
			if (model()->data(model()->index(loop_cells.at(i), 0)).isNull() || !calculation->inputNumbersJudging(model()->data(model()->index(i, 0)).toString()))
				return false;
		}		
	}
	else if (t_type == 3 || t_type == 4)
	{
		if (t_type ==3 && calculation->checkPunctuationInString(model()->data(model()->index(0, 0)).toString()))
		{
			chk_result = tr("工程名不能包含标点符号");
			return false;
		}
		if (t_type ==3 && !calculation->chkStrOnlyUseLetterOrNumber(model()->data(model()->index(10, 0)).toString()))
		{
			chk_result = tr("密码不能由数字或字母以外符号构成");
			return false;
		}		
		if (t_type ==3 && model()->data(model()->index(11, 0)).toString()==tr("操作者"))
		{
			chk_result = tr("操作者不能创建工程");
			return false;
		}
		for (int i = 0; i < model()->rowCount(); i++)
		{
			bool chk_num = false;
			if (i==2 || i==4 || i==5 || i==6 || i==7 || i==8)
				chk_num = true;
			QString save_string = model()->data(model()->index(i, 0)).toString();
			if (chk_num && !calculation->inputNumbersJudging(save_string))
			{
				if (chk_result.isEmpty())
					chk_result = QString("%1").arg(i)+","+"no_number";
				else
					chk_result += ";"+QString("%1").arg(i)+","+"no_number";	
				return false;
			}
			if (save_string.isEmpty())
			{
				chk_result += QString("%1").arg(i)+","+"empty";
				return false;
			}
		}
	}
	else if (t_type == 5)
	{
		QStandardItemModel * dt_model = qobject_cast<QStandardItemModel *>(model());
		QList<QStandardItem *> f_pname = dt_model->findItems(tr("修改后名称"), Qt::MatchRecursive, 1);
		QList<QStandardItem *> f_same = dt_model->findItems(tr("修改后密码"), Qt::MatchRecursive, 2);
		bool f_all = false;
		for (int i = 0; i < dt_model->rowCount(); i++)
		{
			for (int j = 0; j < dt_model->columnCount(); j++)
			{
				if (!f_pname.isEmpty() && !f_same.isEmpty())
				{
					if (f_pname.at(0)->index().column()==j && i>f_pname.at(0)->index().row() && i<f_same.at(0)->index().row())
					{
						if (dt_model->data(dt_model->index(i, j)).toString()==dt_model->data(dt_model->index(i, j-1)).toString() || dt_model->data(dt_model->index(i, j)).toString().isEmpty())
						{
							if (dt_model->data(dt_model->index(i, j)).toString() == dt_model->data(dt_model->index(i, j-1)).toString())
							{
								QVariant null;
								dt_model -> setData(dt_model->index(i, j), null);
							}
							dt_model -> setData(dt_model->index(i, j), QColor(Qt::red), Qt::BackgroundRole);
							f_all = true;						
						}	
					}						
					if (f_same.at(0)->index().column()==j && i>f_same.at(0)->index().row())
					{
						if (dt_model->data(dt_model->index(i, j)).toString().isEmpty() || (dt_model->data(dt_model->index(i, j+1), Qt::BackgroundRole)!=QVariant(QColor(Qt::darkGreen)) && dt_model->data(dt_model->index(i, j)).toString()==dt_model->data(dt_model->index(i, j-1)).toString()))
						{
							QVariant null;
							dt_model -> setData(dt_model->index(i, j), null);
							dt_model -> setData(dt_model->index(i, j), QColor(Qt::red), Qt::BackgroundRole);
							f_all = true;						
						}
					}				
				}
				else if (!f_pname.isEmpty())
				{
					if (f_pname.at(0)->index().column()==j && i>f_pname.at(0)->index().row() && dt_model->data(dt_model->index(i, j)).toString().isEmpty())
					{
						dt_model -> setData(dt_model->index(i, j), QColor(Qt::red), Qt::BackgroundRole);
						f_all = true;
					}					
				}
				else
				{
					if (f_same.at(0)->index().column()==j && i>f_same.at(0)->index().row() && dt_model->data(dt_model->index(i, j)).toString().isEmpty())
					{				
						dt_model -> setData(dt_model->index(i, j), QColor(Qt::red), Qt::BackgroundRole);
						f_all = true;
					}					
				}
			}			
		}
		if (f_all)
			return false;		
		for (int i = 0; i < dt_model->rowCount(); i++)
		{
			for (int j = 0; j < dt_model->columnCount(); j++)
			{
				if (!f_pname.isEmpty() && !f_same.isEmpty())
				{
					if (f_pname.at(0)->index().column()==j && i>f_pname.at(0)->index().row() && i<f_same.at(0)->index().row())
					{
						QHashIterator<QString, QString> ip_hash(data_db->tmpProjsHashForDbsConfliction());
						while (ip_hash.hasNext())
						{
							ip_hash.next();
							if (ip_hash.key().contains(dt_model->data(dt_model->index(i, j-1)).toString()))
							{
								data_db->tmpProjsHashForDbsConfliction()[ip_hash.key()] = dt_model->data(dt_model->index(i, j)).toString();
								break;
							}
						}
					}						
					if (f_same.at(0)->index().column()==j && i>f_same.at(0)->index().row())
					{
						QHashIterator<QString, QString> im_hash(data_db->tmpMngsHashForDbsConfliction());				  
						if (dt_model->data(dt_model->index(i, j+1), Qt::BackgroundRole) == QVariant(QColor(Qt::darkGreen)))
						{
							while (im_hash.hasNext())
							{
								im_hash.next();
								if (im_hash.key().contains(dt_model->data(dt_model->index(i, j-1)).toString()))
								{
									data_db->tmpMngsHashForDbsConfliction().remove(im_hash.key());
									break;
								}
							}
						}
						else
						{
							while (im_hash.hasNext())
							{
								im_hash.next();
								if (im_hash.key().contains(dt_model->data(dt_model->index(i, j-1)).toString()))
								{
									data_db->tmpMngsHashForDbsConfliction()[im_hash.key()] = dt_model->data(dt_model->index(i, j)).toString();
									break;
								}
							}						  
						}
					}				
				}
				else if (!f_pname.isEmpty())
				{
					if (f_pname.at(0)->index().column()==j && i>f_pname.at(0)->index().row())
					{
						QHashIterator<QString, QString> ip_hash(data_db->tmpProjsHashForDbsConfliction());
						while (ip_hash.hasNext())
						{
							ip_hash.next();
							if (ip_hash.key().contains(dt_model->data(dt_model->index(i, j-1)).toString()))
							{
								data_db->tmpProjsHashForDbsConfliction()[ip_hash.key()] = dt_model->data(dt_model->index(i, j)).toString();
								break;
							}
						}
					}					
				}
				else
				{
					if (f_same.at(0)->index().column()==j && i>f_same.at(0)->index().row())
					{				
						QHashIterator<QString, QString> im_hash(data_db->tmpMngsHashForDbsConfliction());				  
						if (dt_model->data(dt_model->index(i, j+1), Qt::BackgroundRole) == QVariant(QColor(Qt::darkGreen)))
						{
							while (im_hash.hasNext())
							{
								im_hash.next();
								if (im_hash.key().contains(dt_model->data(dt_model->index(i, j-1)).toString()))
								{
									data_db->tmpMngsHashForDbsConfliction().remove(im_hash.key());
									break;
								}
							}
						}
						else
						{
							while (im_hash.hasNext())
							{
								im_hash.next();
								if (im_hash.key().contains(dt_model->data(dt_model->index(i, j-1)).toString()))
								{
									data_db->tmpMngsHashForDbsConfliction()[im_hash.key()] = dt_model->data(dt_model->index(i, j)).toString();
									break;
								}
							}						  
						}
					}					
				}
			}			
		}
	}
	return true;
}

bool DataesTableView::saveProjKeyDataesCheck(bool init_proj, const QString & test_dt)
{
	QStringList keys;
	QStringList other_keys;
	for (int i = 0; i < 13; i++)
	{
		if (i < 11)
			keys << model()->data(model()->index(i, 0)).toString();
		if (init_proj)
			other_keys << model()->data(model()->index(i, 0)).toString();	
	}
	if (!data_db->storeProjectsKeyDataes(keys, init_proj, test_dt))
		return false;	
	if (init_proj)
	{
		other_keys.push_front("projkeys");	  
		if (!data_db->storeManageInfoTable(other_keys) || !data_db->storeProjectsInfoTable(other_keys) || !data_db->storeDepartmentTable(other_keys))
			return false;
	}
	return true;	
}

bool DataesTableView::saveDailySpcDataes()
{
  	QStringList engi_time = current_plots->currentEngi().split(tr("，，。"));	
	QStringList all_tbls = data_db->allTables();
	QString pro_ktime(QString("%1").arg(QDateTime::fromString(engi_time[1]).toTime_t()));	
	QStringList cpk_chints;
	QString create_table(engi_time[0]+tr("，，。")+"dailydataes");
	bool found = false;
	foreach (QString e_str, all_tbls)
	{
		if (e_str.contains(pro_ktime) && e_str.contains("cpk"))
		{
			QStringList cpk_thints = e_str.split(tr("，，。"));		  
			cpk_chints = cpk_thints[2].split(tr("，"));
		}
		if (e_str.contains(pro_ktime) && e_str.contains("dailydataes"))
			found = true;		
	}
	QString c_time = QDateTime::fromTime_t(cpk_chints[0].toUInt()).toString()+";"+engi_time[1]+";"+QDateTime::currentDateTime().toString()+";"+inputor.second;
	if (model()->headerData(model()->rowCount()-3, Qt::Vertical).toString().contains(tr("西格玛")))
		create_table += tr("，，。")+"dailydev";
	else
		create_table += tr("，，。")+"dailyrng";	
	if (!found)
	{	  
		if (!data_db->createEngiTables(create_table, inputor.second, *data_db->currentConnectionDb(), c_time))
			return false;
	}
	if (!data_db->storeSpcDataes(create_table, this, 0, 0, c_time))
		return false;
	next_index = model()->index(1, 0);
	return true;	
}

bool DataesTableView::cpkInitDataesCalucationCheck()
{
	SpcNum * calculation = qobject_cast<SpcNum *>(data_db->parent());
	for (int i = 0; i < model()->rowCount(); i++)
	{
		for (int j = 0; j < model()->columnCount(); j++)
		{
			if (!calculation->inputNumbersJudging(model()->data(model()->index(i, j)).toString()))
				return false;
		}
	}
	return true;
}

bool DataesTableView::dailyDataesCalucationCheck()
{
	if (!model()->data(model()->index(model()->rowCount()-4, 0)).toString().isEmpty())
		return false;
	return true;	
}

bool DataesTableView::pasteModelDataesAction(bool paste_act, QStandardItemModel * guide_model, QString & p_result)
{
	if (!copy_specs)
		return false;
	if (copy_specs->rowCount()!=guide_model->data(guide_model->index(7, 0)).toInt() || copy_specs->columnCount()!=guide_model->data(guide_model->index(6, 0)).toInt())
	{
		p_result = tr("行列不同");
		return false;
	}
	DataesTableView * key_view = qobject_cast<DataesTableView *>(guide_model->parent());	  	
	QString key_time = getInfoByModel(copy_specs, "");
	QStandardItemModel * version_model = key_view->matchTimeKeyModel(key_time);
	if (guide_model == version_model)
	{
		p_result = tr("已经存在");
		return false;
	}
	bool is_same = true;
	for (int j = 0; j < model()->rowCount(); j++)
	{
		for (int k = 0; k < model()->columnCount(); k++)
		{
			if (model()->data(model()->index(j, k)) != copy_specs->data(copy_specs->index(j, k)))
			{
				is_same = false;
				break;
			}
		}
		if (!is_same)
			break;
	}
	if (is_same)
	{
		p_result = tr("目标相同");
		return false;
	}
	QString this_time = key_view->getInfoByModel(guide_model, tr("时间"));
	if (paste_act)
		changeModelForEditting(copy_specs, this_time);
	return true;
}

bool DataesTableView::dataesModelExistedInDb(QStandardItemModel * chk_model)
{
	QString chk_time = getInfoByModel(chk_model, tr("时间"));
	if (t_type == 0)
	{
		QString m_stamp = QString("%1").arg(QDateTime::fromString(chk_time).toTime_t());
		QStringList t_list = data_db->allTables().filter(m_stamp);
		if (!t_list.isEmpty())
			return true;
	}
	else
	{
		QString p_row("name="+chk_model->data(chk_model->index(0, 0)).toString());
		QString u_cell = data_db->dataFromTable("projectskeys", "unit", p_row).toString();
		if (u_cell.contains(chk_time))
			return true;
	}
	return false;
}

bool DataesTableView::chkAndDeleteEngiDataesInDb()
{
	QStringList db_tbls = data_db->allTables();
	QHashIterator<QString, QStandardItemModel *> i_hash(models_cache);
	while (i_hash.hasNext())
	{
		i_hash.next();
		if (i_hash.key().contains(";"))
		{
			QStringList key_din = i_hash.key().split(";");
			QString fil_stamp = QString("%1").arg(QDateTime::fromString(key_din.at(1)).toTime_t());
			QStringList del_tbls = db_tbls.filter(fil_stamp);
			foreach (QString t_del, del_tbls)
			{
				if (!data_db->delTable(t_del))
					return false;
			}
		}
		else
		{
			QString fil_stamp = QString("%1").arg(QDateTime::fromString(i_hash.key()).toTime_t());
			QStringList del_tbls = db_tbls.filter(fil_stamp);
			foreach (QString t_del, del_tbls)
			{
				if (!data_db->delTable(t_del))
					return false;
			}
			QHashIterator<QString, QStandardItemModel *> u_hash(union_viewer->tmpDeletionModelsCache());
			while (u_hash.hasNext())
			{
				u_hash.next();
				if (u_hash.key().contains(i_hash.key()))
					union_viewer->tmpDeletionModelsCache().remove(u_hash.key());
			}
			QString proj = i_hash.value()->data(i_hash.value()->index(0, 0)).toString();	
			QStringList proj_keys;
			data_db -> getDataesFromTable("projectskeys", "name", proj_keys, proj);
			QString row_name("name="+proj);
			bool total_null = false;
			foreach (QString p_key, proj_keys)
			{
				if (proj_keys.indexOf(p_key)!=0 && proj_keys.indexOf(p_key)!=10)
				{
					QStringList cell_contents = p_key.split(tr("；"));				
					QStringList fil_contents = cell_contents.filter(i_hash.key());
					cell_contents.removeOne(fil_contents.at(0));
					if (cell_contents.isEmpty())
					{
						total_null = true;
						break;
					}
					QString c_col;
					if (proj_keys.indexOf(p_key) == 1)
						c_col = "unit";
					else if (proj_keys.indexOf(p_key) == 2)
						c_col = "precision";
					else if (proj_keys.indexOf(p_key) == 3)
						c_col = "ctrlplot";
					else if (proj_keys.indexOf(p_key) == 4)
						c_col = "uplimit";
					else if (proj_keys.indexOf(p_key) == 5)
						c_col = "lowlimit";
					else if (proj_keys.indexOf(p_key) == 6)
						c_col = "grpsamles";
					else if (proj_keys.indexOf(p_key) == 7)
						c_col = "groups";
					else if (proj_keys.indexOf(p_key) == 8)
						c_col = "desnum";
					else
						c_col = "constructor";					
					if (!data_db->updateCellData("projectskeys", row_name, c_col, cell_contents.join(tr("；"))))
						return false;					
				}
			}
			if (total_null)
			{
				if (!data_db->deleteProject(proj))
					return false;
				emit deleteTreeProjForPromotion(proj);
			}
		}
		delete i_hash.value();
	}
	models_cache.clear();
	return true;
}

int DataesTableView::tableType()
{
	return t_type;
}

QStandardItemModel * DataesTableView::existedEntirelyModelInHash(const QString & proj, QStandardItemModel * match_model)
{
	if (!projs_models.contains(proj))
		return 0;
	QList<QPair<QString, QStandardItemModel *> > chk_list = projs_models.value(proj); 
	QStandardItemModel * chk_model = 0;
	if (match_model)
		chk_model = match_model;
	else
		chk_model = qobject_cast<QStandardItemModel *>(model());
	for (int i = 0; i < chk_list.size(); i++)
	{
		QPair<QString, QStandardItemModel *> m_pair = chk_list.at(i);
		if (t_type==0 && (chk_model->rowCount()!=m_pair.second->rowCount() || chk_model->columnCount()!=m_pair.second->columnCount()))
			continue;
		bool is_same = true;
		for (int j = 0; j < chk_model->rowCount(); j++)
		{
			if ((t_type==3 || t_type==4) && j>9)
				break;
			for (int k = 0; k < chk_model->columnCount(); k++)
			{
				if (chk_model->data(chk_model->index(j, k)) != m_pair.second->data(m_pair.second->index(j, k)))
				{
					is_same = false;
					break;
				}
			}
			if (!is_same)
				break;
		}		
		if (is_same)
			return m_pair.second;
	}
	return 0;
}

QStandardItemModel * DataesTableView::matchTimeKeyModel(const QString & saveTime)
{
	QHashIterator<QString, QList<QPair<QString, QStandardItemModel *> > > m_hash(projs_models);
	while (m_hash.hasNext())
	{
		m_hash.next();
		QList<QPair<QString, QStandardItemModel *> > n_list = m_hash.value();
		for (int i = 0; i < n_list.size(); i++)
		{
			if (n_list.at(i).first.contains(saveTime))
				return n_list.at(i).second;
		}
	}
	return 0;
}

QStandardItemModel * DataesTableView::matchNewKeysModel(const QString & saveProj, const QString & saveTime)
{
 	QList<QPair<QString, QStandardItemModel *> > dest_list = projs_models.value(saveProj);
	for (int i = 0; i < dest_list.size(); i++)
	{
		if (dest_list.at(i).first==saveTime || dest_list.at(i).first.contains(saveTime))
			return dest_list.at(i).second;
	}
	return 0; 
}

QStandardItemModel * DataesTableView::shiftedKeyForModelSelection()
{
	return cursor_model;
}
	
QStandardItem * DataesTableView::engiPerformanceItemOrSampleItem()
{
	QStandardItem * engi_item = improve_tree->currentEngiItem();
	QString item_keystr;	
	if (t_type == 4)
	{
		QList<QPair<QString, QStandardItemModel *> > dest_list = projs_models.value(engi_item->text());
		bool found = false;
		for (int i = 0; i < dest_list.size(); i++)
		{
			if (dest_list.at(i).second == cursor_model)
			{
				found = true;
				break;
			}
		}
		if (!found)
			cursor_model = dest_list.at(0).second;
		QHash<QStandardItemModel *, QStandardItemModel *> union_shifts = union_viewer->cursorModels();	
		if (!union_shifts.contains(cursor_model))
		{
			QString k_time = modelConstructTime(engi_item->text(), cursor_model);
			QList<QPair<QString, QStandardItemModel *> > models_list = union_viewer->modelHashs().value(k_time);			
			union_viewer -> setShiftModel(models_list.at(0).second, cursor_model);
		}
		
	}
	item_keystr = getInfoByModel(cursor_model, tr("时间"));
	QList<QStandardItem *> f_items = qobject_cast<QStandardItemModel *>(improve_tree->model())->findItems(item_keystr, Qt::MatchRecursive);
	QStandardItem * r_item = 0;
	if (t_type == 4)
		r_item = f_items.at(0)->parent()->parent()->parent();
	else
		r_item = f_items.at(0)->parent()->parent();
	return r_item;
}

EditLongToolsViewer * DataesTableView::qmlViewer()
{
	return multi_viewer;
}

PlotWidget * DataesTableView::plotsInfomations()
{
	return current_plots;
}

EngiTreeView * DataesTableView::improveTreePtr()
{
	return improve_tree;
}

QUndoStack * DataesTableView::commandStack()
{
	return m_commandStack;
}

QUndoStack * DataesTableView::delModelsStack()
{
	return model_deleteStack;
}

DataesTableView * DataesTableView::nestingUnionView()
{
	return union_viewer;
}

SpcDataBase * DataesTableView::backDatabase()
{
	return data_db;  
}

QString DataesTableView::modelConstructTime(const QString & chk_proj, QStandardItemModel * dest_model)
{
	QList<QPair<QString, QStandardItemModel *> > dest_list = projs_models.value(chk_proj);
	for (int i = 0; i < dest_list.size(); i++)
	{
		if (dest_list.at(i).second == dest_model)
			return dest_list.at(i).first;
	}
	return tr("草稿工程，，。");
}

const QString & DataesTableView::getInfoByModel(QStandardItemModel * dest_model, const QString & by_what)
{
	QHashIterator<QString, QList<QPair<QString, QStandardItemModel *> > > m_hash(projs_models);
	while (m_hash.hasNext())
	{
		m_hash.next();
		QList<QPair<QString, QStandardItemModel *> > n_list = m_hash.value();
		for (int i = 0; i < n_list.size(); i++)
		{
			if (n_list.at(i).second==dest_model)
			{
				if (by_what == tr("时间"))
					return n_list.at(i).first;
				else	
					return m_hash.key();
			}
		}
	}
	return by_what;
}

const QModelIndex & DataesTableView::curNextIndex()
{
	return next_index;
}

const QRect & DataesTableView::originShowRect()
{
	return right_origin;
}

QPair<QString, QString> & DataesTableView::projectInputor()
{
	return inputor;
}

const QStringList & DataesTableView::projsInsertSequence()
{
	return projs_sequence;
}

const QHash<QModelIndex, QString> & DataesTableView::dailyCalculations()
{
	return dataes_indexes;  
}

QHash<QString, QStandardItemModel *> & DataesTableView::tmpDeletionModelsCache()
{
	return models_cache;
}

const QHash<QStandardItemModel *, QStandardItemModel *> & DataesTableView::cursorModels()
{
	return pd_cursors;
}

QHash<QString, QList<QPair<QString, QStandardItemModel *> > > & DataesTableView::modelHashs()
{
	return projs_models;
}

void DataesTableView::clearRelatedCellsForReinput()
{
	if (t_type == 2)
	{
		int cur_group = model()->data(model()->index(0, 0)).toInt();
		if (current_plots->currentEngiGroups() == cur_group)
			current_plots -> deleteNewDataOnMultiDaily();
		for (int i = 1; i < model()->rowCount(); i++)
		{
			if (i == model()->rowCount()-5)
				continue;
			model() -> setData(model()->index(i, 0), "");
		}
	}	
}

void DataesTableView::replyForSelectedStrs(int s_num)
{
	if (s_num != 0)
	{
		QModelIndex editted_index = multi_viewer->currentNesttingItem()->index();
		m_commandStack -> push(new DataesTableEditCmd(this, editted_index, cell_oldText, qml_strlist[s_num]));
	}
	closeNestedQmlViewerOnCell(multi_viewer);
}

void DataesTableView::changeViewStateForNewModel()
{
	QFontMetrics fm(font());
	int max_vwidth = 0;
	for (int i = 0; i < model()->rowCount(); i++)
	{
		int h_width = fm.width(model()->headerData(i, Qt::Vertical).toString())+16;
		if (h_width > max_vwidth)
			max_vwidth = h_width;
		for (int j = 0; j < model()->columnCount(); j++)
		{
			int str_width = fm.width(model()->data(model()->index(i, j)).toString());
			if (str_width > columnWidth(j))
				resizeColumnToContents(j);
		}
	}
	verticalHeader() -> setFixedWidth(max_vwidth);
	setFixedWidth(columnViewportPosition(model()->columnCount()-1)+columnWidth(model()->columnCount()-1)+max_vwidth);
	setFixedHeight(rowViewportPosition(model()->rowCount()-1)+rowHeight(model()->rowCount()-1)+horizontalHeader()->height());
	if (multi_viewer)
	{
		QModelIndex editted_index = multi_viewer->currentNesttingItem()->index();
		recalSizeForOpeningNestedViewer(editted_index, multi_viewer->rect());
	}
	else
		emit sizeChanged();  
}

void DataesTableView::cellClicked(const QModelIndex & c_index)
{
	if (!can_edit)
		return;
	if (t_type == 3 || t_type==4)
	{
		int time_passed = 0;
		if (press_time)
		{
			time_passed = press_time->elapsed();
			delete press_time;
			press_time = 0;
		}
		if ((t_type==3 || t_type==4) && !inputor.second.isEmpty() && c_index.row()>8)
			return;
		if (t_type==4 && c_index.row()==0)
			return;
		if (c_index.row()!=3 && c_index.row()!=11)
			edit(c_index);
		else if (time_passed > 500)
			initStrListViewer(c_index);		
	}
	else if (t_type==2)
	{
		if (c_index.row()>5 || c_index.row()==0)
			return;		
		edit(c_index);
	}
	else
	{		
		if (t_type == 5)
		{
			QStandardItemModel * dt_model = qobject_cast<QStandardItemModel *>(model());
			QList<QStandardItem *> f_engi = dt_model->findItems(tr("冲突的工程"), Qt::MatchRecursive); 
			QList<QStandardItem *> f_mng = dt_model->findItems(tr("冲突的密码"), Qt::MatchRecursive);
			QList<QStandardItem *> f_pwd = dt_model->findItems(tr("修改后密码"), Qt::MatchRecursive, 2);
			QList<QStandardItem *> f_same = dt_model->findItems(tr("同一人？"), Qt::MatchRecursive, 3);
			if (c_index.column()==0 || (!f_engi.isEmpty() && c_index.row()>f_engi.at(0)->index().row()+1 && c_index.column()==0) || (!f_mng.isEmpty() && c_index.row()>f_mng.at(0)->index().row()+1 && (c_index.column()==0 || c_index.column()==1)))
				return;			
			if (!f_engi.isEmpty() && (f_engi.at(0)->index().row()==c_index.row() || c_index.row()-1==f_engi.at(0)->index().row()))
				return;			
			if (!f_mng.isEmpty() && (f_mng.at(0)->index().row()==c_index.row() || c_index.row()-1==f_mng.at(0)->index().row()))
				return;
			if (!f_pwd.isEmpty() && f_pwd.at(0)->index().column()==c_index.column() && c_index.row()>f_pwd.at(0)->index().row() && model()->data(model()->index(c_index.row(),c_index.column()+1), Qt::BackgroundRole) == QVariant(QColor(Qt::darkGreen)))				
				return;			
			if (!f_same.isEmpty() && f_same.at(0)->index().column()==c_index.column() && c_index.row()>f_same.at(0)->index().row())
			{				
				if (model()->data(c_index, Qt::BackgroundRole) == QVariant(QColor(Qt::darkGreen)))
				{
					model() -> setData(c_index, QColor(Qt::white), Qt::BackgroundRole);
					QVariant null;
					model() -> setData(model()->index(c_index.row(), c_index.column()-1), null);
				}
				else
				{
					model() -> setData(c_index, QColor(Qt::darkGreen), Qt::BackgroundRole);
					model() -> setData(model()->index(c_index.row(), c_index.column()-1), QColor(Qt::white), Qt::BackgroundRole);
					model() -> setData(model()->index(c_index.row(), c_index.column()-1), model()->data(model()->index(c_index.row(), c_index.column()-2)));
				}
				return;
			}
			if (model()->data(c_index, Qt::BackgroundRole) == QVariant(QColor(Qt::red)))
				model() -> setData(c_index, QColor(Qt::white), Qt::BackgroundRole);
		}
		else
			emit changeStackCommOrder(m_commandStack);
		edit(c_index);
	}	
	cell_oldText = model()->data(c_index).toString();
}

void DataesTableView::closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint)
{
	QModelIndex editted_index = indexAt(editor->geometry().center());
	QString editted_str = model()->data(editted_index).toString();
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	if (t_type==0 || t_type==2)
	{
		if (next_index!=editted_index || model()->data(editted_index).isNull())
		{
			model() -> setData(editted_index, cell_oldText);
			QString w_info = "warning:"+tr("请按顺序输入");
			DiaLog * w_dialog = new DiaLog;
			MainWindow * mw = qobject_cast<MainWindow *>(data_db->parent()->parent());
			w_dialog -> initWinDialog(mw, w_info, 1);
		}
		if (editted_str!=cell_oldText)
			m_commandStack->push(new DataesTableEditCmd(this, next_index, cell_oldText, editted_str));
	}
	else if (t_type == 3)
	{
		if (editted_str != cell_oldText)
		{
			if (model()->headerData(editted_index.row(), Qt::Vertical).toString() == tr("工程名称"))
			{
				QString proj = editted_str;
				if (data_db->existedProjectNameIndb(proj))
				{
					QString w_info = "warning:"+tr("输入的工程名已存在\n不能重名!");
					DiaLog * w_dialog = new DiaLog;
					MainWindow * mw = qobject_cast<MainWindow *>(data_db->parent()->parent());
					w_dialog -> initWinDialog(mw, w_info, 1);
					return;
				}
			}
			m_commandStack->push(new DataesTableEditCmd(this, editted_index, cell_oldText, editted_str));
		}
	}
	QFontMetrics fm(font());
	int str_width = fm.width(editted_str);
	if (str_width > columnWidth(editted_index.column()))
	{
		resizeColumnToContents(editted_index.column());
		changeViewStateForNewModel();
	}
	QTableView::closeEditor(editor, hint);		
}
	
void DataesTableView::clearQmlViewStrsPt(QWidget * clear_pt)
{
	Q_UNUSED(clear_pt);
	multi_viewer = 0;
}
	
void DataesTableView::clearNestedAnimation()
{
	if (multi_viewer)
		multi_viewer -> setFixedWidth(nested_animation->endValue().toRect().width());
	nested_animation -> deleteLater();
	nested_animation = 0;
	update();
}

void DataesTableView::mousePressEvent(QMouseEvent * event)
{
	QModelIndex pressing = indexAt(event->pos());	
	if ((t_type==3 || t_type==4) && (pressing==model()->index(3, 0) || pressing==model()->index(11, 0)))
	{
		setEditTriggers(QAbstractItemView::NoEditTriggers);
		press_time = new QTime;
		press_time -> start();
	}
	QTableView::mousePressEvent(event);
}

/*void DataesTableView::focusInEvent(QFocusEvent * event)
{
	if (event->reason() == Qt::OtherFocusReason)
	{
		reset();
		delete childAt(visualRect(model()->index(0, 0)).center());
		return;
	}
	QTableView::focusInEvent(event);
}*/

void DataesTableView::initDifferentDataesModel(QStandardItemModel * n_model, DataesTableView * guider, const QString & proj, const QList<double> & vals_list)
{
	if (t_type == 0)
	{
		if (guider)
		{
			n_model -> setRowCount(guider->model()->data(guider->model()->index(7, 0)).toInt());
			n_model -> setColumnCount(guider->model()->data(guider->model()->index(6, 0)).toInt());
		}
		else
		{		  
			n_model -> setRowCount(current_plots->currentEngiGroups());
			n_model -> setColumnCount(current_plots->currentEngiSamples());
		}
		if (n_model->rowCount()==0 || n_model->columnCount()==0)
		{
			n_model -> setRowCount(10);
			n_model -> setColumnCount(25);			
		}
		for (int i = 0; i < n_model->rowCount(); i++)
			n_model -> setHeaderData(i, Qt::Vertical, QString(tr("样本组%1")).arg(i+1));
		for (int i = 0; i < n_model->columnCount(); i++)
			n_model -> setHeaderData(i, Qt::Horizontal, QString(tr("样本%1")).arg(i+1));
		next_index = n_model->index(0, 0);
	}
	else if (t_type == 1)
	{
		n_model -> setRowCount(11);
		n_model -> setColumnCount(1);
		n_model -> setHeaderData(0, Qt::Vertical, QString(tr("工程名称")));
		n_model -> setHeaderData(1, Qt::Vertical, QString(tr("控图组合")));
		n_model -> setHeaderData(2, Qt::Vertical, QString(tr("CPK结果")));
		n_model -> setHeaderData(3, Qt::Vertical, QString(tr("标准差")));
		n_model -> setHeaderData(4, Qt::Vertical, QString(tr("平均值")));
		n_model -> setHeaderData(5, Qt::Vertical, QString(tr("均值上限")));
		n_model -> setHeaderData(6, Qt::Vertical, QString(tr("均值下限")));
		QString sigma_extra;
		if (proj.contains(tr("西格玛")))
		{
			sigma_extra = tr("均值西格玛");
			n_model -> setHeaderData(7, Qt::Vertical, QString(tr("离散值")));
			n_model -> setHeaderData(8, Qt::Vertical, QString(tr("西格玛上限")));
			n_model -> setHeaderData(9, Qt::Vertical, QString(tr("西格玛下限")));
		}
		else
		{
			sigma_extra = tr("均值极差");		  
			n_model -> setHeaderData(7, Qt::Vertical, QString(tr("极差值")));
			n_model -> setHeaderData(8, Qt::Vertical, QString(tr("极差上限")));
			n_model -> setHeaderData(9, Qt::Vertical, QString(tr("极差下限")));
		}
		n_model -> setHeaderData(10, Qt::Vertical, QString(tr("等级")));
		if (!vals_list.isEmpty())
		{
			QStringList engi_list = proj.split("^");
			for (int i = 0; i < 10; i++)
			{
				if (i == 0)
					n_model -> setData(n_model->index(0, 0), engi_list.at(0));
				else if (i == 1)
					n_model -> setData(n_model->index(1, 0), sigma_extra);
				else if (i == 2)
					n_model -> setData(n_model->index(2, 0), vals_list.at(0));
				else if (i == 3)
					n_model -> setData(n_model->index(3, 0), vals_list.at(1));
				else if (i == 4)
					n_model -> setData(n_model->index(4, 0), vals_list.at(2));
				else if (i == 5)
					n_model -> setData(n_model->index(5, 0), vals_list.at(3));
				else if (i == 6)
					n_model -> setData(n_model->index(6, 0), vals_list.at(4));
				else if (i == 7)
					n_model -> setData(n_model->index(7, 0), vals_list.at(5));
				else if (i == 8)
					n_model -> setData(n_model->index(8, 0), vals_list.at(6));
				else
					n_model -> setData(n_model->index(9, 0), vals_list.at(7));
			}
			if (vals_list.size() == 8)
			{
				SpcNum * calculator = qobject_cast<SpcNum *>(data_db->parent());
				QString class_res = calculator->ensureCpkGrade(vals_list.at(0));
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
				n_model -> setData(n_model->index(10, 0), class_res);
			}
			else
				n_model -> setData(n_model->index(10, 0), vals_list.at(8));
		}
		QString lead_time = guider->getInfoByModel(qobject_cast<QStandardItemModel *>(guider->model()), tr("时间"));
		QPair<QString, QStandardItemModel *> save_pair(lead_time, n_model);
		projs_models[lead_time] << save_pair;
	}
	else if (t_type == 2)
	{
		QStringList engi_time = current_plots->currentEngi().split(tr("，，。"));	
		QStringList all_tbls = data_db->allTables();
		QString pro_ktime(QString("%1").arg(QDateTime::fromString(engi_time[1]).toTime_t()));		
		int back_h = 0;
		foreach (QString e_str, all_tbls)
		{		  
			if (e_str.contains(pro_ktime) && e_str.contains("dailydataes"))
			{
				back_h = data_db->lastDataFromTable(e_str, "groupnum").toInt();
				break;
			}
		}
		int rows = current_plots->currentEngiSamples();
		bool ctrl_plot = current_plots->engiCtrlType();
		n_model -> setRowCount(rows+6);
		n_model -> setColumnCount(1);
		n_model -> setHeaderData(0, Qt::Vertical, QString(tr("当前组")));
		n_model -> setData(n_model->index(0, 0), back_h+1);
		for (int i = 1; i < rows+1; i++)
			n_model -> setHeaderData(i, Qt::Vertical, QString(tr("样本%1")).arg(i));
		n_model -> setHeaderData(rows+1, Qt::Vertical, QString(tr("输入者")));
		n_model -> setData(n_model->index(rows+1, 0), inputor.first);
		if (ctrl_plot)
		{
			n_model -> setHeaderData(rows+2, Qt::Vertical, QString(tr("标准差")));
			n_model -> setHeaderData(rows+3, Qt::Vertical, QString(tr("西格玛点状态")));
		}
		else
		{
			n_model -> setHeaderData(rows+2, Qt::Vertical, QString(tr("极差值")));
			n_model -> setHeaderData(rows+3, Qt::Vertical, QString(tr("极差点状态")));
		}
		n_model -> setHeaderData(rows+4, Qt::Vertical, QString(tr("平均值")));
		n_model -> setHeaderData(rows+5, Qt::Vertical, QString(tr("均值点状态")));	
		next_index = n_model->index(1, 0);
	}
	else if (t_type == 3 || t_type == 4)
	{
		n_model -> setRowCount(13);
		n_model -> setColumnCount(1);
		n_model -> setHeaderData(0, Qt::Vertical, QString(tr("工程名称")));
		n_model -> setHeaderData(1, Qt::Vertical, QString(tr("测量单位")));
		n_model -> setHeaderData(2, Qt::Vertical, QString(tr("测量精度")));
		n_model -> setHeaderData(3, Qt::Vertical, QString(tr("控图选择")));
		n_model -> setHeaderData(4, Qt::Vertical, QString(tr("上规范限")));
		n_model -> setHeaderData(5, Qt::Vertical, QString(tr("下规范限")));
		n_model -> setHeaderData(6, Qt::Vertical, QString(tr("每组样本")));
		n_model -> setHeaderData(7, Qt::Vertical, QString(tr("样本组数")));
		n_model -> setHeaderData(8, Qt::Vertical, QString(tr("目标值")));
		n_model -> setHeaderData(9, Qt::Vertical, QString(tr("创建者")));	
		n_model -> setHeaderData(10, Qt::Vertical, QString(tr("密码")));
		n_model -> setHeaderData(11, Qt::Vertical, QString(tr("职位")));
		n_model -> setHeaderData(12, Qt::Vertical, QString(tr("所属部门")));
		QStandardItemModel * del_model = n_model;
		if (t_type == 4)
		{
			QList<QStandardItem *> versions;
			improve_tree -> findChildrenNotAll(improve_tree->currentEngiItem(), versions);
			versions.pop_front();
			QString w_proj("name="+proj);
			QStringList testors = data_db->dataFromTable("projectskeys", "constructor", w_proj).toString().split(tr("；"));
			foreach (QStandardItem * v_item, versions)
			{
				QList<QStandardItem *> paras_children;
				improve_tree -> findChildrenNotAll(v_item->child(0), paras_children);	  
				QStandardItemModel * v_model = new QStandardItemModel(this);
				v_model -> setRowCount(13);
				v_model -> setColumnCount(1);	
				v_model -> setHeaderData(0, Qt::Vertical, QString(tr("工程名称")));
				v_model -> setHeaderData(1, Qt::Vertical, QString(tr("测量单位")));
				v_model -> setHeaderData(2, Qt::Vertical, QString(tr("测量精度")));
				v_model -> setHeaderData(3, Qt::Vertical, QString(tr("控图选择")));
				v_model -> setHeaderData(4, Qt::Vertical, QString(tr("上规范限")));
				v_model -> setHeaderData(5, Qt::Vertical, QString(tr("下规范限")));
				v_model -> setHeaderData(6, Qt::Vertical, QString(tr("每组样本")));
				v_model -> setHeaderData(7, Qt::Vertical, QString(tr("样本组数")));
				v_model -> setHeaderData(8, Qt::Vertical, QString(tr("目标值")));
				v_model -> setHeaderData(9, Qt::Vertical, QString(tr("创建者")));	
				v_model -> setHeaderData(10, Qt::Vertical, QString(tr("密码")));
				v_model -> setHeaderData(11, Qt::Vertical, QString(tr("职位")));
				v_model -> setHeaderData(12, Qt::Vertical, QString(tr("所属部门")));
				QStringList detail_testor = testors.filter(v_item->child(0)->child(0)->child(0)->text());
				QStringList testor = detail_testor.at(0).split(tr("，"));
				QStringList detail_list = data_db->curDBmanageTable().value(testor.back());			
				v_model -> setData(v_model->index(0, 0), proj);
				v_model -> setData(v_model->index(1, 0), paras_children.at(1)->child(0)->text());
				v_model -> setData(v_model->index(2, 0), paras_children.at(2)->child(0)->text());
				v_model -> setData(v_model->index(3, 0), paras_children.at(3)->child(0)->text());
				v_model -> setData(v_model->index(4, 0), paras_children.at(4)->child(0)->text());
				v_model -> setData(v_model->index(5, 0), paras_children.at(5)->child(0)->text());
				v_model -> setData(v_model->index(6, 0), paras_children.at(6)->child(0)->text());
				v_model -> setData(v_model->index(7, 0), paras_children.at(7)->child(0)->text());
				v_model -> setData(v_model->index(8, 0), paras_children.at(8)->child(0)->text());
				v_model -> setData(v_model->index(9, 0), testor.at(1));
				v_model -> setData(v_model->index(10, 0), testor.at(2));
				v_model -> setData(v_model->index(11, 0), detail_list.at(4));
				v_model -> setData(v_model->index(12, 0), detail_list.at(2));
				QPair<QString, QStandardItemModel *> time_version(paras_children.at(0)->child(0)->text(), v_model);
				projs_models[proj] << time_version;
				if (versions.indexOf(v_item) == 0)
					setModel(v_model);	
			}
			if (projs_models.contains(tr("草稿工程，，。")))
			{
				delete del_model;
				del_model = 0;
			}
		}		
		if (!inputor.second.isEmpty() && del_model)
		{
			QStringList detail_list = data_db->curDBmanageTable().value(inputor.second);
			del_model -> setData(del_model->index(9, 0), inputor.first);
			del_model -> setData(del_model->index(10, 0), inputor.second);
			del_model -> setData(del_model->index(11, 0), detail_list.at(4));
			del_model -> setData(del_model->index(12, 0), detail_list.at(2));
		}
	}  
}

void DataesTableView::initStrListViewer(const QModelIndex & pos_index)
{
	if (multi_viewer)
		return;
	multi_viewer = new EditLongToolsViewer(this);
	connect(multi_viewer, SIGNAL(killMyself(QWidget *)), this, SLOT(clearQmlViewStrsPt(QWidget *)));
	QDeclarativeContext * ctxt = multi_viewer->rootContext();
	multi_viewer -> setOrientation(EditLongToolsViewer::ScreenOrientationLockLandscape);
	QStringList fill_strs;
	if (pos_index.row() == 3)
		fill_strs << tr("不选择") << tr("西格玛图") << tr("极差图");
	else if (pos_index.row() == 11)
		fill_strs << tr("不选择") << tr("经理") << tr("工程师") << tr("操作者");  
	qml_strlist = fill_strs;
	ctxt -> setContextProperty("StringesList", QVariant::fromValue(qml_strlist));
	multi_viewer -> setMainQmlFile(QLatin1String("spc_qml/slideSelector.qml"), 10);
	multi_viewer -> setOtherEditingTable(this);
	multi_viewer -> setNestedPosItem(qobject_cast<QStandardItemModel *>(model())->itemFromIndex(pos_index));
	QRect v_rect = visualRect(pos_index);
	multi_viewer -> setGeometry(verticalHeader()->width(), v_rect.topLeft().y(), v_rect.width(), v_rect.height());
	QFontMetrics fm(font());	
	int height = fm.height()*5;
	int v_width = biggerWidthBetweenTextList(fm, qml_strlist)*1.2;
	recalSizeForOpeningNestedViewer(pos_index, QRect(0, 0, v_width, height));	
	openNestedQmlViewerOnCell(multi_viewer, v_width, height);	
}

void DataesTableView::openNestedQmlViewerOnCell(EditLongToolsViewer * nesting, int end_width, int end_height)
{
	nested_animation = new QPropertyAnimation(this);
	connect(nested_animation, SIGNAL(valueChanged(const QVariant &)), nesting, SLOT(rectangleAnimating(const QVariant &)));
	connect(nested_animation, SIGNAL(finished()), this, SLOT(clearNestedAnimation()));
	nested_animation -> setTargetObject(nesting);
	nested_animation -> setPropertyName("geometry");
	nested_animation->setDuration(500);
	QRect nested_startRect(QRect(nesting->frameGeometry().topLeft().x(), nesting->frameGeometry().topLeft().y(), 0, 0));
	QRect nested_endRect(QRect(nesting->frameGeometry().topLeft().x(), nesting->frameGeometry().topLeft().y(), end_width, end_height));
	nested_animation->setStartValue(nested_startRect);
	nested_animation->setEndValue(nested_endRect);
	nested_animation -> start();
	nesting -> showExpanded();	
}

void DataesTableView::canInputtedCells(QList<int> & cells)
{
	int n = model()->rowCount();  
	for (int i = 0; i < n; i++)
	{
		if (isInputtedCell(model()->index(i, 0)))
			cells << i;
	}
}

void DataesTableView::recalSizeForOpeningNestedViewer(const QModelIndex & index, const QRect & n_rect)
{
 	QRect pos_rect = visualRect(index);
	if (n_rect.width()+pos_rect.left()>width() || n_rect.height()+pos_rect.top()>height())
	{
		if (n_rect.width()+pos_rect.left() > width())
			setFixedWidth(n_rect.width()+pos_rect.left());
		if (n_rect.height()+pos_rect.top() > height())
			setFixedHeight(n_rect.height()+pos_rect.top());		
		emit sizeChanged();
	}  
}

bool DataesTableView::isInputtedCell(const QModelIndex & index)
{
	QString p_str = model()->headerData(index.row(), Qt::Vertical).toString();
	if (p_str.contains(tr("样本")))
		return true;
	else
		return false;	
}

int DataesTableView::biggerWidthBetweenTextList(const QFontMetrics & fm, const QStringList & strs_list)
{	
	int max_listwidth = 0;
	foreach (QString w_str, strs_list)
	{	
		if (max_listwidth < fm.width(w_str))
			max_listwidth = fm.width(w_str);
	}		
	return max_listwidth;
}

QStandardItemModel * DataesTableView::matchProjTestTimeModel(const QString & saveProj, const QString & testTime)
{
	QList<QPair<QString, QStandardItemModel *> > dest_list = projs_models.value(saveProj);
	for (int i = 0; i < dest_list.size(); i++)
	{
		if (dest_list.at(i).first == testTime)
			return dest_list.at(i).second;
	}
	return 0;
}

QModelIndex DataesTableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
	if (t_type==0 || (t_type==2 && currentIndex().row()<6 && currentIndex().row()>0))
		return QTableView::moveCursor(cursorAction, modifiers);
	return currentIndex();
}