#include <QtGui>
#include "dialog.h"
#include "mainwindow.h"
#include "tablemanagement.h"
#include "dataestableview.h"
#include "relatedtreeview.h"
#include "littlewidgetsview.h"
#include "littlewidgetitem.h"
#include "spcdatabase.h"

DiaLog::DiaLog(QWidget * parent)
:QDialog(parent, Qt::FramelessWindowHint), r_chk(false), sure(false), unsure(false), dialog_animation(0), w_inspector(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setModal(true);	
}

DiaLog::~DiaLog()
{}

void DiaLog::initWinDialog(MainWindow * guider, const QString & information, int type)
{
	w_inspector = guider;
	if (information.contains("hint:"))
		initHintDialog(information);
	else if (information.contains("warning:"))
		initWarningDialog(information, type);
	else
		initErrorDialog(information);
}

void DiaLog::initSaveStrDefineDialog(MainWindow * guider, SpcDataBase * db, const QString & passwd)
{
	inputor = passwd;
	w_inspector = guider;
	search_db = db;	
	QVBoxLayout * v_layout = new QVBoxLayout(this);
	QLabel * input_label = new QLabel;
	input_label -> setFont(QFont("宋体", 10));
	input_label -> setStyleSheet("QLabel{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-height: 13px; max-height: 13px;}");
	if (passwd == tr("导出图片"))
		input_label -> setText(tr("请输入图片名"));
	else
		input_label -> setText(tr("请输入文件名"));
	QTextEdit * editor = new QTextEdit;
	connect(editor, SIGNAL(cursorPositionChanged()), this, SLOT(resetLabelColor()));
	editor -> setStyleSheet("color: white; border: 1px solid rgb(50, 50, 50); border-radius: 3; background: rgb(50, 50, 50)");
	QFontMetrics fm(input_label->font());
	int width = fm.width(input_label->text());
	input_label -> setFixedWidth(width);
	editor -> setFixedWidth(width*2);
	editor -> setFixedHeight(60);
	QHBoxLayout * h_layout = new QHBoxLayout;
	QPushButton * ok_button = new QPushButton(tr("确定"));
	ok_button -> setFocusPolicy(Qt::NoFocus);
	ok_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
	if (passwd!=tr("导出文件") && passwd!=tr("导出图片"))
		connect(ok_button, SIGNAL(clicked()), this, SLOT(judgeInputTextForSave()));
	else
		connect(ok_button, SIGNAL(clicked()), this, SLOT(judgeInputTextForExport()));
	QPushButton * cancel_button = new QPushButton(tr("取消"));
	cancel_button -> setFocusPolicy(Qt::NoFocus);
	cancel_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(onCancel()));
	h_layout -> addWidget(ok_button);  
	h_layout -> addWidget(cancel_button);	 
	v_layout -> addWidget(input_label); 
	v_layout -> setAlignment(input_label, Qt::AlignCenter);        
	v_layout -> addWidget(editor);
 	v_layout -> setAlignment(editor, Qt::AlignCenter);       
	v_layout -> addLayout(h_layout);    
	v_layout->setSpacing(5);
	v_layout->setContentsMargins(0, 0, 0, 0);	
	show();
	openMeOnAnimation();	
}

