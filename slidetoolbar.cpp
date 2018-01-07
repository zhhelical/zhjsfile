#include <QtGui>
#include <QtSql/QSqlTableModel>
#include "editlongtoolsviewer.h" 
#include "mainwindow.h"
#include "slidetoolbar.h"
#include "spcdatabase.h"

SlideBar::SlideBar(QWidget * parent)
:QWidget(parent), table_commandStack(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setStyleSheet(QString("font-family: %1; font-size:12px; border-right: 2px groove gray; border-bottom: 2px groove gray; padding: 0px; color: white; background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4A708B, stop:0.66 #292929, stop:1 #080808);").arg(tr("宋体")));
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
	father = qobject_cast<MainWindow *>(parent);  
}

SlideBar::~SlideBar()
{}

void SlideBar::showMainTools(int user_class)
{
	QToolButton * loginButton = new QToolButton(this);
	loginButton -> setStyleSheet("QToolButton::pressed {background-color: #AEEEEE; }"); 
	connect(loginButton, SIGNAL(clicked()), father, SLOT(createSonLoginMenu()));
	QToolButton * fileButton = new QToolButton(this);
	fileButton -> setStyleSheet("QToolButton::pressed {background-color: #AEEEEE; }"); 
	connect(fileButton, SIGNAL(clicked()), father, SLOT(createFileToolBars()));
	QToolButton * manageButton = new QToolButton(this);
	manageButton -> setStyleSheet("QToolButton::pressed {background-color: #AEEEEE; }");
	connect(manageButton, SIGNAL(clicked()), father, SLOT(createManageToolBars()));
	QToolButton * dbsourceButton = new QToolButton(this);
	dbsourceButton -> setStyleSheet("QToolButton::pressed {background-color: #AEEEEE; }");
	connect(dbsourceButton, SIGNAL(clicked()), father, SLOT(createDbSourceTreeView()));	
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton::pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
	if (user_class < 2 && user_class > -1)
		manageButton -> setEnabled(false);
	if (user_class == -1)
	{
		loginButton -> setEnabled(false);
		fileButton -> setEnabled(false);
		manageButton -> setEnabled(false);		
	}
 	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->addWidget(defineButton(loginButton, QIcon(":/images/login.png"), tr("登录操作")));
	hbox->addWidget(defineButton(fileButton, QIcon(":/images/file.png"), tr("工程项目")));
	hbox->addWidget(defineButton(manageButton, QIcon(":/images/preset.png"), tr("属性管理")));     
	hbox->addWidget(defineButton(dbsourceButton, QIcon(":/images/dbsource.png"), tr("数据库源"))); 
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));        
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showMultiWinsEditTools()
{
	QToolButton * newButton = new QToolButton(this);
	newButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	connect(newButton, SIGNAL(clicked()), father, SLOT(userAddNewWin()));
	QToolButton * closeButton = new QToolButton(this);
	closeButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	connect(closeButton, SIGNAL(clicked()), father, SLOT(userCloseCurWin()));
	QToolButton * nextButton = new QToolButton(this);
	nextButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(nextButton, SIGNAL(clicked()), father, SLOT(userViewNextWin()));
	QToolButton * lastButton = new QToolButton(this);
	lastButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(lastButton, SIGNAL(clicked()), father, SLOT(userViewLastWin()));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
 	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->addWidget(defineButton(newButton, QIcon(":/images/newwin.png"), tr("新建窗口")));
	hbox->addWidget(defineButton(closeButton, QIcon(":/images/closewin.png"), tr("关闭当前")));
	hbox->addWidget(defineButton(nextButton, QIcon(":/images/nextwin.png"), tr("下一窗口")));
	hbox->addWidget(defineButton(lastButton, QIcon(":/images/lastwin.png"), tr("上一窗口")));       
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));        
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showSonLoginMenu(const QPair<QString, QString> & user_pair, SpcDataBase* chk_db)
{
	QToolButton * singleButton = new QToolButton(this);
	singleButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	connect(singleButton, SIGNAL(clicked()), father, SLOT(createSingleLogin()));
	QToolButton * multiButton = new QToolButton(this);
	multiButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	connect(multiButton, SIGNAL(clicked()), father, SLOT(createMultiLogin()));
	QToolButton * logoutButton = new QToolButton(this);
	logoutButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(logoutButton, SIGNAL(clicked()), father, SLOT(userLogout()));
	QToolButton * modypassButton = new QToolButton(this);
	modypassButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	modypassButton -> setEnabled(father->logingState());	
	if (chk_db)
	{
		QSqlTableModel db_sql(this, chk_db->currentConnectionDb()->database(chk_db->currentConnectionDb()->connectionName()));		
		chk_db -> varsModelFromTable("dbInformations", &db_sql);
		QString jug_str = user_pair.first+","+user_pair.second;
		if (db_sql.data(db_sql.index(0, 1)).toString() == jug_str)
			modypassButton -> setEnabled(false);  
	}
	if (modypassButton->isEnabled())
		connect(modypassButton, SIGNAL(clicked()), father, SLOT(userChangePasswd()));  
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
 	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->addWidget(defineButton(singleButton, QIcon(":/images/user.png"), tr("单用户")));
	hbox->addWidget(defineButton(multiButton, QIcon(":/images/users.png"), tr("多用户")));
	multiButton -> setEnabled(father->logingModeDefinition() | !father->logingState());
	singleButton -> setEnabled(!father->logingModeDefinition() & !father->logingState());
	logoutButton -> setEnabled(father->logingState());
	hbox->addWidget(defineButton(logoutButton, QIcon(":/images/logout.png"), tr("退出登录")));
	hbox->addWidget(defineButton(modypassButton, QIcon(":/images/modypass.png"), tr("修改密码")));       
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));        
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);
}

