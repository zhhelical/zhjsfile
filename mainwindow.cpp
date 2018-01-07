#include <QtSql/QSqlTableModel>
#include "mainwindow.h"
#include "titlebar.h"
#include "spcnum.h"
#include "plotwidget.h"
#include "toolsplot.h"
#include "dialog.h"
#include "slidetoolbar.h"
#include "editlongtoolsviewer.h"
#include "engitreeview.h"
#include "tablemanagement.h"
#include "tablesgroup.h"
#include "dataestableview.h"
#include "relatedtreeview.h"
#include "helpcontents.h"
#include "littlewidgetsview.h"
#include "littlewidgetitem.h"
#include "spcdatabase.h"

MainWindow::MainWindow(QWidget * parent)
:QFrame(parent, Qt::FramelessWindowHint), user_mode(false), protect_mode(false), wins_moving(false), showing_toolBar(0), leftw_container(0), rightw_container(0), protect_widget(0), l_animation1(0), l_animation2(0), l_animation3(0), l_animation4(0), r_animation1(0), r_animation2(0), r_animation3(0), r_animation4(0), v1_group(0), v2_group(0), v3_group(0), v4_group(0), s_group(0), p_animation(0), c_animation(0), e_group(0)
{
 	back_science = new SpcNum(this);
	base_db = new SpcDataBase(back_science); 
	QString default_db = checkDBfile();
	if (!default_db.isEmpty())
		initDbAndSpcCalculation(default_db); 
	else
		sys_userState = -1;
	leftw_container = new LittleWidgetsView(this);
	leftw_container -> setObjectName("l_container");	
	setStyleSheet("border:none");
	setFont(QFont(tr("宋体")));	
}

MainWindow::~MainWindow()
{}

bool MainWindow::logingState()
{
	if (loging_pairs.isEmpty())
		return false;
	return true;
}

bool MainWindow::logingModeDefinition()
{
	return user_mode;
}

SpcDataBase * MainWindow::curBackGuider()
{
	return base_db;
}

QWidget * MainWindow::curToolBar()
{
	return showing_toolBar;
}

void MainWindow::createMainToolBars()
{
	if (protect_mode)
	{
		if (showing_toolBar)
			return;
		if (!user_mode && !loging_pairs.isEmpty())
			createLoginToolbar(0, loging_user.first);
		else
		{
			QString name = "";
			createLoginToolbar(0, name);
		}
		return;
	}
	if (leftw_container->objectName() == "helppages")
		return;
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	QWidget * tmp_bar = showing_toolBar;
	if (stored_bar)
	{
		if (tmp_bar==stored_bar)
		{
			if (stored_bar->height() == 0)
				animateToolBarWidgetUpDown(0, leftw_container, stored_bar, rightw_container);
			return;
		}
		else
			animateToolBarWidgetUpDown(tmp_bar, leftw_container, stored_bar, rightw_container, false);
		return;
	}
	QPair<QString, QString> name_pair; 
	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user;		
	if (tmp_bar)
	{	  
		if (tmp_bar->objectName()=="maintoolBar")
			return;
		DataesTableView * init_engi = 0;
		if (rightw_container)
			init_engi = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
		if (name_pair.second.isEmpty() && init_engi)
		{
			name_pair.first = init_engi->model()->data(init_engi->model()->index(9, 0)).toString(); 
			name_pair.second = init_engi->model()->data(init_engi->model()->index(10, 0)).toString(); 			
		}
		if (!ws_cpktests.value(name_pair).isEmpty() && ws_cpktests.value(name_pair).at(0) && tmp_bar->height()==0)
		{  
			animateToolBarWidgetUpDown(0, leftw_container, tmp_bar, rightw_container);
			return;
		}	
		if (tmp_bar->objectName() == tr("okcancel_数据库编辑"))	
		{
			QString default_db = checkDBfile();
			if (!default_db.isEmpty())
				initDbAndSpcCalculation(default_db); 
			else
				sys_userState = -1;		  
		}
		if (tmp_bar->objectName() != "helpsBar")
			animateToolBarLeftRight(tmp_bar, 0);
		slideWidgetView(leftw_container, rightw_container, 0);			
	}		
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("maintoolBar");	
	new_toolBar -> showMainTools(sys_userState);
	new_toolBar -> show();
	animateToolBarLeftRight(0, new_toolBar);
}

void MainWindow::createDbSourceTreeView()
{
	QString warn_info;
	if (base_db)
		warn_info = "warning:"+tr("当前有数据库处于使用状态\n确认此操作需要关闭数据库\n并同时需要您输入姓名密码");
	else
		warn_info = "warning:"+tr("编辑创建数据库等操作需要您\n输入姓名密码");
	DiaLog * w_dialog = new DiaLog;
	w_dialog -> initWinDialog(this, warn_info, 0);	
	if (!w_dialog->exec())
		return;
	else
		clearVariantsBackToOriginalState();	
	RelatedTreeView * db_tree = new RelatedTreeView;
	db_tree -> initTree(base_db, new_login, 7);
	db_tree -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	db_tree -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	connect(db_tree, SIGNAL(secondOpenDatabase(const QString &)), this, SLOT(initDbAndSpcCalculation(const QString &)));	
	rightw_container = new LittleWidgetsView(this);
	rightw_container -> setObjectName("r_container");
	rightw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/treeviewer.qml");	
	QRect o_rect;
	db_tree -> setFixedHeight(height()-64);
	o_rect.setWidth(db_tree->sizeHint().width());
	o_rect.setHeight(db_tree->height());
	rightw_container -> initMarginForParent(o_rect);
	rightw_container -> setViewForRelatedTable(db_tree);	
	QString db_tool = "dbsediting";
	createOkCancelBars(showing_toolBar, db_tool, db_tree->commandStack());	
	slideWidgetView(leftw_container, rightw_container);
	db_tree -> setInitViewRect(rightw_container->initRect());
}

void MainWindow::usrOwnedMultiWinsTools()
{
	if (protect_mode)
		return;
	if (loging_pairs.isEmpty() || (showing_toolBar && showing_toolBar->objectName()=="multiwinsBar"))
		return;
	if (leftw_container->objectName() == "helppages")
		return;	
	QWidget * tmp_bar = showing_toolBar;
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("multiwinsBar");
	new_toolBar -> showMultiWinsEditTools();
	if (stored_bar)
	{
		new_toolBar -> setGeometry(width()-new_toolBar->sizeHint().width(), -64, new_toolBar->sizeHint().width(), 64);
		animateToolBarWidgetUpDown(tmp_bar, leftw_container, new_toolBar, rightw_container);
	}
	else
	{
		syncCurrentWinWithHash(leftw_container, rightw_container, loging_user);
		if (tmp_bar && tmp_bar->objectName()=="helpsBar")
			animateToolBarLeftRight(0, new_toolBar);
		else
			animateToolBarLeftRight(tmp_bar, new_toolBar, !storedBarOnHash(leftw_container, rightw_container));
	}
	new_toolBar -> show();
}

void MainWindow::okToolPressed()
{
	if (wins_moving)
		return;
	QString show_str = showing_toolBar->objectName();
	QWidget * tmp_showBar = showing_toolBar;	
	if (show_str == tr("okcancel_数据库编辑"))
	{
		RelatedTreeView * db_tree = qobject_cast<RelatedTreeView *>(rightw_container->qmlAgent()->widget());
		QString error_info;
		if (!db_tree->dbsEdittingAction(error_info))
		{
			if (error_info == tr("没有编辑项"))
				return;
			QString fur_lbl = "error:"+error_info;
			DiaLog * w_dialog = new DiaLog;
			w_dialog -> initWinDialog(this, fur_lbl, 3);
			return;		  
		}
		animateToolBarLeftRight(tmp_showBar, 0);			
		slideWidgetView(leftw_container, rightw_container, 0);			
	}
	else if (show_str == tr("okcancel_CPK数据") || show_str == tr("okcancel_新建1")/*进入点图*/)
	{	
		QPair<QString, QString> name_pair; 
		if (loging_user.second.isEmpty())
			name_pair = new_login;
		else
			name_pair = loging_user;		  
		DataesTableView * initEngi_table = 0;			  
		bool init_open = !qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());	  
		if (show_str == tr("okcancel_CPK数据"))
		{  
			initEngi_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
			initEngi_table->commandStack()->clear();
			QString proj_name = initEngi_table->model()->data(initEngi_table->model()->index(0, 0)).toString();	
			QString checking;		
			if (!initEngi_table->checkInputState(checking))
			{
				QString error_info("error:"+tr("输入有误，请检查输入！"));
				DiaLog * w_dialog = new DiaLog;
				w_dialog -> initWinDialog(this, error_info, 1);
				return;
			}
			QString chk_pw("name="+proj_name);
			QString chk_str = base_db->dataFromTable("projectskeys", "unit", chk_pw).toString();
			QString key_time = initEngi_table->modelConstructTime(proj_name, qobject_cast<QStandardItemModel *>(initEngi_table->model()));
			toggleCurrentBarButton(tr("保存当前"), tr("使能"), !chk_str.contains(key_time));
			toggleCurrentBarButton(tr("删除新建"), tr("使能"), !chk_str.contains(key_time));				
			if (!init_open)
			{		  
				DataesTableView * reset_tbl = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
				QStandardItemModel * chk_model = initEngi_table->existedEntirelyModelInHash(proj_name);				
				if (chk_model)
				{
					reset_tbl -> changeModelForEditting(qobject_cast<QStandardItemModel *>(reset_tbl->model()), tr("使能"), initEngi_table);
					reset_tbl -> resetEditState(true);					
				}
				else
				{
					QStringList init_projs = initEngi_table->projsInsertSequence();								  
					QString test_time = QDateTime::currentDateTime().toString();
					initEngi_table -> tmpSaveProjsModels(proj_name, test_time);				
					QStandardItemModel * cursor = initEngi_table->matchNewKeysModel(proj_name, test_time);
					initEngi_table -> setShiftModel(cursor);
					test_time.clear();
					reset_tbl -> changeModelForEditting(qobject_cast<QStandardItemModel *>(reset_tbl->model()), tr("使能"), initEngi_table);
					reset_tbl -> resetEditState(true);					
				}
				QWidget * cache_bar = ws_cpktests.value(name_pair).at(0);				
				reset_tbl->commandStack()->clear();
				if (qobject_cast<SlideBar *>(cache_bar)->currentCommand() != reset_tbl->commandStack()) 
					qobject_cast<SlideBar *>(cache_bar) -> setCommandStack(reset_tbl->commandStack());		
				animateToolBarWidgetUpDown(tmp_showBar, 0, cache_bar, 0);				
			}
			else
			{
				if (name_pair.second.isEmpty())
				{
					new_login.first = initEngi_table->model()->data(initEngi_table->model()->index(9, 0)).toString();
					new_login.second = initEngi_table->model()->data(initEngi_table->model()->index(10, 0)).toString();
				}
				ws_cpktests[new_login] << 0 << 0 << 0 << 0 << 0 << 0 << 0;	
				ws_cpktests[new_login][0] = tmp_showBar;
				leftw_container -> setObjectName(tr("点图"));
				ws_cpktests[new_login][1] = leftw_container;
				rightw_container -> setObjectName(tr("关键数据表"));
				ws_cpktests[new_login][2] = rightw_container;
				QString test_time = QDateTime::currentDateTime().toString();
				initEngi_table -> tmpSaveProjsModels(proj_name, test_time);
				QStandardItemModel * cursor = initEngi_table->matchNewKeysModel(proj_name, test_time);
				initEngi_table -> setShiftModel(cursor);								
				PlotWidget * plot_widget = qobject_cast<PlotWidget *>(leftw_container->qmlAgent()->widget());
				leftw_container = new LittleWidgetsView(this);
				leftw_container -> setObjectName(tr("CPK数据表"));
				ws_cpktests[new_login][3] = leftw_container;
				QRect o_rect(0, 0 , width(), height());
				leftw_container -> initMarginForParent(o_rect);
				leftw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");
				DataesTableView * dataesin_table = new DataesTableView(0, 0, base_db); 
				dataesin_table -> setObjectName("dataesin");
				dataesin_table -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
				dataesin_table -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
				connect(dataesin_table, SIGNAL(changeBtnEnable(const QString &, const QString &, bool)), this, SLOT(toggleCurrentBarButton(const QString &, const QString &, bool)));
				if (plot_widget)
					dataesin_table -> setPlotsBackGuide(plot_widget);
				dataesin_table -> setInputor(new_login);				
				dataesin_table -> initTable(proj_name, initEngi_table);
				dataesin_table -> setFixedSize(dataesin_table->horizontalHeader()->length()+dataesin_table->verticalHeader()->width(), dataesin_table->verticalHeader()->length()+dataesin_table->horizontalHeader()->height());
				initEngi_table -> setCpkProcessPartner(dataesin_table);
				leftw_container -> setViewForRelatedTable(dataesin_table);
				leftw_container -> setGeometry(-leftw_container->width(), 0, leftw_container->width(), height());
				createOkCancelBars(ws_cpktests.value(new_login).at(0));				
				slideLeftQmlWidgets(ws_cpktests.value(new_login).at(1), leftw_container, rightw_container, true);										
				tmp_showBar -> raise();	
				ws_cpktests.value(new_login).at(1) -> raise();
				QString win_lbl = tr("CPK数据编辑中");
				emit newTheme(win_lbl);			
			}
			initEngi_table -> resetEditState(false);	
			DataesTableView * current_din = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
			current_din -> syncUnredoRelatedCmds(qobject_cast<QStandardItemModel *>(initEngi_table->model()));
		}
		else
		{
			initEngi_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(2))->qmlAgent()->widget());		  
			QString current_proj = initEngi_table->model()->data(initEngi_table->model()->index(0, 0)).toString();
			QString chk_pw("name="+current_proj);
			QString chk_str = base_db->dataFromTable("projectskeys", "unit", chk_pw).toString();
			QString key_time = initEngi_table->modelConstructTime(current_proj, qobject_cast<QStandardItemModel *>(initEngi_table->model()));
			QString tengi_stamp = QString("%1").arg(QDateTime::fromString(key_time).toTime_t());
			QStringList chk_all = base_db->allTables();
			if (!chk_str.contains(key_time) || chk_all.filter(tengi_stamp).isEmpty())
			{
				QString w_info = "warning:"+tr("此工程未计算或保存\n计算并且保存后才能进入下个界面\n要保存吗？");
				DiaLog * w_dialog = new DiaLog;
				w_dialog -> initWinDialog(this, w_info, 0);
				if (w_dialog->exec())
				{
					if (!saveProjInitPerformance())
						return;
				}
				else
					return;
			}			
			clearRalatedEdittingBarOnHash(tmp_showBar);			
			LittleWidgetsView * old_container = leftw_container;
			LittleWidgetsView * g_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(1));
			leftw_container = g_container;
			PlotWidget * plot_set = qobject_cast<PlotWidget *>(g_container->qmlAgent()->widget());			
			plot_set -> setEnginame(current_proj+tr("，，。")+key_time);
			if (loging_pairs.isEmpty())
			{
				loging_user.first = initEngi_table->model()->data(initEngi_table->model()->index(9, 0)).toString();
				loging_user.second = initEngi_table->model()->data(initEngi_table->model()->index(10, 0)).toString();
				loging_pairs << loging_user;
				sys_userState = base_db->userClass(loging_user);
			}
			LittleWidgetsView * old_right = rightw_container;
			rightw_container = new LittleWidgetsView(this);
			rightw_container -> setObjectName("r_container");
			rightw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/treeviewer.qml");	
			DataesTableView * tmpinput_table = new DataesTableView(0, 2, base_db);
			tmpinput_table -> setObjectName("tmpinput");
			tmpinput_table -> setPlotsBackGuide(plot_set);
			tmpinput_table -> setInputor(loging_user);
			tmpinput_table -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			tmpinput_table -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	
			tmpinput_table -> setFixedHeight(height()-64);
			QRect o_rect;
			o_rect.setWidth(old_right->sizeHint().width());
			o_rect.setHeight(old_right->height());
			tmpinput_table -> initTable(current_proj);
			rightw_container -> initMarginForParent(o_rect);	
			rightw_container -> setViewForRelatedTable(tmpinput_table);
			rightw_container -> setGeometry(-width()+old_right->width(), 64, old_right->width(), height()-64);
			slideMultiWidgets(leftw_container, rightw_container, old_container, old_right, true, true);		
			tmpinput_table -> setRightOrigRect(o_rect);  
			createOkCancelBars(tmp_showBar, plot_set->objectName());			
			syncCurrentWinWithHash(leftw_container, rightw_container, loging_user);
			storeRalatedEdittingBar(leftw_container, rightw_container, showing_toolBar);			
			emit newTheme(current_proj);	
		}
	}
	else if (show_str == tr("okcancel_输入"))
	{
		DataesTableView * tmpinput_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
		QString checking;		
		if (!tmpinput_table->checkInputState(checking) || tmpinput_table->model()->data(tmpinput_table->model()->index(tmpinput_table->model()->rowCount()-2, 0)).isNull())
			tmpinput_table -> clearRelatedCellsForReinput();
		else
		{
			QString warn_info = "warning:"+tr("数据未存储，\n不保存而清空数据吗？");
			DiaLog * w_dialog = new DiaLog;
			w_dialog -> initWinDialog(this, warn_info, 0);			
		}
	}
	else if (show_str==tr("okcancel_加入工程") || show_str==tr("okcancel_授权用户") || show_str==tr("okcancel_权限") || show_str==tr("okcancel_产品") || show_str==tr("okcancel_属性数据"))
	{
		RelatedTreeView * related_tree = qobject_cast<RelatedTreeView *>(rightw_container->qmlAgent()->widget());
		if (!related_tree -> checkInputState())
		{
			DiaLog * h_dialog = new DiaLog;
			h_dialog -> initWinDialog(this, QString(tr("hint:您没有做任何改动\n无需保存")), 0);
			return;
		}
		if (!related_tree->storeDataesTodb()) 
		{
			DiaLog * w_dialog = new DiaLog;
			w_dialog -> initWinDialog(this, QString(tr("error:数据未存储\n请再试一次")), 0);
		}
		else
		{
			animateToolBarLeftRight(tmp_showBar, 0);				
			slideWidgetView(leftw_container, rightw_container, 0);				
		}
	}
	else if (show_str==tr("okcancel_备份数据"))
	{
		TableManagement * bkup_tree = qobject_cast<TableManagement *>(rightw_container->qmlAgent()->widget());
		if (bkup_tree->backupRelatedDbForItems()) 
		{
			animateToolBarLeftRight(tmp_showBar, 0);	
			slideWidgetView(leftw_container, rightw_container, 0);				
		}
		else
		{
			DiaLog * w_dialog = new DiaLog;
			w_dialog -> initWinDialog(this, QString(tr("error:数据未备份成功\n请再试一次")), 0);
		}
	}
}

void MainWindow::hideToolPressed()
{
	QWidget * tmp_showBar = showing_toolBar;
	QPair<QString, QString> name_pair; 
	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user;	
	if (rightw_container && rightw_container->geometry().topLeft().x()<width())
	{
		if (showing_toolBar->objectName() == "multiwinsBar")
			animateToolBarWidgetUpDown(tmp_showBar, leftw_container, 0, rightw_container, false);
		else
			animateToolBarWidgetUpDown(tmp_showBar, leftw_container, 0, rightw_container);
		rightw_container -> resetViewWidth(rightw_container->rect());
		rightw_container->qmlAgent()->widget() -> setFixedHeight(height());
	}
	else
	{
		QWidget * cur_stored = storedBarOnHash(leftw_container, rightw_container);
		if (cur_stored)
		{
			if (cur_stored==tmp_showBar)
				animateToolBarWidgetUpDown(tmp_showBar, leftw_container, 0, 0);
			else
				animateToolBarWidgetUpDown(tmp_showBar, leftw_container, 0, 0, false);
		}
		else if (!(ws_cpktests.value(name_pair).isEmpty() && ws_cpktests.value(name_pair).at(0)) || rightw_container)
		{
			if (rightw_container->geometry().topLeft().x() < width())
				animateToolBarWidgetUpDown(tmp_showBar, 0, 0, rightw_container);
			else
				animateToolBarWidgetUpDown(tmp_showBar, 0, 0, 0);
		}
		else
			animateToolBarLeftRight(tmp_showBar, 0);		
	}		
}

void MainWindow::hideRwinPressed(bool down)
{
	if (!rightw_container)
		return;
	toggleCurrentBarButton(tr("隐藏右窗"), tr("切换"), down);
	if (down)
		slideWidgetView(leftw_container, rightw_container, 1);
	else
		slideWidgetView(leftw_container, rightw_container);	
}

void MainWindow::relatedEditting(bool down)
{
	if (!rightw_container)
		return;
	QWidget * editting = rightw_container->qmlAgent()->widget();
	if (qobject_cast<RelatedTreeView *>(editting))
	{
		if (sys_userState>1 && !base_db->isDatabaseConstructor(loging_user) && qobject_cast<RelatedTreeView *>(editting)->treeType()!=1 && allManageProjsReadOnly())
			return;	  
		qobject_cast<RelatedTreeView *>(editting) -> setFreeForEdit(down);
	}
	else
	{
		if (sys_userState>1 && !base_db->isDatabaseConstructor(loging_user) && allManageProjsReadOnly())
			return;	  
		qobject_cast<TableManagement *>(editting) -> setTypeTwoEditting(down);	
	}
	toggleCurrentBarButton(tr("开始编辑"), tr("切换"), down);		
}

void MainWindow::savePromotingResultation()//need add engies minus codes——finished??
{
 	if (!ws_cpktests.value(loging_user).at(3)) 
		return;
	EngiTreeView * improve_tree = qobject_cast<EngiTreeView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(2))->qmlAgent()->widget());
	DataesTableView * improveEngi_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(3))->qmlAgent()->widget());
	DataesTableView * dataesin_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(4))->qmlAgent()->widget());	
	QStandardItemModel * dt_model = qobject_cast<QStandardItemModel *>(improve_tree->model());
	QList<QStandardItem *> fds_new = dt_model->findItems(tr("新增"), Qt::MatchRecursive | Qt::MatchContains); 
	if (fds_new.isEmpty() && improveEngi_table->tmpDeletionModelsCache().isEmpty() && dataesin_table->tmpDeletionModelsCache().isEmpty())
		return;
	DataesTableView * calculate_table = 0;
	if (ws_cpktests.value(loging_user).at(5))
		calculate_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(5))->qmlAgent()->widget());
	if (!improve_tree->saveProjsPromotions(fds_new, dataesin_table, calculate_table, improveEngi_table))
	{
		QString warn_info = "warning:"+tr("数据未存储，\n请再试一次");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 0);
	}
}

void MainWindow::projsPromotingAction()
{
 	QWidget * tmp_bar = showing_toolBar;
	clearRalatedEdittingBarOnHash(tmp_bar);
	EngiTreeView * improve_tree = qobject_cast<EngiTreeView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(2))->qmlAgent()->widget());	
	QString c_proj = improve_tree->currentEngiItem()->text();
	if (sys_userState>1 && !base_db->isDatabaseConstructor(loging_user) && !base_db->isConstructor(loging_user, c_proj))
	{
		QStringList m_proj = base_db->curDBmanageTable().value(loging_user.second).at(3).split(";").filter(c_proj);
		if (!m_proj.at(0).contains(tr("读写")))
			return;
	}	
	SlideBar * new_toolBar = 0;
	if (ws_cpktests.value(loging_user).at(0) && ws_cpktests.value(loging_user).at(0)->objectName()=="engitestsBar")
	{
		new_toolBar = qobject_cast<SlideBar *>(ws_cpktests.value(loging_user).at(0));
		rightw_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(3));	
		if (rightw_container->width() == 0)
			rightw_container -> setFixedWidth(rightw_container->initRect().width());
		leftw_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(4));
		DataesTableView * improveEngi_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
		if (improveEngi_table->model()->data(improveEngi_table->model()->index(0, 0)).toString() != c_proj)
		{
			DataesTableView * dataesin_table = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
			if (!improveEngi_table->modelHashs().contains(c_proj))
			{
				improve_tree -> initModelsForRelatedView(improveEngi_table);	
				improve_tree -> initModelsForRelatedView(dataesin_table, improveEngi_table);				
			}
			QStandardItemModel * key_model = improveEngi_table->modelHashs().value(c_proj).at(0).second;
			if (key_model != improveEngi_table->model())
				improveEngi_table -> setModel(key_model);
			improveEngi_table -> setShiftModel(key_model);
			QString by_time(tr("时间"));
			QString d_time = improveEngi_table->getInfoByModel(key_model, by_time);
			QStandardItemModel * din_model = dataesin_table->modelHashs().value(d_time).at(0).second;
			if (din_model != dataesin_table->model())
			{
				dataesin_table -> setModel(din_model);
				dataesin_table -> changeViewStateForNewModel();
			}
			dataesin_table -> setShiftModel(din_model);	
		}
		QString manual_chk(tr("草稿工程，，。"));
		if (improveEngi_table->model() == improveEngi_table->modelHashs().value(manual_chk).at(0).second)
			new_toolBar -> setCommandStack(improveEngi_table->commandStack());
		else
			new_toolBar -> setCommandStack(improveEngi_table->delModelsStack());
	}
	else
	{	  
		ws_cpktests[loging_user][1] = leftw_container;	  
		DataesTableView * improveEngi_table = new DataesTableView(0, 4, base_db, false);
		improveEngi_table -> setObjectName("improveEngi");
		new_toolBar = new SlideBar(this);
		new_toolBar -> setObjectName("engitestsBar");
		QToolButton * sams_edit = new QToolButton(new_toolBar);
		sams_edit -> setIcon(QIcon(":/images/import.png"));
		sams_edit -> setText(tr("编辑样本"));
		sams_edit -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		sams_edit -> setFont(QFont(tr("宋体"), 8));
		sams_edit -> setIconSize(QSize(40, 40));
		sams_edit -> setFixedHeight(64);
		sams_edit -> setFixedWidth(55);        
		sams_edit -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE;}");
		connect(sams_edit, SIGNAL(clicked()), this, SLOT(improveEngiSamples()));				
		improveEngi_table -> setInputor(loging_user);
		improveEngi_table -> setPromotionTreePtr(improve_tree);
		improveEngi_table -> setFixedHeight(height()-64);
		improveEngi_table -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		improveEngi_table -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	
		connect(improveEngi_table, SIGNAL(changeBtnEnable(const QString &, const QString &, bool)), this, SLOT(toggleCurrentBarButton(const QString &, const QString &, bool)));
		connect(improveEngi_table, SIGNAL(changeStackCommOrder(QUndoStack *)), this, SLOT(promotionEdittingOrderTrans(QUndoStack *)));
		connect(improveEngi_table, SIGNAL(inputted(DataesTableView *)), this, SLOT(refreshTableSize(DataesTableView *)));	
		DataesTableView * dataesin_table = new DataesTableView(0, 0, base_db, true); 
		dataesin_table -> setObjectName("dataesin");		
		dataesin_table -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		dataesin_table -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		connect(dataesin_table, SIGNAL(changeBtnEnable(const QString &, const QString &, bool)), this, SLOT(toggleCurrentBarButton(const QString &, const QString &, bool)));
		dataesin_table -> setInputor(loging_user);	
		dataesin_table -> setFixedSize(dataesin_table->horizontalHeader()->length()+dataesin_table->verticalHeader()->width(), dataesin_table->verticalHeader()->length()+dataesin_table->horizontalHeader()->height());
		improveEngi_table -> setCpkProcessPartner(dataesin_table);
		dataesin_table -> setPromotionTreePtr(improve_tree);		
		improve_tree -> initModelsForRelatedView(improveEngi_table);	
		improve_tree -> initModelsForRelatedView(dataesin_table, improveEngi_table);			
		dataesin_table -> resetEditState(false);
		new_toolBar -> setCommandStack(improveEngi_table->delModelsStack());
		new_toolBar -> showImproveTools(sams_edit);
		new_toolBar -> setGeometry(width()-new_toolBar->sizeHint().width(), -64, new_toolBar->sizeHint().width(), 64);
		ws_cpktests[loging_user][0] = new_toolBar;			
		leftw_container = new LittleWidgetsView(this);
		leftw_container -> setObjectName(tr("CPK数据表"));
		leftw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");	
		QRect lo_rect(0, 0 , width(), height());
		leftw_container -> initMarginForParent(lo_rect);		
		leftw_container -> setViewForRelatedTable(dataesin_table);
		leftw_container -> setGeometry(width(), 0, width()-rightw_container->width(), height());
		rightw_container = new LittleWidgetsView(this);
		rightw_container -> setObjectName("r_container");
		rightw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablesviewer.qml");	
		QRect ro_rect(0, 0, ws_cpktests.value(loging_user).at(2)->sizeHint().width(), ws_cpktests.value(loging_user).at(2)->height());
		rightw_container -> initMarginForParent(ro_rect);	
		rightw_container -> setViewForRelatedTable(improveEngi_table);	
		rightw_container -> setGeometry(2*width()-ws_cpktests.value(loging_user).at(2)->width(), 64, ws_cpktests.value(loging_user).at(2)->width(), height()-64);
		ws_cpktests[loging_user][3] = rightw_container;	
		ws_cpktests[loging_user][4] = leftw_container;
		improveEngi_table -> setRightOrigRect(rightw_container->initRect());
	}
	animateToolBarWidgetUpDown(tmp_bar, 0, new_toolBar, 0);	
	new_toolBar -> show();
	storeRalatedEdittingBar(leftw_container, rightw_container, new_toolBar);
	slideMultiWidgets(leftw_container, rightw_container, ws_cpktests.value(loging_user).at(1), ws_cpktests.value(loging_user).at(2), false, false);	 
	syncCurrentWinWithHash(leftw_container, rightw_container, loging_user);
	new_toolBar -> raise();
	QString win_lbl = tr("CPK数据编辑中");
	emit newTheme(win_lbl);				
}