void DiaLog::showForCollisionDbsDataes(const QHash<QString, QString> & projs, const QHash<QString, QString> & managers, MainWindow * p_mw)
{
	w_inspector = p_mw;
	search_db = w_inspector->curBackGuider();
	QVBoxLayout * v_layout = new QVBoxLayout(this);
	QLabel * warn_label = new QLabel;
	warn_label -> setFixedHeight(30);
	warn_label -> setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	QPixmap pixmap(":/images/warn.png");
	QPixmap w_pix = pixmap.scaled(warn_label->frameSize(), Qt::KeepAspectRatio);
	warn_label -> setPixmap(w_pix);	
	QString n_info(tr("两个数据库内工程名称、人员密码冲突\n请在如下的表格中修改冲突的工程名称、人员密码\n修改规则：\nA 修改内容不能为空\nB 工程名称不能包含标点符号\nC 人员密码不能包含标点符号"));
	QLabel * info_label = new QLabel(n_info);
	info_label -> setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	info_label -> setFont(QFont("Microsoft YaHei", 8));
	defineDialogLabelSize(info_label);	
	QString style_sheet("color: white; border: 1px solid rgb(50, 50, 50); border-radius: 5px; background: rgb(50, 50, 50)");
	info_label -> setStyleSheet(style_sheet);	
	QStandardItemModel * d_model = new QStandardItemModel(this);
	d_model -> setColumnCount(4);
	DataesTableView * collide_view = new DataesTableView(0, 5, search_db);
	collide_view -> setStyleSheet("font-family: Microsoft YaHei; font-size:12px; border: 0px groove gray; border-radius: 4px;");	
	collide_view -> setModel(d_model);	
	QModelIndex projs_title;
	if (!projs.isEmpty())
	{
		d_model -> insertRows(0, 2+projs.size());
		d_model -> setData(d_model->index(0, 0), tr("冲突的工程"));
		d_model -> setData(d_model->index(0, 1), tr("工程名称相同"));
		projs_title = d_model->index(0, 1);
		d_model -> setData(d_model->index(1, 0), tr("工程原名称"));
		d_model -> setData(d_model->index(1, 1), tr("修改后名称"));
		foreach (QString n_proj, projs.keys())
		{
			QStringList real_proj = n_proj.split(tr("，，。"));
			d_model -> setData(d_model->index(2+projs.keys().indexOf(n_proj), 0), real_proj.at(1));
		}
	}
	QModelIndex mngs_title;
	if (!managers.isEmpty())
	{
		int bottomn = d_model->rowCount();
		d_model -> insertRows(bottomn, 2+managers.size());
		d_model -> setData(d_model->index(bottomn, 0), tr("冲突的密码"));
		d_model -> setData(d_model->index(bottomn, 1), tr("人员密码相同"));	
		mngs_title = d_model->index(bottomn, 1);
		d_model -> setData(d_model->index(bottomn+1, 0), tr("人员姓名"));
		d_model -> setData(d_model->index(bottomn+1, 1), tr("人员密码"));
		d_model -> setData(d_model->index(bottomn+1, 2), tr("修改后密码"));
		d_model -> setData(d_model->index(bottomn+1, 3), tr("同一人？"));
		foreach (QString n_manager, managers.keys())
		{
			QStringList real_mng = n_manager.split(tr("，，。"));
			d_model -> setData(d_model->index(bottomn+2+managers.keys().indexOf(n_manager), 0), real_mng.at(1));
			d_model -> setData(d_model->index(bottomn+2+managers.keys().indexOf(n_manager), 1), real_mng.at(2));	
		}
	}
	collide_view -> resizeColumnsToContents();
	collide_view -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	collide_view -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	
	collide_view->verticalHeader()->close();
	collide_view->horizontalHeader()->close();	
	collide_view -> changeViewStateForNewModel();
	if (projs_title.isValid())
		collide_view -> setSpan(projs_title.row(), projs_title.column(), 1, 3);
	if (mngs_title.isValid())
		collide_view -> setSpan(mngs_title.row(), mngs_title.column(), 1, 3);	
	LittleWidgetsView * dec_view = new LittleWidgetsView(this);
	dec_view -> setMainQmlFile("/home/dapang/workstation/spc-tablet/spc_qml/tablesviewer.qml");
	QRect v_rect(0, 0, info_label->width(), info_label->height()*2);
	dec_view -> initMarginForParent(v_rect);
	dec_view -> setViewForRelatedTable(collide_view);
	QHBoxLayout * h_layout = new QHBoxLayout;
	QPushButton * ok_button = new QPushButton(tr("确定"));
	ok_button -> setFocusPolicy(Qt::NoFocus);
	ok_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
	connect(ok_button, SIGNAL(clicked()), this, SLOT(chkSaveDbsCollideDataes()));
	QPushButton * cancel_button = new QPushButton(tr("取消"));
	cancel_button -> setFocusPolicy(Qt::NoFocus);
	cancel_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(onCancel()));
	h_layout -> addWidget(ok_button);  
	h_layout -> addWidget(cancel_button);	 
	v_layout -> addWidget(warn_label);        
	v_layout -> addWidget(info_label);
	v_layout -> addWidget(dec_view);
	v_layout -> addLayout(h_layout);
	v_layout -> setContentsMargins(0, 0, 0, 0);  
	v_layout -> setSpacing(2);
	show();
	openMeOnAnimation();	
}