void SlideBar::showHelpViewMenu()
{
	QToolButton * backSpc= new QToolButton(this);
	backSpc -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	connect(backSpc, SIGNAL(clicked()), father, SLOT(helpToSpcView()));
	QToolButton * selectTree = new QToolButton(this);
	selectTree -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	selectTree -> setCheckable(true);	
	connect(selectTree, SIGNAL(toggled(bool)), father, SLOT(searchHelpOnTree(bool)));	
	QToolButton * lastsButton= new QToolButton(this);
	lastsButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	connect(lastsButton, SIGNAL(clicked()), father, SLOT(lastHelpPapers()));
	QToolButton * nextsButton = new QToolButton(this);
	nextsButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(nextsButton, SIGNAL(clicked()), father, SLOT(nextHelpPapers()));
	QToolButton * hideButton = new QToolButton(this); 
	hideButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(hideButton, SIGNAL(clicked()), father, SLOT(hideToolPressed())); 
	QToolButton * hideRwin = new QToolButton(this); 
	hideRwin -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	hideRwin -> setCheckable(true);
      	connect(hideRwin, SIGNAL(toggled(bool)), father, SLOT(hideRwinPressed(bool)));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));	
 	QHBoxLayout *hbox = new QHBoxLayout(this);	
	hbox->addWidget(defineButton(backSpc, QIcon(":/images/lastwin.png"), tr("原界面")));
	hbox->addWidget(defineButton(selectTree, QIcon(":/images/defaulttree.png"), tr("自由浏览")));      
	hbox->addWidget(defineButton(lastsButton, QIcon(":/images/last.png"), tr("上一组"))); 
	hbox->addWidget(defineButton(nextsButton, QIcon(":/images/next.png"), tr("下一组")));
	hbox->addWidget(defineButton(hideButton, QIcon(":/images/hide.png"), tr("隐藏菜单")));
	hbox->addWidget(defineButton(hideRwin, QIcon(":/images/hidecol.png"), tr("隐藏右窗")));	
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));        
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showPowersToolsMenu()
{
	QToolButton * joinButton= new QToolButton(this);
	joinButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	connect(joinButton, SIGNAL(clicked()), father, SLOT(joinProject()));
	QToolButton * empowerButton= new QToolButton(this);
	empowerButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	connect(empowerButton, SIGNAL(clicked()), father, SLOT(empowerUserForProj()));
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
 	QHBoxLayout *hbox = new QHBoxLayout(this);	
	hbox->addWidget(defineButton(joinButton, QIcon(":/images/join.png"), tr("加入工程")));
	hbox->addWidget(defineButton(empowerButton, QIcon(":/images/empower.png"), tr("授权用户")));      
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层"))); 
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));        
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);
}

void SlideBar::showLoginTools(const QString & name)
{
	QLabel * userLabel = new QLabel(tr("登录者:"), this);
	userLabel -> setFixedHeight(32);
	userLabel -> setFixedWidth(55);
	QLineEdit * user_name = new QLineEdit(this);
	user_name -> setFixedHeight(32);
	user_name -> setFixedWidth(80);
	connect(user_name, SIGNAL(textEdited(const QString &)), father, SLOT(storeLoginName(const QString &)));
	if (!name.isEmpty())
	{
		father -> storeLoginName(name);
		user_name -> setText(name);
	}
	connect(father, SIGNAL(changeNameColor(const QString &)), user_name, SLOT(setStyleSheet(const QString &)));
	connect(father, SIGNAL(lastText(const QString &)), user_name, SLOT(setText(const QString &)));
	QLabel * passWordLabel = new QLabel(tr("密码:"), this);
	passWordLabel -> setFixedHeight(32);
	passWordLabel -> setFixedWidth(55);
	QLineEdit * pass_word = new QLineEdit(this);
	pass_word -> setFixedHeight(32);
	pass_word -> setFixedWidth(80);
	connect(pass_word, SIGNAL(textEdited(const QString &)), father, SLOT(storeLoginPasswd(const QString &)));
	connect(father, SIGNAL(changePasswdColor(const QString &)), pass_word, SLOT(setStyleSheet(const QString &)));
	QToolButton * okButton= new QToolButton(this);
	okButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(okButton, SIGNAL(clicked()), father, SLOT(verifyPermission()));
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * noButton = new QToolButton(this);
	noButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(noButton, SIGNAL(clicked()), user_name, SLOT(clear()));
	connect(noButton, SIGNAL(clicked()), pass_word, SLOT(clear()));
	connect(noButton, SIGNAL(clicked()), father, SLOT(clearLogpair()));
 	QVBoxLayout *vbox = new QVBoxLayout;
 	QHBoxLayout *hbox1 = new QHBoxLayout;
 	QHBoxLayout *hbox2 = new QHBoxLayout;
 	QHBoxLayout *tol_hbox = new QHBoxLayout(this);
	hbox1->addWidget(userLabel); 
	hbox1->addWidget(user_name);        
	hbox2->addWidget(passWordLabel);
	hbox2->addWidget(pass_word);  
	vbox->addLayout(hbox1);
	vbox->addLayout(hbox2);
	tol_hbox->addLayout(vbox);
	tol_hbox->addWidget(defineButton(okButton, QIcon(":/images/ok.png"), tr("确定")));
	tol_hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层"))); 
	tol_hbox->addWidget(defineButton(noButton, QIcon(":/images/no.png"), tr("取消")));       
	tol_hbox->setSpacing(0);
	tol_hbox->setContentsMargins(0, 0, 0, 0);
}