void MainWindow::cancelToolPressed()
{
	QWidget * clear_bar = showing_toolBar;
	if (clear_bar->objectName() == tr("okcancel_数据库编辑"))
	{
		QString default_db = checkDBfile();
		if (!default_db.isEmpty())
			initDbAndSpcCalculation(default_db); 
		else
			sys_userState = -1;		  
	}
	if (!protect_mode)
	{
		QWidget * last_widget = leftw_container->qmlAgent()->widget();
		PlotWidget * plot_find = 0;			
		if (last_widget->objectName() != "plot")
		{
			LittleWidgetsView * g_leftw = new LittleWidgetsView(this);
			g_leftw -> setObjectName("l_container");
			g_leftw -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");
			QRect o_rect(0, 0 , width(), height());
			g_leftw -> initMarginForParent(o_rect);
			plot_find = new PlotWidget(back_science, base_db);
			plot_find -> setObjectName("plot");
			plot_find -> setFixedWidth(width());
			plot_find -> setFixedHeight(height());		
			g_leftw -> setViewForRelatedTable(plot_find);	
			LittleWidgetsView * old_left = leftw_container;
			LittleWidgetsView * old_right = rightw_container;
			bool helps_show = false;
			QWidget * del_left = 0;
			QWidget * del_right = 0;
			QWidget * del_bar = 0;
			if (old_left->objectName() == "helppages")
			{
				helps_show = true;
				stored_widgets.remove(QPair<QString, QString>("helps", ""));
				stored_bars.remove(old_left);
				QList<QPair<QWidget *, QWidget *> > lr_widgets = stored_widgets.value(loging_user);
				for (int i = 0; i < lr_widgets.size(); i++)
				{
					if (stored_bars.contains(lr_widgets.at(i).first))
					{		  
						del_left = lr_widgets.at(i).first;
						del_right = lr_widgets.at(i).second;
						QList<QPair<QWidget *, QWidget *> > lr_bars = stored_bars.value(del_left);
						for (int j = 0; j < lr_bars.size(); j++)
						{
							if (lr_bars.at(j).first == del_right)
							{
								del_bar = lr_bars.at(j).second;
								break;
							}
						}
						break;
					}		
				}			
			}
			if (helps_show)
			{
				if (del_left)
					delete del_left;
				if (del_right)
					delete del_right;
				if (del_bar)
					delete del_bar;
			}
			else
				clearRalatedEdittingBarOnHash(clear_bar);			
			slideMultiWidgets(g_leftw, 0, old_left, old_right, true, true);
			syncCurrentWinWithHash(leftw_container, 0, loging_user);
		}
		else
		{
			plot_find = qobject_cast<PlotWidget *>(last_widget);	 
			slideWidgetView(leftw_container, rightw_container, 0); 			
		}
		plot_find -> setEnginame();	
		QString null_theme = tr("工程");
		emit newTheme(null_theme);	
	}
	animateToolBarLeftRight(clear_bar, 0);
}

void MainWindow::createSonLoginMenu()
{
	QWidget * tmp_bar = showing_toolBar;
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("sonloginBar");
	if (loging_pairs.isEmpty())
		new_toolBar -> showSonLoginMenu(new_login);
	else
	 	new_toolBar -> showSonLoginMenu(loging_user, base_db); 
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);
}

void MainWindow::createPowersTool()
{
	QWidget * tmp_bar = showing_toolBar;	
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("powerstoolBar");
	new_toolBar -> showPowersToolsMenu();
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);	
}

void MainWindow::joinProject()
{
	RelatedTreeView * waitpower_tree = new RelatedTreeView;
	waitpower_tree -> setObjectName("waitpower");
	QPair<QString, QString> joinor;
	if (newUserDifWithLoginger())
		joinor = new_login;
	else
		joinor = loging_user;
	waitpower_tree -> initTree(base_db, joinor, 1);
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	QWidget * tmp_bar = showing_toolBar;
	if (stored_bar && stored_bar->objectName()==tr("okcancel_加入工程"))
	{
		animateToolBarLeftRight(tmp_bar, stored_bar);
		resetToolBarsVarsValue(stored_bar, true);
	}
	else
		createOkCancelBars(tmp_bar, waitpower_tree->objectName(), waitpower_tree->commandStack());
	embededOrSlideWidgetOnRightQml(waitpower_tree);
	waitpower_tree -> setInitViewRect(rightw_container->initRect());	
}

void MainWindow::empowerUserForProj()
{
	RelatedTreeView * empower_tree = new RelatedTreeView;
	empower_tree -> setObjectName("empower");
	QPair<QString, QString> empower;
	if (newUserDifWithLoginger())
		empower = new_login;
	else
		empower = loging_user;
	empower_tree -> initTree(base_db, empower, 2);
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	QWidget * tmp_bar = showing_toolBar;
	if (stored_bar && stored_bar->objectName()==tr("okcancel_授权用户"))
	{
		animateToolBarLeftRight(tmp_bar, stored_bar);
		resetToolBarsVarsValue(stored_bar, true);
	}
	else
		createOkCancelBars(tmp_bar, empower_tree->objectName(), empower_tree->commandStack());
	embededOrSlideWidgetOnRightQml(empower_tree);	
	empower_tree -> setInitViewRect(rightw_container->initRect());	
}

void MainWindow::createSingleLogin()
{
	QString log_in = "";
	QWidget * tmp_bar = showing_toolBar;
	createLoginToolbar(tmp_bar, log_in);		
	user_mode = false;	
}

void MainWindow::createMultiLogin()//no finished
{
	user_mode = true;
	QString log_in = "";
	QWidget * tmp_bar = showing_toolBar;
	createLoginToolbar(tmp_bar, log_in);
}

void MainWindow::verifyPermission()
{
	if (new_login.first.isEmpty() || new_login.second.isEmpty())
	{
		QString warn_info = "warning:"+tr("输入不完全\n请补齐输入");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 1);
/*		QTimeLine * timeLine = new QTimeLine(1200, this);
		timeLine->setFrameRange(0, 1200);
		connect(timeLine, SIGNAL(frameChanged(int)), this, SLOT(intervalTimer(int)));
		connect(timeLine, SIGNAL(finished()), this, SLOT(warningTimeOut()));
		timeLine->start();*/
		user_mode = false;
		return;		
	}
	QWidget * tmp_bar = showing_toolBar;
	if (newComerSameInLogingPairs(new_login))
	{
		animateToolBarLeftRight(tmp_bar, 0);	
		initEnvireForLogedUser();
	}
	else if (base_db->theSameNameAndPasswdInManageList(new_login))
	{
		animateToolBarLeftRight(tmp_bar, 0);	
		initEnvireForLogingUser();
	}
	else
	{
		if (base_db->theSameNameInManageList(new_login.first))
		{
			QString warn_info = "warning:"+tr("密码输入错误？\n请重新核对一下");
			DiaLog * w_dialog = new DiaLog;
			w_dialog -> initWinDialog(this, warn_info, 1);
		}
		else
		{
			animateToolBarLeftRight(tmp_bar, 0);	
			QString error_info = "error:"+tr("您没有管理的项目或未经授权");
			DiaLog * e_dialog = new DiaLog;
			e_dialog -> initWinDialog(this, error_info, 0);
			new_login.first.clear();
			new_login.second.clear();						
		}
		user_mode = false;
	}
	sys_userState = base_db->userClass(new_login);
}

void MainWindow::userLogout()
{
	if (protect_mode)
		return;
	QWidget * last_left = leftw_container;
	QWidget * last_right = rightw_container;	
	QList<QPair<QWidget *, QWidget *> > usr_widgets = stored_widgets.value(loging_user);
	for (int i = 0; i < usr_widgets.size(); i++)
	{
		if (stored_bars.contains(usr_widgets.at(i).first))
		{
			stored_bars.remove(usr_widgets.at(i).first);
			break;			
		}
	}
	stored_widgets.remove(loging_user);
	loging_pairs.removeAll(loging_user);				
	new_login.first.clear();
	new_login.second.clear();
	if (!loging_pairs.isEmpty())
	{
		loging_user.first = loging_pairs.back().first;
		loging_user.second = loging_pairs.back().second;
		protect_mode = true;
		protect_widget = new QWidget(this);
		setProtectWidgetBackground(protect_widget);
		slideMultiWidgets(protect_widget, 0, last_left, last_right, true, true);
		leftw_container = 0;
	}
	else
	{
		loging_user.first.clear();
		loging_user.second.clear();
		if (stored_widgets.contains(QPair<QString, QString>("helps", "")) && stored_bars.value(stored_widgets.value(QPair<QString, QString>("helps", "")).at(0).first).size()>1)
			stored_bars[stored_widgets.value(QPair<QString, QString>("helps", "")).at(0).first].pop_back();
		if (user_mode)
			user_mode = false;		
		PlotWidget * old_plot = qobject_cast<PlotWidget *>(leftw_container->qmlAgent()->widget());
		if (!old_plot->currentEngi().isEmpty())
			old_plot -> setEnginame();
		
	}
	sys_userState = 0;	
	QString null_theme = tr("工程");
	emit newTheme(null_theme);	
	animateToolBarLeftRight(showing_toolBar, 0);
}

void MainWindow::userChangePasswd()
{
	if (protect_mode)
		return;
	QWidget * tmp_bar = showing_toolBar;
	QString ch_wd = tr("修改密码");
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> showChangePasswdTool(loging_user);
	animateToolBarLeftRight(tmp_bar, new_toolBar);
	new_toolBar -> show();
}

void MainWindow::userNewPasswdTodb()
{
	QHBoxLayout * h_layout = qobject_cast<QHBoxLayout *>(showing_toolBar->layout());
	QVBoxLayout * v_layout = qobject_cast<QVBoxLayout *>(h_layout->itemAt(0)->layout());
	QHBoxLayout * vh_layout = qobject_cast<QHBoxLayout *>(v_layout->itemAt(1)->layout());
	QLineEdit * pswd_editor = qobject_cast<QLineEdit *>(vh_layout->itemAt(1)->widget());
	for (int i = 0; i < pswd_editor->text().size(); i++)
	{
		if (!pswd_editor->text().at(i).toLatin1() || !pswd_editor->text().at(i).isLetterOrNumber())
		{
			//warn dialog
			return;
		}
	}
	if (base_db->theSamePasswdIndb(pswd_editor->text()))
	{
		pswd_editor -> clear();
		return;
	}
	if (!base_db->storeChangedPasswd(new_login, loging_user.second))
	{	
		QString warn_info = "warning:"+tr("新密码存储失败\n再试一次");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 5);
	}
	else
	{
		for (int i = 0; i < loging_pairs.size(); i++)
		{
			if (loging_pairs[i].second == loging_user.second)
			{
				loging_pairs[i].second = new_login.second;
				break;
			}
		}
		loging_user = new_login;
		animateToolBarLeftRight(showing_toolBar, 0);
	}
}

void MainWindow::helpToSpcView()
{
	LittleWidgetsView * old_left = leftw_container;
	LittleWidgetsView * old_right = rightw_container;	
	LittleWidgetsView * g_leftw = 0;
	LittleWidgetsView * g_rightw = 0;
	QWidget * re_bar = 0;
	QList<QPair<QWidget *, QWidget *> > ws_list = stored_bars.value(old_left);
	if (ws_list.size() == 1)
	{
		g_leftw = new LittleWidgetsView(this);
		g_leftw -> setObjectName("l_container");
		g_leftw -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");
		QRect o_rect(0, 0 , width(), height());
		g_leftw -> initMarginForParent(o_rect);
		PlotWidget * plot_new = new PlotWidget(back_science, base_db);
		plot_new -> setObjectName("plot");
		plot_new -> setFixedWidth(width());
		plot_new -> setFixedHeight(height());
		plot_new -> setEnginame();	
		g_leftw -> setViewForRelatedTable(plot_new);				
	}
	else
	{
		g_leftw = qobject_cast<LittleWidgetsView *>(ws_list.back().first);
		QList<QPair<QWidget *, QWidget *> > lws_list = stored_widgets.value(loging_user);
		for (int i = 0; i < lws_list.size(); i++)
		{
			if (lws_list.at(i).first == g_leftw)
			{
				g_rightw = qobject_cast<LittleWidgetsView *>(lws_list.at(i).second);
				break;
			}
		}
		QList<QPair<QWidget *, QWidget *> > rs_list = stored_bars.value(g_leftw);
		for (int i = 0; i < rs_list.size(); i++)
		{
			if (rs_list.at(i).first==g_rightw || rs_list.at(i).first==g_leftw)
			{
				re_bar = rs_list.at(i).second;
				break;
			}
		}
	}
	slideMultiWidgets(g_leftw, g_rightw, old_left, old_right, false, true);
	animateToolBarWidgetUpDown(showing_toolBar, 0, re_bar, 0);
}

void MainWindow::searchHelpOnTree(bool on)
{
	QLayout * menu_layout = showing_toolBar->layout();
	int tools = menu_layout->count();
	for (int i = 0; i < tools; i++)
	{
		QToolButton * e_button = qobject_cast<QToolButton *>(menu_layout->itemAt(i)->widget());
		if (e_button->text() == tr("自由浏览"))
		{
			e_button -> setChecked(on);
			e_button -> setDown(on);
			break;
		}
	}	
}
void MainWindow::lastHelpPapers()
{
	
}

void MainWindow::nextHelpPapers()
{
	
}

void MainWindow::createFileToolBars()
{
	if (showing_toolBar->objectName() == "engitoolBar")
		return;
	QWidget * tmp_bar = showing_toolBar;	
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("engitoolBar");
	new_toolBar -> showFileTools(sys_userState);
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);
	if (rightw_container) 
		slideWidgetView(leftw_container, rightw_container, 0);	
}

void MainWindow::userAddNewWin()
{
	if (wins_moving)
		return;
	LittleWidgetsView * old_left = leftw_container;
	LittleWidgetsView * old_right = rightw_container;	
	LittleWidgetsView * g_leftw = new LittleWidgetsView(this);
	g_leftw -> setObjectName("l_container");
	g_leftw -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");
	QRect o_rect(0, 0 , width(), height());
	g_leftw -> initMarginForParent(o_rect);
	PlotWidget * plot_new = new PlotWidget(back_science, base_db);
	plot_new -> setObjectName("plot");
	plot_new -> setFixedWidth(width());
	plot_new -> setFixedHeight(height());	
	g_leftw -> setViewForRelatedTable(plot_new);
	plot_new -> setEnginame();
	slideMultiWidgets(g_leftw, 0, old_left, old_right, false, false);
	syncNewWinWithHash(loging_user, g_leftw);
	showing_toolBar -> raise();
}

void MainWindow::userCloseCurWin()
{
	if (wins_moving)
		return;	
	if (!canTransOtherWinForUsr(loging_user))
		return;
	adjacentShowingWidgets(loging_user, true, true);
	showing_toolBar -> raise();	
}

void MainWindow::userViewNextWin()
{
	if (wins_moving)
		return;
	if (!canTransOtherWinForUsr(loging_user))
		return;
	adjacentShowingWidgets(loging_user, true, false);
	showing_toolBar -> raise();
}

void MainWindow::userViewLastWin()
{
	if (wins_moving)
		return;
	if (!canTransOtherWinForUsr(loging_user))
		return;
	adjacentShowingWidgets(loging_user, false, false);
	showing_toolBar -> raise();
}

void MainWindow::newFile()
{
	if (!base_db->curDBmanageTable().isEmpty() && loging_pairs.isEmpty())
	{
		QString warn_info = "warning:"+tr("您的个人信息已在数据库中吗？\n在可以登陆后再创建");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 4);
		if (w_dialog->exec())
		{
			createSonLoginMenu();
			return;
		}
	}	
	DataesTableView * initEngi_table = new DataesTableView(0, 3, base_db);
	initEngi_table -> setObjectName("initEngi");
	QPair<QString, QString> test_pair;
	if (loging_pairs.isEmpty())
		test_pair = new_login;
	else
		test_pair = loging_user;
	initEngi_table -> setInputor(test_pair);	
	initEngi_table -> initTable("");
	QWidget * tmp_bar = showing_toolBar; 	
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	if (stored_bar && stored_bar->objectName()==tr("okcancel_CPK数据"))
	{
		animateToolBarLeftRight(tmp_bar, stored_bar);
		resetToolBarsVarsValue(stored_bar, true);
	}
	else
		createOkCancelBars(tmp_bar, initEngi_table->objectName(), initEngi_table->commandStack());	
	embededOrSlideWidgetOnRightQml(initEngi_table);
	initEngi_table -> setRightOrigRect(rightw_container->initRect());
	connect(initEngi_table, SIGNAL(inputted(DataesTableView *)), this, SLOT(refreshTableSize(DataesTableView *)));
	connect(initEngi_table, SIGNAL(changeBtnEnable(const QString &, const QString &, bool)), this, SLOT(toggleCurrentBarButton(const QString &, const QString &, bool)));
}

void MainWindow::openFile()
{
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	QWidget * tmp_bar = showing_toolBar;
	SlideBar * new_toolBar = 0;
	if (stored_bar && qobject_cast<SlideBar *>(stored_bar))
		new_toolBar = qobject_cast<SlideBar *>(stored_bar);
	if (!new_toolBar)
		new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("openmenuBar");
	new_toolBar -> showOpenfilemenus();
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);
	EngiTreeView * fileView = new EngiTreeView;
	fileView -> setObjectName("fileView");
	fileView -> setInspector(base_db);
	fileView -> initTreeInformation(loging_user, 0);
	embededOrSlideWidgetOnRightQml(fileView);
	fileView -> setInitViewRect(rightw_container->initRect());
	fileView -> defaultSettedModelTransaction();
	toggleCurrentBarButton(tr("默认显示"), tr("使能"), false, new_toolBar);
	connect(fileView, SIGNAL(treeViewSizeChanged()), rightw_container, SLOT(resizeAgentSize()));	
}

void MainWindow::createManageToolBars()
{
	QWidget * tmp_bar = showing_toolBar;  
	if (rightw_container)
		slideWidgetView(leftw_container, rightw_container, 0); 
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("managetoolBar");
	new_toolBar -> showManageTools();
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);
	slideWidgetView(leftw_container, rightw_container, 0);
}

void MainWindow::backToNewFileState()
{
	if (protect_mode)
		return;	
	PlotWidget * cur_plot = qobject_cast<PlotWidget *>(leftw_container->qmlAgent()->widget());
	if (leftw_container->objectName() == "helppages")
	{
		stored_widgets.remove(QPair<QString, QString>("helps", ""));
		LittleWidgetsView * old_left = leftw_container;
		LittleWidgetsView * old_right = rightw_container;	
		stored_bars.remove(old_left);		
		QWidget * usr_tools = 0;		
		QList<QPair<QWidget *, QWidget *> > lr_widgets = stored_widgets.value(loging_user);
		for (int i = 0; i < lr_widgets.size(); i++)
		{
			if (stored_bars.contains(lr_widgets.at(i).first))
			{		  
				leftw_container = qobject_cast<LittleWidgetsView *>(lr_widgets.at(i).first);
				rightw_container = qobject_cast<LittleWidgetsView *>(lr_widgets.at(i).second);
				QList<QPair<QWidget *, QWidget *> > lr_bars = stored_bars.value(leftw_container);
				for (int j = 0; j < lr_bars.size(); j++)
				{
					if (lr_bars.at(j).first == rightw_container)
					{
						usr_tools = lr_bars.at(j).second;
						break;
					}
				}
				break;
			}		
		}					
		slideMultiWidgets(leftw_container, rightw_container, old_left, old_right, true, true);
		animateToolBarWidgetUpDown(showing_toolBar, 0, usr_tools, 0, false);
	}
	if (loging_pairs.isEmpty() || (!canTransOtherWinForUsr(loging_user) && !rightw_container && cur_plot && cur_plot->currentEngi().isEmpty()))
		return;
	QString warn_info = "warning:"+tr("要清空所有数据及窗口吗?");
	DiaLog * w_dialog = new DiaLog;
	w_dialog -> initWinDialog(this, warn_info, 0);
	if (w_dialog->exec())
	{
		animateToolBarLeftRight(showing_toolBar, 0);
		QList<QPair<QWidget *, QWidget *> > s_wps = stored_widgets.take(loging_user);
		QWidget * bar_leader = 0;
		QPair<QWidget *, QWidget *> cur_wins;		
		for (int i = 0; i < s_wps.size(); i++)
		{
			if (stored_bars.contains(s_wps.at(i).first))
			{
				bar_leader = s_wps.at(i).first;
				cur_wins.first = s_wps.at(i).first;
				cur_wins.second = s_wps.at(i).second;
				continue;
			}
			s_wps.at(i).first -> deleteLater();
			if (s_wps.at(i).second)
				s_wps.at(i).second -> deleteLater();
		}
		s_wps.clear();
		s_wps << cur_wins;
		if (bar_leader)
		{
			QList<QPair<QWidget *, QWidget *> > s_bars = stored_bars.take(bar_leader);
			for (int i = 0; i < s_bars.size(); i++)
				s_bars.at(i).second -> deleteLater();
		}
		if (ws_cpktests.contains(loging_user))
			clearCpkProcessWidgets();
		LittleWidgetsView * l_first = qobject_cast<LittleWidgetsView *>(cur_wins.first);
		if (l_first && qobject_cast<PlotWidget *>(l_first->qmlAgent()->widget()))
		{
			PlotWidget * left_plot = qobject_cast<PlotWidget *>(l_first->qmlAgent()->widget());
			left_plot -> setEnginame();					
			if (cur_wins.second)
				slideWidgetView(l_first, cur_wins.second, 0);
		}
		else
		{
			leftw_container = new LittleWidgetsView(this);
			leftw_container -> setObjectName("l_container");
			leftw_container -> setMainQmlFile("spc_qml/tablegroupviewer.qml");
			QRect o_rect(0, 0 , width(), height());
			leftw_container -> initMarginForParent(o_rect);
			PlotWidget * plot_new = new PlotWidget(back_science, base_db);
			plot_new -> setObjectName("plot");
			plot_new -> setFixedWidth(width());
			plot_new -> setFixedHeight(height());					
			plot_new -> setEnginame();
			leftw_container -> setViewForRelatedTable(plot_new);
			slideMultiWidgets(leftw_container, 0, cur_wins.first, cur_wins.second, true, true);
			syncCurrentWinWithHash(leftw_container, 0, loging_user);
			s_wps[0].first = leftw_container;
		}
		s_wps[0].second = 0;
		stored_widgets.insert(loging_user, s_wps);
	}
	emit newTheme(tr("工程"));
}

void MainWindow::storeLoginName(const QString & name)
{	
	new_login.first = name;
}
	
void MainWindow::storeLoginPasswd(const QString & passwd)
{
	new_login.second = passwd;
}

void MainWindow::clearLogpair()
{
	new_login.first = "";
	new_login.second = "";
}

void MainWindow::showProjectsKeyTable()
{
	if (newUserDifWithLoginger())
		return;
	QWidget * tmp_bar = showing_toolBar;
	RelatedTreeView * projects_tree = new RelatedTreeView;
	projects_tree -> setObjectName("projects");
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	if (stored_bar && stored_bar->objectName()==tr("okcancel_关键数据"))
	{
		animateToolBarLeftRight(tmp_bar, stored_bar);
		resetToolBarsVarsValue(stored_bar, true);
	}
	else	
		createOkCancelBars(tmp_bar, projects_tree->objectName());	
	projects_tree -> initTree(base_db, loging_user, 3);
	embededOrSlideWidgetOnRightQml(projects_tree);	
	projects_tree -> setInitViewRect(rightw_container->initRect());	
}