bool DiaLog::chkCollideDbsState()
{
	return r_chk;
}

void DiaLog::initHintDialog(const QString & info)
{
	QVBoxLayout * v_layout = new QVBoxLayout(this);
	QLabel * error_label = new QLabel;
	error_label -> setFixedHeight(30);
	error_label -> setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	QPixmap pixmap(":/images/lamp.png");
	QPixmap e_pix = pixmap.scaled(error_label->frameSize(), Qt::KeepAspectRatio);
	error_label -> setPixmap(e_pix);
	QString n_info = info;
	QStringList info_list = n_info.split(":");
	QLabel * info_label = new QLabel(info_list[1]);
	info_label -> setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	info_label -> setFont(QFont("Microsoft YaHei", 10));
	defineDialogLabelSize(info_label);
/*	QFontMetrics fm(info_label->font());
	int width = fm.width(info_label->text());
	info_label -> setFixedWidth(width);
	int height = fm.height()*(info_list[1].count("\n")+1);
	info_label -> setFixedHeight(height);*/
	QString radius;
	if (!info_list[1].contains("\n"))
		radius = "3px";
	else
		radius = "10px";
	QString style_sheet = QString("color: white; border: 1px solid rgb(50, 50, 50); border-radius: %1; background: rgb(50, 50, 50)").arg(radius);
	info_label -> setStyleSheet(style_sheet);
	QPushButton * ok_button = new QPushButton(tr("知道了"));
	ok_button -> setFocusPolicy(Qt::NoFocus);
	ok_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
	connect(ok_button, SIGNAL(clicked()), this, SLOT(onSure()));
	ok_button -> setFixedHeight(15);
	ok_button -> setFixedWidth(30);
	v_layout -> addWidget(error_label);        
	v_layout -> addWidget(info_label);        
	v_layout -> addWidget(ok_button);
	v_layout -> setAlignment(ok_button, Qt::AlignCenter);     
	v_layout->setSpacing(5);
	v_layout->setContentsMargins(0, 0, 0, 0);	
	show();
	openMeOnAnimation();	
}