void SlideBar::showChangePasswdTool(const QPair<QString, QString> & user_pair)
{
	QLabel * userLabel = new QLabel(tr("登录者:"), this);
	userLabel -> setFixedHeight(32);
	userLabel -> setFixedWidth(55);
	QLineEdit * user_name = new QLineEdit(this);
	user_name -> setFixedHeight(32);
	user_name -> setFixedWidth(80);
	user_name -> setText(user_pair.first);
	user_name -> setReadOnly(true);
	QLabel * passWordLabel = new QLabel(tr("改密码:"), this);
	passWordLabel -> setFixedHeight(32);
	passWordLabel -> setFixedWidth(55);
	QLineEdit * pass_word = new QLineEdit(this);
	pass_word -> setFixedHeight(32);
	pass_word -> setFixedWidth(80);
	pass_word -> setText(user_pair.second);
	connect(pass_word, SIGNAL(textEdited(const QString &)), father, SLOT(storeLoginPasswd(const QString &)));
	connect(father, SIGNAL(changePasswdColor(const QString &)), pass_word, SLOT(setStyleSheet(const QString &)));
	QToolButton * okButton= new QToolButton(this);
	okButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(okButton, SIGNAL(clicked()), father, SLOT(userNewPasswdTodb()));
	QToolButton * noButton = new QToolButton(this);
	noButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(noButton, SIGNAL(clicked()), pass_word, SLOT(clear()));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));	
 	QVBoxLayout *vbox = new QVBoxLayout;
 	QHBoxLayout *hbox1 = new QHBoxLayout;
 	QHBoxLayout *hbox2 = new QHBoxLayout;
 	QHBoxLayout *tol_hbox = new QHBoxLayout(this);
	hbox1->addWidget(userLabel); 
	hbox1->addWidget(user_name);        
	hbox2->addWidget(passWordLabel);
	hbox2->addWidget(pass_word);  
	vbox->addLayout(hbox1);
	vbox->addLayout(hbox2);
	tol_hbox->addLayout(vbox);
	tol_hbox->addWidget(defineButton(okButton, QIcon(":/images/ok.png"), tr("确定修改")));
	tol_hbox->addWidget(defineButton(noButton, QIcon(":/images/no.png"), tr("取消修改")));    
	tol_hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消"))); 	
	tol_hbox->setSpacing(0);
	tol_hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showFileTools(int user_state)
{
	QToolButton * newButton = new QToolButton(this); 
	newButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(newButton, SIGNAL(clicked()), father, SLOT(newFile()));
	QToolButton * openButton = new QToolButton(this);
	openButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(openButton, SIGNAL(clicked()), father, SLOT(openFile())); 
	QToolButton * powermanageButton= new QToolButton(this);
	powermanageButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	connect(powermanageButton, SIGNAL(clicked()), father, SLOT(createPowersTool()));             
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
	if (user_state == 0)
		openButton -> setEnabled(false);
	else if (user_state == 1)
		newButton -> setEnabled(false); 
 	QHBoxLayout *hbox = new QHBoxLayout(this); 
	hbox->addWidget(defineButton(newButton, QIcon(":/images/new.png"), tr("新建")));        
	hbox->addWidget(defineButton(openButton, QIcon(":/images/open.png"), tr("打开")));
	hbox->addWidget(defineButton(powermanageButton, QIcon(":/images/login-manager.png"), tr("授权管理")));
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));         
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showOpenfilemenus()
{
	QToolButton * dailyButton = new QToolButton(this); 
	dailyButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(dailyButton, SIGNAL(clicked()), father, SLOT(openDailyTableByButton())); 
	QToolButton * defaultButton = new QToolButton(this); 
	defaultButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(defaultButton, SIGNAL(clicked()), father, SLOT(showEngiTreeByDefault())); 
	QToolButton * specialButton = new QToolButton(this); 
	specialButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(specialButton, SIGNAL(clicked()), father, SLOT(showEngiTreeBySet())); 	
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * hideButton = new QToolButton(this); 
	hideButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(hideButton, SIGNAL(clicked()), father, SLOT(hideToolPressed())); 
	QToolButton * hideRwin = new QToolButton(this); 
	hideRwin -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	hideRwin -> setCheckable(true);
      	connect(hideRwin, SIGNAL(toggled(bool)), father, SLOT(hideRwinPressed(bool)));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
 	QHBoxLayout *hbox = new QHBoxLayout(this); 
	hbox->addWidget(defineButton(dailyButton, QIcon(":/images/daily.png"), tr("日常编辑")));
	hbox->addWidget(defineButton(defaultButton, QIcon(":/images/defaulttree.png"), tr("默认显示")));
	hbox->addWidget(defineButton(specialButton, QIcon(":/images/specialtree.png"), tr("分类显示")));	
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层"))); 
	hbox->addWidget(defineButton(hideButton, QIcon(":/images/hide.png"), tr("隐藏菜单")));
	hbox->addWidget(defineButton(hideRwin, QIcon(":/images/hidecol.png"), tr("隐藏右窗")));
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));        
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showManageTools()
{
	QToolButton * projectsButton = new QToolButton(this);
	projectsButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	projectsButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(projectsButton, SIGNAL(clicked()), father, SLOT(projectsKeysPropertys()));
	QToolButton * powerButton = new QToolButton(this);
	powerButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(powerButton, SIGNAL(clicked()), father, SLOT(setAuthority()));
	QToolButton * productsButton = new QToolButton(this);
	productsButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	productsButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(productsButton, SIGNAL(clicked()), father, SLOT(productsDefine()));
	QToolButton * dataesButton = new QToolButton(this);
	dataesButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(dataesButton, SIGNAL(clicked()), father, SLOT(dataesManagement()));
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
 	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->addWidget(defineButton(projectsButton, QIcon(":/images/property.png"), tr("工程管理")));
	hbox->addWidget(defineButton(powerButton, QIcon(":/images/authority.png"), tr("权限管理")));
	hbox->addWidget(defineButton(productsButton, QIcon(":/images/product.png"), tr("产品管理")));        
	hbox->addWidget(defineButton(dataesButton, QIcon(":/images/dataes.png"), tr("综合管理")));        
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));  
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));     
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showDataesManageTool()
{
	QToolButton * dataesButton = new QToolButton(this);
	dataesButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	dataesButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(dataesButton, SIGNAL(clicked()), father, SLOT(totalDataesMrgment()));
	QToolButton * tableChartButton = new QToolButton(this);
	tableChartButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	tableChartButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(tableChartButton, SIGNAL(clicked()), father, SLOT(plotsTolMrgment()));
	QToolButton * overallButton = new QToolButton(this);
	overallButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	overallButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(overallButton, SIGNAL(clicked()), father, SLOT(overallConstruction()));
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
 	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->addWidget(defineButton(dataesButton, QIcon(":/images/rebuilddata.png"), tr("非图综合")));
	hbox->addWidget(defineButton(tableChartButton, QIcon(":/images/chartplot.png"), tr("图示综合")));
	hbox->addWidget(defineButton(overallButton, QIcon(":/images/overall.png"), tr("综合构建")));        
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));  
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));     
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showOnlyForDataesTool()
{
	QToolButton * manualButton = new QToolButton(this);
	manualButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	manualButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(manualButton, SIGNAL(clicked()), father, SLOT(manualTblForSpec()));
	QToolButton * managementButton = new QToolButton(this);
	managementButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(managementButton, SIGNAL(clicked()), father, SLOT(manualTblForMrgmnt()));
	QToolButton * dsMergeButton = new QToolButton(this);
	dsMergeButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	dsMergeButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(dsMergeButton, SIGNAL(clicked()), father, SLOT(dataesInfoesMrgment()));	
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
 	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->addWidget(defineButton(manualButton, QIcon(":/images/tmanual.png"), tr("数据制表")));
	hbox->addWidget(defineButton(managementButton, QIcon(":/images/allmanage.png"), tr("管理制表")));  
	hbox->addWidget(defineButton(dsMergeButton, QIcon(":/images/mplots.png"), tr("数据合并")));	
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));  
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));     
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);
}