void MainWindow::improveExistedProjects()
{
 	if (!ws_cpktests.value(loging_user).isEmpty() && ws_cpktests.value(loging_user).at(1))
	{
		//warn dialog 
		return;
	}	 
	QWidget * tmp_bar = showing_toolBar;
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("improveProjBar");	
	new_toolBar -> showProjsPromotionTools();
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);
	EngiTreeView * improve_fileView = new EngiTreeView;
	improve_fileView -> setObjectName("projsPromoteView");
	improve_fileView -> setInspector(base_db);
	improve_fileView -> initTreeInformation(loging_user, 1);
	embededOrSlideWidgetOnRightQml(improve_fileView);
	improve_fileView -> setInitViewRect(rightw_container->initRect());
	ws_cpktests[loging_user] << 0 << 0 << rightw_container << 0 << 0 << 0 << 0 << 0; 
	connect(improve_fileView, SIGNAL(treeViewSizeChanged()), rightw_container, SLOT(resizeAgentSize()));		
}

void MainWindow::projsPropertys()
{
	if (newUserDifWithLoginger())
		return;
	QWidget * tmp_bar = showing_toolBar;
	RelatedTreeView * propertys_tree = new RelatedTreeView;
	propertys_tree -> setObjectName("propertys");
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	if (stored_bar && stored_bar->objectName()==tr("okcancel_属性数据"))
	{
		animateToolBarLeftRight(tmp_bar, stored_bar);
		resetToolBarsVarsValue(stored_bar, true);
	}
	else	
		createOkCancelBars(tmp_bar, propertys_tree->objectName(), propertys_tree->commandStack());
	propertys_tree -> initTree(base_db, loging_user, 5);
	propertys_tree -> setFreeForEdit(false);	
	embededOrSlideWidgetOnRightQml(propertys_tree);	
	propertys_tree -> setInitViewRect(rightw_container->initRect());		
}

void MainWindow::helpContent()
{
	if (wins_moving || protect_mode)
		return;
	if (leftw_container->objectName() == "helppages")
	{
		if (showing_toolBar->height() == 0)
			animateToolBarWidgetUpDown(0, 0, showing_toolBar, rightw_container);
		return;
	}
	LittleWidgetsView * old_left = leftw_container;
	LittleWidgetsView * old_right = rightw_container;	
	QPair<QString, QString> hl_pair("helps", "");
	QPair<QWidget *, QWidget *> hint_pair(old_left, 0);	
	LittleWidgetsView * h_leftw = 0;
	LittleWidgetsView * helps_view = 0;
	SlideBar * help_tools = 0;
	if (!stored_widgets.contains(hl_pair))
	{
		h_leftw = new LittleWidgetsView(this);
		h_leftw -> setObjectName("helppages");
		h_leftw -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/helpviewer.qml");
		QRect o_rect(0, 0 , width(), height());
		h_leftw -> initMarginForParent(o_rect);	
		HelpContents * h_pages = new HelpContents;
		h_leftw -> setHelpPages(h_pages);
		RelatedTreeView * helps_tree = new RelatedTreeView;
		helps_tree -> setObjectName("helptree");
		helps_tree -> initHelpTree(8);		
		helps_view = new LittleWidgetsView(this);
		helps_view -> setObjectName(tr("树目录"));	
		QRect or_rect;
		helps_view -> setFixedHeight(height()-64);
		helps_view -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/treeviewer.qml");
		helps_tree -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		helps_tree -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		or_rect.setWidth(helps_tree->sizeHint().width());
		or_rect.setHeight(helps_view->height());
		helps_view -> initMarginForParent(or_rect);
		helps_view -> setViewForRelatedTable(helps_tree);
		helps_tree -> setInitViewRect(or_rect);
		QPair<QWidget *, QWidget *> hlp_pair(h_leftw, helps_view);
		QList<QPair<QWidget *, QWidget *> > ws_helps;
		ws_helps << hlp_pair;
		stored_widgets.insert(hl_pair, ws_helps); 
		h_leftw -> setGeometry(width(), 0, width(), height());
		helps_view -> setGeometry(width()-or_rect.width(), 64, or_rect.width(), helps_view->height());
		help_tools = new SlideBar(this);
		help_tools -> setObjectName("helpsBar");
		help_tools -> showHelpViewMenu();
		help_tools -> setGeometry(width()-help_tools->sizeHint().width(), -64, help_tools->sizeHint().width(), 64);
		QPair<QWidget *, QWidget *> hlpb_pair(helps_tree, help_tools);
		QList<QPair<QWidget *, QWidget *> > ws_bares;
		ws_bares << hlpb_pair;
		if (stored_bars.contains(old_left))
			ws_bares << hint_pair;
		stored_bars.insert(h_leftw, ws_bares);
		help_tools -> show();	
	}
	else
	{
		h_leftw = qobject_cast<LittleWidgetsView *>(stored_widgets.value(hl_pair).at(0).first);
		helps_view = qobject_cast<LittleWidgetsView *>(stored_widgets.value(hl_pair).at(0).second);		
		help_tools = qobject_cast<SlideBar *>(stored_bars.value(h_leftw).at(0).second);
		if (stored_bars.contains(old_left))
		{
			if (stored_bars.value(h_leftw).size() == 1)
				stored_bars[h_leftw] << hint_pair;
			else
				stored_bars[h_leftw].replace(1, hint_pair);
		}
	}
	if (showing_toolBar == help_tools)
		animateToolBarWidgetUpDown(0, 0, help_tools, 0);
	else
	{
		if (showing_toolBar && showing_toolBar==storedBarOnHash(old_left, old_right))
			animateToolBarWidgetUpDown(showing_toolBar, 0, help_tools, 0);
		else
			animateToolBarWidgetUpDown(showing_toolBar, 0, help_tools, 0, false);
	}	
	if (stored_bars.contains(old_left))
		slideMultiWidgets(h_leftw, helps_view, old_left, old_right, false, false);
	else	
		slideMultiWidgets(h_leftw, helps_view, old_left, old_right, true, false);	
	help_tools -> raise();		
}

void MainWindow::returnLastMenu()
{
 	if (loging_pairs.isEmpty())
		user_mode = false; 
	if (!showing_toolBar->objectName().contains("toleditBar") && !showing_toolBar->objectName().contains("mainplotsBar"))
	{
		if (showing_toolBar->objectName()=="engitoolBar" || showing_toolBar->objectName()=="managetoolBar" || showing_toolBar->objectName()=="sonloginBar")
		{
			if (showing_toolBar->objectName()=="sonloginBar")
			{
				new_login.first.clear();
				new_login.second.clear();
			}
			createMainToolBars();
		}
		else if (showing_toolBar->objectName()=="logintoolBar")	  
			createSonLoginMenu();
		else if (showing_toolBar->objectName()=="engipropertyBar" || showing_toolBar->objectName()=="dataesallBar" || showing_toolBar->objectName()==tr("okcancel_权限") || showing_toolBar->objectName()==tr("okcancel_产品"))		  
			createManageToolBars();
		else if (showing_toolBar->objectName()=="dataesBar" || showing_toolBar->objectName()=="mrgmentBar" || showing_toolBar->objectName()=="plotMrgBar" || showing_toolBar->objectName()=="overallMrgBar")
			dataesManagement();
		else if (showing_toolBar->objectName()=="openmenuBar" || showing_toolBar->objectName()=="powerstoolBar")		  
			createFileToolBars();
		else if (showing_toolBar->objectName()=="edittoolBar")
			createOkCancelBars(showing_toolBar, rightw_container->qmlAgent()->widget()->objectName());
		else if (showing_toolBar->objectName() == "improveProjBar" || showing_toolBar->objectName() == "engitestsBar" || showing_toolBar->objectName() == "samtestsBar")	
		{			  
			if (showing_toolBar->objectName() == "improveProjBar")
			{
				clearRalatedEdittingBarOnHash(showing_toolBar);
				slideRightWidgets(leftw_container, 0, rightw_container, true);
				projectsKeysPropertys();
				emit newTheme(tr("工程"));		
			}
			else if (showing_toolBar->objectName() == "engitestsBar")
			{
				if (ws_cpktests.value(loging_user).at(0) != showing_toolBar)
					delete ws_cpktests.value(loging_user).at(0);
				SlideBar * new_toolBar = new SlideBar(this);
				new_toolBar -> setObjectName("improveProjBar");	
				new_toolBar -> showProjsPromotionTools();
				new_toolBar -> setGeometry(width()-new_toolBar->sizeHint().width(), -64, new_toolBar->sizeHint().width(), 64);			
				QWidget * old_left = leftw_container;
				leftw_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(1));
				QWidget * old_right = rightw_container;
				rightw_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(2));	
				DataesTableView * key_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(3))->qmlAgent()->widget());
				key_table->commandStack()->clear();
				key_table->delModelsStack()->clear();	
				key_table->modelHashs()[tr("草稿工程，，。")][0].first = "";
				clearRalatedEdittingBarOnHash(showing_toolBar);				
				animateToolBarWidgetUpDown(showing_toolBar, 0, new_toolBar, 0);
				new_toolBar -> show();			
				slideMultiWidgets(leftw_container, rightw_container, old_left, old_right, false, true);
				syncCurrentWinWithHash(leftw_container, rightw_container, loging_user);
				storeRalatedEdittingBar(leftw_container, rightw_container, showing_toolBar);
				new_toolBar -> raise();				
			}
			else
			{
				DataesTableView * cur_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
				if (cur_table->tableType() != 4)
				{
					QWidget * old_right = rightw_container;
					rightw_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(3));	
					if (rightw_container->width() < rightw_container->initRect().width())
						rightw_container -> setFixedWidth(rightw_container->initRect().width());					
					slideRightWidgets(leftw_container, rightw_container, old_right);
				}
				DataesTableView * din_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(4))->qmlAgent()->widget());
				din_table->commandStack()->clear();
				din_table->delModelsStack()->clear();	
				din_table->modelHashs()[tr("草稿工程，，。")][0].first = "";				
				QWidget * tmp_bar = showing_toolBar;
				storeRalatedEdittingBar(leftw_container, rightw_container, ws_cpktests.value(loging_user).at(0));				
				animateToolBarWidgetUpDown(tmp_bar, 0, ws_cpktests.value(loging_user).at(0), 0);
			}
		}		
		else if (showing_toolBar->objectName().contains("okcancel"))
		{	  
			if (showing_toolBar->objectName() == tr("okcancel_数据库编辑"))
			{		  
				createMainToolBars();
				slideWidgetView(leftw_container, rightw_container, 0);				
			}
			else if (showing_toolBar->objectName() == tr("okcancel_CPK数据")/*initEngi*/ || showing_toolBar->objectName().contains(tr("okcancel_新建1"))/*进入点图*/)
			{
				QPair<QString, QString> test_pair;
				if (loging_pairs.isEmpty())
					test_pair = new_login;
				else
					test_pair = loging_user;
				int w_pos = 2;							
				DataesTableView * din_table = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());							  
				if (showing_toolBar->objectName() == tr("okcancel_CPK数据"))
				{				  
					if (din_table)
					{
						DataesTableView * key_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(test_pair).at(w_pos))->qmlAgent()->widget());				  
						if (loging_pairs.isEmpty() && !test_pair.first.isEmpty() && !test_pair.second.isEmpty())
						{
							QStringList ps_owned;
							base_db -> constructorOwnedProjs(test_pair, ps_owned);
							if (!ps_owned.contains(key_table->model()->data(key_table->model()->index(0, 0)).toString()))
							{
								new_login.first.clear();
								new_login.second.clear();
							}
						}
						LittleWidgetsView * old_left = leftw_container;	
						LittleWidgetsView * old_right = rightw_container;	
						LittleWidgetsView * g_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(test_pair).at(1));					
						leftw_container -> resetViewWidth(rect());
						rightw_container = 0;	
						clearRalatedEdittingBarOnHash(showing_toolBar);
						slideMultiWidgets(g_container, 0, old_left, old_right, true, true);
						syncCurrentWinWithHash(leftw_container, 0, test_pair);
					}
					createFileToolBars();				
				}
				else
				{
					DataesTableView * key_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(test_pair).at(w_pos))->qmlAgent()->widget());		
					if (din_table->tblEdittingState())
					{
						QString checking;
						if (din_table->checkInputState(checking))
						{
							QString din_time = din_table->getInfoByModel(qobject_cast<QStandardItemModel *>(din_table->model()), tr("时间"));
							if (din_time == tr("时间"))
							{
								QString key_time = key_table->getInfoByModel(qobject_cast<QStandardItemModel *>(key_table->model()), tr("时间"));						  
								din_table -> tmpSaveProjsModels(key_time, QDateTime::currentDateTime().toString());
								din_table -> setShiftModel(qobject_cast<QStandardItemModel *>(din_table->model()), qobject_cast<QStandardItemModel *>(key_table->model()));
							}
						}
						else
						{
							if (checking.contains("empty") || checking.contains("no_number"))
							{
								QStringList fur_chk = checking.split("empty");
								if (!fur_chk[0].contains("0,0"))
								{
									QString warn_info;
									if (checking.contains("empty"))
										warn_info = "warning:"+tr("您没有完成样本\n");
									if (checking.contains("no_number"))//add no number poses
									{
										if (warn_info.isEmpty())
											warn_info = "warning:"+tr("您的新样本输入有误\n");
										else
											warn_info += "warning:"+tr("并且您的输入有误\n");
									}
									warn_info += tr("返回后会删除这次输入\n您确定吗？");
									DiaLog * w_dialog = new DiaLog;
									w_dialog -> initWinDialog(this, warn_info, 4);
									if (!w_dialog->exec())
										return;
								}
							}
						}
					}
					QWidget * tmp_bar = showing_toolBar;										
					QWidget * cache_bar = ws_cpktests.value(test_pair).at(0);
					animateToolBarWidgetUpDown(tmp_bar, 0, cache_bar, 0);						
					if (ws_cpktests.value(test_pair).size()>w_pos+1 && ws_cpktests.value(test_pair).at(w_pos+2) && rightw_container==ws_cpktests.value(test_pair).at(w_pos+2))
					{
						toggleCurrentBarButton(tr("计算结果"), tr("切换"), false, tmp_bar);			  
						LittleWidgetsView * tmp_right = rightw_container;
						rightw_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(test_pair).at(w_pos));
						slideRightWidgets(leftw_container, rightw_container, tmp_right);							
					}
					cache_bar -> raise();	
					ws_cpktests[test_pair][0] = tmp_bar;
					din_table->commandStack()->clear();
					din_table -> resetEditState(false);
				}
			}			
			else if (showing_toolBar->objectName() == tr("okcancel_输入"))	
			{
				QWidget * tmp_right = rightw_container;
				QWidget * tmp_bar = showing_toolBar;
				PlotWidget * cur_plot = qobject_cast<PlotWidget *>(leftw_container->qmlAgent()->widget());
				cur_plot -> setEnginame();
				SlideBar * new_toolBar = new SlideBar(this);
				new_toolBar -> setObjectName("openmenuBar");
				new_toolBar -> showOpenfilemenus();
				new_toolBar -> show();
				animateToolBarLeftRight(tmp_bar, new_toolBar);				
				EngiTreeView * fileView = new EngiTreeView;
				fileView -> setObjectName("fileView");
				fileView -> setInspector(base_db);
				fileView -> initTreeInformation(loging_user, 0);
				fileView -> setInitViewRect(rightw_container->initRect());
				fileView -> defaultSettedModelTransaction();				
				fileView -> setFixedHeight(height()-64);					
				rightw_container = new LittleWidgetsView(this);
				rightw_container -> setGeometry(width(), 64, tmp_right->width(), height()-64);
				rightw_container -> setObjectName("r_container");
				rightw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/treeviewer.qml");	
				QRect o_rect;
				o_rect.setWidth(tmp_right->sizeHint().width());
				o_rect.setHeight(tmp_right->height());
				rightw_container -> initMarginForParent(o_rect);	
				rightw_container -> setViewForRelatedTable(fileView);	
				slideRightWidgets(leftw_container, rightw_container, tmp_right, true);
				toggleCurrentBarButton(tr("默认显示"), tr("使能"), false, new_toolBar);	
				connect(fileView, SIGNAL(treeViewSizeChanged()), rightw_container, SLOT(resizeAgentSize()));			
			}			
			else if (showing_toolBar->objectName() == tr("okcancel_加入工程") || showing_toolBar->objectName() == tr("okcancel_授权用户"))	
			{			  
				createPowersTool();
				slideWidgetView(leftw_container, rightw_container, 0);		
			}
			else if (showing_toolBar->objectName() == tr("okcancel_权限数据编辑") || showing_toolBar->objectName() == tr("okcancel_产品数据编辑"))
			{
				createManageToolBars();
				slideWidgetView(leftw_container, rightw_container, 0);		
			}
			else if (showing_toolBar->objectName() == tr("okcancel_关键数据") || showing_toolBar->objectName() == tr("okcancel_属性数据"))	
			{
				projectsKeysPropertys();
				slideWidgetView(leftw_container, rightw_container, 0);			
			}
			else if (showing_toolBar->objectName() == tr("okcancel_备份数据"))
			{		  
				overallConstruction();
				slideWidgetView(leftw_container, rightw_container, 0);					
			}
		}
	}	
	else if (showing_toolBar->objectName().contains("toleditBar"))
	{
		if (showing_toolBar->objectName().contains(tr("数据总编辑条")) || showing_toolBar->objectName().contains(tr("信息总编辑条")) || showing_toolBar->objectName().contains(tr("数据信息管理条")))
			totalDataesMrgment();	
		else if (showing_toolBar->objectName().contains(tr("表图管理条")) || showing_toolBar->objectName().contains("free"))
		{
			overallConstruction();	
			slideWidgetView(leftw_container, rightw_container, 0);
		}
	}
	else
		plotsTolMrgment();	
}

void MainWindow::dailyDataesInSure()
{
	if (!rightw_container)
		return;
	DataesTableView * tmpinput_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
	QString checking;
	if (!tmpinput_table->checkInputState(checking))
	{
		QString warn_info = "warning:"+tr("数据输入不全，\n不能存储！");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 1);
		return;
	}
	if (tmpinput_table->model()->data(tmpinput_table->model()->index(tmpinput_table->model()->rowCount()-2, 0)).isNull())
	{
		QString warn_info = "warning:"+tr("数据未计算，\n不能存储！");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 1);
		return;
	}
	if (!tmpinput_table->saveDailySpcDataes())
	{
		QString warn_info = "warning:"+tr("数据未存储，\n请再试一次");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 2);
		return;
	}
	tmpinput_table -> clearRelatedCellsForNextInput();
}

void MainWindow::openDailyTableByButton()
{
 	EngiTreeView * engi_tree = qobject_cast<EngiTreeView *>(rightw_container->qmlAgent()->widget());	
	QStandardItem * item_ontree = engi_tree->currentEngiItem();
	if (!item_ontree)
		return;
	QString open_engi = item_ontree->text();
	if (projectOpenCheck(open_engi))
	{
		QString warn_info("warning:"+tr("工程正在编辑\n不能再次打开"));
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 0);	
		w_dialog->exec();
		return;		
	}	
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	PlotWidget * cur_plot = qobject_cast<PlotWidget *>(leftw_container->qmlAgent()->widget());
	QWidget * tmp_bar = showing_toolBar;
	if (stored_bar && stored_bar->objectName()==tr("okcancel_输入"))
	{
		animateToolBarLeftRight(tmp_bar, stored_bar);
		resetToolBarsVarsValue(stored_bar, true);
	}
	else	
		createOkCancelBars(tmp_bar, cur_plot->objectName());
	open_engi += tr("，，。")+item_ontree->child(1)->child(1)->child(0)->child(0)->child(0)->text();
	cur_plot -> setEnginame(open_engi);
	QWidget * tmp_right = rightw_container;	
	rightw_container = new LittleWidgetsView(this);
	rightw_container -> setGeometry(width(), 64, tmp_right->width(), height()-64);
	rightw_container -> setObjectName("r_container");
	rightw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/treeviewer.qml");	
	DataesTableView * tmpinput_table = new DataesTableView(0, 2, base_db);
	tmpinput_table -> setObjectName("tmpinput");
	tmpinput_table -> setPlotsBackGuide(cur_plot);
	tmpinput_table -> setInputor(loging_user);
	tmpinput_table -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	tmpinput_table -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	
	tmpinput_table -> setFixedHeight(height()-64);
	QRect o_rect;
	o_rect.setWidth(tmp_right->sizeHint().width());
	o_rect.setHeight(tmp_right->height());
	tmpinput_table -> initTable(open_engi);
	rightw_container -> initMarginForParent(o_rect);	
	rightw_container -> setViewForRelatedTable(tmpinput_table);	
	slideRightWidgets(leftw_container, rightw_container, tmp_right, true);
	tmpinput_table -> setRightOrigRect(rightw_container->initRect());	
	emit newTheme(item_ontree->text());
	QStringList m_proj = base_db->curDBmanageTable().value(loging_user.second).at(3).split(";").filter(item_ontree->text());
	if (!base_db->isConstructor(loging_user, item_ontree->text()) && (m_proj.isEmpty() || !m_proj.at(0).contains(tr("读写"))))
		tmpinput_table -> resetEditState(false);
}

void MainWindow::showEngiTreeByDefault()
{
	EngiTreeView * fileView = qobject_cast<EngiTreeView *>(rightw_container->qmlAgent()->widget());
	fileView -> defaultSettedModelTransaction();
	toggleCurrentBarButton(tr("分类显示"), tr("使能"), true, showing_toolBar);
	toggleCurrentBarButton(tr("默认显示"), tr("使能"), false, showing_toolBar);	
}
	
void MainWindow::showEngiTreeBySet()
{
	EngiTreeView * fileView = qobject_cast<EngiTreeView *>(rightw_container->qmlAgent()->widget());
	fileView -> defaultSettedModelTransaction();	
	toggleCurrentBarButton(tr("默认显示"), tr("使能"), true, showing_toolBar);  
	toggleCurrentBarButton(tr("分类显示"), tr("使能"), false, showing_toolBar);
}

void MainWindow::setAuthority()
{
	RelatedTreeView * power_tree = new RelatedTreeView;
	power_tree -> setObjectName("power");
	power_tree -> initTree(base_db, loging_user, 0);
	power_tree -> setFreeForEdit(false);	
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	QWidget * tmp_bar = showing_toolBar;
	if (stored_bar && stored_bar->objectName()==tr("okcancel_权限"))
	{
		animateToolBarLeftRight(tmp_bar, stored_bar);
		resetToolBarsVarsValue(stored_bar, true);
	}
	else	
		createOkCancelBars(tmp_bar, power_tree->objectName(), power_tree->commandStack());
	embededOrSlideWidgetOnRightQml(power_tree);	
	power_tree -> setInitViewRect(rightw_container->initRect());	
}

void MainWindow::projectsKeysPropertys()
{
	QWidget * tmp_bar = showing_toolBar;
	SlideBar * new_toolBar = new SlideBar(this);					
	new_toolBar -> setObjectName("engipropertyBar");
	new_toolBar -> showEngiPropertys();
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);
}

void MainWindow::productsDefine()
{
	RelatedTreeView * products_tree = new RelatedTreeView;
	products_tree -> setObjectName("products");
	products_tree -> initTree(base_db, loging_user, 4);
	products_tree -> setFreeForEdit(false); 	
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	QWidget * tmp_bar = showing_toolBar;
	if (stored_bar && stored_bar->objectName()==tr("okcancel_产品"))
	{
		animateToolBarLeftRight(tmp_bar, stored_bar);
		resetToolBarsVarsValue(stored_bar, true);
	}
	else	
		createOkCancelBars(tmp_bar, products_tree->objectName(), products_tree->commandStack());
	embededOrSlideWidgetOnRightQml(products_tree);
	products_tree -> setInitViewRect(rightw_container->initRect());	
}

void MainWindow::dataesManagement()
{
	QWidget * tmp_bar = showing_toolBar;
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("dataesallBar");
	new_toolBar -> showDataesManageTool();
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);
}

void MainWindow::totalDataesMrgment()
{
	QWidget * tmp_bar = showing_toolBar;	
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("dataesBar");
	new_toolBar -> showOnlyForDataesTool();
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);	
}

void MainWindow::plotsTolMrgment()
{
	QWidget * tmp_bar = showing_toolBar;
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("plotMrgBar");
	new_toolBar -> showPlotsMrgTools();
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);	
}

void MainWindow::dataesInfoesMrgment()
{
	QStandardItemModel * t_model = new QStandardItemModel(16, 10, this);
	initManualTableConstruct(t_model, tr("数据信息管理条"));
}
	
void MainWindow::dataesInfoesPlotsMrgment()
{
	QStandardItemModel * t_model = new QStandardItemModel(16, 10, this);
	initManualTableConstruct(t_model, tr("表图管理条"));
}

void MainWindow::freeConstructionAct()
{ 
	QString projs_power = base_db->curDBmanageTable().value(loging_user.second).at(3);
	if (!projs_power.contains(tr("读写")))
	{
		QStringList own_projs;
		base_db -> constructorOwnedProjs(loging_user, own_projs);
		if (own_projs.isEmpty())
			return;
	}
 	RelatedTreeView * free_rtree = new RelatedTreeView;
	free_rtree -> setObjectName("freer");
	free_rtree -> setFreeForEdit(false);	
	free_rtree -> initTree(base_db, loging_user, 6);
	embededOrSlideWidgetOnRightQml(free_rtree); 	
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
	QWidget * tmp_bar = showing_toolBar;
	if (stored_bar && stored_bar->objectName()=="freetoleditBar")
	{
		animateToolBarLeftRight(tmp_bar, stored_bar);
		resetToolBarsVarsValue(stored_bar, true);
	}
	else
	{	
		SlideBar * new_toolBar = new SlideBar(this);
		new_toolBar -> setObjectName("freetoleditBar");
		new_toolBar -> setCommandStack(free_rtree->commandStack());		
		new_toolBar -> showFreeConstructTool();
		animateToolBarLeftRight(tmp_bar, new_toolBar);
		new_toolBar -> show();
	}
	syncCurrentWinWithHash(leftw_container, rightw_container, loging_user);
	storeRalatedEdittingBar(leftw_container, rightw_container, showing_toolBar);
	free_rtree -> setInitViewRect(rightw_container->initRect());	
}

void MainWindow::backupTreeViewAction()
{
	if (projectsEditting())
	{
		QString warn_info("warning:"+tr("有正在编辑的工程\n不能进行备份工作"));
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 0);	
		w_dialog->exec();
		return;
	}
	QStringList ps_constructed;
	base_db -> constructorOwnedProjs(loging_user, ps_constructed);	
	QStringList ps_agented;
	base_db -> constructorOwnedProjs(loging_user, ps_agented);
	if (ps_constructed.isEmpty() && ps_agented.isEmpty())
		return;
	TableManagement * bkdb_tree = new TableManagement;
	connect(bkdb_tree, SIGNAL(showSavedManualPlots(TableManagement *, LittleWidgetsView *, bool)), this, SLOT(showManualTableFromDB(TableManagement *, LittleWidgetsView *, bool)));
	bkdb_tree -> setObjectName("bkdb");	
	bkdb_tree -> initManageTree(loging_user, 0, base_db, 2, leftw_container, 0);
	QWidget * tmp_bar = showing_toolBar;	
	createOkCancelBars(tmp_bar, bkdb_tree->objectName(), bkdb_tree->commandStack());
	embededOrSlideWidgetOnRightQml(bkdb_tree);  
	bkdb_tree -> setInitViewRect(rightw_container->initRect());
}