void DiaLog::initWarningDialog(const QString & info, int type)
{
	QHBoxLayout * h_layout = new QHBoxLayout;
	QVBoxLayout * v_layout = new QVBoxLayout(this);
	QLabel * warn_label = new QLabel;
	warn_label -> setFixedHeight(30);
	warn_label -> setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	QPixmap pixmap(":/images/warn.png");
	QPixmap w_pix = pixmap.scaled(warn_label->frameSize(), Qt::KeepAspectRatio);
	warn_label -> setPixmap(w_pix);
	QString n_info = info;
	QStringList info_list = n_info.split(":");
	QLabel * info_label = new QLabel(info_list[1]);
	info_label -> setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	info_label -> setFont(QFont("宋体", 10));
	defineDialogLabelSize(info_label);
	QString radius;
	if (!info_list[1].contains("\n"))
		radius = "3px";
	else
		radius = "10px";
	QString style_sheet = QString("color: white; border: 1px solid rgb(50, 50, 50); border-radius: %1; background: rgb(50, 50, 50)").arg(radius);
	info_label -> setStyleSheet(style_sheet);
	QPushButton * ok_button = new QPushButton(tr("确定"));
	ok_button -> setFocusPolicy(Qt::NoFocus);
	ok_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
	QPushButton * cancel_button = new QPushButton(tr("取消"));
	if (type == 1)
	{
		delete cancel_button;
		delete h_layout;
	}
	else if (type == 2)
		connect(ok_button, SIGNAL(clicked()), w_inspector, SLOT(returnLastMenu()));		
	else
	{
		cancel_button -> setFocusPolicy(Qt::NoFocus);
		cancel_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
		connect(cancel_button, SIGNAL(clicked()), this, SLOT(onCancel()));
	}	
	connect(ok_button, SIGNAL(clicked()), this, SLOT(onSure()));  
	v_layout -> addWidget(warn_label);        
	v_layout -> addWidget(info_label);
	if (info.contains(tr("输入姓名密码")))
	{
		QLabel * name_label = new QLabel(tr("姓名:"));
		name_label -> setStyleSheet("QLabel{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
		QLabel * pwd_label = new QLabel(tr("密码:"));
		pwd_label -> setStyleSheet("QLabel{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
		name_label -> setFixedHeight(30);
		pwd_label -> setFixedHeight(30);	
		QLineEdit * name_line = new QLineEdit;
		connect (name_line, SIGNAL(textEdited(const QString &)), w_inspector, SLOT(dbEditorName(const QString &)));
		QLineEdit * pwd_line = new QLineEdit;
		connect (pwd_line, SIGNAL(textEdited(const QString &)), w_inspector, SLOT(dbEditorPassword(const QString &)));		
		name_line -> setFixedHeight(30);
		pwd_line -> setFixedHeight(30);
		QGridLayout * gnpl_layout = new QGridLayout;
		gnpl_layout -> addWidget(name_label, 0, 0, 1, 1, Qt::AlignJustify); 
		gnpl_layout -> addWidget(name_line, 0, 1, 1, 2, Qt::AlignJustify); 
		gnpl_layout -> addWidget(pwd_label, 1, 0, 1, 1, Qt::AlignJustify); 
		gnpl_layout -> addWidget(pwd_line, 1, 1, 1, 2, Qt::AlignJustify); 		
		v_layout -> addLayout(gnpl_layout);
	}
	if (type != 1)  
	{
		h_layout -> addWidget(ok_button);  
		h_layout -> addWidget(cancel_button);    
		v_layout -> addLayout(h_layout);
	}
	else
	{
		v_layout -> addWidget(ok_button); 
		v_layout -> setAlignment(ok_button, Qt::AlignCenter);
	}  
	v_layout -> setContentsMargins(0, 0, 0, 0);  
	v_layout -> setSpacing(2);
	show();
	openMeOnAnimation();		
}

void DiaLog::initErrorDialog(const QString & info)
{
	QVBoxLayout * v_layout = new QVBoxLayout(this);
	QLabel * error_label = new QLabel;
	error_label -> setFixedHeight(30);
	error_label -> setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	QPixmap pixmap(":/images/error.png");
	QPixmap e_pix = pixmap.scaled(error_label->frameSize(), Qt::KeepAspectRatio);
	error_label -> setPixmap(e_pix);
	QString n_info = info;
	QStringList info_list = n_info.split(":");
	QLabel * info_label = new QLabel(info_list[1]);
	info_label -> setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	info_label -> setFont(QFont("宋体", 10));
	defineDialogLabelSize(info_label);
	QString radius;
	if (!info_list[1].contains("\n"))
		radius = "3px";
	else
		radius = "10px";
	QString style_sheet = QString("color: white; border: 1px solid rgb(50, 50, 50); border-radius: %1; background: rgb(50, 50, 50)").arg(radius);
	info_label -> setStyleSheet(style_sheet);
	QHBoxLayout * h_layout = 0;
	QPushButton * ok_button = 0;
	QPushButton * cancel_button = 0;
	if (info.contains(tr("您可以按确定继续或\n按取消结束此次操作")))
	{
		ok_button = new QPushButton(tr("确定"));
		cancel_button = new QPushButton(tr("取消"));
		connect(cancel_button, SIGNAL(clicked()), this, SLOT(onCancel()));
		cancel_button -> setFocusPolicy(Qt::NoFocus);
		cancel_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
		h_layout = new QHBoxLayout;
	}
	else
	{
		ok_button = new QPushButton(tr("知道了"));
		ok_button -> setFixedHeight(15);
		ok_button -> setFixedWidth(30);		
	}
	connect(ok_button, SIGNAL(clicked()), this, SLOT(onSure()));	
	ok_button -> setFocusPolicy(Qt::NoFocus);
	ok_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
	v_layout -> addWidget(error_label);        
	v_layout -> addWidget(info_label);  
	if (cancel_button)
	{
		h_layout -> addWidget(ok_button);  
		h_layout -> addWidget(cancel_button);    
		v_layout -> addLayout(h_layout);		
	}
	else
	{     
		v_layout -> addWidget(ok_button);
		v_layout -> setAlignment(ok_button, Qt::AlignCenter); 
	}
	v_layout->setSpacing(5);
	v_layout->setContentsMargins(0, 0, 0, 0);	
	show();
	openMeOnAnimation();	
}

void DiaLog::defineDialogLabelSize(QLabel * dest_label)
{
	QString text = dest_label->text();
	if (!text.contains("\n"))
		return;
	QFontMetrics fm(dest_label->font());
	QStringList text_list = text.split("\n");
	int max_width = 0;
	for (int i = 0; i < text_list.size(); i++)
	{
		if (fm.width(text_list.at(i)) > max_width)
			max_width = fm.width(text_list.at(i));	
	}
	dest_label -> setFixedWidth(max_width+fm.height());
	int height = fm.height()*(text_list.size()+1);
	dest_label -> setFixedHeight(height);	
}

void DiaLog::openMeOnAnimation()
{
	dialog_animation = new QPropertyAnimation(this);
	connect(dialog_animation, SIGNAL(finished()), this, SLOT(killAnimation()));
	dialog_animation -> setTargetObject(this);
	dialog_animation -> setPropertyName("geometry");
	dialog_animation -> setDuration(500);
	QRect d_startRect(0, 0, 0, 0);
	QRect d_endRect;
	d_endRect.setRect(w_inspector->geometry().topRight().x()/2-width()/2, w_inspector->geometry().bottomRight().y()/2-height(), width(), height());
	dialog_animation -> setStartValue(d_startRect);
	dialog_animation -> setEndValue(d_endRect);
	connect(dialog_animation, SIGNAL(valueChanged(const QVariant &)), this, SLOT(animatedSize(const QVariant &)));
	dialog_animation -> start();		
}

void DiaLog::closeMeOnAnimation()
{
	dialog_animation = new QPropertyAnimation(this);
	connect(dialog_animation, SIGNAL(valueChanged(const QVariant &)), this, SLOT(animatedSize(const QVariant &)));
	connect(dialog_animation, SIGNAL(finished()), this, SLOT(killMyself()));	
	dialog_animation -> setTargetObject(this);
	dialog_animation -> setPropertyName("geometry");
	dialog_animation -> setDuration(500);
	QRect d_startRect;
	d_startRect.setRect(w_inspector->geometry().topRight().x()/2-width()/2, w_inspector->geometry().bottomRight().y()/2-height(), width(), height());
	QRect d_endRect(0, 0, 0, 0);
	dialog_animation -> setStartValue(d_startRect);
	dialog_animation -> setEndValue(d_endRect);
	dialog_animation -> start();
}

void DiaLog::onSure()
{
	QVBoxLayout * chk_layout = qobject_cast<QVBoxLayout *>(layout());
	QLabel * info_label = qobject_cast<QLabel *>(chk_layout->itemAt(1)->widget());
	if (info_label && info_label->text().contains(tr("输入姓名密码")))
	{
		QGridLayout * g_layout = qobject_cast<QGridLayout *>(chk_layout->itemAt(2)->layout());
		QLineEdit * n_edit = qobject_cast<QLineEdit *>(g_layout->itemAtPosition(0, 1)->widget());
		QLineEdit * pw_edit = qobject_cast<QLineEdit *>(g_layout->itemAtPosition(1, 1)->widget());
		if (n_edit->text().isEmpty() || pw_edit->text().isEmpty())
			return;
	}
	sure = true;
	closeMeOnAnimation();
}

void DiaLog::onCancel()
{
	unsure = true;
	closeMeOnAnimation();
}

void DiaLog::judgeInputTextForSave()
{
	QLayout * cur_layout = layout();
	QTextEdit * editor = qobject_cast<QTextEdit *>(cur_layout->itemAt(1)->widget());
	QString save_text = editor->toPlainText();
	if (save_text.isEmpty())
		return;
	QStringList tables = search_db->allTables();
	bool found = false;
	foreach (QString str, tables)
	{
		if (str.contains(inputor))
		{
			QStringList infoes = str.split(tr("，，。"));
			if (infoes.size()>2 && infoes.at(2)==save_text)
			{
				found = true;
				break;
			}
		}
	}
	if (found)
	{
		QLabel * label = qobject_cast<QLabel *>(cur_layout->itemAt(0)->widget());
		label -> setText(tr("文件名与数据库中重名"));
		label -> setStyleSheet("QLabel{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FF0000, stop: 0.1 #EE0000, stop: 0.49 #CD0000, stop: 0.5 #CD0000, stop: 1 #FF0000); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-height: 13px; max-height: 13px;}");
		QFontMetrics fm(label->font());
		int width = fm.width(label->text());
		label -> setFixedWidth(width+20);
		return;
	}
	emit sendSaveFileName(save_text);
	onSure();
}

void DiaLog::judgeInputTextForExport()
{
	QLayout * cur_layout = layout();
	QTextEdit * editor = qobject_cast<QTextEdit *>(cur_layout->itemAt(1)->widget());
	QString save_text = editor->toPlainText();
	if (save_text.isEmpty())
		return;
	QLabel * file_lbl = qobject_cast<QLabel *>(cur_layout->itemAt(0)->widget());
	QFileInfoList qfi_list;
	QString tol_fname;	
	if (file_lbl->text().contains(tr("图片")))
	{
		QDir image_dir(tr("/home/dapang/workstation/image文件"));
		qfi_list = image_dir.entryInfoList(QDir::Dirs | QDir::Files);
		tol_fname = save_text+".png";	
	}
	else
	{
		QDir pdf_dir(tr("/home/dapang/workstation/pdf文件"));
		qfi_list = pdf_dir.entryInfoList(QDir::Dirs | QDir::Files);	
		tol_fname = save_text+".pdf";	
	}
	foreach (QFileInfo fi, qfi_list)	
	{
		if (fi.fileName() == tol_fname)
		{
			if (file_lbl->text().contains(tr("图片")))
				file_lbl -> setText(tr("图片名与目标文件夹中重名"));
			else
				file_lbl -> setText(tr("文件名与目标文件夹中重名"));
			file_lbl -> setStyleSheet("QLabel{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FF0000, stop: 0.1 #EE0000, stop: 0.49 #CD0000, stop: 0.5 #CD0000, stop: 1 #FF0000); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-height: 13px; max-height: 13px;}");
			QFontMetrics fm(file_lbl->font());
			int width = fm.width(file_lbl->text());
			file_lbl -> setFixedWidth(width+20);
			return;
		}
	}
	emit sendSaveFileName(save_text);
	onSure();	
}

void DiaLog::chkSaveDbsCollideDataes()
{
	QLayout * cur_layout = layout();
 	LittleWidgetsView * qml_view = qobject_cast<LittleWidgetsView *>(cur_layout->itemAt(2)->widget());
	DataesTableView * save_collides = qobject_cast<DataesTableView *>(qml_view->qmlAgent()->widget());
	QString chk_str;
	r_chk = save_collides->checkInputState(chk_str);	
	if (!r_chk)
		return; 
	closeMeOnAnimation();	
}

void DiaLog::animatedSize(const QVariant & v_size)
{
	setFixedSize(v_size.toRect().size());
}

void DiaLog::resetLabelColor()
{
	QLayout * cur_layout = layout();
	QLabel * label = qobject_cast<QLabel *>(cur_layout->itemAt(0)->widget());
	label -> setStyleSheet("QLabel{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-height: 13px; max-height: 13px;}");
	if (label->text().contains(tr("图片")))
		label -> setText(tr("请输入图片名"));
	else
		label -> setText(tr("请输入文件名"));
	QFontMetrics fm(label->font());
	int width = fm.width(label->text());
	label -> setFixedWidth(width+20);			
}

void DiaLog::killAnimation()
{
	delete dialog_animation;
	dialog_animation = 0;
}

void DiaLog::killMyself()
{
	delete dialog_animation;
	if (sure)
		done(sure);
	if (unsure)
		done(!unsure);
}