void SlideBar::showPlotsMrgTools()
{
	QToolButton * majorsButton = new QToolButton(this);
	majorsButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	majorsButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(majorsButton, SIGNAL(clicked()), father, SLOT(mainPlotsMerge()));
	QToolButton * assistsButton = new QToolButton(this);
	assistsButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	assistsButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(assistsButton, SIGNAL(clicked()), father, SLOT(assistantPlotsMerge()));
	QToolButton * allButton = new QToolButton(this);
	allButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	allButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(allButton, SIGNAL(clicked()), father, SLOT(allPlotsMerge()));
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
 	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->addWidget(defineButton(majorsButton, QIcon(":/images/plotmerge.png"), tr("主图编辑")));
	hbox->addWidget(defineButton(assistsButton, QIcon(":/images/assistant.png"), tr("辅图编辑")));
	hbox->addWidget(defineButton(allButton, QIcon(":/images/allmerge.png"), tr("主图辅图")));        
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));  
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));     
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showOverallMrgTools()
{
	QToolButton * tpMergeButton = new QToolButton(this);
	tpMergeButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	tpMergeButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(tpMergeButton, SIGNAL(clicked()), father, SLOT(dataesInfoesPlotsMrgment()));
	QToolButton * freeButton = new QToolButton(this);
	freeButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	freeButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(freeButton, SIGNAL(clicked()), father, SLOT(freeConstructionAct()));
	QToolButton * backupButton = new QToolButton(this);
	backupButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	backupButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(backupButton, SIGNAL(clicked()), father, SLOT(backupTreeViewAction()));	
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * cancelButton = new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
 	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->addWidget(defineButton(tpMergeButton, QIcon(":/images/tpmerge.png"), tr("图表合并"))); 
	hbox->addWidget(defineButton(freeButton, QIcon(":/images/freecons.png"), tr("分类构建"))); 
	hbox->addWidget(defineButton(backupButton, QIcon(":/images/backup.png"), tr("综合备份")));  	
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));  
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));     
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showEngiPropertys()
{
	QToolButton * keyButton= new QToolButton(this);
	keyButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(keyButton, SIGNAL(clicked()), father, SLOT(showProjectsKeyTable()));
	QToolButton * updateButton= new QToolButton(this);
	updateButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(updateButton, SIGNAL(clicked()), father, SLOT(improveExistedProjects()));	
	QToolButton * attriButton= new QToolButton(this);
	attriButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(attriButton, SIGNAL(clicked()), father, SLOT(projsPropertys()));
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * cancelButton= new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed()));
 	QHBoxLayout *hbox = new QHBoxLayout(this); 
	hbox->addWidget(defineButton(keyButton, QIcon(":/images/keydata.png"), tr("关键数据")));
	hbox->addWidget(defineButton(updateButton, QIcon(":/images/update.png"), tr("优化完善")));	
	hbox->addWidget(defineButton(attriButton, QIcon(":/images/attribution.png"), tr("属性数据")));
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消"))); 	
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);
}