void MainWindow::overallConstruction()
{
	QWidget * tmp_bar = showing_toolBar;
	SlideBar * new_toolBar = new SlideBar(this);
	new_toolBar -> setObjectName("overallMrgBar");
	new_toolBar -> showOverallMrgTools();
	new_toolBar -> show();
	animateToolBarLeftRight(tmp_bar, new_toolBar);	
}

void MainWindow::mainPlotsMerge()
{
	QWidget * tmp_left = leftw_container;
	leftw_container = new LittleWidgetsView(this);
	leftw_container -> setObjectName("p_container");
	leftw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/plotsviewer.qml");
	QRect rect(0, 0, 300, 200);
	QRect o_rect(0, 0, width(), height()-64);
	leftw_container -> initMarginForParent(o_rect);
	leftw_container -> initViewMatrix(5, 5, rect);
	QWidget * tmp_bar = showing_toolBar;	
	EditLongToolsViewer * main_plotsBar = new EditLongToolsViewer(this);
	main_plotsBar -> setObjectName("mainplotsBar3");
	main_plotsBar -> setOrientation(EditLongToolsViewer::ScreenOrientationLockLandscape);
	main_plotsBar -> setMainQmlFile(QLatin1String("/home/dapang/workstation/spc-tablet/spc_qml/plotseditor.qml"), 3);
	main_plotsBar -> setPlotsContainerPt(leftw_container);
	connect(main_plotsBar, SIGNAL(relatedSelectedResource(QString, bool)), this, SLOT(slideMatchResources(QString, bool)));
	connect(main_plotsBar, SIGNAL(sendSaveSigToWin(bool)), this, SLOT(prepairForDBsaveWork(bool)));
	connect(main_plotsBar, SIGNAL(sendStopInfoToMainWin(QString)), this, SLOT(stopQmlEditorFunc(QString)));	
	main_plotsBar -> showExpanded();
	animateToolBarLeftRight(tmp_bar, main_plotsBar);
	slideWholeWidgets(tmp_left, leftw_container, true);		
}

void MainWindow::assistantPlotsMerge()
{
	QWidget * tmp_left = leftw_container;
	leftw_container = new LittleWidgetsView(this);
	leftw_container -> setObjectName("p_container");
	QRect o_rect(0, 0 , width(), height()-64);
	leftw_container -> initMarginForParent(o_rect);
	leftw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/plotsviewer.qml");
	QRect rect(0, 0, 300, 200);
	leftw_container -> initViewMatrix(7, 8, rect);
	QWidget * tmp_bar = showing_toolBar;
	EditLongToolsViewer * main_plotsBar = new EditLongToolsViewer(this);
	main_plotsBar -> setObjectName("mainplotsBar4");
	main_plotsBar -> setOrientation(EditLongToolsViewer::ScreenOrientationLockLandscape);
	main_plotsBar -> setMainQmlFile(QLatin1String("/home/dapang/workstation/spc-tablet/spc_qml/plotseditor.qml"), 4);
	main_plotsBar -> setPlotsContainerPt(leftw_container);
	connect(main_plotsBar, SIGNAL(relatedSelectedResource(QString, bool)), this, SLOT(slideMatchResources(QString, bool)));
	connect(main_plotsBar, SIGNAL(sendSaveSigToWin(bool)), this, SLOT(prepairForDBsaveWork(bool)));
	connect(main_plotsBar, SIGNAL(sendStopInfoToMainWin(QString)), this, SLOT(stopQmlEditorFunc(QString)));	
	main_plotsBar -> showExpanded();
	animateToolBarLeftRight(tmp_bar, main_plotsBar);
	slideWholeWidgets(tmp_left, leftw_container, true);
}

void MainWindow::allPlotsMerge()
{
	QWidget * tmp_left = leftw_container;
	leftw_container = new LittleWidgetsView(this);
	leftw_container -> setObjectName("p_container");
	QRect o_rect(0, 0 , width(), height()-64);
	leftw_container -> initMarginForParent(o_rect);
	leftw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/plotsviewer.qml");
	QRect rect(0, 0, 300, 200);
	leftw_container -> initViewMatrix(3, 3, rect);
	QWidget * stored_bar = storedBarOnHash(tmp_left, rightw_container);
	QWidget * tmp_bar = showing_toolBar;
	EditLongToolsViewer * main_plotsBar = 0;
	if (stored_bar && qobject_cast<EditLongToolsViewer *>(stored_bar))
		main_plotsBar = qobject_cast<EditLongToolsViewer *>(stored_bar);
	if (!main_plotsBar)
		main_plotsBar = new EditLongToolsViewer(this);
	if (main_plotsBar->objectName().isEmpty() || !main_plotsBar->objectName().contains("mainplotsBar"))
	{
		if (main_plotsBar->objectName().isEmpty())
			main_plotsBar -> setObjectName("mainplotsBar5");
		else
		{
			main_plotsBar = new EditLongToolsViewer(this);
			main_plotsBar -> setObjectName("mainplotsBar5");
		}
	}
	main_plotsBar -> setOrientation(EditLongToolsViewer::ScreenOrientationLockLandscape);
	main_plotsBar -> setMainQmlFile(QLatin1String("/home/dapang/workstation/spc-tablet/spc_qml/plotseditor.qml"), 5);
	main_plotsBar -> setPlotsContainerPt(leftw_container);
	connect(main_plotsBar, SIGNAL(relatedSelectedResource(QString, bool)), this, SLOT(slideMatchResources(QString, bool)));
	connect(main_plotsBar, SIGNAL(sendSaveSigToWin(bool)), this, SLOT(prepairForDBsaveWork(bool)));
	connect(main_plotsBar, SIGNAL(sendStopInfoToMainWin(QString)), this, SLOT(stopQmlEditorFunc(QString)));	
	main_plotsBar -> showExpanded();		
	animateToolBarLeftRight(tmp_bar, main_plotsBar);
	slideWholeWidgets(tmp_left, leftw_container, true);
}

void MainWindow::plotsPrepareForUnfinishedEngi()
{
	LittleWidgetsView * old_left = leftw_container;
	LittleWidgetsView * old_right = rightw_container;
	LittleWidgetsView * g_leftw = new LittleWidgetsView(this);
	g_leftw -> setObjectName("l_container");
	QRect o_rect(0, 0 , width(), height());
	g_leftw -> initMarginForParent(o_rect);
	g_leftw -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablesviewer.qml");
	DataesTableView * dataesin_table = new DataesTableView(0, 0, base_db);
	dataesin_table -> setObjectName("dataesin"); 
	dataesin_table -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	dataesin_table -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
/*	PlotWidget * cur_plot = qobject_cast<PlotWidget *>(leftw_container->qmlAgent()->widget());
	if (cur_plot && cur_plot->currentEngi().isEmpty())//? empty?
		dataesin_table -> setPlotsBackGuide(cur_plot);	
	QString proj_key("projectskeys");
	QString engi_col = "name="+opened_file;
	QString sam_col("grpsamles");
	QString grp_col("groups");
	int model_rows = base_db->dataFromTable(proj_key, grp_col, engi_col).toInt();
	int model_cols = base_db->dataFromTable(proj_key, sam_col, engi_col).toInt();
	QStandardItemModel * init_model = new QStandardItemModel(model_rows, model_cols, this);		
	for (int i = 0; i < init_model->rowCount(); i++)
		init_model -> setHeaderData(i, Qt::Vertical, QString(tr("样本组%1")).arg(i+1));
	for (int i = 0; i < init_model->columnCount(); i++)
		init_model -> setHeaderData(i, Qt::Horizontal, QString(tr("样本%1")).arg(i+1));
	dataesin_table -> initTable(init_model, base_db, opened_file, 0);*/
	dataesin_table -> setInputor(loging_user);
	dataesin_table -> setFixedSize(dataesin_table->horizontalHeader()->length()+dataesin_table->verticalHeader()->width(), dataesin_table->verticalHeader()->length()+dataesin_table->horizontalHeader()->height());
	g_leftw -> setViewForRelatedTable(dataesin_table);//no finished stop here0
	g_leftw -> setGeometry(-old_left->width(), 0, old_left->width(), height());	
	QWidget * tmp_bar = showing_toolBar;
	createOkCancelBars(tmp_bar, tr("新建1未完成"));
	clearRalatedEdittingBarOnHash(tmp_bar);	
	slideMultiWidgets(g_leftw, 0, old_left, old_right, true, true);
	syncCurrentWinWithHash(leftw_container, 0, loging_user);	
	storeRalatedEdittingBar(leftw_container, 0, showing_toolBar);
}

void MainWindow::showLastEngiInitDataes()
{
 	QPair<QString, QString> test_pair;
	if (loging_pairs.isEmpty())
		test_pair = new_login;
	else
		test_pair = loging_user;	 
 	DataesTableView * key_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(test_pair).at(2))->qmlAgent()->widget()); 
  	DataesTableView * din_table = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
	QList<QStandardItemModel *> show_list;
	din_table -> initEditNewConstructionList(show_list, key_table);
	if (show_list.size() == 1)
	{		  
		if (qobject_cast<SlideBar *>(showing_toolBar)->currentCommand() != din_table->commandStack())
			qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(din_table->commandStack());	  
		toggleCurrentBarButton(tr("删除样本"), tr("使能"), false);
		toggleCurrentBarButton(tr("保存当前"), tr("使能"), false);			
		return;
	}	
	QStandardItemModel * din_next = 0;	
	if (show_list.indexOf(qobject_cast<QStandardItemModel *>(din_table->model())) == 0)
		din_next = show_list.at(show_list.size()-1);
	else
		din_next = show_list.at(show_list.indexOf(qobject_cast<QStandardItemModel *>(din_table->model()))-1);		
	din_table -> setModel(din_next);
	din_table -> changeViewStateForNewModel();
	din_table -> setShiftModel(din_next, qobject_cast<QStandardItemModel *>(key_table->model()));
	if (din_table->modelHashs().value(tr("草稿工程，，。")).at(0).second == din_next)
	{
		din_table -> resetEditState(true);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(din_table->commandStack());
		toggleCurrentBarButton(tr("删除样本"), tr("使能"), false);
		toggleCurrentBarButton(tr("保存当前"), tr("使能"), false);		
	}
	else
	{
		din_table -> resetEditState(false);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(din_table->delModelsStack());	
		toggleCurrentBarButton(tr("删除样本"), tr("使能"), true);
		if (!din_table->dataesModelExistedInDb(din_next))
			toggleCurrentBarButton(tr("保存当前"), tr("使能"), true);
	} 
}

void MainWindow::showNextEngiInitDataes()
{
  	QPair<QString, QString> test_pair;
	if (loging_pairs.isEmpty())
		test_pair = new_login;
	else
		test_pair = loging_user; 
 	DataesTableView * key_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(test_pair).at(2))->qmlAgent()->widget()); 
  	DataesTableView * din_table = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
	QList<QStandardItemModel *> show_list;
	din_table -> initEditNewConstructionList(show_list, key_table);
	if (show_list.size() == 1)
	{
		if (qobject_cast<SlideBar *>(showing_toolBar)->currentCommand() != din_table->delModelsStack())
			qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(din_table->delModelsStack());		  	  
		toggleCurrentBarButton(tr("删除样本"), tr("使能"), false);
		toggleCurrentBarButton(tr("保存当前"), tr("使能"), false);			
		return;
	}	
	QStandardItemModel * din_next = 0;	
	if (show_list.indexOf(qobject_cast<QStandardItemModel *>(din_table->model())) == show_list.size()-1)
		din_next = show_list.at(0);
	else
		din_next = show_list.at(show_list.indexOf(qobject_cast<QStandardItemModel *>(din_table->model()))+1);		
	din_table -> setModel(din_next);
	din_table -> changeViewStateForNewModel();
	din_table -> setShiftModel(din_next, qobject_cast<QStandardItemModel *>(key_table->model()));
	if (din_table->modelHashs().value(tr("草稿工程，，。")).at(0).second == din_next)
	{
		din_table -> resetEditState(true);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(din_table->commandStack());
		toggleCurrentBarButton(tr("删除样本"), tr("使能"), false);
		toggleCurrentBarButton(tr("保存当前"), tr("使能"), false);		
	}
	else
	{
		din_table -> resetEditState(false);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(din_table->delModelsStack());	
		toggleCurrentBarButton(tr("删除样本"), tr("使能"), true);
		if (!din_table->dataesModelExistedInDb(din_next))
			toggleCurrentBarButton(tr("保存当前"), tr("使能"), true);
	} 	
}

void MainWindow::copyCurrentInitDataes()
{
	DataesTableView * din_table = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget()); 
	din_table -> resetCopyingModelPtr(qobject_cast<QStandardItemModel *>(din_table->model()));
}

void MainWindow::pasteCopiedInitDataes()
{
   	QPair<QString, QString> test_pair;
	if (loging_pairs.isEmpty())
		test_pair = new_login;
	else
		test_pair = loging_user; 
	DataesTableView * din_table = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
	DataesTableView * key_table = 0;
	int chk_size = ws_cpktests.value(test_pair).size();
	if (chk_size == 7)
		key_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(test_pair).at(2))->qmlAgent()->widget());
	else
		key_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(test_pair).at(3))->qmlAgent()->widget());
	QString chk_res;
	bool pasted = din_table->pasteModelDataesAction(true, qobject_cast<QStandardItemModel *>(key_table->model()), chk_res);
	if (!pasted && !chk_res.isEmpty())
	{
		DiaLog * d_hint = new DiaLog;
		QString d_info("hint:");
		if (chk_res == tr("行列不同"))
			d_info += tr("拷贝的数据组数及样本数\n与此工程不匹配");
		else if (chk_res == tr("已经存在"))
			d_info += tr("拷贝的数据在此\n工程中已经存在");
		else
			d_info += tr("拷贝的数据与\n此表完全相同");
		d_hint -> initWinDialog(this, d_info, 0);
		d_hint -> exec();	
	}
}

void MainWindow::deleteNewProjOneSamples()
{
    	QPair<QString, QString> name_pair; 
 	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user;  
	DataesTableView * chk_din = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
 	DataesTableView * key_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(2))->qmlAgent()->widget());
	chk_din -> deleteTmpModelsPreparation(qobject_cast<QStandardItemModel *>(chk_din->model()), key_table);	
}

void MainWindow::turnToLastNewProject()
{	
 	DataesTableView * key_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
	QList<QStandardItemModel *> show_list;
	key_table -> initEditNewConstructionList(show_list);
	if (show_list.size() == 1)
	{	  
		if (qobject_cast<SlideBar *>(showing_toolBar)->currentCommand() != key_table->commandStack())
			qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->commandStack());	  
		toggleCurrentBarButton(tr("删除新建"), tr("使能"), false);
		toggleCurrentBarButton(tr("保存当前"), tr("使能"), false);			
		return;
	}	
	QStandardItemModel * k_next = 0;	
	if (show_list.indexOf(qobject_cast<QStandardItemModel *>(key_table->model())) == 0)
		k_next = show_list.at(show_list.size()-1);
	else
		k_next = show_list.at(show_list.indexOf(qobject_cast<QStandardItemModel *>(key_table->model()))-1);		
	key_table -> setModel(k_next);
	if (key_table->modelHashs().value(tr("草稿工程，，。")).at(0).second == k_next)
	{
		key_table -> resetEditState(true);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->commandStack());
		toggleCurrentBarButton(tr("删除新建"), tr("使能"), false);
		toggleCurrentBarButton(tr("保存当前"), tr("使能"), false);		
	}
	else
	{
		key_table -> resetEditState(false);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->delModelsStack());	
		toggleCurrentBarButton(tr("删除新建"), tr("使能"), true);
		if (!key_table->dataesModelExistedInDb(k_next))
			toggleCurrentBarButton(tr("保存当前"), tr("使能"), true);
	}
	DataesTableView * next_din = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());	
	if (next_din && !next_din->cursorModels().isEmpty())
	{
		QStandardItemModel * din_cursor = next_din->cursorModels().value(k_next);
		if (din_cursor)
		{
			next_din -> setModel(din_cursor);
			next_din -> changeViewStateForNewModel();
		}
		next_din -> resetEditState(false);		
	}				
}

void MainWindow::turnToNextNewProject()
{
 	DataesTableView * key_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
	QList<QStandardItemModel *> show_list;
	key_table -> initEditNewConstructionList(show_list);
	if (show_list.size() == 1)
	{
		if (qobject_cast<SlideBar *>(showing_toolBar)->currentCommand() != key_table->delModelsStack())
			qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->delModelsStack());		  	  
		toggleCurrentBarButton(tr("删除新建"), tr("使能"), false);
		toggleCurrentBarButton(tr("保存当前"), tr("使能"), false);			
		return;
	}	
	QStandardItemModel * k_next = 0;	
	if (show_list.indexOf(qobject_cast<QStandardItemModel *>(key_table->model())) == show_list.size()-1)
		k_next = show_list.at(0);
	else
		k_next = show_list.at(show_list.indexOf(qobject_cast<QStandardItemModel *>(key_table->model()))+1);		
	key_table -> setModel(k_next);
	if (key_table->modelHashs().value(tr("草稿工程，，。")).at(0).second == k_next)
	{
		key_table -> resetEditState(true);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->commandStack());
		toggleCurrentBarButton(tr("删除新建"), tr("使能"), false);
		toggleCurrentBarButton(tr("保存当前"), tr("使能"), false);		
	}
	else
	{
		key_table -> resetEditState(false);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->delModelsStack());	
		toggleCurrentBarButton(tr("删除新建"), tr("使能"), true);
		if (!key_table->dataesModelExistedInDb(k_next))
			toggleCurrentBarButton(tr("保存当前"), tr("使能"), true);
	}
	DataesTableView * next_din = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());	
	if (next_din && !next_din->cursorModels().isEmpty())
	{
		QStandardItemModel * din_cursor = next_din->cursorModels().value(k_next);
		if (din_cursor)
		{
			next_din -> setModel(din_cursor);
			next_din -> changeViewStateForNewModel();
		}
		next_din -> resetEditState(false);		
	}
}

void MainWindow::deleteNewProject()
{
   	QPair<QString, QString> test_pair;
	if (loging_pairs.isEmpty())
		test_pair = new_login;
	else
		test_pair = loging_user;   
	DataesTableView * key_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(test_pair).at(2))->qmlAgent()->widget());
	key_table -> deleteTmpModelsPreparation(qobject_cast<QStandardItemModel *>(key_table->model()), 0);
}
	
void MainWindow::showLastDbEngiDataes()
{
	testEngiSampPromotionTransition(false);
}
	
void MainWindow::showNextDbEngiDataes()
{
 	testEngiSampPromotionTransition(true); 
}
	
void MainWindow::deleteDbProjOneSamples()
{
 	EngiTreeView * improve_tree = qobject_cast<EngiTreeView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(2))->qmlAgent()->widget());
	DataesTableView * key_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
	DataesTableView * next_din = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
	QStandardItemModel * d_model = qobject_cast<QStandardItemModel *>(next_din->model());
	if (d_model == next_din->modelHashs().value(tr("草稿工程，，。")).at(0).second)
		return;
	QString din_time = next_din->getInfoByModel(d_model, tr("时间"));
	QList<QStandardItem *> time_item = qobject_cast<QStandardItemModel *>(improve_tree->model())->findItems(din_time, Qt::MatchRecursive | Qt::MatchExactly);		
	next_din -> deletePromotionModel(time_item.at(0), key_table);  
}
	
void MainWindow::turnDbLastProject()
{
	testEngiPromotionTransition(false);			
}
	
void MainWindow::turnDbNextProject()
{
	testEngiPromotionTransition(true);	
}

void MainWindow::newTestForEngiVersion()
{
	DataesTableView * key_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
	DataesTableView * next_din = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
	QStandardItemModel * manual_model = key_table->modelHashs().value(tr("草稿工程，，。")).at(0).second;
	if (key_table->model() != manual_model)
	{
		QString key_time = key_table->getInfoByModel(qobject_cast<QStandardItemModel *>(key_table->model()), tr("时间")); 
		next_din -> changeModelForEditting(qobject_cast<QStandardItemModel *>(next_din->model()), key_time, key_table);	  
		key_table -> changeModelForEditting(qobject_cast<QStandardItemModel *>(key_table->model()), key_table->model()->data(key_table->model()->index(0, 0)).toString());
		QStringList detail_list = base_db->curDBmanageTable().value(loging_user.second);
		for (int i = 1; i < key_table->model()->rowCount(); i++) 
		{
			if (i > 8)
			{
				if (loging_user.second != key_table->model()->data(key_table->model()->index(10, 0)).toString())
				{
					if (i == 9)
						key_table->model() -> setData(key_table->model()->index(9, 0), loging_user.first);
					else if (i == 10)
						key_table->model() -> setData(key_table->model()->index(10, 0), loging_user.second);
					else if (i == 11)
						key_table->model() -> setData(key_table->model()->index(11, 0), detail_list.at(4));
					else
						key_table->model() -> setData(key_table->model()->index(12, 0), detail_list.at(2));		
				}
			}
			else
				key_table->model()->setData(key_table->model()->index(i, 0), "");
		}
	}
	key_table -> resetEditState(true);
	qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->commandStack());	
}

void MainWindow::deleteDbProject()
{
	DataesTableView * key_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
	if (loging_user.second != key_table->model()->data(key_table->model()->index(10, 0)).toString())
		return;
	QStandardItemModel * key_model = qobject_cast<QStandardItemModel *>(key_table->model());	
	QString proj = key_model->data(key_model->index(0, 0)).toString();
	if (key_model == key_table->modelHashs().value(tr("草稿工程，，。")).at(0).second)
	{
		key_table->modelHashs()[tr("草稿工程，，。")][0].first = "";
		key_table->commandStack()->clear();
		key_table -> setModel(key_table->modelHashs().value(proj).at(0).second);
		return;
	}	
	EngiTreeView * improve_tree = qobject_cast<EngiTreeView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(2))->qmlAgent()->widget());	
	DataesTableView * next_din = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());		
	QString k_time = key_table->modelConstructTime(proj, key_model);
	QList<QStandardItem *> time_item = qobject_cast<QStandardItemModel *>(improve_tree->model())->findItems(k_time, Qt::MatchRecursive | Qt::MatchExactly);
	foreach (QStandardItem * e_item, time_item)
	{
		if (e_item->parent()->text() == tr("创建时间"))
			time_item.removeOne(e_item);
	}
	key_table -> deletePromotionModel(time_item.at(0), next_din);
}

void MainWindow::engiInitAuxiliaryGraph(bool down)
{
 	QPair<QString, QString> name_pair;
	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user; 
	int w_pos = 3;
	if (ws_cpktests.value(name_pair).size() == 8)
		w_pos += 1;
	DataesTableView * chk_din = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos))->qmlAgent()->widget());
	QString checking;	
	if (!chk_din->checkInputState(checking))
	{
		QBoxLayout * menu_layout = qobject_cast<QBoxLayout *>(showing_toolBar->layout());
		for (int i = 0; i < menu_layout->count(); i++)
		{
			QToolButton * chk_button = qobject_cast<QToolButton *>(menu_layout->itemAt(i)->widget());
			if (chk_button->text() == tr("显示辅图"))
			{
				chk_button -> setChecked(false);
				break;
			}
		}	
		return;
	}	
	LittleWidgetsView * h_qml = 0;
	if (ws_cpktests.value(name_pair).at(w_pos+2))
		h_qml = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos+2));
	LittleWidgetsView * g_qml = 0;
	if (ws_cpktests.value(name_pair).at(w_pos+3))
		g_qml = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos+3));
	if (leftw_container==h_qml || leftw_container==g_qml)
	{	  
		LittleWidgetsView * tmp_container = 0;
		if (leftw_container == h_qml)
			tmp_container = h_qml;
		else
			tmp_container = g_qml;
		leftw_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos));
		slideLeftQmlWidgets(tmp_container, leftw_container, rightw_container, false);		
	}
	qobject_cast<SlideBar *>(showing_toolBar) -> rearrangeLayoutBtns(down);
	showing_toolBar -> setGeometry(width()-showing_toolBar->width(), 0, showing_toolBar->width(), 64);	
}

void MainWindow::histogramGenerationForInitProj()
{
  	QPair<QString, QString> name_pair;
	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user;  
	int w_pos = 2;
	if (ws_cpktests.value(name_pair).size() == 8)
		w_pos += 1;	
	DataesTableView * cur_din = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos+1))->qmlAgent()->widget());	
	DataesTableView * keys_tbl = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos))->qmlAgent()->widget());
	QString ktbl_time = keys_tbl->modelConstructTime(keys_tbl->model()->data(keys_tbl->model()->index(0, 0)).toString(), qobject_cast<QStandardItemModel *>(keys_tbl->model()));
	QString proj_ctime = cur_din->modelConstructTime(ktbl_time, qobject_cast<QStandardItemModel *>(cur_din->model()));	
	if (proj_ctime == tr("草稿工程，，。"))
	{
		QString edit_time(keys_tbl->modelConstructTime(keys_tbl->model()->data(keys_tbl->model()->index(0, 0)).toString(), keys_tbl->shiftedKeyForModelSelection()));						  
		QString nd_time(edit_time+";"+QDateTime::currentDateTime().toString());
		cur_din -> tmpSaveProjsModels(edit_time, nd_time);
		cur_din -> setShiftModel(qobject_cast<QStandardItemModel *>(cur_din->model()), qobject_cast<QStandardItemModel *>(keys_tbl->model()));		
	}	
	if (ws_cpktests.value(name_pair).at(w_pos+3))
	{
		LittleWidgetsView * h_qml = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos+3));
		PlotWidget * h_plot = qobject_cast<PlotWidget *>(h_qml->qmlAgent()->widget());
		HistogramPlot * histogram = qobject_cast<HistogramPlot *>(h_plot->respondMatchPtr(tr("直方图")));
		QStandardItemModel * histo_with = histogram->currentCpkDataesModel();
		bool found = false;
		if (histo_with && cur_din->model()!=histo_with)
		{			
			QList<HistogramPlot *> e_histoes = h_plot->currentHistograms();
			foreach (HistogramPlot * histo, e_histoes)
			{
				if (cur_din->model() == histo->currentCpkDataesModel())
				{
					found = true;
					h_plot -> resetRelatedToolPlot(qobject_cast<QStandardItemModel *>(cur_din->model()));
					break;
				}
			}
		}
		if (!found)
			h_plot -> generateSingleToolplot(qobject_cast<QStandardItemModel *>(cur_din->model()), true);
		if (leftw_container != h_qml)
		{
			LittleWidgetsView * tmp_container = leftw_container;
			leftw_container = h_qml;
			slideLeftQmlWidgets(tmp_container, leftw_container, rightw_container, false);
		}
	}
	else
	{
		LittleWidgetsView * plot_container = new LittleWidgetsView(this);
		plot_container -> setObjectName(tr("直方图容器"));
		plot_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");
		QRect o_rect(0, 0 , width(), height());
		plot_container -> initMarginForParent(o_rect);
		PlotWidget * h_plot = new PlotWidget(back_science, base_db);
		h_plot -> setObjectName("histogram");
		h_plot -> setFixedWidth(width());
		h_plot -> setFixedHeight(height());
		plot_container -> setViewForRelatedTable(h_plot);
		h_plot -> generateSingleToolplot(qobject_cast<QStandardItemModel *>(cur_din->model()), true);
		ws_cpktests[name_pair][w_pos+3] = plot_container;	
		LittleWidgetsView * tmp_container = leftw_container;
		leftw_container = plot_container;
		leftw_container -> lower();
		slideLeftQmlWidgets(tmp_container, leftw_container, rightw_container, true);		
	}
	if (cur_din->commandStack()->count())
		cur_din->commandStack()->clear();	
	showing_toolBar -> raise();
}

