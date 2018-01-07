#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include<QtGui>
#include<QtCore>

class LittleWidgetsView;
class EditLongToolsViewer;
class TableManagement;
class SpcDataBase;
class SpcNum;
class DataesTableView;
class MainWindow : public QFrame
{
	Q_OBJECT
public:
	MainWindow(QWidget * parent = 0);
	~MainWindow();
	bool logingState();
	bool logingModeDefinition();
	SpcDataBase * curBackGuider();
	QWidget * curToolBar();

signals:
	void changePasswdColor(const QString & new_color);
	void changeNameColor(const QString & new_color);
	void lastText(const QString & old_text);
	void softwareFirstUsed(bool first);
	void newTheme(const QString & newText);
	void openingUser(bool owned);
	void saveProgressforwarded(int forwards);
	void closeSystem();

public slots:
	void createMainToolBars();//no about left right widgets actions
	void createDbSourceTreeView();	
	void usrOwnedMultiWinsTools();
	void okToolPressed();
	void hideToolPressed();
	void hideRwinPressed(bool down);
	void relatedEditting(bool down);
	void savePromotingResultation();
	void projsPromotingAction();
	void cancelToolPressed();
	void createSonLoginMenu();
	void createPowersTool();//no about left right widgets actions
	void joinProject();
	void empowerUserForProj();
	void createSingleLogin();
	void createMultiLogin();
	void verifyPermission();
	void userLogout();
	void userChangePasswd();
	void userNewPasswdTodb();
	void helpToSpcView();
	void searchHelpOnTree(bool on);
	void lastHelpPapers();
	void nextHelpPapers();
	void createFileToolBars();//has about left right widgets actions
	void userAddNewWin();
	void userCloseCurWin();
	void userViewNextWin();
	void userViewLastWin();
	void newFile();
	void openFile();
	void createManageToolBars();//has about left right widgets actions
	void backToNewFileState();
	void storeLoginName(const QString & name);
	void storeLoginPasswd(const QString & passwd);
	void clearLogpair();
	void showProjectsKeyTable();
	void improveExistedProjects();
	void projsPropertys();
	void helpContent();
	void returnLastMenu();
	void dailyDataesInSure();
	void openDailyTableByButton();
	void showEngiTreeByDefault();
	void showEngiTreeBySet();
	void setAuthority();
	void projectsKeysPropertys();
	void productsDefine();
	void dataesManagement();
	void totalDataesMrgment();
	void plotsTolMrgment();
	void dataesInfoesMrgment();
	void dataesInfoesPlotsMrgment();
	void freeConstructionAct();
	void backupTreeViewAction();
	void overallConstruction();
	void mainPlotsMerge(); // plots merge
	void assistantPlotsMerge();
	void allPlotsMerge();
	void plotsPrepareForUnfinishedEngi();
	void showLastEngiInitDataes();
	void showNextEngiInitDataes();
	void copyCurrentInitDataes();
	void pasteCopiedInitDataes();
	void deleteNewProjOneSamples();
	void turnToLastNewProject();
	void turnToNextNewProject();
	void deleteNewProject();
	void showLastDbEngiDataes();
	void showNextDbEngiDataes();
	void deleteDbProjOneSamples();		
	void turnDbLastProject();
	void turnDbNextProject();
	void newTestForEngiVersion();
	void deleteDbProject();
	void engiInitAuxiliaryGraph(bool down);
	void histogramGenerationForInitProj();
	void gaussPlotGenerationForInitProj();
	void calForEngiInitDataes(bool calculating);
	void calForDailydata();
	void setProtectTimer(int msec);
	void setNoProtect();
	void setAutoProtect();
	void setManualProtect();
	void initUserState();
	void startProtectionMode();
	void errorToRestartSys();
	void manualTblForSpec();
	void manualTblForMrgmnt();
	void saveAllFreeConstruction();
	void dbEditorName(const QString & n_text);
	void dbEditorPassword(const QString & pwd_text);
	bool saveProjInitPerformance();	

protected:
	void resizeEvent(QResizeEvent *event);

private slots:
	void clearBarsAnimation(QObject * obj_dest);
	void clearWidgetsAnimation(QObject * obj_dest);
	void showCurSpcCalRes(QString & t_proj, const QList<double> & show_caleds);
	void improveEngiSamples();
	void refreshTableSize(DataesTableView * desTable);
	void slideMatchResources(QString dest, bool s_h);
	void intervalTimer(int ellap_time);
	void warningTimeOut();
	void stopQmlEditorFunc(QString detail);
	void dataesFailedSetIndb();
	void setWinsMovingFinished();
	void prepairForDBsaveWork(bool save_del);
	void receiveSaveFileName(const QString & r_name);
	void showManualTableFromDB(TableManagement * sending, LittleWidgetsView * trans_viewer, bool to_direct);
	void initDbAndSpcCalculation(const QString & db_file);	
	void clearCpkProcessWidgets();
	void finishProcessProtection();
	void toggleCurrentBarButton(const QString & b_lbl, const QString & toggle_hint, bool state, QWidget * w_bar = 0);	
	void promotionEdittingOrderTrans(QUndoStack * comm);
	void showDigitalTime();

private: 
	void initEnvireForLogedUser();
	void initEnvireForLogingUser();
	void transWidgetForLoging(bool stored_state);
	void createLoginToolbar(QWidget * father, const QString & login);//no about left right widgets actions
	void createOkCancelBars(QWidget * last, const QString & hint = QString(), QUndoStack * with_comm = 0);
	void animateToolBarLeftRight(QWidget * parent, QWidget * child, bool close_hide = true);
	void animateToolBarWidgetUpDown(QWidget * old_bar, QWidget * left_widget, QWidget * new_bar, QWidget * right_widget, bool hide_close = true);
	void slideWidgetView(QWidget * leftW, QWidget * rightW, int show_type = 2); 
	void slideWholeWidgets(QWidget * old_leftW, QWidget * new_leftW, bool del_not);
	void slideLeftQmlWidgets(QWidget * leftW1, QWidget * leftW2, QWidget * rightW, bool direction, bool del = false);
	void slideRightWidgets(QWidget * leftW, QWidget * rightW1, QWidget * rightW2, bool del = false);
	void slideMultiWidgets(QWidget * new_left, QWidget * new_right, QWidget * old_left, QWidget * old_right, bool del_not, bool left_right);
	void embededOrSlideWidgetOnRightQml(QWidget * embeded);
	void initManualTableConstruct(QStandardItemModel * dest_model, const QString & further);	
	void syncNewWinWithHash(const QPair<QString, QString> user, QWidget * newLeft, QWidget * newRight = 0);
	void resetToolBarsVarsValue(QWidget * bar, bool show_hide);
	void storeRalatedEdittingBar(QWidget * ld_widget, QWidget * r_widget, QWidget * bar);
	void clearWidgetOnHash(const QPair<QString, QString> user, QWidget * w_clear);	
	void clearRalatedEdittingBarOnHash(QWidget * bar);
	void stopProtectionMode(bool new_old);
	void clearVariantsBackToOriginalState();
	void adjacentShowingWidgets(const QPair<QString, QString> & owner, bool next_last, bool close_last);
	void testEngiPromotionTransition(bool direction);
	void testEngiSampPromotionTransition(bool direction);
	void setProtectWidgetBackground(QWidget * bk_widget);
	bool syncCurrentWinWithHash(QWidget * leftW, QWidget * rightW, const QPair<QString, QString> & viewer);	
	bool projectsEmpty();
	bool powerListsEmpty();
	bool usernameInLogingPairs();
	bool newComerSameInLogingPairs(const QPair<QString, QString> & new_comer);
	bool newUserDifWithLoginger();
	bool canTransOtherWinForUsr(const QPair<QString, QString> & user);
	bool allManageProjsReadOnly();
	bool projectsEditting();
	bool projectOpenCheck(const QString & chk_proj);
	QWidget * storedBarOnHash(QWidget * ld_widget, QWidget * r_widget);
	QStandardItemModel * setManualTableModel(int row, int colummn);
	QString checkDBfile();		
	QPair<QWidget *, QWidget *> usrOwnedShowingWidgets(const QPair<QString, QString> & owner);
	bool user_mode;
	bool protect_mode;
	bool wins_moving;
	int sys_userState; //tool setting for different user
	int protect_state;
	int protect_time;
	QList<QPair<QString, QString> > loging_pairs;
	QPair<QString, QString> loging_user;
	QPair<QString, QString> new_login;
	QWidget * showing_toolBar;
//	SlideBar * help_toolBar;// no finished			
	QString save_name;	
	LittleWidgetsView * leftw_container;//0,1,2,3,4
	LittleWidgetsView * rightw_container;
	QWidget * protect_widget;
	QHash<QPair<QString, QString>, QWidgetList> ws_cpktests;	
	QHash<QWidget *, QList<QPair<QWidget *, QWidget *> > > stored_bars;	
	QHash<QPair<QString, QString>, QList<QPair<QWidget *, QWidget *> > > stored_widgets; 
	QLCDNumber * digital_clock;
	QTimer * protect_timer;
	QPropertyAnimation * l_animation1;
	QPropertyAnimation * l_animation2;
	QPropertyAnimation * l_animation3;
	QPropertyAnimation * l_animation4;	
	QPropertyAnimation * r_animation1;
	QPropertyAnimation * r_animation2;
	QPropertyAnimation * r_animation3;
	QPropertyAnimation * r_animation4;	
	QParallelAnimationGroup * v1_group;
	QParallelAnimationGroup * v2_group;
	QParallelAnimationGroup * v3_group;
	QParallelAnimationGroup * v4_group;	
	QParallelAnimationGroup * s_group;
	QPropertyAnimation * p_animation;
	QPropertyAnimation * c_animation;
	QSequentialAnimationGroup * e_group;
	SpcNum * back_science;
	SpcDataBase * base_db;	
};
      
#endif