void SlideBar::showProjsPromotionTools()
{
	QToolButton * improveButton = new QToolButton(this);
	improveButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	improveButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(improveButton, SIGNAL(clicked()), father, SLOT(savePromotingResultation()));	
	QToolButton * editButton= new QToolButton(this);
	editButton -> setCheckable(true);
	editButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(editButton, SIGNAL(clicked()), father, SLOT(projsPromotingAction()));	
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * hideButton= new QToolButton(this); 
	hideButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(hideButton, SIGNAL(clicked()), father, SLOT(hideToolPressed())); 
	QToolButton * hideRwin = new QToolButton(this); 
	hideRwin -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	hideRwin -> setCheckable(true);
      	connect(hideRwin, SIGNAL(toggled(bool)), father, SLOT(hideRwinPressed(bool)));
	QToolButton * cancelButton= new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed())); 
	QHBoxLayout *hbox = new QHBoxLayout(this); 
	hbox->addWidget(defineButton(improveButton, QIcon(":/images/update.png"), tr("完成优化")));	
	hbox->addWidget(defineButton(editButton, QIcon(":/images/edit.png"), tr("开始编辑")));	
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));
	hbox->addWidget(defineButton(hideButton, QIcon(":/images/hide.png"), tr("隐藏菜单")));
	hbox->addWidget(defineButton(hideRwin, QIcon(":/images/hidecol.png"), tr("隐藏右窗")));
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));	       
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);  	
}

void SlideBar::showImproveTools(QToolButton * g_bar)
{
 	QHBoxLayout *hbox = new QHBoxLayout(this); 
	if (g_bar)
		hbox->addWidget(g_bar);	
	QToolButton * undoButton = new QToolButton(this);
	undoButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	QToolButton * redoButton = new QToolButton(this);
	redoButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	QAction * undo_act = 0;
	QAction * redo_act = 0;
	if (g_bar && g_bar->text() == tr("编辑样本"))
	{
		undo_act = table_commandStack->createUndoAction(this, tr("撤销删除"));
		redo_act = table_commandStack->createRedoAction(this, tr("重做删除"));		  
		QToolButton * lastnewButton= new QToolButton(this);
		lastnewButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(lastnewButton, SIGNAL(clicked()), father, SLOT(turnDbLastProject()));	
		QToolButton * nextnewButton= new QToolButton(this);
		nextnewButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(nextnewButton, SIGNAL(clicked()), father, SLOT(turnDbNextProject()));
		QToolButton * addtestButton= new QToolButton(this);
		addtestButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(addtestButton, SIGNAL(clicked()), father, SLOT(newTestForEngiVersion()));		
		QToolButton * delprojButton= new QToolButton(this);
		delprojButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(delprojButton, SIGNAL(clicked()), father, SLOT(deleteDbProject()));			
		hbox->addWidget(defineButton(lastnewButton, QIcon(":/images/last.png"), tr("上个测试"))); 
		hbox->addWidget(defineButton(nextnewButton, QIcon(":/images/next.png"), tr("下个测试")));	
		hbox->addWidget(defineButton(addtestButton, QIcon(":/images/newtest.png"), tr("新增测试")));
		hbox->addWidget(defineButton(delprojButton, QIcon(":/images/pdelete.png"), tr("删除测试")));			
	}
	else
	{	
		undo_act = table_commandStack->createUndoAction(this, tr("撤销输入"));
		redo_act = table_commandStack->createRedoAction(this, tr("重做输入"));	  
		QToolButton * lastsamButton= new QToolButton(this);
		lastsamButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(lastsamButton, SIGNAL(clicked()), father, SLOT(showLastDbEngiDataes())); 		  
		QToolButton * nextsamButton= new QToolButton(this);
		nextsamButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(nextsamButton, SIGNAL(clicked()), father, SLOT(showNextDbEngiDataes()));
		QToolButton * copyButton= new QToolButton(this);
		copyButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(copyButton, SIGNAL(clicked()), father, SLOT(copyCurrentInitDataes())); 		  
		QToolButton * pasteButton= new QToolButton(this);
		pasteButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(pasteButton, SIGNAL(clicked()), father, SLOT(pasteCopiedInitDataes()));				
		QToolButton * semiplotsButton= new QToolButton(this);
		semiplotsButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		semiplotsButton -> setCheckable(true);			
		connect(semiplotsButton, SIGNAL(toggled(bool)), father, SLOT(engiInitAuxiliaryGraph(bool))); 	
		QToolButton * calButton= new QToolButton(this);
		calButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		calButton -> setCheckable(true);			
		connect(calButton, SIGNAL(toggled(bool)), father, SLOT(calForEngiInitDataes(bool))); 
		QToolButton * delsamButton= new QToolButton(this);
		delsamButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(delsamButton, SIGNAL(clicked()), father, SLOT(deleteDbProjOneSamples()));			
		QToolButton * freezeButton= new QToolButton(this);
		freezeButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
//		connect(freezeButton, SIGNAL(clicked()), father, SLOT(calForEngiInitDataes())); 				
		hbox->addWidget(defineButton(lastsamButton, QIcon(":/images/datainfo.png"), tr("上个样本")));	
		hbox->addWidget(defineButton(nextsamButton, QIcon(":/images/datanext.png"), tr("下个样本")));
		hbox->addWidget(defineButton(copyButton, QIcon(":/images/copy.png"), tr("复制样本")));	
		hbox->addWidget(defineButton(pasteButton, QIcon(":/images/paste.png"), tr("粘贴样本")));			
		hbox->addWidget(defineButton(semiplotsButton, QIcon(":/images/semiplots.png"), tr("显示辅图")));	
		hbox->addWidget(defineButton(calButton, QIcon(":/images/calculator.png"), tr("计算结果")));	
		hbox->addWidget(defineButton(delsamButton, QIcon(":/images/delete.png"), tr("删除样本")));
		hbox->addWidget(defineButton(freezeButton, QIcon(":/images/freezing.png"), tr("冻结滑动")));			
	}
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * hideButton= new QToolButton(this); 
	hideButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(hideButton, SIGNAL(clicked()), father, SLOT(hideToolPressed())); 
	QToolButton * hideRwin = new QToolButton(this); 
	hideRwin -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	hideRwin -> setCheckable(true);
      	connect(hideRwin, SIGNAL(toggled(bool)), father, SLOT(hideRwinPressed(bool)));
	QToolButton * cancelButton= new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed())); 
	undo_act -> setIcon(QIcon(":/images/undo.png"));
	undoButton -> setDefaultAction(undo_act);
	undoButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	undoButton -> setFixedHeight(64);
	undoButton -> setFixedWidth(55);
	undoButton -> setIconSize(QSize(40, 40));
	connect(table_commandStack, SIGNAL(canUndoChanged(bool)), undoButton, SLOT(setEnabled(bool)));					
	redo_act -> setIcon(QIcon(":/images/redo.png"));
	redoButton -> setDefaultAction(redo_act);
	redoButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	redoButton -> setFixedHeight(64);
	redoButton -> setFixedWidth(55);
	redoButton -> setIconSize(QSize(40, 40));
	connect(table_commandStack, SIGNAL(canRedoChanged(bool)), redoButton, SLOT(setEnabled(bool)));	
	hbox->addWidget(undoButton);     
	hbox->addWidget(redoButton);	
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));
	hbox->addWidget(defineButton(hideButton, QIcon(":/images/hide.png"), tr("隐藏菜单")));
	hbox->addWidget(defineButton(hideRwin, QIcon(":/images/hidecol.png"), tr("隐藏右窗")));	
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));	
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);	
}