void MainWindow::gaussPlotGenerationForInitProj()
{
   	QPair<QString, QString> name_pair;
	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user;   
	int w_pos = 2;
	if (ws_cpktests.value(name_pair).size() == 8)
		w_pos += 1;	
	DataesTableView * cur_din = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos+1))->qmlAgent()->widget());	
	DataesTableView * keys_tbl = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos))->qmlAgent()->widget());
	QString ktbl_time = keys_tbl->modelConstructTime(keys_tbl->model()->data(keys_tbl->model()->index(0, 0)).toString(), qobject_cast<QStandardItemModel *>(keys_tbl->model()));
	QString proj_ctime = cur_din->modelConstructTime(ktbl_time, qobject_cast<QStandardItemModel *>(cur_din->model()));	
	if (proj_ctime == tr("草稿工程，，。"))
	{
		QString edit_time(keys_tbl->modelConstructTime(keys_tbl->model()->data(keys_tbl->model()->index(0, 0)).toString(), keys_tbl->shiftedKeyForModelSelection()));						  
		QString nd_time(edit_time+";"+QDateTime::currentDateTime().toString());
		cur_din -> tmpSaveProjsModels(edit_time, nd_time);
		cur_din -> setShiftModel(qobject_cast<QStandardItemModel *>(cur_din->model()), qobject_cast<QStandardItemModel *>(keys_tbl->model()));		
	}	
	if (ws_cpktests.value(name_pair).at(w_pos+4))
	{
		LittleWidgetsView * g_qml = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos+4));
		PlotWidget * g_plot = qobject_cast<PlotWidget *>(g_qml->qmlAgent()->widget());
		GaussPlot * gauss = qobject_cast<GaussPlot *>(g_plot->respondMatchPtr(tr("高斯图")));		
		QStandardItemModel * gauss_with = gauss->currentCpkDataesModel();
		bool found = false;
		if (gauss_with && cur_din->model()!=gauss_with)
		{
			QList<GaussPlot *> e_gausses = g_plot->currentGausses();
			foreach (GaussPlot * gas, e_gausses)
			{
				if (cur_din->model() == gas->currentCpkDataesModel())
				{
					found = true;
					g_plot -> resetRelatedToolPlot(qobject_cast<QStandardItemModel *>(cur_din->model()));				
					break;
				}
			}
		}
		if (!found)
			g_plot -> generateSingleToolplot(qobject_cast<QStandardItemModel *>(cur_din->model()), false);
		if (leftw_container != g_qml)
		{
			LittleWidgetsView * tmp_container = leftw_container;
			leftw_container = g_qml;
			slideLeftQmlWidgets(tmp_container, leftw_container, rightw_container, false);
		}
	}
	else
	{
		LittleWidgetsView * plot_container = new LittleWidgetsView(this);
		plot_container -> setObjectName(tr("高斯图容器"));
		plot_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");
		QRect o_rect(0, 0 , width(), height());
		plot_container -> initMarginForParent(o_rect);
		PlotWidget * g_plot = new PlotWidget(back_science, base_db);
		g_plot -> setObjectName("gauss");
		g_plot -> setFixedWidth(width());
		g_plot -> setFixedHeight(height());
		plot_container -> setViewForRelatedTable(g_plot);
		g_plot -> generateSingleToolplot(qobject_cast<QStandardItemModel *>(cur_din->model()), false);
		ws_cpktests[name_pair][w_pos+4] = plot_container;
		LittleWidgetsView * tmp_container = leftw_container;
		leftw_container = plot_container;
		leftw_container -> lower();
		slideLeftQmlWidgets(tmp_container, leftw_container, rightw_container, true);		
	}
	if (cur_din->commandStack()->count())
		cur_din->commandStack()->clear();	
	showing_toolBar -> raise();	
}

void MainWindow::calForEngiInitDataes(bool calculating)
{
	toggleCurrentBarButton(tr("计算结果"), tr("切换"), calculating);  
 	LittleWidgetsView * tmp_right = rightw_container; 
	QPair<QString, QString> name_pair;
	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user;
	int w_pos = 2;
	if (ws_cpktests.value(name_pair).size() == 8)
		w_pos += 1;	
	if (!calculating)
	{
		rightw_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos));
		slideRightWidgets(leftw_container, rightw_container, tmp_right);		
		return;		
	}
	DataesTableView * cur_dataesin = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos+1))->qmlAgent()->widget());
	QString checking;	
	if (!cur_dataesin->checkInputState(checking))
	{
		QString warn_info = "warning:"+tr("未输入数据或数据不完全\n无法进行计算");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 1);
		toggleCurrentBarButton(tr("计算结果"), tr("切换"), false); 
		return;
	}
	if (!cur_dataesin->cpkInitDataesCalucationCheck()) //need add some warning
		return;
	DataesTableView * keys_tbl = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos))->qmlAgent()->widget());		
	QString current_proj = keys_tbl->model()->data(keys_tbl->model()->index(0, 0)).toString();
	QString ktbl_time = keys_tbl->modelConstructTime(current_proj, qobject_cast<QStandardItemModel *>(keys_tbl->model()));
	QString proj_ctime = cur_dataesin->modelConstructTime(ktbl_time, qobject_cast<QStandardItemModel *>(cur_dataesin->model()));
	QStandardItemModel * cal_model = 0;	
	if (proj_ctime != tr("草稿工程，，。"))
	{
		QString tbl_stamp = QString("%1").arg(QDateTime::fromString(proj_ctime).toTime_t());
		QStringList tbl_name = base_db->allTables().filter(tbl_stamp).filter(tr("，，。cpk"));	
		if (!tbl_name.isEmpty())
		{
			QString cpk_stamp = QString("%1").arg(QDateTime::fromString(ktbl_time).toTime_t());
			QString row_time = QDateTime::fromString(proj_ctime).toString(Qt::ISODate);
			QStringList cpk_name = tbl_name.filter(tr("，，。cpkdataes"));
			tbl_name.removeOne(cpk_name.at(0));
			QString time_row("time="+row_time);
			double tolAvr = base_db->dataFromTable(cpk_name.at(0), "avr", time_row).toDouble();
			QSqlTableModel cpk_sql(this, base_db->currentConnectionDb()->database(base_db->currentConnectionDb()->connectionName()));
			base_db -> varsModelFromTable(tbl_name.at(0), &cpk_sql);
			QList<double> cal_dataes;	
			cal_dataes << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
			for (int i = 1; i < cpk_sql.columnCount()-1; i++)//calings << s_cpk << s_sigma << cpk_tolAvr << upper_avrPara << lower_avrPara << num_s << upper_sPara << lower_sPara;dev, dsigma, cpk, upavr, lowavr, updev, lowdev
			{
				if (i == 1)
					cal_dataes[5] = cpk_sql.data(cpk_sql.index(0, i)).toDouble();
				else if (i == 2)
					cal_dataes[1] = cpk_sql.data(cpk_sql.index(0, i)).toDouble();
				else if (i == 3)
					cal_dataes[0] = cpk_sql.data(cpk_sql.index(0, i)).toDouble();				
				else if (i == 4)
					cal_dataes[3] = cpk_sql.data(cpk_sql.index(0, i)).toDouble();	
				else if (i == 5)
					cal_dataes[4] = cpk_sql.data(cpk_sql.index(0, i)).toDouble();	
				else
					cal_dataes[i] = cpk_sql.data(cpk_sql.index(0, i)).toDouble();					
			}
			cal_dataes[2] = tolAvr;
			QString dev_rng;
			if (tbl_name.at(0).contains("dev"))
				dev_rng = tr("均值西格玛");
			else
				dev_rng = tr("均值极差");  
			QString proj_info(current_proj+"^"+dev_rng+"^"+tr("数据库中存在"));
			showCurSpcCalRes(proj_info, cal_dataes);
			return;
		}
		if (ws_cpktests.value(name_pair).at(w_pos+2) && cur_dataesin->nestingUnionView()->modelHashs().contains(proj_ctime))
			cal_model = cur_dataesin->nestingUnionView()->modelHashs().value(proj_ctime).at(0).second;
	}
	else
		proj_ctime = QDateTime::currentDateTime().toString();
	if (cal_model)
	{
		qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos+2))->qmlAgent()->widget())->setModel(cal_model);
		rightw_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos+2));
		if (rightw_container->width() < rightw_container->initRect().width())
			  rightw_container -> setFixedWidth(rightw_container->initRect().width());
		slideRightWidgets(leftw_container, rightw_container, tmp_right);		
		return;
	}
	cur_dataesin -> tmpSaveProjsModels(ktbl_time, proj_ctime);
	if (cur_dataesin->commandStack()->count())
		cur_dataesin->commandStack()->clear();		
	toggleCurrentBarButton(tr("删除新建"), tr("使能"), true, ws_cpktests.value(name_pair).at(0));
	toggleCurrentBarButton(tr("保存当前"), tr("使能"), true, ws_cpktests.value(name_pair).at(0));
	toggleCurrentBarButton(tr("保存当前"), tr("使能"), true);
	toggleCurrentBarButton(tr("删除样本"), tr("使能"), true);	
	back_science -> runSpcInitCal(cur_dataesin, keys_tbl);	
}

void MainWindow::calForDailydata()
{
	if (!rightw_container)
		return;
	DataesTableView * tmpinput_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
	QString checking;	
	if (!tmpinput_table->checkInputState(checking))
	{
		QString warn_info = "warning:"+tr("数据输入不全或有误\n无法进行计算");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 1);
		return;
	}
	if (!tmpinput_table->dailyDataesCalucationCheck())
		return;
	PlotWidget * plot_widget = qobject_cast<PlotWidget *>(leftw_container->qmlAgent()->widget());
	back_science -> setCalculatingPlots(plot_widget);
	back_science -> dailyDatesCal(tmpinput_table);
}

void MainWindow::setProtectTimer(int msec)
{
	if (!protect_timer->isActive())
		protect_timer -> start(msec);	
}

void MainWindow::setNoProtect()
{
	protect_state = 0;
}
	
void MainWindow::setAutoProtect()
{
	protect_state = 1;
}
	
void MainWindow::setManualProtect()
{
	protect_state = 2;
}

void MainWindow::initUserState()
{
	sys_userState = 0;	
}

void MainWindow::startProtectionMode()
{
	if (protect_widget || loging_pairs.isEmpty())
		return;
	protect_mode = true;
	LittleWidgetsView * pro_left = leftw_container;
	LittleWidgetsView * old_right = rightw_container;	
	protect_widget = new QWidget(this);
	setProtectWidgetBackground(protect_widget);	
	QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
/*	QString tmp_save(tr("保护模式存储工具条"));
	QString tmp_bar(tr("工具条"));
	QPair<QString, QWidget *> protect_pair(tmp_bar, showing_toolBar);
	ws_cpktests << protect_pair;*/	
	if (showing_toolBar)
	{
		if (showing_toolBar == stored_bar)
			animateToolBarLeftRight(showing_toolBar, 0, false);
		else
			animateToolBarLeftRight(showing_toolBar, 0);
	}
	slideMultiWidgets(protect_widget, 0, pro_left, old_right, false, true);
}

void MainWindow::errorToRestartSys()
{
	emit closeSystem();
}
	
void MainWindow::manualTblForSpec()
{
	QString fur_desciption = tr("数据手工制表条");
	initManualTableConstruct(0, fur_desciption);	
}

void MainWindow::manualTblForMrgmnt()
{
	QString fur_desciption = tr("管理手工制表条");
	initManualTableConstruct(0, fur_desciption);  
}

void MainWindow::saveAllFreeConstruction()
{
	RelatedTreeView * r_mgt = qobject_cast<RelatedTreeView *>(rightw_container->qmlAgent()->widget());
	r_mgt -> saveFreeConstructionTodb();	
}

void MainWindow::dbEditorName(const QString & n_text)
{
	new_login.first = n_text;
}
	
void MainWindow::dbEditorPassword(const QString & pwd_text)
{
	new_login.second = pwd_text;	
}

bool MainWindow::saveProjInitPerformance()
{
 	QPair<QString, QString> name_pair; 
 	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user; 
	DataesTableView * initEngi_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(2))->qmlAgent()->widget());
	QString checking;	
	if (!initEngi_table->checkInputState(checking))
	{
		QString warn_info;
		if (!checking.contains("no_number") && !checking.contains("empty"))
			warn_info = "warning:"+checking;
		else
			warn_info = tr("warning:")+tr("数据输入有误！\n请检查输入");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 0);
		return false;
	}	
	QString cur_proj = initEngi_table->model()->data(initEngi_table->model()->index(0, 0)).toString();
	QString save_time = initEngi_table->modelConstructTime(cur_proj, qobject_cast<QStandardItemModel *>(initEngi_table->model()));	
	if (save_time == tr("草稿工程，，。"))
	{
		save_time = QDateTime::currentDateTime().toString();
		initEngi_table -> tmpSaveProjsModels(cur_proj, save_time);
		initEngi_table -> setShiftModel(qobject_cast<QStandardItemModel *>(initEngi_table->model()));			  
	}
	QString sel_var("name="+cur_proj);
	QString key_var = base_db->dataFromTable("projectskeys", "grpsamles", sel_var).toString();	
	QDateTime test_save(QDateTime::fromString(save_time));
	if (key_var.isEmpty())
	{
		QString origin_time;	
		if (!initEngi_table->modelHashs().value(cur_proj).isEmpty())
			origin_time = initEngi_table->modelHashs().value(cur_proj).at(0).first;
		if (!origin_time.isEmpty())
		{					  
			QDateTime test_origin(QDateTime::fromString(origin_time));			
			if (test_save > test_origin)
			{
				initEngi_table -> swapModelsForOriginToDb(qobject_cast<QStandardItemModel *>(initEngi_table->model()));
				save_time = origin_time;
			}
		}
	}
	if (!key_var.contains(save_time))
	{
		if (!initEngi_table->saveProjKeyDataesCheck(key_var.isEmpty(), save_time))
		{
			QString error_info("error:"+tr("严重错误\n工程未保存\n请再试一次！"));
			DiaLog * w_dialog = new DiaLog;
			w_dialog -> initWinDialog(this, error_info, 0);
			if (w_dialog->exec())
				saveProjInitPerformance();
			return false;	
		}	
	}
	if (showing_toolBar->objectName() == tr("okcancel_新建1"))
	{
		DataesTableView * cur_dataesin = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
		QString checking;	
		if (!cur_dataesin->checkInputState(checking))
		{
			QString warn_info("warning:"+tr("数据输入有误\n请检查输入！"));
			DiaLog * w_dialog = new DiaLog;
			w_dialog -> initWinDialog(this, warn_info, 1);
			return false;		  
		}
		DataesTableView * calres_table = 0;		
		if (ws_cpktests.value(name_pair).at(4))
			calres_table = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(4))->qmlAgent()->widget());	
		QString din_time = cur_dataesin->modelConstructTime(save_time, qobject_cast<QStandardItemModel *>(cur_dataesin->model()));		
		QStandardItemModel * cal_model = 0; 
		if (din_time != tr("草稿工程，，。"))
			cal_model = calres_table->modelHashs().value(din_time).at(0).second;
		if (!cal_model)
		{
			QString warn_info("warning:"+tr("数据未计算，\n不能保存！"));
			DiaLog * w_dialog = new DiaLog;
			w_dialog -> initWinDialog(this, warn_info, 1);
			return false;			
		}				
		QString save_cpk;		
		if (initEngi_table->model()->data(initEngi_table->model()->index(3, 0)).toString().contains(tr("西格玛")))
			save_cpk = cur_proj+tr("，，。")+"cpkdataes"+tr("，，。")+"cpkdev";
		else
			save_cpk = cur_proj+tr("，，。")+"cpkdataes"+tr("，，。")+"cpkrng";	
		QString inputor = initEngi_table->model()->data(initEngi_table->model()->index(10, 0)).toString();
		if (!base_db->createEngiTables(save_cpk, inputor, *base_db->currentConnectionDb(), save_time+";"+din_time))
		{
			QString error_info("error:"+tr("严重错误\n表格未创建，请再试一次！"));
			DiaLog * w_dialog = new DiaLog;
			w_dialog -> initWinDialog(this, error_info, 0);
			return false;
		}
		if (!base_db->storeSpcDataes(save_cpk, cur_dataesin, calres_table, initEngi_table, save_time+";"+din_time))
		{
			QString error_info("error:"+tr("严重错误\n数据未保存，请再试一次！"));
			DiaLog * w_dialog = new DiaLog;
			w_dialog -> initWinDialog(this, error_info, 0);
			return false;	  
		} 		
	}	
	toggleCurrentBarButton(tr("保存当前"), tr("使能"), false);
	loging_user.first = initEngi_table->model()->data(initEngi_table->model()->index(9, 0)).toString();
	loging_user.second = initEngi_table->model()->data(initEngi_table->model()->index(10, 0)).toString();
	loging_pairs << loging_user;
	sys_userState = base_db->userClass(loging_user);
	base_db -> resetManagersDbMap();
	base_db -> resetDepartmentsDbMap();
	base_db -> resetProjsDbMap();	
	return true;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	if (rightw_container && leftw_container)
	{
		leftw_container -> setGeometry(0, 0, width()-rightw_container->width(), height());
		rightw_container -> setGeometry(width()-rightw_container->width(), 64, rightw_container->width(), height()-64);
	}
	else if (leftw_container)
	{
		leftw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablesviewer.qml");
		leftw_container -> setGeometry(0, 0, width(), height());
		QRect o_rect(0, 0 , width(), height());
		leftw_container -> initMarginForParent(o_rect);
		PlotWidget * plot_new = new PlotWidget(back_science, base_db);
		plot_new -> setObjectName("plot");
		plot_new -> setEnginame();
		plot_new -> setFixedWidth(width());
		plot_new -> setFixedHeight(height());
		leftw_container -> setViewForRelatedTable(plot_new);
	}
	if (showing_toolBar)
		showing_toolBar -> setGeometry(width()-showing_toolBar->width(), 0, showing_toolBar->width(), 64);
	QFrame::resizeEvent(event);
}

void MainWindow::clearBarsAnimation(QObject * obj_dest)
{
	if (obj_dest == p_animation)
		p_animation = 0;
	else if (obj_dest == c_animation)
		c_animation = 0;
	else if (obj_dest == l_animation1)
		l_animation1 = 0;
	else if (obj_dest == l_animation2)
		l_animation2 = 0;	
	else if (obj_dest == r_animation1)
		r_animation1 = 0;
	else if (obj_dest == r_animation2)
		r_animation2 = 0;	
	else if (obj_dest == v1_group)
		v1_group = 0;
	else if (obj_dest == v2_group)
		v2_group = 0;	
	else if (obj_dest == s_group)
		s_group = 0;

}

void MainWindow::clearWidgetsAnimation(QObject * obj_dest)
{
	if (obj_dest == l_animation3)
		l_animation3 = 0;
	else if (obj_dest == l_animation4)
		l_animation4 = 0;	
	else if (obj_dest == r_animation3)
		r_animation3 = 0;
	else if (obj_dest == r_animation4)
		r_animation4 = 0;
	else if (obj_dest == v3_group)
		v3_group = 0;
	else if (obj_dest == v4_group)
		v4_group = 0;	
	else if (obj_dest == e_group)
		e_group = 0;	
}

void MainWindow::showCurSpcCalRes(QString & t_proj, const QList<double> & show_caleds)
{
   	QPair<QString, QString> name_pair; 
 	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user; 
	QStringList show_projs = t_proj.split("^");  
	LittleWidgetsView * tmp_right = rightw_container;	
	int w_pos = 4;
	if (ws_cpktests.value(name_pair).size() == 8)
		w_pos += 1;
	DataesTableView * keys_tbl = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos-2))->qmlAgent()->widget());	
	DataesTableView * cur_dataesin = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos-1))->qmlAgent()->widget());	
	QString cur_testTime = cur_dataesin->getInfoByModel(qobject_cast<QStandardItemModel *>(cur_dataesin->model()), tr("时间"));	
	DataesTableView * calres_table = 0;
	if (ws_cpktests.value(name_pair).at(w_pos))
	{
		rightw_container = qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(w_pos));
		if (rightw_container->width() < rightw_container->initRect().width())
			rightw_container -> setFixedWidth(rightw_container->initRect().width());
		calres_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());	
		calres_table -> initTable(t_proj, cur_dataesin, show_caleds);	
	}
	else
	{
		calres_table = new DataesTableView(0, 1, base_db, false);
		calres_table -> setObjectName("calres");
		calres_table -> initTable(t_proj, cur_dataesin, show_caleds);
		cur_dataesin -> setCpkProcessPartner(calres_table);
		if (w_pos > 4)
		{
			EngiTreeView * improve_tree = qobject_cast<EngiTreeView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(2))->qmlAgent()->widget());
			calres_table -> setPromotionTreePtr(improve_tree);
		}		
		rightw_container = new LittleWidgetsView(this);
		rightw_container -> setObjectName(tr("计算结果表"));
		ws_cpktests[name_pair][w_pos] = rightw_container;		
		QRect o_rect;
		calres_table -> setFixedHeight(height()-64);
		rightw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/treeviewer.qml");
		calres_table -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		calres_table -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		o_rect.setWidth(calres_table->sizeHint().width());
		o_rect.setHeight(calres_table->height());
		rightw_container -> initMarginForParent(o_rect);
		rightw_container -> setViewForRelatedTable(calres_table);
		calres_table -> setRightOrigRect(rightw_container->initRect());	
	}
	if (w_pos == 4)
		calres_table -> tmpSaveProjsModels(cur_testTime, cur_testTime);	
	cur_dataesin -> resetEditState(false);		
	if (tmp_right != rightw_container)
	{
		rightw_container -> setGeometry(width(), 64, rightw_container->width(), height()-64);		
		showing_toolBar -> raise();	
	}
	slideRightWidgets(leftw_container, rightw_container, tmp_right);
	if (w_pos > 4)
	{
		EngiTreeView * improve_tree = qobject_cast<EngiTreeView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(name_pair).at(2))->qmlAgent()->widget());
		if (show_projs.size()>2 && show_projs.at(2)==tr("数据库中存在"))
			calres_table -> tmpSaveProjsModels(cur_testTime, cur_testTime);
		else
			improve_tree -> addItemForNewSamp(calres_table, cur_dataesin, keys_tbl);
	}
}

void MainWindow::improveEngiSamples()
{
	DataesTableView * keys_tbl = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(3))->qmlAgent()->widget());
	DataesTableView * din_tbl = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(4))->qmlAgent()->widget());
	QString chk_first;
	if (!keys_tbl->checkInputState(chk_first))
	{
		QString warn_info;
		if (!chk_first.contains("no_number") && !chk_first.contains("empty"))
			warn_info = "warning:"+chk_first;
		else
			warn_info = tr("warning:")+tr("数据输入有误！\n请检查输入");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, warn_info, 0);
		return;
	}
	QString test_proj(keys_tbl->model()->data(keys_tbl->model()->index(0, 0)).toString());
	if (!keys_tbl->existedEntirelyModelInHash(test_proj, qobject_cast<QStandardItemModel *>(keys_tbl->model())))
	{
		EngiTreeView * improve_tree = qobject_cast<EngiTreeView *>(qobject_cast<LittleWidgetsView *>(ws_cpktests.value(loging_user).at(2))->qmlAgent()->widget());
		QStandardItem * selected_item = improve_tree->currentEngiItem();
		improve_tree -> addItemForNewTest(selected_item, keys_tbl);
	}
	QWidget * tmp_bar = showing_toolBar;
	clearRalatedEdittingBarOnHash(tmp_bar);
	SlideBar * new_toolBar = 0;
	if (ws_cpktests.value(loging_user).at(0) && ws_cpktests.value(loging_user).at(0)->objectName()=="samtestsBar")
		new_toolBar = qobject_cast<SlideBar *>(ws_cpktests.value(loging_user).at(0));
	else
	{
		new_toolBar = new SlideBar(this);
		new_toolBar -> setObjectName("samtestsBar");
		new_toolBar -> setCommandStack(din_tbl->commandStack());
		new_toolBar -> showImproveTools(0);
		new_toolBar -> setGeometry(width()-new_toolBar->sizeHint().width(), -64, new_toolBar->sizeHint().width(), 64);	
	}
	ws_cpktests[loging_user][0] = tmp_bar;	
	animateToolBarWidgetUpDown(tmp_bar, 0, new_toolBar, 0);	
	new_toolBar -> show();
	storeRalatedEdittingBar(leftw_container, rightw_container, new_toolBar);
	QString key_time = keys_tbl->getInfoByModel(qobject_cast<QStandardItemModel *>(keys_tbl->model()), tr("时间"));
	if (keys_tbl->model()->data(keys_tbl->model()->index(10, 0)).toString() == loging_user.second)
	{	
		din_tbl -> changeModelForEditting(qobject_cast<QStandardItemModel *>(din_tbl->model()), key_time, keys_tbl);	
		promotionEdittingOrderTrans(din_tbl->commandStack());
		din_tbl -> resetEditState(true);
	}
	else	
	{
		din_tbl -> setModel(din_tbl->modelHashs().value(key_time).at(0).second);
		din_tbl -> changeViewStateForNewModel();	
		din_tbl -> resetEditState(false);
		toggleCurrentBarButton(tr("删除样本"), tr("使能"), false);	
	}		
}

void MainWindow::refreshTableSize(DataesTableView * desTable)
{
	if(desTable->objectName() == "initEngi")
	{
		PlotWidget * cur_plot = qobject_cast<PlotWidget *>(leftw_container->qmlAgent()->widget());
		desTable -> resizeColumnsToContents();
		if (cur_plot)
		{
			leftw_container -> setGeometry(0, 0, width()-desTable->verticalHeader()->width()- desTable->columnWidth(0), height());
			desTable -> setGeometry(width()-desTable->verticalHeader()->width()-desTable->columnWidth(0), 64, desTable->verticalHeader()->width()+desTable->columnWidth(0), height()-64);		
		}
	}
}

void MainWindow::slideMatchResources(QString dest, bool s_h)
{
	if (dest==tr("数据源") || dest==tr("数据编辑"))
	{
		if (!s_h)
		{
			TableManagement * dataes_mtable = 0;
			if (!rightw_container)
			{
				dataes_mtable = new TableManagement;
				connect(dataes_mtable, SIGNAL(showSavedManualPlots(TableManagement *, LittleWidgetsView *, bool)), this, SLOT(showManualTableFromDB(TableManagement *, LittleWidgetsView *, bool)));
				dataes_mtable -> setObjectName("dataesm");
				dataes_mtable -> initManageTree(loging_user, back_science, base_db, 0, leftw_container, qobject_cast<EditLongToolsViewer *>(showing_toolBar));
				qobject_cast<EditLongToolsViewer *>(showing_toolBar) -> setRelatedTreePt(0, dataes_mtable);
				
			}
			else
				dataes_mtable = qobject_cast<TableManagement *>(rightw_container->qmlAgent()->widget());
			embededOrSlideWidgetOnRightQml(dataes_mtable);
			dataes_mtable -> setInitViewRect(rightw_container->initRect());
		}
		else
			slideWidgetView(leftw_container, rightw_container, 1);
	}
	else if (dest==tr("信息源") || dest==tr("编辑信息"))
	{
		if (!s_h)
		{
			TableManagement * minfo_mtable = 0;
			if (!rightw_container)
			{
				minfo_mtable = new TableManagement;
				connect(minfo_mtable, SIGNAL(showSavedManualPlots(TableManagement *, LittleWidgetsView *, bool)), this, SLOT(showManualTableFromDB(TableManagement *, LittleWidgetsView *, bool)));
				minfo_mtable -> setObjectName("minfotable");
				minfo_mtable -> initManageTree(loging_user, back_science, base_db, 1, leftw_container, qobject_cast<EditLongToolsViewer *>(showing_toolBar));
				qobject_cast<EditLongToolsViewer *>(showing_toolBar) -> setRelatedTreePt(1, minfo_mtable);
			}
			else
				minfo_mtable = qobject_cast<TableManagement *>(rightw_container->qmlAgent()->widget());
			embededOrSlideWidgetOnRightQml(minfo_mtable);
			minfo_mtable -> setInitViewRect(rightw_container->initRect());
		}
		else
			slideWidgetView(leftw_container, rightw_container, 1);
	}
	else if (dest==tr("图数据源"))
	{
		if (!s_h)
		{
			int plots_type = 0;
			if (showing_toolBar->objectName() == "mainplotsBar3")
				plots_type = 3;
			else if (showing_toolBar->objectName() == "mainplotsBar4")
				plots_type = 4;
			else if (showing_toolBar->objectName() == "mainplotsBar5")
				plots_type = 5;
			else 
				return;
			TableManagement * plots_mtable = 0;
			if (!rightw_container)
			{
				plots_mtable = new TableManagement;
				connect(plots_mtable, SIGNAL(showSavedManualPlots(TableManagement *, LittleWidgetsView *, bool)), this, SLOT(showManualTableFromDB(TableManagement *, LittleWidgetsView *, bool)));
				plots_mtable -> setObjectName("plotsmtable");
				plots_mtable -> initManageTree(loging_user, back_science, base_db, plots_type, leftw_container, qobject_cast<EditLongToolsViewer *>(showing_toolBar));
				qobject_cast<EditLongToolsViewer *>(showing_toolBar) -> setRelatedTreePt(plots_type, plots_mtable);
			}
			else
				plots_mtable = qobject_cast<TableManagement *>(rightw_container->qmlAgent()->widget());
			embededOrSlideWidgetOnRightQml(plots_mtable);
			plots_mtable -> setInitViewRect(rightw_container->initRect());			
		}
		else
			slideWidgetView(leftw_container, rightw_container, 1);
	}
	else if (dest==tr("综合数据") || dest==tr("综合图表"))
	{
		if (!s_h)
		{
			int t_tree = 0;
			if (dest==tr("综合数据"))
				t_tree = 6;
			else	
				t_tree = 7;
			TableManagement * total_mtable = 0;		
			if (!rightw_container)
			{
				total_mtable = new TableManagement;
				connect(total_mtable, SIGNAL(showSavedManualPlots(TableManagement *, LittleWidgetsView *, bool)), this, SLOT(showManualTableFromDB(TableManagement *, LittleWidgetsView *, bool)));
				total_mtable -> setObjectName("totalmtable");
				total_mtable -> initManageTree(loging_user, back_science, base_db, t_tree, leftw_container, qobject_cast<EditLongToolsViewer *>(showing_toolBar));
				qobject_cast<EditLongToolsViewer *>(showing_toolBar) -> setRelatedTreePt(t_tree, total_mtable);			
			}
			else
				total_mtable = qobject_cast<TableManagement *>(rightw_container->qmlAgent()->widget());
			embededOrSlideWidgetOnRightQml(total_mtable);
			total_mtable -> setInitViewRect(rightw_container->initRect());
		}
		else
			slideWidgetView(leftw_container, rightw_container, 1);
	}
}

void MainWindow::intervalTimer(int ellap_time)
{
	QString color1 = "background-color: #AEEEEE";
	QString color2 = "background-color: #FF3030";
	if (loging_user.first.isEmpty())
	{
		if (ellap_time/300%2 == 0)
			emit changeNameColor(color1);
		else if (ellap_time/300%2 == 1)
			emit changeNameColor(color2);
	}
	if (loging_user.second.isEmpty())	
	{
		if (ellap_time/300%2 == 0)
			emit changePasswdColor(color1);
		else if (ellap_time/300%2 == 1)
			emit changePasswdColor(color2);
	}	
}

void MainWindow::warningTimeOut()
{
	QString color3 = "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(102, 137, 175, 255), stop:0.595591 rgba(146, 172, 401, 255), stop:1 rgba(406, 219, 255, 255))";
	if (loging_user.first.isEmpty())
		emit changeNameColor(color3);
	if (loging_user.second.isEmpty())
		emit changePasswdColor(color3);
}

void MainWindow::stopQmlEditorFunc(QString detail)
{
	if (detail == tr("返回"))
	{
		LittleWidgetsView * old_left = leftw_container;
		LittleWidgetsView * old_right = rightw_container;		
		LittleWidgetsView * g_leftw = new LittleWidgetsView(this);
		g_leftw -> setObjectName("l_container");
		g_leftw -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");
		QRect o_rect(0, 0 , width(), height());
		g_leftw -> initMarginForParent(o_rect);
		PlotWidget * plot_new = new PlotWidget(back_science, base_db);
		plot_new -> setObjectName("plot");
		plot_new -> setFixedWidth(width());
		plot_new -> setFixedHeight(height());
		g_leftw -> setViewForRelatedTable(plot_new);
		plot_new -> setEnginame();
		QWidget * tmp_bar = showing_toolBar;
		clearRalatedEdittingBarOnHash(tmp_bar);			
		slideMultiWidgets(g_leftw, 0, old_left, old_right, true, true);
		QString theme = tr("工程");
		emit newTheme(theme);
		returnLastMenu();
		syncCurrentWinWithHash(leftw_container, 0, loging_user);		
	}
	else if (detail == tr("隐藏"))
		hideToolPressed();
	else
		cancelToolPressed();
}

void MainWindow::dataesFailedSetIndb()
{}

void MainWindow::setWinsMovingFinished()
{
	wins_moving = false;
}

void MainWindow::prepairForDBsaveWork(bool save_del)
{
	if (!rightw_container)
		return;
	TableManagement * g_table = qobject_cast<TableManagement *>(rightw_container->qmlAgent()->widget());
	if (save_del)
	{
		QString judge_state;
		if (!g_table->considerSaveNewOrOldManuals(judge_state))
		{
			if (judge_state == tr("保存未成功"))
			{
				//warn dialog
				return;
			}
		}
		else
			return;	
		DiaLog * save_dialog = new DiaLog;
		connect(save_dialog, SIGNAL(sendSaveFileName(const QString &)), this, SLOT(receiveSaveFileName(const QString &)));
		save_dialog -> initSaveStrDefineDialog(this, base_db, loging_user.second);
		if (save_dialog->exec())
		{
			if (g_table->treeType()>2 && g_table->treeType()<6)
			{
				if (!base_db -> databaseSavePlots(loging_user.second, save_name, g_table, leftw_container))
				{
					// warning dialog
				}			  
			}
			else
			{
				if (!base_db -> saveManualTable(loging_user.second, save_name, g_table, leftw_container))
				{
					// warning dialog
				}
			}
		}
		save_name.clear();
	}
	else
	{
		QString del_name;
		g_table -> feedbackForManualsName(del_name);
		if (!del_name.isEmpty())
		{
			g_table -> actionForOtherDelManualWork(del_name);
			if (!base_db->delTable(del_name))
			{
				// warning dialog
				return;
			}			
		}
	}
}

void MainWindow::receiveSaveFileName(const QString & r_name)
{
	save_name = r_name;
}

void MainWindow::showManualTableFromDB(TableManagement * sending, LittleWidgetsView * trans_viewer, bool to_direct)
{
	if (trans_viewer == leftw_container)
	{
		trans_viewer = sending->currentPartner();
		slideLeftQmlWidgets(leftw_container, trans_viewer, rightw_container, to_direct, true);
	}
	else
		slideLeftQmlWidgets(leftw_container, trans_viewer, rightw_container, to_direct);		
	if (sending->treeType() > 5 && sending->treeType() < 2)
		qobject_cast<EditLongToolsViewer *>(showing_toolBar) -> setEditingTable(qobject_cast<TablesGroup *>(trans_viewer->qmlAgent()->widget()));
	else
		showing_toolBar -> raise();
	leftw_container = trans_viewer;
}

void MainWindow::initDbAndSpcCalculation(const QString & db_file)
{
	if (!base_db->dbConnection(db_file))
	{
		QString error_info = "error:"+tr("致命错误！\n数据库不能打开，请重启");
		DiaLog * w_dialog = new DiaLog;
		w_dialog -> initWinDialog(this, error_info, 1);//need add system close in dialog
		delete  back_science;
		back_science = 0;
		base_db = 0;	
		return;
	}
	new_login.first.clear();
	new_login.second.clear();
	sys_userState = 0;
	back_science -> setDatabase(base_db);
	connect(back_science, SIGNAL(showCalResTable(QString &, const QList<double> &)), this, SLOT(showCurSpcCalRes(QString &, const QList<double> &)));
}

void MainWindow::clearCpkProcessWidgets()
{
    	QPair<QString, QString> name_pair; 
 	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user;  
	foreach (QWidget * w_close, ws_cpktests.value(name_pair))
	{
		if (ws_cpktests.value(name_pair).indexOf(w_close)==0 || ws_cpktests.value(name_pair).indexOf(w_close)==1 || ws_cpktests.value(name_pair).indexOf(w_close) == 2)
			continue;		  
		if (w_close)
			w_close -> close();
	}
	ws_cpktests.remove(name_pair);	
}

void MainWindow::finishProcessProtection()
{
	protect_mode = false;
}

void MainWindow::toggleCurrentBarButton(const QString & b_lbl, const QString & toggle_hint, bool state, QWidget * w_bar)
{
	QLayout * menu_layout = 0;
	if (w_bar)
		menu_layout = w_bar->layout();	  
	else  
		menu_layout = showing_toolBar->layout();
	int tools = menu_layout->count();
	for (int i = 0; i < tools; i++)
	{
		QToolButton * e_button = qobject_cast<QToolButton *>(menu_layout->itemAt(i)->widget());
		if (e_button->text() == b_lbl)
		{
			if (toggle_hint == tr("切换"))
			{
				if (e_button->isDown()!=state)
				{
					e_button -> setChecked(state);
					e_button -> setDown(state);
				}
			}
			else
			{
				if (e_button->isEnabled()!=state)
					e_button -> setEnabled(state);
			}
			break;
		}
	}
}

void MainWindow::promotionEdittingOrderTrans(QUndoStack * comm)
{
	SlideBar * current_bar = qobject_cast<SlideBar *>(showing_toolBar);	
	if (current_bar->currentCommand() != comm)
		current_bar -> setCommandStack(comm);
}

void MainWindow::showDigitalTime()
{
	QTime time = QTime::currentTime();
	QString text = time.toString("hh:mm");
	if ((time.second() % 2) == 0)
		text[2] = ' ';
	digital_clock -> display(text);	  
}

void MainWindow::initEnvireForLogedUser()
{
	if (protect_mode)	
		stopProtectionMode(false);
	if (new_login != loging_user)
		transWidgetForLoging(true);
}
	
void MainWindow::initEnvireForLogingUser()
{
	if (protect_mode)
		stopProtectionMode(true);
	transWidgetForLoging(false);
}

void MainWindow::transWidgetForLoging(bool stored_state)
{
	LittleWidgetsView * old_left = leftw_container;
	LittleWidgetsView * old_right = rightw_container;
	if (stored_state)
	{
		QPair<QWidget *, QWidget *> s_wp = usrOwnedShowingWidgets(new_login);
		LittleWidgetsView * g_leftw = qobject_cast<LittleWidgetsView *>(s_wp.first);
		LittleWidgetsView * g_rightw = qobject_cast<LittleWidgetsView *>(s_wp.second);
		slideMultiWidgets(g_leftw, g_rightw, old_left, old_right, false, true);
		if (leftw_container->qmlAgent()->widget()->objectName() == "plot")
			emit newTheme(qobject_cast<PlotWidget *>(leftw_container->qmlAgent()->widget())->currentEngi()); 
	}
	else
	{
		PlotWidget * plot_new = 0;
		rightw_container = 0;
		if (!loging_pairs.isEmpty())
		{	
			LittleWidgetsView * g_leftw = new LittleWidgetsView(this);
			g_leftw -> setObjectName("l_container");
			g_leftw -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");
			QRect o_rect(0, 0 , width(), height());
			g_leftw -> initMarginForParent(o_rect);
			plot_new = new PlotWidget(back_science, base_db);
			plot_new -> setObjectName("plot");
			plot_new -> setFixedWidth(width());
			plot_new -> setFixedHeight(height());			
			g_leftw -> setViewForRelatedTable(plot_new);
			plot_new -> setEnginame();
			slideMultiWidgets(g_leftw, 0, old_left, old_right, false, true);	
		}
		if (plot_new)
			emit newTheme(plot_new->currentEngi());
	}
	loging_user = new_login;
	if (!syncCurrentWinWithHash(leftw_container, rightw_container, loging_user))
		syncNewWinWithHash(loging_user, leftw_container, rightw_container);	
	if (leftw_container->qmlAgent()->widget()->objectName() != "plot")
		resetToolBarsVarsValue(storedBarOnHash(leftw_container, rightw_container), true);
	else
		resetToolBarsVarsValue(storedBarOnHash(leftw_container, rightw_container), true);
	if (!loging_pairs.contains(new_login))
		loging_pairs << new_login;
}

void MainWindow::createLoginToolbar(QWidget * father, const QString & login)
{
	SlideBar * new_bar = new SlideBar(this);	
	new_bar -> setObjectName("logintoolBar");
	new_bar -> showLoginTools(login);
	new_bar -> show();
	animateToolBarLeftRight(father, new_bar);	
}

void MainWindow::createOkCancelBars(QWidget * last, const QString & hint, QUndoStack * with_comm)
{
	SlideBar * new_toolBar = new SlideBar(this);
	QToolButton * continueLabel = new QToolButton(new_toolBar);
	continueLabel -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	continueLabel -> setFont(QFont(tr("宋体"), 8));
	continueLabel -> setIconSize(QSize(40, 40));
	continueLabel -> setFixedHeight(64);
	continueLabel -> setFixedWidth(55);        
	continueLabel -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE;}");
	connect(continueLabel, SIGNAL(clicked()), this, SLOT(okToolPressed()));
	bool toggle_enabled = false;
	if (!hint.isEmpty())
	{	
		if (hint == "dbsediting")
		{       
			continueLabel -> setIcon(QIcon(":/images/dbsource.png"));
			continueLabel -> setText(tr("确定更改"));
			new_toolBar -> setObjectName(tr("okcancel_数据库编辑"));
			new_toolBar -> setCommandStack(with_comm);
		}		
		else if (hint == "initEngi")
		{       
			continueLabel -> setIcon(QIcon(":/images/cpkstatics.png"));
			continueLabel -> setText(tr("编辑样本"));
			new_toolBar -> setObjectName(tr("okcancel_CPK数据"));
			new_toolBar -> setCommandStack(with_comm);
			toggle_enabled = true;
		}
		else if (hint == "waitpower")
		{       
			continueLabel -> setIcon(QIcon(":/images/join.png"));
			continueLabel -> setText(tr("加入工程"));
			new_toolBar -> setObjectName(tr("okcancel_加入工程"));
			new_toolBar -> setCommandStack(with_comm);	
		}
		else if (hint == "empower")
		{        
			continueLabel -> setIcon(QIcon(":/images/empower.png"));
			continueLabel -> setText(tr("授权用户"));
			new_toolBar -> setObjectName(tr("okcancel_授权用户"));
			new_toolBar -> setCommandStack(with_comm);		
		}
		else if (hint == tr("在线工程") || hint == "plot")
		{       
			continueLabel -> setIcon(QIcon(":/images/input.png"));
			continueLabel -> setText(tr("输入"));
			new_toolBar -> setObjectName(tr("okcancel_输入"));	
		}
		else if (hint == "power" || hint == "products" || hint == "propertys")
		{ 
			if (hint == "power")
			{       
				continueLabel -> setIcon(QIcon(":/images/authority.png"));
				continueLabel -> setText(tr("确定权限"));
				new_toolBar -> setObjectName(tr("okcancel_权限"));				
			}
			else if (hint == "products")
			{
				continueLabel -> setIcon(QIcon(":/images/product.png"));
				continueLabel -> setText(tr("确定产品"));
				new_toolBar -> setObjectName(tr("okcancel_产品"));
			}
			else
			{
				continueLabel -> setIcon(QIcon(":/images/attribution.png"));
				continueLabel -> setText(tr("确定属性"));
				new_toolBar -> setObjectName(tr("okcancel_属性数据"));
			}
			new_toolBar -> setCommandStack(with_comm);		
		}
		else if (hint == "projects")
		{       
			continueLabel -> setIcon(QIcon(":/images/keydata.png"));
			continueLabel -> setText(tr("关键数据"));
			new_toolBar -> setObjectName(tr("okcancel_关键数据"));	
		}
		else if (hint == "bkdb")
		{
			continueLabel -> setIcon(QIcon(":/images/backup.png"));
			continueLabel -> setText(tr("确定备份"));
			new_toolBar -> setObjectName(tr("okcancel_备份数据"));
			new_toolBar -> setCommandStack(with_comm);
		}
	}
	else
	{
		QString last_name = last->objectName();
		if (last_name == tr("okcancel_CPK数据"))// not in odg
		{    
			continueLabel -> setIcon(QIcon(":/images/bydots.png"));
			continueLabel -> setText(tr("进入点图"));
			new_toolBar -> setObjectName(tr("okcancel_新建1"));
			new_toolBar -> setCommandStack(qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget())->commandStack());			
			toggle_enabled = true;			
		}
		else if (last_name == tr("okcancel_权限")/*??*/ || last_name == tr("okcancel_产品")/*??*/)
		{        
			continueLabel -> setIcon(QIcon(":/images/modify.png"));
			continueLabel -> setText(tr("数据编辑"));
			if (last_name.contains(tr("权限")))
				new_toolBar -> setObjectName(tr("okcancel_权限数据编辑"));// not in odg
			else
				new_toolBar -> setObjectName(tr("okcancel_产品数据编辑"));// not in odg
			new_toolBar -> setCommandStack(with_comm);			
		}
	}
	new_toolBar -> showOkCancelBar(continueLabel);
    	QPair<QString, QString> name_pair; 
 	if (loging_user.second.isEmpty())
		name_pair = new_login;
	else
		name_pair = loging_user;	
	if (!ws_cpktests.value(name_pair).isEmpty() && ws_cpktests.value(name_pair).at(0)/*for newfile() func*/)
		animateToolBarWidgetUpDown(last, 0, new_toolBar, 0);
	else
		animateToolBarLeftRight(last, new_toolBar);
	if (toggle_enabled)
	{			
		QString delbtn;
		if (hint == "initEngi")
		{
			delbtn = tr("删除新建");	
			toggleCurrentBarButton(tr("粘贴样本"), tr("使能"), false);		  			
		}
		else
			delbtn = tr("删除样本");			
		toggleCurrentBarButton(tr("保存当前"), tr("使能"), false);		
		toggleCurrentBarButton(delbtn, tr("使能"), false);		
	}	
	new_toolBar -> show();	
}

void MainWindow::animateToolBarLeftRight(QWidget * parent, QWidget * child, bool close_hide)
{
	if (parent==0 && child==0)
		return;
	int btnRight = geometry().right();
	s_group = new QParallelAnimationGroup(this);
	connect(s_group, SIGNAL(finished()), s_group, SLOT(deleteLater()));
	connect(s_group, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));
	if (parent)
	{
		p_animation = new QPropertyAnimation(parent, "geometry", this);
		connect(p_animation, SIGNAL(finished()), p_animation, SLOT(deleteLater()));
		connect(p_animation, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));
		p_animation -> setDuration(400);
		QRect p_startRect(QRect(btnRight-parent->sizeHint().width(), 0, parent->sizeHint().width(), 64));
		QRect p_endRect(QRect(btnRight, 0, 0, 64));
		p_animation -> setStartValue(p_startRect);
		p_animation -> setEndValue(p_endRect);
		if (close_hide)
		{
			connect(p_animation, SIGNAL(finished()), parent, SLOT(close()));
			clearRalatedEdittingBarOnHash(parent);
			if (parent==showing_toolBar)
				showing_toolBar = 0;
		}
		s_group -> addAnimation(p_animation);		
	}
	if (child)
	{
		c_animation = new QPropertyAnimation(child, "geometry", this);
		connect(c_animation, SIGNAL(finished()), c_animation, SLOT(deleteLater()));
		connect(c_animation, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));
		c_animation -> setDuration(400);
		QRect c_startRect(QRect(btnRight, 0, 0, 64));
		QRect c_endRect(QRect(btnRight-child->sizeHint().width(), 0, child->sizeHint().width(), 64));
		c_animation -> setStartValue(c_startRect);
		c_animation -> setEndValue(c_endRect);
		s_group -> addAnimation(c_animation);
		showing_toolBar = child;		
	}
	s_group -> start();
}

void MainWindow::animateToolBarWidgetUpDown(QWidget * old_bar, QWidget * left_widget, QWidget * new_bar, QWidget * right_widget, bool hide_close)//need promotion for menu height
{
	if (old_bar == new_bar)
		return;
	QRect p_startRect;
	QRect p_endRect;
	QRect c_startRect;
	QRect c_endRect;
	QRect l_startRect1;
	QRect l_endRect1;
	QRect l_startRect2;
	QRect l_endRect2;
	QRect r_startRect1;
	QRect r_endRect1;
	QRect r_startRect2;
	QRect r_endRect2;
	QWidget * nested_widget = 0;
	if (left_widget)
		nested_widget = qobject_cast<LittleWidgetsView *>(left_widget)->qmlAgent()->widget();
	if (old_bar)
	{
		v1_group = new QParallelAnimationGroup(this);
		p_animation = new QPropertyAnimation(old_bar, "geometry", this);
		int btnRight = geometry().right();
		p_animation -> setDuration(260);
		p_startRect.setRect(btnRight-old_bar->sizeHint().width(), 0, old_bar->sizeHint().width(), 64);
		p_endRect.setRect(btnRight-old_bar->sizeHint().width(), 0, old_bar->sizeHint().width(), 0);
		p_animation -> setStartValue(p_startRect);
		p_animation -> setEndValue(p_endRect);
		v1_group -> addAnimation(p_animation);
		connect(p_animation, SIGNAL(finished()), p_animation, SLOT(deleteLater()));
		connect(p_animation, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));			
		if (nested_widget && left_widget->geometry().top() > 0)
		{
			l_animation1 = new QPropertyAnimation(left_widget, "geometry", this);
			connect(l_animation1, SIGNAL(valueChanged(const QVariant &)), leftw_container, SLOT(resetViewWidth(const QVariant &)));
			connect(l_animation1, SIGNAL(finished()), l_animation1, SLOT(deleteLater()));
			connect(l_animation1, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));
			l_animation1 -> setDuration(260);
			l_startRect1.setRect(left_widget->geometry().topLeft().x(), left_widget->geometry().top(), left_widget->geometry().width(), height()-left_widget->geometry().top());
			l_endRect1.setRect(left_widget->geometry().topLeft().x(), 0, left_widget->geometry().width(), height());
			l_animation1 -> setStartValue(l_startRect1);
			l_animation1 -> setEndValue(l_endRect1);
			v1_group -> addAnimation(l_animation1);
		}
		if (right_widget && right_widget->geometry().top()==64)
		{
			r_animation1 = new QPropertyAnimation(right_widget, "geometry", this);
			connect(r_animation1, SIGNAL(finished()), r_animation1, SLOT(deleteLater()));
			connect(r_animation1, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));
			r_animation1 -> setDuration(260);
			r_startRect1.setRect(right_widget->geometry().topLeft().x(), 64, right_widget->geometry().width(), height()-64);
			r_endRect1.setRect(right_widget->geometry().topLeft().x(), 0, right_widget->geometry().width(), height());
			r_animation1 -> setStartValue(r_startRect1);
			r_animation1 -> setEndValue(r_endRect1);
			v1_group -> addAnimation(r_animation1);
		}
		QPair<QString, QString> name_pair; 
		if (loging_user.second.isEmpty())
			name_pair = new_login;
		else
			name_pair = loging_user;		
		if (!hide_close)
		{
			connect(p_animation, SIGNAL(finished()), old_bar, SLOT(close()));
			clearRalatedEdittingBarOnHash(old_bar);
			if (showing_toolBar==old_bar && ((!ws_cpktests.value(name_pair).isEmpty() && ws_cpktests.value(name_pair).at(0) && old_bar!=ws_cpktests.value(name_pair).at(0)) || ws_cpktests.value(name_pair).isEmpty()))
				showing_toolBar = 0;
		}
		else if (!ws_cpktests.value(name_pair).isEmpty() && new_bar)
		{
			if (old_bar != ws_cpktests.value(name_pair).at(0) && (old_bar->objectName()==tr("okcancel_CPK数据") || old_bar->objectName()==tr("okcancel_新建1")))
			{
				if (!stored_bars.isEmpty())
					storeRalatedEdittingBar(leftw_container, rightw_container, ws_cpktests.value(name_pair).at(0));					
				ws_cpktests[name_pair][0] = old_bar;
			}
		}
		connect(v1_group, SIGNAL(finished()), v1_group, SLOT(deleteLater()));
		connect(v1_group, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));
	}	
	if (new_bar)
	{		
		v2_group = new QParallelAnimationGroup(this);
		c_animation = new QPropertyAnimation(new_bar, "geometry", this);
		connect(c_animation, SIGNAL(finished()), c_animation, SLOT(deleteLater()));
		connect(c_animation, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));
		c_animation -> setDuration(260);
		c_startRect.setRect(width()-new_bar->sizeHint().width(), -64, new_bar->sizeHint().width(), 64);
		c_endRect.setRect(width()-new_bar->sizeHint().width(), 0, new_bar->sizeHint().width(), 64);
		c_animation -> setStartValue(c_startRect);
		c_animation -> setEndValue(c_endRect);
		v2_group -> addAnimation(c_animation);
		if (nested_widget && nested_widget->objectName()!="plot" && !nested_widget->objectName().contains("backuptree"))
		{
			l_animation2 = new QPropertyAnimation(left_widget, "geometry", this);
			connect(l_animation2, SIGNAL(valueChanged(const QVariant &)), leftw_container, SLOT(resetViewWidth(const QVariant &)));
			connect(l_animation2, SIGNAL(finished()), l_animation2, SLOT(deleteLater()));
			connect(l_animation2, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));
			l_animation2 -> setDuration(260);
			l_startRect2.setRect(left_widget->geometry().topLeft().x(), left_widget->geometry().top(), left_widget->geometry().width(), height());
			l_endRect2.setRect(left_widget->geometry().topLeft().x(), 64, left_widget->geometry().width(), height()-64);
			l_animation2 -> setStartValue(l_startRect2);
			l_animation2 -> setEndValue(l_endRect2);
			v2_group -> addAnimation(l_animation2);
		}
		if (right_widget)
		{
			r_animation2 = new QPropertyAnimation(right_widget, "geometry", this);
			connect(r_animation2, SIGNAL(finished()), r_animation2, SLOT(deleteLater()));
			connect(r_animation2, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));
			r_animation2 -> setDuration(260);
			r_startRect2.setRect(right_widget->geometry().topLeft().x(), 0, right_widget->geometry().width(), height());
			r_endRect2.setRect(right_widget->geometry().topLeft().x(), 64, right_widget->geometry().width(), height()-64);
			r_animation2 -> setStartValue(r_startRect2);
			r_animation2 -> setEndValue(r_endRect2);
			v2_group -> addAnimation(r_animation2);			
		}
		connect(v2_group, SIGNAL(finished()), v2_group, SLOT(deleteLater()));
		connect(v2_group, SIGNAL(destroyed(QObject *)), this, SLOT(clearBarsAnimation(QObject *)));
		showing_toolBar = new_bar;	
	}
	if (v1_group && v2_group)
	{
		v1_group -> start();
		connect(v1_group, SIGNAL(finished()), v2_group, SLOT(start()));
	}
	else
	{
		if (v1_group)
			v1_group -> start();
		if (v2_group)
			v2_group -> start();
	}		
}