void SlideBar::showOkCancelBar(QToolButton * firstbar)
{
 	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->addWidget(firstbar);
	if (firstbar->text() != tr("关键数据") && firstbar->text() != tr("输入"))
	{
		QToolButton * undoButton = new QToolButton(this);
		undoButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		QToolButton * redoButton = new QToolButton(this);
		redoButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");		
		QAction * undo_act;
		QAction * redo_act;		
		if (firstbar->text()==tr("新建样本") || firstbar->text()==tr("进入点图"))
		{
			undo_act = table_commandStack->createUndoAction(this, tr("撤销输入"));
			redo_act = table_commandStack->createRedoAction(this, tr("重做输入"));
		}
		else  
		{
			undo_act = table_commandStack->createUndoAction(this, tr("撤销"));
			redo_act = table_commandStack->createRedoAction(this, tr("重做"));
		}
		undo_act -> setIcon(QIcon(":/images/undo.png"));
		undoButton -> setDefaultAction(undo_act);
		undoButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		undoButton -> setFixedHeight(64);
		undoButton -> setFixedWidth(55);
		undoButton -> setIconSize(QSize(40, 40));
		connect(table_commandStack, SIGNAL(canUndoChanged(bool)), undoButton, SLOT(setEnabled(bool)));					
		redo_act -> setIcon(QIcon(":/images/redo.png"));
		redoButton -> setDefaultAction(redo_act);
		redoButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		redoButton -> setFixedHeight(64);
		redoButton -> setFixedWidth(55);
		redoButton -> setIconSize(QSize(40, 40));
		connect(table_commandStack, SIGNAL(canRedoChanged(bool)), redoButton, SLOT(setEnabled(bool)));
		hbox->addWidget(undoButton);        
		hbox->addWidget(redoButton);
	}
	if (firstbar->text()==tr("编辑样本") || firstbar->text()==tr("进入点图"))
	{
		QToolButton * saveButton= new QToolButton(this);
		saveButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	      	connect(saveButton, SIGNAL(clicked()), father, SLOT(saveProjInitPerformance())); 
		hbox->addWidget(defineButton(saveButton, QIcon(":/images/dsave.png"), tr("保存当前"))); 
		if (firstbar->text() == tr("编辑样本"))
		{			  
			QToolButton * lastnewButton= new QToolButton(this);
			lastnewButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
			connect(lastnewButton, SIGNAL(clicked()), father, SLOT(turnToLastNewProject()));	
			QToolButton * nextnewButton= new QToolButton(this);
			nextnewButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
			connect(nextnewButton, SIGNAL(clicked()), father, SLOT(turnToNextNewProject()));	
			QToolButton * delprojButton= new QToolButton(this);
			delprojButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
			connect(delprojButton, SIGNAL(clicked()), father, SLOT(deleteNewProject()));				
			hbox->addWidget(defineButton(lastnewButton, QIcon(":/images/lastnew.png"), tr("上个新建"))); 
			hbox->addWidget(defineButton(nextnewButton, QIcon(":/images/nextnew.png"), tr("下个新建")));	
			hbox->addWidget(defineButton(delprojButton, QIcon(":/images/pdelete.png"), tr("删除新建")));		
		}
		else
		{	  
			QToolButton * lastsamButton= new QToolButton(this);
			lastsamButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
			connect(lastsamButton, SIGNAL(clicked()), father, SLOT(showLastEngiInitDataes())); 		  
			QToolButton * nextsamButton= new QToolButton(this);
			nextsamButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
			connect(nextsamButton, SIGNAL(clicked()), father, SLOT(showNextEngiInitDataes()));
			QToolButton * copyButton= new QToolButton(this);
			copyButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
			connect(copyButton, SIGNAL(clicked()), father, SLOT(copyCurrentInitDataes())); 		  
			QToolButton * pasteButton= new QToolButton(this);
			pasteButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
			connect(pasteButton, SIGNAL(clicked()), father, SLOT(pasteCopiedInitDataes()));				
			QToolButton * semiplotsButton= new QToolButton(this);
			semiplotsButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
			semiplotsButton -> setCheckable(true);			
			connect(semiplotsButton, SIGNAL(toggled(bool)), father, SLOT(engiInitAuxiliaryGraph(bool))); 	
			QToolButton * calButton= new QToolButton(this);
			calButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
			calButton -> setCheckable(true);			
			connect(calButton, SIGNAL(toggled(bool)), father, SLOT(calForEngiInitDataes(bool))); 
			QToolButton * delsamButton= new QToolButton(this);
			delsamButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
			connect(delsamButton, SIGNAL(clicked()), father, SLOT(deleteNewProjOneSamples()));			
			QToolButton * freezeButton= new QToolButton(this);
			freezeButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
//			connect(freezeButton, SIGNAL(clicked()), father, SLOT(calForEngiInitDataes())); 				
			hbox->addWidget(defineButton(lastsamButton, QIcon(":/images/datainfo.png"), tr("上个样本")));	
			hbox->addWidget(defineButton(nextsamButton, QIcon(":/images/datanext.png"), tr("下个样本")));
			hbox->addWidget(defineButton(copyButton, QIcon(":/images/copy.png"), tr("复制样本")));	
			hbox->addWidget(defineButton(pasteButton, QIcon(":/images/paste.png"), tr("粘贴样本")));			
			hbox->addWidget(defineButton(semiplotsButton, QIcon(":/images/semiplots.png"), tr("显示辅图")));	
			hbox->addWidget(defineButton(calButton, QIcon(":/images/calculator.png"), tr("计算结果")));	
			hbox->addWidget(defineButton(delsamButton, QIcon(":/images/delete.png"), tr("删除样本")));
			hbox->addWidget(defineButton(freezeButton, QIcon(":/images/freezing.png"), tr("冻结滑动")));			
		}
	}         
	else if (firstbar->text() == tr("输入"))	
	{
		QToolButton * calButton= new QToolButton(this);
		calButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(calButton, SIGNAL(clicked()), father, SLOT(calForDailydata()));
		QToolButton * okButton= new QToolButton(this);
		okButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(okButton, SIGNAL(clicked()), father, SLOT(dailyDataesInSure()));
		hbox->addWidget(defineButton(calButton, QIcon(":/images/calculator.png"), tr("计算")));
		hbox->addWidget(defineButton(okButton, QIcon(":/images/save.png"), tr("保存")));		
	}
	else if (firstbar->text()==tr("确定更改") || firstbar->text()==tr("加入工程") || firstbar->text()==tr("授权用户") || firstbar->text()==tr("确定权限") || firstbar->text()==tr("确定产品") || firstbar->text()==tr("确定属性") || firstbar->text()==tr("确定备份"))
	{
		QToolButton * editButton = new QToolButton(this);
		editButton -> setCheckable(true);
		editButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(editButton, SIGNAL(toggled(bool)), father, SLOT(relatedEditting(bool)));
		hbox->addWidget(defineButton(editButton, QIcon(":/images/edit.png"), tr("开始编辑")));		
	}
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * hideButton= new QToolButton(this); 
	hideButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(hideButton, SIGNAL(clicked()), father, SLOT(hideToolPressed())); 
	QToolButton * hideRwin = new QToolButton(this); 
	hideRwin -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	hideRwin -> setCheckable(true);
      	connect(hideRwin, SIGNAL(toggled(bool)), father, SLOT(hideRwinPressed(bool)));
	QToolButton * cancelButton= new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed())); 
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));
	if (firstbar->text() != tr("确定更改"))
		hbox->addWidget(defineButton(hideButton, QIcon(":/images/hide.png"), tr("隐藏菜单")));
	else
		delete hideButton;
	hbox->addWidget(defineButton(hideRwin, QIcon(":/images/hidecol.png"), tr("隐藏右窗")));	
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));	
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);
}