void MainWindow::slideWidgetView(QWidget * leftW, QWidget * rightW, int show_type)
{
	if (!rightW || wins_moving)
		return;	
	if (show_type==0 && rightW->geometry().topLeft().x()>=width())
	{
		rightw_container -> deleteLater();
		rightw_container = 0;	
		if (!loging_pairs.isEmpty())		
			syncCurrentWinWithHash(leftW, 0, loging_user);
		return;		
	}
	l_animation3 = new QPropertyAnimation(leftW, "geometry", this);
	connect(l_animation3, SIGNAL(valueChanged(const QVariant &)), leftw_container, SLOT(resetViewWidth(const QVariant &)));
	connect(l_animation3, SIGNAL(finished()), l_animation3, SLOT(deleteLater()));
	connect(l_animation3, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	r_animation3 = new QPropertyAnimation(rightW, "geometry", this);
	connect(r_animation3, SIGNAL(finished()), r_animation3, SLOT(deleteLater()));
	connect(r_animation3, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	v3_group = new QParallelAnimationGroup(this);
	connect(v3_group, SIGNAL(finished()), v3_group, SLOT(deleteLater()));
	connect(v3_group, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	QRect l_startRect;
	QRect l_endRect;
	QRect r_startRect;
	QRect r_endRect;
	int w_reshow = rightW->sizeHint().width();
	LittleWidgetsView * r_view = qobject_cast<LittleWidgetsView *>(rightW);
	DataesTableView * d_view = qobject_cast<DataesTableView *>(r_view->qmlAgent()->widget());
	EngiTreeView * e_view = qobject_cast<EngiTreeView *>(r_view->qmlAgent()->widget());
	RelatedTreeView * rt_view = qobject_cast<RelatedTreeView *>(r_view->qmlAgent()->widget());
	TableManagement * t_view = qobject_cast<TableManagement *>(r_view->qmlAgent()->widget());
	if (d_view && d_view->originShowRect().isValid())
		w_reshow = d_view->originShowRect().width();	
	else if (e_view && e_view->originShowRect().isValid())
		w_reshow = e_view->originShowRect().width();
	else if (rt_view && rt_view->originShowRect().isValid())
		w_reshow = rt_view->originShowRect().width();  
	else if (t_view && t_view->originShowRect().isValid())
		w_reshow = t_view->originShowRect().width();	
	l_animation3->setDuration(400);
	r_animation3->setDuration(400);
	if (show_type == 2)
	{
		connect(r_animation3, SIGNAL(finished()), r_view, SLOT(resizeAgentSize()));	  
		l_startRect.setRect(0, leftW->geometry().top(), width(), height()-leftW->geometry().top());
		l_endRect.setRect(0, leftW->geometry().top(), width()-w_reshow, height()-leftW->geometry().top());
		r_startRect.setRect(width(), 64, 0, height()-64);
		r_endRect.setRect(width()-w_reshow, 64, w_reshow, height()-64);
		if (!loging_pairs.isEmpty() && leftW->objectName()!="helppages" && !syncCurrentWinWithHash(leftW, rightW, loging_user))
			syncNewWinWithHash(loging_user, leftW, rightW);
		if (!loging_pairs.isEmpty() && leftW->objectName()!="helppages")
			storeRalatedEdittingBar(leftW, rightW, showing_toolBar);
	}
	else if (show_type == 1 || show_type == 0)
	{	  
		l_startRect.setRect(0, leftW->geometry().top(), width()-w_reshow, height()-leftW->geometry().top());
		l_endRect.setRect(0, leftW->geometry().top(), width(), height()-leftW->geometry().top());
		r_startRect.setRect(width()-w_reshow, 64, w_reshow, height()-64);
		r_endRect.setRect(width(), 64, 0, height()-64);
		QPair<QString, QString> name_pair; 
		if (loging_user.second.isEmpty())
			name_pair = new_login;
		else
			name_pair = loging_user;			
		if (show_type == 0)
		{
			if (!loging_pairs.isEmpty())
				syncCurrentWinWithHash(leftW, 0, loging_user);
			connect(r_animation3, SIGNAL(finished()), rightW, SLOT(close()));
			if (!ws_cpktests.value(name_pair).isEmpty() && (ws_cpktests.value(name_pair).at(0) || ws_cpktests.value(name_pair).at(2)))
				connect(v3_group, SIGNAL(finished()), this, SLOT(clearCpkProcessWidgets()));			
			rightw_container = 0;			
		}
		else
		{
			if (!loging_pairs.isEmpty() && leftW->objectName()!="helppages" && !syncCurrentWinWithHash(leftW, rightW, loging_user))
				syncNewWinWithHash(loging_user, leftW, rightW);
			if (!loging_pairs.isEmpty() && leftW->objectName()!="helppages")
				storeRalatedEdittingBar(leftW, rightW, showing_toolBar);			
		}	
	}
	l_animation3 -> setStartValue(l_startRect);
	l_animation3 -> setEndValue(l_endRect);
	r_animation3 -> setStartValue(r_startRect);
	r_animation3 -> setEndValue(r_endRect);
	v3_group -> addAnimation(l_animation3);
	v3_group -> addAnimation(r_animation3);
	wins_moving = true;
	connect(v3_group, SIGNAL(finished()), this, SLOT(setWinsMovingFinished()));	
	v3_group -> start();
}

void MainWindow::slideWholeWidgets(QWidget * old_leftW, QWidget * new_leftW, bool del_not)
{
	if (wins_moving)
		return;
	l_animation3 = new QPropertyAnimation(this);
	l_animation4 = new QPropertyAnimation(this);
	connect(l_animation4, SIGNAL(finished()), l_animation4, SLOT(deleteLater()));
	connect(l_animation4, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	v3_group = new QParallelAnimationGroup(this);
	connect(v3_group, SIGNAL(finished()), v3_group, SLOT(deleteLater()));
	connect(v3_group, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	if (old_leftW)
	{
		l_animation3 -> setTargetObject(old_leftW);
		l_animation3 -> setPropertyName("geometry");
		l_animation3 -> setDuration(500);
		QRect l_startRect1(0, 0, width(), height());
		QRect l_endRect1(width(), 0, width(), height());
		l_animation3 -> setStartValue(l_startRect1);
		l_animation3 -> setEndValue(l_endRect1);
		v3_group -> addAnimation(l_animation3);
		if (del_not)	
		{
			syncCurrentWinWithHash(new_leftW, 0, loging_user);
			connect(l_animation3, SIGNAL(finished()), old_leftW, SLOT(close()));
		}
	}
	connect(l_animation3, SIGNAL(finished()), l_animation3, SLOT(deleteLater()));
	connect(l_animation3, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	if (new_leftW)
	{
		l_animation4 -> setTargetObject(new_leftW);
		l_animation4 -> setPropertyName("geometry");
		l_animation4 -> setDuration(500);
		QRect l_startRect2;
		QRect l_endRect2;
		QWidget * new_nested = qobject_cast<LittleWidgetsView *>(new_leftW)->qmlAgent()->widget();
		if (new_nested->objectName() != "plot")
		{
			l_startRect2.setRect(-width(), 64, width(), height()-64);
			l_endRect2.setRect(0, 64, width(), height()-64);
		}
		else
		{
			l_startRect2.setRect(-width(), 0, width(), height());
			l_endRect2.setRect(0, 0, width(), height());			
		}
		syncCurrentWinWithHash(new_leftW, 0, loging_user);
		storeRalatedEdittingBar(new_leftW, 0, showing_toolBar);
		l_animation4 -> setStartValue(l_startRect2);
		l_animation4 -> setEndValue(l_endRect2);
		v3_group -> addAnimation(l_animation4);
	}
	wins_moving = true;
	connect(v3_group, SIGNAL(finished()), this, SLOT(setWinsMovingFinished()));
	v3_group -> start();	
}

void MainWindow::slideLeftQmlWidgets(QWidget * leftW1, QWidget * leftW2, QWidget * rightW, bool direction, bool del)
{
	if (wins_moving)
		return;
	rightW -> raise();
	l_animation3 = new QPropertyAnimation(leftW1, "geometry", this);
	connect(l_animation3, SIGNAL(finished()), l_animation3, SLOT(deleteLater()));
	connect(l_animation3, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	LittleWidgetsView * sync_view = qobject_cast<LittleWidgetsView *>(leftW2);
	l_animation4 = new QPropertyAnimation(leftW2, "geometry", this);
	connect(l_animation4, SIGNAL(finished()), l_animation4, SLOT(deleteLater()));
	connect(l_animation4, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	connect(l_animation4, SIGNAL(valueChanged(const QVariant &)), sync_view, SLOT(resetViewWidth(const QVariant &)));
	QRect l_startRect1(0, leftW1->geometry().top(), width()-rightW->width(), height()-leftW1->geometry().top());
	QRect l_endRect1;
	QRect l_startRect2;
	QRect l_endRect2;	
	if (direction)
	{
		l_endRect1.setRect(rightW->width()-width(), leftW1->geometry().top(), width()-rightW->width(), height()-leftW1->geometry().top());
		l_startRect2.setRect(width()-rightW->width(), leftW1->geometry().top(), width()-rightW->width(), height()-leftW1->geometry().top());
	}
	else
	{
		l_endRect1.setRect(width(), leftW1->geometry().top(), width()-rightW->width(), height()-leftW1->geometry().top());
		l_startRect2.setRect(rightW->width()-width(), leftW1->geometry().top(), width()-rightW->width(), height()-leftW1->geometry().top());
	}
	l_endRect2.setRect(0, leftW1->geometry().top(), width()-rightW->width(), height()-leftW1->geometry().top());
	l_animation3 -> setStartValue(l_startRect1);
	l_animation3 -> setEndValue(l_endRect1);
	l_animation3 -> setDuration(400);
	l_animation4 -> setStartValue(l_startRect2);
	l_animation4 -> setEndValue(l_endRect2);
	l_animation4 -> setDuration(400);	
	v3_group = new QParallelAnimationGroup(this);
	connect(v3_group, SIGNAL(finished()), v3_group, SLOT(deleteLater()));
	connect(v3_group, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	v3_group -> addAnimation(l_animation3);
	v3_group -> addAnimation(l_animation4);
	wins_moving = true;
	syncCurrentWinWithHash(leftW2, rightW, loging_user);
	storeRalatedEdittingBar(leftW2, rightW, showing_toolBar);
	connect(v3_group, SIGNAL(finished()), this, SLOT(setWinsMovingFinished()));
	if (del)	
		connect(v3_group, SIGNAL(finished()), leftW1, SLOT(deleteLater()));
	v3_group -> start();	
}

void MainWindow::slideRightWidgets(QWidget * leftW, QWidget * rightW1, QWidget * rightW2, bool del)
{
	if (rightW1 == rightW2 || wins_moving)
		return;
	l_animation3 = new QPropertyAnimation(leftW, "geometry", this);
	connect(l_animation3, SIGNAL(valueChanged(const QVariant &)), leftw_container, SLOT(resetViewWidth(const QVariant &)));	
	connect(l_animation3, SIGNAL(finished()), l_animation3, SLOT(deleteLater()));
	connect(l_animation3, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	int w_reshow = rightW2->sizeHint().width();
	LittleWidgetsView * r_view = qobject_cast<LittleWidgetsView *>(rightW2);
	DataesTableView * d_view = qobject_cast<DataesTableView *>(r_view->qmlAgent()->widget());
	if (d_view && d_view->originShowRect().isValid())
		w_reshow = d_view->originShowRect().width();		
	r_animation3 = new QPropertyAnimation(rightW2, "geometry", this);
	connect(r_animation3, SIGNAL(finished()), r_animation3, SLOT(deleteLater()));
	connect(r_animation3, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));	
	QRect l_startRect1(0, 0, width()-rightW2->width(), height());
	QRect l_endRect1(0, 0, width(), height());
	QRect r_startRect1(width()-rightW2->width(), 64, rightW2->width(), height()-64);
	QRect r_endRect1(width(), 64,  0, height()-64);
	l_animation3 -> setStartValue(l_startRect1);
	l_animation3 -> setEndValue(l_endRect1);
	l_animation3 -> setDuration(300);
	r_animation3 -> setStartValue(r_startRect1);
	r_animation3 -> setEndValue(r_endRect1);
	r_animation3 -> setDuration(300);	
	v3_group = new QParallelAnimationGroup(this);
	connect(v3_group, SIGNAL(finished()), v3_group, SLOT(deleteLater()));
	connect(v3_group, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	v3_group -> addAnimation(l_animation3);
	v3_group -> addAnimation(r_animation3);
	e_group = new QSequentialAnimationGroup(this);
	connect(e_group, SIGNAL(finished()), e_group, SLOT(deleteLater()));
	connect(e_group, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));		
	e_group -> addAnimation(v3_group);
	if (del)
	{
		if (rightW2 == rightw_container)
			rightw_container = 0;	
		connect(r_animation3, SIGNAL(finished()), rightW2, SLOT(close()));
		QPair<QString, QString> name_pair; 
		if (loging_user.second.isEmpty())
			name_pair = new_login;
		else
			name_pair = loging_user;
		if (!ws_cpktests.value(name_pair).isEmpty() && ws_cpktests.value(name_pair).at(2))
			connect(r_animation3, SIGNAL(finished()), this, SLOT(clearCpkProcessWidgets()));		
	}
	if (rightW1)
	{
		l_animation4 = new QPropertyAnimation(leftW, "geometry", this);
		connect(l_animation4, SIGNAL(valueChanged(const QVariant &)), leftw_container, SLOT(resetViewWidth(const QVariant &)));	
		connect(l_animation4, SIGNAL(finished()), l_animation4, SLOT(deleteLater()));
		connect(l_animation4, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
		LittleWidgetsView * r_view = qobject_cast<LittleWidgetsView *>(rightW1);
		r_animation4 = new QPropertyAnimation(rightW1, "geometry", this);
		connect(r_animation4, SIGNAL(finished()), r_view, SLOT(resizeAgentSize()));
		connect(r_animation4, SIGNAL(finished()), r_animation4, SLOT(deleteLater()));
		connect(r_animation4, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
		v4_group = new QParallelAnimationGroup(this);
		connect(v4_group, SIGNAL(finished()), v4_group, SLOT(deleteLater()));
		connect(v4_group, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
		QRect l_startRect2(0, 0, width(), height());
		QRect l_endRect2(0, 0, width()-rightW1->sizeHint().width(), height());
		QRect r_startRect2(width(), 64, w_reshow, height()-64);
		QRect r_endRect2(width()-w_reshow, 64, w_reshow, height()-64);
		l_animation4 -> setStartValue(l_startRect2);
		l_animation4 -> setEndValue(l_endRect2);
		r_animation4 -> setStartValue(r_startRect2);
		r_animation4 -> setEndValue(r_endRect2);
		l_animation4 -> setDuration(300);
		r_animation4 -> setDuration(300);	
		v4_group -> addAnimation(l_animation4);
		v4_group -> addAnimation(r_animation4);
		e_group -> addAnimation(v4_group);
		rightw_container = qobject_cast<LittleWidgetsView *>(rightW1);	
		syncCurrentWinWithHash(leftW, rightW1, loging_user);
		storeRalatedEdittingBar(leftW, rightW1, showing_toolBar);
	}
	wins_moving = true;
	connect(e_group, SIGNAL(finished()), this, SLOT(setWinsMovingFinished()));
	e_group -> start();	
}

void MainWindow::slideMultiWidgets(QWidget * new_left, QWidget * new_right, QWidget * old_left,  QWidget * old_right, bool del_not, bool left_right)
{
	if (wins_moving)
		return;
	if (new_left != protect_widget)
	{
		leftw_container = qobject_cast<LittleWidgetsView *>(new_left);
		DataesTableView * engi_view = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
		if (engi_view && engi_view->objectName()=="dataesin")
		{
			QWidget * tmp_widget = 0;
			QPair<QString, QString> name_pair; 
			if (loging_user.second.isEmpty())
				name_pair = new_login;
			else
				name_pair = loging_user;		
			if (engi_view->tblEdittingState())
			{
				if (ws_cpktests.value(name_pair).at(0)->objectName()==tr("okcancel_新建1"))
				{
					tmp_widget = ws_cpktests.value(name_pair).at(0);
					ws_cpktests[name_pair][0] = storedBarOnHash(leftw_container, new_right);
					storeRalatedEdittingBar(leftw_container, new_right, tmp_widget);					
				}
			}
			else
			{
				if (ws_cpktests.value(name_pair).at(0)->objectName() == tr("okcancel_CPK数据"))
				{
					tmp_widget = ws_cpktests.value(name_pair).at(0);
					ws_cpktests[name_pair][0] = storedBarOnHash(leftw_container, new_right);
					storeRalatedEdittingBar(leftw_container, new_right, tmp_widget);					
				}				
			}
		}
	}
	l_animation3 = new QPropertyAnimation(new_left, "geometry", this);
	connect(l_animation3, SIGNAL(finished()), l_animation3, SLOT(deleteLater()));
	connect(l_animation3, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	l_animation4 = new QPropertyAnimation(this);
	connect(l_animation4, SIGNAL(finished()), l_animation4, SLOT(deleteLater()));
	connect(l_animation4, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	r_animation3 = new QPropertyAnimation(old_left, "geometry", this);
	r_animation4 = new QPropertyAnimation(this);
	v3_group = new QParallelAnimationGroup(this);
	connect(v3_group, SIGNAL(finished()), v3_group, SLOT(deleteLater()));
	connect(v3_group, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	QRect l_startRect1;
	QRect l_endRect1;
	QRect l_startRect2;
	QRect l_endRect2;
	QRect r_startRect1;
	QRect r_endRect1;
	QRect r_startRect2;
	QRect r_endRect2;
	if (new_right)
	{
		if (left_right)
			l_startRect1.setRect(-width(), new_left->geometry().top(), width()-new_right->width(), new_left->height());
		else
			l_startRect1.setRect(width(), new_left->geometry().top(), width()-new_right->width(), new_left->height());
		l_endRect1.setRect(0, new_left->geometry().top(), width()-new_right->width(), new_left->height());			
		l_animation4 -> setTargetObject(new_right);
		l_animation4 -> setPropertyName("geometry");
		if (left_right)
			l_startRect2.setRect(-width()+new_right->width(), 64, new_right->width(), height()-64);
		else
			l_startRect2.setRect(width()+new_right->width(), 64, new_right->width(), height()-64);
		l_endRect2.setRect(width()-new_right->width(), 64, new_right->width(), height()-64);
		l_animation3 -> setStartValue(l_startRect1);
		l_animation3 -> setEndValue(l_endRect1);
		l_animation3 -> setDuration(400);
		l_animation4 -> setStartValue(l_startRect2);
		l_animation4 -> setEndValue(l_endRect2);
		l_animation4 -> setDuration(400);
		v3_group -> addAnimation(l_animation3);
		v3_group -> addAnimation(l_animation4);
		rightw_container = qobject_cast<LittleWidgetsView *>(new_right);
		new_right -> show();
	}
	else
	{
		rightw_container = 0;
		if (new_left->geometry().top()+new_left->height() == height())
		{
			if (left_right)
				l_startRect1.setRect(-width(), new_left->geometry().top(), width(), new_left->height());
			else
				l_startRect1.setRect(width(), new_left->geometry().top(), width(), new_left->height());
			l_endRect1.setRect(0, new_left->geometry().top(), width(), new_left->height());
		}
		else
		{
			if (left_right)
				l_startRect1.setRect(-width(), new_left->geometry().top(), width(), height());
			else
				l_startRect1.setRect(width(), new_left->geometry().top(), width(), height());
			l_endRect1.setRect(0, new_left->geometry().top(), width(), height());
		}
		l_animation3 -> setStartValue(l_startRect1);
		l_animation3 -> setEndValue(l_endRect1);
		l_animation3 -> setDuration(400);
		v3_group -> addAnimation(l_animation3);			
	}
	if (old_right)
	{ 
		r_animation4 -> setTargetObject(old_right);
		r_animation4 -> setPropertyName("geometry");
		r_startRect1.setRect(0, old_left->geometry().top(), width()-old_right->width(), old_left->height());		
		r_startRect2.setRect(width()-old_right->width(), old_right->geometry().top(), old_right->width(), old_right->height());
		if (left_right)
		{
			r_endRect1.setRect(width(), old_left->geometry().top(), width()-old_right->width(), old_left->height());
			r_endRect2.setRect(width()+old_left->width(), old_right->geometry().top(), old_right->width(), old_right->height());
		}
		else
		{
			r_endRect1.setRect(-width(), old_left->geometry().top(), width()-old_right->width(), old_left->height());
			r_endRect2.setRect(-old_left->width(), old_right->geometry().top(), old_right->width(), old_right->height());
		}		
		r_animation3 -> setStartValue(r_startRect1);
		r_animation3 -> setEndValue(r_endRect1);
		r_animation3 -> setDuration(400);
		r_animation4 -> setStartValue(r_startRect2);
		r_animation4 -> setEndValue(r_endRect2);
		r_animation4 -> setDuration(400);
		v3_group -> addAnimation(r_animation3);
		v3_group -> addAnimation(r_animation4);			
	}
	else
	{
		r_startRect1.setRect(0, old_left->geometry().top(), width(), old_left->height());
		if (left_right)
			r_endRect1.setRect(width(), old_left->geometry().top(), width(), old_left->height());
		else
			r_endRect1.setRect(-width(), old_left->geometry().top(), width(), old_left->height());
		r_animation3 -> setStartValue(r_startRect1);
		r_animation3 -> setEndValue(r_endRect1);
		r_animation3 -> setDuration(400);
		v3_group -> addAnimation(r_animation3);			
	}
	if (del_not)
	{
		if (old_left)
			connect(r_animation3, SIGNAL(finished()), old_left, SLOT(close()));
		if (old_right)
			connect(r_animation4, SIGNAL(finished()), old_right, SLOT(close()));
		QPair<QString, QString> name_pair; 
		if (loging_user.second.isEmpty())
			name_pair = new_login;
		else
			name_pair = loging_user;			
		if (!ws_cpktests.value(name_pair).isEmpty() && ((ws_cpktests.value(name_pair).at(2) && ws_cpktests.value(name_pair).at(2)==old_right) || (ws_cpktests.value(name_pair).at(4) && ws_cpktests.value(name_pair).at(4)==old_right)))
			connect(v3_group, SIGNAL(finished()), this, SLOT(clearCpkProcessWidgets()));	
	}
	connect(r_animation4, SIGNAL(finished()), r_animation4, SLOT(deleteLater()));
	connect(r_animation4, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	connect(r_animation3, SIGNAL(finished()), r_animation3, SLOT(deleteLater()));
	connect(r_animation3, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	connect(r_animation4, SIGNAL(finished()), r_animation4, SLOT(deleteLater()));
	connect(r_animation4, SIGNAL(destroyed(QObject *)), this, SLOT(clearWidgetsAnimation(QObject *)));
	wins_moving = true;
	connect(v3_group, SIGNAL(finished()), this, SLOT(setWinsMovingFinished()));
	v3_group -> start();
}

void MainWindow::embededOrSlideWidgetOnRightQml(QWidget * embeded)
{
	if (!rightw_container)
	{
		rightw_container = new LittleWidgetsView(this);
		rightw_container -> setObjectName("r_container");
		rightw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/treeviewer.qml");		
		QRect o_rect;	 
		embeded -> setFixedHeight(height()-64);
		if (qobject_cast<TableManagement *>(embeded))
		{
			TableManagement * tree_table = qobject_cast<TableManagement *>(embeded);
			tree_table -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			tree_table -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			o_rect.setWidth(tree_table->sizeHint().width());
			o_rect.setHeight(tree_table->height());
		}
		else if (qobject_cast<RelatedTreeView *>(embeded))
		{
			RelatedTreeView * nestting_tree = qobject_cast<RelatedTreeView *>(embeded);
			nestting_tree -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			nestting_tree -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			o_rect.setWidth(nestting_tree->sizeHint().width());
			o_rect.setHeight(nestting_tree->height());
		}
		else if (qobject_cast<DataesTableView *>(embeded))
		{
			QTableView * nestting_table = qobject_cast<QTableView *>(embeded);// need finished
			nestting_table -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			nestting_table -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			o_rect.setWidth(nestting_table->sizeHint().width());
			o_rect.setHeight(nestting_table->height());
		}
		else if (qobject_cast<EngiTreeView *>(embeded))
		{
			EngiTreeView * nestting_tree = qobject_cast<EngiTreeView *>(embeded);
			nestting_tree -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			nestting_tree -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			o_rect.setWidth(nestting_tree->sizeHint().width());
			o_rect.setHeight(nestting_tree->height());
		}
		rightw_container -> initMarginForParent(o_rect);
		rightw_container -> setViewForRelatedTable(embeded);	
	}
	slideWidgetView(leftw_container, rightw_container);	
}

void MainWindow::initManualTableConstruct(QStandardItemModel * dest_model, const QString & further)
{
	QWidget * tmp_left = leftw_container;
	QWidget * tmp_bar = showing_toolBar;	
	leftw_container = new LittleWidgetsView(this);
	leftw_container -> setObjectName("l_container");
	leftw_container -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");
	TablesGroup * manual_table = new TablesGroup;
	manual_table -> setObjectName("manualtable");
	manual_table -> setInspectors(back_science, base_db);
	QWidget * stored_bar = storedBarOnHash(tmp_left, rightw_container);//need add clear stored bar and widgets here??
	EditLongToolsViewer * tol_editBar = 0;
	if (stored_bar && qobject_cast<EditLongToolsViewer *>(stored_bar))//???
		tol_editBar = qobject_cast<EditLongToolsViewer *>(stored_bar);
	if (!tol_editBar)
		tol_editBar = new EditLongToolsViewer(this);
	if (tol_editBar->objectName().isEmpty() || !tol_editBar->objectName().contains("toleditBar"))
	{
		if (tol_editBar->objectName().isEmpty())
			tol_editBar -> setObjectName("toleditBar");
		else
		{
			tol_editBar = new EditLongToolsViewer(this);
			tol_editBar -> setObjectName("toleditBar");
		}
	}
	manual_table -> setFixedWidth(width());
	manual_table -> setFixedHeight(height()-64);
	QRect o_rect(0, 0, manual_table->width(), manual_table->height());
	tol_editBar -> setOrientation(EditLongToolsViewer::ScreenOrientationLockLandscape);
	if (dest_model)
	{
		if (further == tr("数据信息管理条"))
		{
			tol_editBar -> setObjectName(tol_editBar->objectName()+tr("_数据信息管理条"));
			tol_editBar -> setMainQmlFile(QLatin1String("/home/dapang/workstation/spc-tablet/spc_qml/dataesinfoes.qml"), 6);
		}
		else
		{
			tol_editBar -> setObjectName(tol_editBar->objectName()+tr("_表图管理条"));
			tol_editBar -> setMainQmlFile(QLatin1String("/home/dapang/workstation/spc-tablet/spc_qml/datainfoploteditor.qml"), 7);
		}
		manual_table -> initTable(loging_user, dest_model);
	}
	else
	{
		manual_table -> initTable(loging_user, setManualTableModel(16, 8));
		if (further == tr("数据手工制表条"))
		{
			tol_editBar -> setObjectName(tol_editBar->objectName()+tr("_数据总编辑条"));
			tol_editBar -> setMainQmlFile(QLatin1String("/home/dapang/workstation/spc-tablet/spc_qml/dataeditor.qml"), 0);
		}
		if (further == tr("管理手工制表条"))
		{
			tol_editBar -> setObjectName(tol_editBar->objectName()+tr("_信息总编辑条"));
			tol_editBar -> setMainQmlFile(QLatin1String("/home/dapang/workstation/spc-tablet/spc_qml/infoeseditor.qml"), 1);
		}
	}
	manual_table -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	manual_table -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	
	tol_editBar -> setEditingTable(manual_table);
	leftw_container -> initMarginForParent(o_rect);
	leftw_container -> setViewForRelatedTable(manual_table);
	connect(tol_editBar, SIGNAL(relatedSelectedResource(QString, bool)), this, SLOT(slideMatchResources(QString, bool)));
	connect(tol_editBar, SIGNAL(sendSaveSigToWin(bool)), this, SLOT(prepairForDBsaveWork(bool)));
	connect(tol_editBar, SIGNAL(sendStopInfoToMainWin(QString)), this, SLOT(stopQmlEditorFunc(QString)));
	tol_editBar -> showExpanded();
	animateToolBarLeftRight(tmp_bar, tol_editBar);
	slideWholeWidgets(tmp_left, leftw_container, true);
}

void MainWindow::syncNewWinWithHash(const QPair<QString, QString> user, QWidget * newLeft, QWidget * newRight)
{
	QPair<QWidget *, QWidget *> ins_pair(newLeft, newRight);
	if (stored_widgets.value(user).contains(ins_pair))
		return;
	stored_widgets[user] << ins_pair;
	QList<QPair<QWidget *, QWidget *> > lr_widgets = stored_widgets.value(user);
	bool found_ldw = false;
	QList<QPair<QWidget *, QWidget *> > re_currents;
	for (int i = 0; i < lr_widgets.size(); i++)
	{
		if (stored_bars.contains(lr_widgets.at(i).first))
		{
			found_ldw = true;
			re_currents = stored_bars.value(lr_widgets.at(i).first);				
			if (lr_widgets.at(i).first != newLeft)
			{
				stored_bars.take(lr_widgets.at(i).first);
				stored_bars.insert(newLeft, re_currents);
			}		
		}		
	}
	if (!found_ldw)
		stored_bars.insert(newLeft, re_currents);
}

void MainWindow::resetToolBarsVarsValue(QWidget * bar, bool show_hide)
{
	if (!bar)
		return;
	if (show_hide)
		showing_toolBar = bar;
	else
		showing_toolBar = 0;
}

void MainWindow::storeRalatedEdittingBar(QWidget * ld_widget, QWidget * r_widget, QWidget * bar)
{
	if (loging_pairs.isEmpty() || !bar)
		return;
	QList<QPair<QWidget *, QWidget *> > widgets_values = stored_widgets.value(loging_user);
	QWidget * last_leader = 0;
	for (int i = 0; i < widgets_values.size(); i++)
	{
		if (stored_bars.contains(widgets_values.at(i).first))
		{
			last_leader = widgets_values.at(i).first;
			break;
		}
	}
	if (!last_leader)
		last_leader = ld_widget;
 	QPair<QWidget *, QWidget *> v_each;
	if (qobject_cast<LittleWidgetsView *>(ld_widget)->qmlAgent()->widget()->objectName() != "plot")
		v_each.first = ld_widget;
	else
		v_each.first = r_widget;
	v_each.second = bar;
	QList<QPair<QWidget *, QWidget *> > bars_values = stored_bars.value(last_leader);
	if (bars_values.contains(v_each))
		return;
	bool barp_found = false;
	for (int i = 0; i < bars_values.size(); i++)
	{
		if (bars_values.at(i).first==ld_widget || bars_values.at(i).first==r_widget)
		{
			barp_found = true;
			if (bars_values.at(i).second != bar)
				stored_bars[last_leader][i].second = bar;
			break;
		}
	}
	if (!barp_found && bar->objectName()!="multiwinsBar")
		stored_bars[last_leader] << v_each;
}

void MainWindow::clearWidgetOnHash(const QPair<QString, QString> user, QWidget * w_clear)
{
	if (loging_pairs.isEmpty())
		return;	
	QList<QPair<QWidget *, QWidget *> >u_list = stored_widgets.value(user);
	for (int i = 0; i < u_list.size(); i++)
	{
		if (u_list.at(i).first == w_clear)
		{
			stored_widgets[user].removeOne(u_list.at(i));
			break;
		}
	}	
}

void MainWindow::clearRalatedEdittingBarOnHash(QWidget * bar)
{
 	if (loging_pairs.isEmpty() || bar->objectName()=="multiwinsBar")
		return; 
	QHashIterator<QWidget *, QList<QPair<QWidget *, QWidget *> > > bar_hash(stored_bars);
	while (bar_hash.hasNext())
	{
		bar_hash.next();
		if (bar_hash.key()->objectName() == "helppages")
			continue;
		for (int i = 0; i < bar_hash.value().size(); i++)
		{
			if (bar_hash.value().at(i).second == bar)
			{
				stored_bars[bar_hash.key()].removeOne(bar_hash.value().at(i));				
				return;
			}
		}	
	}
}

void MainWindow::stopProtectionMode(bool new_old)
{
	protect_mode = false;	
	if (new_old)
	{
		LittleWidgetsView * g_leftw = new LittleWidgetsView(this);
		g_leftw -> setObjectName("l_container");
		g_leftw -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablegroupviewer.qml");
		QRect o_rect(0, 0 , width(), height());
		g_leftw -> initMarginForParent(o_rect);
		PlotWidget * plot_new = new PlotWidget(back_science, base_db);
		plot_new -> setObjectName("plot");
		plot_new -> setFixedWidth(width());
		plot_new -> setFixedHeight(height());		
		g_leftw -> setViewForRelatedTable(plot_new);
		plot_new -> setEnginame();
		slideMultiWidgets(g_leftw, 0, protect_widget, 0, true, true);
		loging_user = new_login;
		loging_pairs << new_login;
		syncCurrentWinWithHash(g_leftw, 0, loging_user);		
	}
	else
	{	  
		QPair<QWidget *, QWidget *> show_pair = usrOwnedShowingWidgets(new_login);
		if (new_login != loging_user)
			loging_user = new_login;
		QWidget * stored_bar = storedBarOnHash(leftw_container, rightw_container);
		if (stored_bar)
			animateToolBarLeftRight(0, stored_bar);		
		slideMultiWidgets(show_pair.first, show_pair.second, protect_widget, 0, true, true);
		syncCurrentWinWithHash(show_pair.first, show_pair.second, loging_user);
		leftw_container = qobject_cast<LittleWidgetsView *>(show_pair.first);
		rightw_container = qobject_cast<LittleWidgetsView *>(show_pair.second);		
	}	
	protect_widget = 0;
}

void MainWindow::clearVariantsBackToOriginalState()
{
	loging_pairs.clear();
	QString empty;
	QPair<QString, QString> null_pair(empty, empty);
	loging_user = null_pair;
	if (!ws_cpktests.isEmpty())
	{
		QHashIterator<QPair<QString, QString>, QWidgetList> cache_hash(ws_cpktests);
		while (cache_hash.hasNext())
		{
			cache_hash.next();
			foreach (QWidget * d_widget, cache_hash.value())
			{
				if (d_widget)
					d_widget -> deleteLater();
			}	
		}
		ws_cpktests.clear();
	}
	QHashIterator<QWidget *, QList<QPair<QWidget *, QWidget *> > > bars_hash(stored_bars);
	while (bars_hash.hasNext())
	{
		bars_hash.next();
		QList<QPair<QWidget *, QWidget *> > v_pairs = bars_hash.value();
		for (int i = 0; i < v_pairs.size(); i++)
		{
			if (v_pairs.at(i).second != showing_toolBar)
				v_pairs[i].second -> deleteLater();
		}
	}
	stored_bars.clear();
	QHashIterator<QPair<QString, QString>, QList<QPair<QWidget *, QWidget *> > > widgets_hash(stored_widgets);
	while (widgets_hash.hasNext())//bug here for not clean all widgets ptrs by slide action
	{
		widgets_hash.next();
		QList<QPair<QWidget *, QWidget *> > v_pairs = widgets_hash.value();
		for (int i = 0; i < v_pairs.size(); i++)
		{
			if (v_pairs.at(i).first != leftw_container)
				v_pairs[i].first -> deleteLater();
			if (v_pairs.at(i).second)
				v_pairs[i].second -> deleteLater();
		}
	}
	stored_widgets.clear();
	PlotWidget * old_plot = qobject_cast<PlotWidget *>(leftw_container->qmlAgent()->widget());
	if (!old_plot->currentEngi().isEmpty())
		old_plot -> setEnginame();	
	sys_userState = -1;
	user_mode = false;
	protect_mode = false;
	wins_moving = false;
	save_name.clear();
	back_science -> initNewFilePara();
	base_db -> closeCurDB();	
}

void MainWindow::adjacentShowingWidgets(const QPair<QString, QString> & owner, bool next_last, bool close_last)
{
 	if (!stored_widgets.contains(owner))
		return; 
	QList<QPair<QWidget *, QWidget *> > widgets_list = stored_widgets.value(owner);
	QPair<QWidget *, QWidget *> current_shows = usrOwnedShowingWidgets(owner);
	int wins_show = widgets_list.indexOf(current_shows);
	int turn_show = 0;
	if (next_last)
	{
		if (wins_show != widgets_list.size()-1)
			turn_show = wins_show+1;	
		if (close_last)
		{			  
			slideMultiWidgets(widgets_list.at(turn_show).first, widgets_list.at(turn_show).second, current_shows.first, current_shows.second, true, false);	
			if (storedBarOnHash(current_shows.first, current_shows.second))
				clearRalatedEdittingBarOnHash(storedBarOnHash(current_shows.first, current_shows.second));
			clearWidgetOnHash(loging_user, current_shows.first);					
		}
		else
			slideMultiWidgets(widgets_list.at(turn_show).first, widgets_list.at(turn_show).second, current_shows.first, current_shows.second, false, false);				
	}
	else
	{
		if (wins_show == 0)
			turn_show = widgets_list.size()-1;
		else
			turn_show = wins_show-1;				
		slideMultiWidgets(widgets_list.at(turn_show).first, widgets_list.at(turn_show).second, current_shows.first, current_shows.second, false, true);			
	}
	QList<QPair<QWidget *, QWidget *> > rearrages = stored_bars.take(current_shows.first);
	stored_bars.insert(widgets_list.at(turn_show).first, rearrages);
}

void MainWindow::testEngiPromotionTransition(bool direction)
{
	DataesTableView * key_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());	
	QList<QStandardItemModel *> chk_models;
	key_table -> initEditPromotionModelsList(chk_models);
	if (chk_models.size() == 1)
	{
		if (direction && qobject_cast<SlideBar *>(showing_toolBar)->currentCommand() != key_table->delModelsStack())
			qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->delModelsStack());		  
		if (!direction && qobject_cast<SlideBar *>(showing_toolBar)->currentCommand() != key_table->commandStack())
			qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->commandStack());
		return;
	}
	DataesTableView * next_din = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());	
	QStandardItemModel * k_next = 0;
	if (!direction)
	{	  
		if (chk_models.indexOf(qobject_cast<QStandardItemModel *>(key_table->model())) == 0)
			k_next = chk_models.at(chk_models.size()-1);
		else
			k_next = chk_models.at(chk_models.indexOf(qobject_cast<QStandardItemModel *>(key_table->model()))-1);		
	}
	else
	{
		if (chk_models.indexOf(qobject_cast<QStandardItemModel *>(key_table->model()))== chk_models.size()-1)
			k_next = chk_models.at(0);
		else
			k_next = chk_models.at(chk_models.indexOf(qobject_cast<QStandardItemModel *>(key_table->model()))+1);		
	}
	key_table -> setModel(k_next);
	key_table -> setShiftModel(k_next);
	if (key_table->modelHashs().value(tr("草稿工程，，。")).at(0).second == k_next)
	{
		key_table -> resetEditState(true);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->commandStack());
	}
	else
	{
		key_table -> resetEditState(false);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(key_table->delModelsStack());		
	}
	QStandardItemModel * din_cursor = next_din->cursorModels().value(k_next);
	if (din_cursor)
	{
		next_din -> setModel(din_cursor);
		next_din -> changeViewStateForNewModel();
	}		
	next_din -> resetEditState(false);  
}

void MainWindow::setProtectWidgetBackground(QWidget * bk_widget)
{
	bk_widget -> setStyleSheet("background-image: url(:/images/qcimage.jpg)");
	digital_clock = new QLCDNumber(bk_widget);
	digital_clock -> setSegmentStyle(QLCDNumber::Filled);
	digital_clock -> setAttribute(Qt::WA_TranslucentBackground, true);
	protect_timer = new QTimer(bk_widget);
	connect(protect_timer, SIGNAL(timeout()), this, SLOT(showDigitalTime()));
	protect_timer -> start(1000);
	digital_clock -> resize(150, 60);
	bk_widget -> show();
	digital_clock -> setGeometry(width()/2-75, height()/2-30, 150, 60);	
	showDigitalTime();
}

void MainWindow::testEngiSampPromotionTransition(bool direction)
{	
	DataesTableView * next_din = qobject_cast<DataesTableView *>(leftw_container->qmlAgent()->widget());
	if (!next_din)
		return;
	DataesTableView * key_table = qobject_cast<DataesTableView *>(rightw_container->qmlAgent()->widget());
	QList<QStandardItemModel *> chk_models;
	next_din -> initEditPromotionModelsList(chk_models, key_table);
	if (chk_models.size() == 1)	
	{
		if (direction && qobject_cast<SlideBar *>(showing_toolBar)->currentCommand() != next_din->delModelsStack())
			qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(next_din->delModelsStack());		  
		if (!direction && qobject_cast<SlideBar *>(showing_toolBar)->currentCommand() != next_din->commandStack())
			qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(next_din->commandStack());
		return;
	}
	QStandardItemModel * n_model = 0;
	if (!direction)
	{
		if (chk_models.indexOf(qobject_cast<QStandardItemModel *>(next_din->model())) == 0)
			n_model = chk_models.at(chk_models.size()-1);
		else
			n_model = chk_models.at(chk_models.indexOf(qobject_cast<QStandardItemModel *>(next_din->model()))-1);	
	}
	else
	{
		if (chk_models.indexOf(qobject_cast<QStandardItemModel *>(next_din->model()))== chk_models.size()-1)
			n_model = chk_models.at(0);
		else
			n_model = chk_models.at(chk_models.indexOf(qobject_cast<QStandardItemModel *>(next_din->model()))+1);			
	}
	next_din -> setModel(n_model);
	next_din -> changeViewStateForNewModel();
	if (next_din->modelHashs().value(tr("草稿工程，，。")).at(0).second == n_model)
	{
		next_din -> resetEditState(true);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(next_din->commandStack());
	}		
	else
	{
		next_din -> resetEditState(false);
		qobject_cast<SlideBar *>(showing_toolBar) -> setCommandStack(next_din->delModelsStack());
	}		
}

bool MainWindow::syncCurrentWinWithHash(QWidget * leftW, QWidget * rightW, const QPair<QString, QString> & viewer)
{
 	if (loging_pairs.isEmpty())
		return false; 
	QPair<QWidget *, QWidget *> lr_pair(leftW, rightW);
	QList<QPair<QWidget *, QWidget *> > lr_widgets = stored_widgets.value(viewer);
	bool sync_found = false;
	bool bars_init = false;
	for (int i = 0; i < lr_widgets.size(); i++)
	{
		if (stored_bars.contains(lr_widgets.at(i).first))
		{
			bars_init = true;
			sync_found = true;
			QList<QPair<QWidget *, QWidget *> > re_currents = stored_bars.value(lr_widgets.at(i).first);				
			if (lr_widgets.at(i).first != leftW)
			{
				stored_widgets[viewer].replace(i, lr_pair);
				stored_bars.take(lr_widgets.at(i).first);
				stored_bars.insert(leftW, re_currents);
			}		
		}		
		if (lr_widgets.at(i).first==lr_pair.first)
		{		  
			sync_found = true;
			if (lr_widgets.at(i) != lr_pair)
				stored_widgets[viewer][i] = lr_pair;			
		}
	}
	if (!bars_init)
	{
		QList<QPair<QWidget *, QWidget *> > pos_pairs;
		stored_bars.insert(leftW, pos_pairs);
	}
	return sync_found;
}

bool MainWindow::projectsEmpty()
{
	QString key_projects = "projectskeys";
	QString projects_names = "name";
	QStringList projects;
	base_db -> getDataesFromTable(key_projects, projects_names, projects);
	if (projects.isEmpty())
		return true;
	return false;
}

bool MainWindow::powerListsEmpty()
{
	QMap<QString, QStringList> jud_managers = base_db->curDBmanageTable();
	if (jud_managers.isEmpty())
		return true;
	else
		return false;
}

bool MainWindow::usernameInLogingPairs()
{
	for (int i = 0; i < loging_pairs.size(); i++)
	{
		if (new_login.first == loging_pairs.at(i).first)
			return true;
	}
	return false;
}

bool MainWindow::newComerSameInLogingPairs(const QPair<QString, QString> & new_comer)
{
	for (int i = 0; i < loging_pairs.size(); i++)
	{
		if (new_comer == loging_pairs.at(i))
			return true;
	}
	return false;
}

bool MainWindow::newUserDifWithLoginger()
{
	if (new_login == loging_user)
		return false;
	return true;
}

bool MainWindow::canTransOtherWinForUsr(const QPair<QString, QString> & user)
{
	if (stored_widgets.isEmpty())
		return false;
	QList<QPair<QWidget *, QWidget *> > usr_widgets = stored_widgets.value(user);
	if (usr_widgets.size() <= 1)
		return false;
	else
		return true;
	return false;		
}

bool MainWindow::allManageProjsReadOnly()
{
 	QStringList con_projs;
	base_db -> constructorOwnedProjs(loging_user, con_projs);
	QString m_projs = base_db->curDBmanageTable().value(loging_user.second).at(3);
	bool read_only = false;
	if (con_projs.isEmpty() && !m_projs.contains(tr("读写"))) 
		read_only = true;
	return read_only;
}

bool MainWindow::projectsEditting()
{
	if (!ws_cpktests.isEmpty())
		return true;
	QHashIterator<QPair<QString, QString>, QList<QPair<QWidget *, QWidget *> > > i_hash(stored_widgets);
	while (i_hash.hasNext())
	{
		i_hash.next();
		for (int i = 0; i < i_hash.value().size(); i++)
		{
			if (i_hash.value().at(i).second)
			{
				if (qobject_cast<LittleWidgetsView *>(i_hash.value().at(i).second)->qmlAgent()->widget()->objectName() == "bkdb")
					return true;
				if (qobject_cast<LittleWidgetsView *>(i_hash.value().at(i).second)->qmlAgent()->widget()->objectName() == "initEngi")
					return true;				
				if (qobject_cast<LittleWidgetsView *>(i_hash.value().at(i).second)->qmlAgent()->widget()->objectName() == "tmpinput")
					return true;
			}
		}
	}
	return false;  
}

bool MainWindow::projectOpenCheck(const QString & chk_proj)
{
	QStringList opened_projs;
	if (!ws_cpktests.isEmpty())
	{
		QHashIterator<QPair<QString, QString>, QWidgetList> ws_hash(ws_cpktests);
		while (ws_hash.hasNext())
		{
			ws_hash.next();
			if (ws_hash.value().at(2))
			{
				QString edit_proj;
				if (ws_hash.value().size() == 7)
				{
					DataesTableView * init_engi = qobject_cast<DataesTableView *>(qobject_cast<LittleWidgetsView *>(ws_hash.value().at(2))->qmlAgent()->widget());
					edit_proj = init_engi->model()->data(init_engi->model()->index(0, 0)).toString();
					if (!edit_proj.isEmpty() && !opened_projs.contains(edit_proj))
						opened_projs << edit_proj;
				}
				else
				{
					EngiTreeView * improve_engi = qobject_cast<EngiTreeView *>(qobject_cast<LittleWidgetsView *>(ws_hash.value().at(2))->qmlAgent()->widget()); 
					edit_proj = improve_engi->currentEngiItem()->text();
					if (!opened_projs.contains(edit_proj) && ws_hash.value().at(3))
						opened_projs << edit_proj;					
				}
			}
		}
	}
	QHashIterator<QPair<QString, QString>, QList<QPair<QWidget *, QWidget *> > > i_hash(stored_widgets);
	while (i_hash.hasNext())
	{
		i_hash.next();
		for (int i = 0; i < i_hash.value().size(); i++)
		{
			if (i_hash.value().at(i).second)
			{
				if (qobject_cast<LittleWidgetsView *>(i_hash.value().at(i).second)->qmlAgent()->widget()->objectName() == "bkdb")
					return true;
				if (qobject_cast<LittleWidgetsView *>(i_hash.value().at(i).second)->qmlAgent()->widget()->objectName() == "tmpinput")
				{
					PlotWidget * left_plot = qobject_cast<PlotWidget *>(qobject_cast<LittleWidgetsView *>(i_hash.value().at(i).first)->qmlAgent()->widget()); 
					QString open_engi = left_plot->currentEngi().split(tr("，，。")).at(0);
					if (!opened_projs.contains(open_engi))
						opened_projs << open_engi;						
				}
			}
		}
	}
	if (opened_projs.contains(chk_proj))
		return true;
	return false;
}

QWidget * MainWindow::storedBarOnHash(QWidget * ld_widget, QWidget * r_widget)
{
	if (loging_pairs.isEmpty())
		return 0;
	QWidget * dest_bar = 0;	
	QList<QPair<QWidget *, QWidget *> > find_bars = stored_bars.value(ld_widget);
	for (int i = 0; i < find_bars.size(); i++)
	{
		if (find_bars.at(i).first==ld_widget || find_bars.at(i).first==r_widget)
		{
			dest_bar = find_bars.at(i).second;
			break;
		}
	}
	return dest_bar;
}

QStandardItemModel * MainWindow::setManualTableModel(int row, int colummn)
{
	QStandardItemModel * manual_model = new QStandardItemModel(this);
	manual_model -> setRowCount(row);
	manual_model -> setColumnCount(colummn);
	return manual_model;
}

QString MainWindow::checkDBfile()
{
	QString db_file;
	QDir related_dir("/home/dapang/workstation/spc-tablet/qlitedb");
	QFileInfoList qfi_list = related_dir.entryInfoList(QDir::Files);
	if (!qfi_list.isEmpty())
		db_file = qfi_list[0].filePath();
	return db_file;		
}

QPair<QWidget *, QWidget *> MainWindow::usrOwnedShowingWidgets(const QPair<QString, QString> & owner)
{
	QList<QPair<QWidget *, QWidget *> > own_widgets = stored_widgets.value(owner);
	QPair<QWidget *, QWidget *> find_pair;
	for (int i = 0; i < own_widgets.size(); i++)
	{
		if (stored_bars.keys().contains(own_widgets.at(i).first))
		{
			find_pair = own_widgets.at(i);
			break;
		}
	}
	return find_pair;
}