void SlideBar::showFreeConstructTool()
{
 	QHBoxLayout * hbox = new QHBoxLayout(this);  
	QToolButton * undoButton = new QToolButton(this);
	undoButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	QAction * undo_act = table_commandStack->createUndoAction(this, tr("撤销"));
	undo_act -> setIcon(QIcon(":/images/undo.png"));
	undoButton -> setDefaultAction(undo_act);
	undoButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	undoButton -> setFixedHeight(64);
	undoButton -> setFixedWidth(55);
	undoButton -> setIconSize(QSize(40, 40));
	connect(table_commandStack, SIGNAL(canUndoChanged(bool)), undoButton, SLOT(setEnabled(bool)));
	QToolButton * redoButton = new QToolButton(this);
	redoButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	QAction * redo_act = table_commandStack->createRedoAction(this, tr("重做"));
	redo_act -> setIcon(QIcon(":/images/redo.png"));
	redoButton -> setDefaultAction(redo_act);
	redoButton -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	redoButton -> setFixedHeight(64);
	redoButton -> setFixedWidth(55);
	redoButton -> setIconSize(QSize(40, 40));
	connect(table_commandStack, SIGNAL(canRedoChanged(bool)), redoButton, SLOT(setEnabled(bool)));  
	hbox->addWidget(undoButton);        
	hbox->addWidget(redoButton);	
	QToolButton * editButton = new QToolButton(this);
	editButton -> setCheckable(true);
	editButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(editButton, SIGNAL(toggled(bool)), father, SLOT(relatedEditting(bool)));	
	QToolButton * saveall = new QToolButton(this);
	saveall -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }"); 
	connect(saveall, SIGNAL(clicked()), father, SLOT(saveAllFreeConstruction()));
	QToolButton * returnButton = new QToolButton(this);
	returnButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(returnButton, SIGNAL(clicked()), father, SLOT(returnLastMenu()));
	QToolButton * hideButton= new QToolButton(this); 
	hideButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
      	connect(hideButton, SIGNAL(clicked()), father, SLOT(hideToolPressed())); 
	QToolButton * hideRwin = new QToolButton(this); 
	hideRwin -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	hideRwin -> setCheckable(true);
      	connect(hideRwin, SIGNAL(toggled(bool)), father, SLOT(hideRwinPressed(bool)));
	QToolButton * cancelButton= new QToolButton(this);
	cancelButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
	connect(cancelButton, SIGNAL(clicked()), father, SLOT(cancelToolPressed())); 
	hbox->addWidget(defineButton(editButton, QIcon(":/images/edit.png"), tr("开始编辑")));	
	hbox->addWidget(defineButton(saveall, QIcon(":/images/saveall.png"), tr("保存构建")));
	hbox->addWidget(defineButton(returnButton, QIcon(":/images/return.png"), tr("返回上层")));
	hbox->addWidget(defineButton(hideButton, QIcon(":/images/hide.png"), tr("隐藏菜单")));
	hbox->addWidget(defineButton(hideRwin, QIcon(":/images/hidecol.png"), tr("隐藏右窗")));
	hbox->addWidget(defineButton(cancelButton, QIcon(":/images/cancel.png"), tr("取消")));	       
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0); 
}

void SlideBar::rearrangeLayoutBtns(bool lay_order)//only for 辅图工具
{
	QBoxLayout * menu_layout = qobject_cast<QBoxLayout *>(layout());
	int auth_pos = 0;
	for (int i = 0; i < menu_layout->count(); i++)
	{
		QToolButton * chk_button = qobject_cast<QToolButton *>(menu_layout->itemAt(i)->widget());
		if (chk_button->text() == tr("显示辅图"))
		{
			auth_pos = i;
			break;
		}
	}
	QToolButton * pos_button = qobject_cast<QToolButton *>(menu_layout->itemAt(auth_pos)->widget());
	pos_button -> setDown(lay_order);
	if (lay_order)
	{
		QToolButton * histogramButton= new QToolButton(this);
		histogramButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(histogramButton, SIGNAL(clicked()), father, SLOT(histogramGenerationForInitProj())); 
		QToolButton * gaussButton= new QToolButton(this);
		gaussButton -> setStyleSheet("QToolButton:pressed {background-color: #AEEEEE; }");
		connect(gaussButton, SIGNAL(clicked()), father, SLOT(gaussPlotGenerationForInitProj())); 		
		menu_layout -> insertWidget(auth_pos+1, defineButton(gaussButton, QIcon(":/images/linear.svg"), tr("概率纸")));		
		menu_layout -> insertWidget(auth_pos+1, defineButton(histogramButton, QIcon(":/images/plot.png"), tr("直方图"))); 
	}
	else
	{ 
		QLayoutItem * del_item = menu_layout->takeAt(auth_pos+1);	  
		delete del_item->widget();	
		delete del_item;	
		del_item = menu_layout->takeAt(auth_pos+1);
		delete del_item->widget();
		delete del_item;
	}
	resize(menu_layout->count()*pos_button->width(), pos_button->height());
}

void SlideBar::setCommandStack(QUndoStack * stack)
{
 	if (table_commandStack && table_commandStack!=stack) 
	{
		QBoxLayout * menu_layout = qobject_cast<QBoxLayout *>(layout());
		int tools = menu_layout->count();
		for (int i = 0; i < tools; i++)
		{
			QToolButton * e_button = qobject_cast<QToolButton *>(menu_layout->itemAt(i)->widget());
			if (e_button->text().contains(tr("撤销")) || e_button->text().contains(tr("重做")))
			{
				disconnect(table_commandStack, SIGNAL(canUndoChanged(bool)), e_button, SLOT(setEnabled(bool)));
				delete e_button->defaultAction();				  
				if (e_button->text().contains(tr("撤销")))
				{				
					QAction * undo_act;
					if (stack->objectName() == "inputComm")					
						undo_act = stack->createUndoAction(this, tr("撤销输入"));
					else
						undo_act = stack->createUndoAction(this, tr("撤销删除"));					  
					undo_act -> setIcon(QIcon(":/images/undo.png"));
					e_button -> setDefaultAction(undo_act);
					connect(stack, SIGNAL(canUndoChanged(bool)), e_button, SLOT(setEnabled(bool)));
				}
				else
				{
					QAction * redo_act;
					if (stack->objectName() == "inputComm")					
						redo_act = stack->createRedoAction(this, tr("重做输入"));
					else
						redo_act = stack->createRedoAction(this, tr("重做删除"));					  
					redo_act -> setIcon(QIcon(":/images/redo.png"));
					e_button -> setDefaultAction(redo_act);
					connect(stack, SIGNAL(canRedoChanged(bool)), e_button, SLOT(setEnabled(bool)));
					break;
				}				
			}
		}
	}
	table_commandStack = stack;
}

QUndoStack * SlideBar::currentCommand()
{
	return table_commandStack;
}

void SlideBar::sigFromOkCancel()
{
//	father -> createEditToolBars(this);
}

void SlideBar::setSure()
{
	sure_not = true;
	if (m_loop)
		m_loop -> exit();
}
	
void SlideBar::setFalse()
{
	sure_not = false;
	if (m_loop)
		m_loop -> exit();
}

QToolButton * SlideBar::defineButton(QToolButton * des, QIcon icon, QString text)
{
	des -> setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	des -> setIcon(icon);
	des -> setText(text);
	des -> setIconSize(QSize(40, 40));
	des -> setFixedHeight(64);
	des -> setFixedWidth(55);
	return des;		
}

