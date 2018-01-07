#ifndef SLIDETOOLBAR_H
#define SLIDETOOLBAR_H
#include <QtGui>
#include <QtCore>

class EditLongToolsViewer;
class MainWindow;
class SpcDataBase;
class SlideBar : public QWidget
{
	Q_OBJECT
public:
	explicit SlideBar(QWidget * parent = 0);
	~SlideBar();
	void showMainTools(int user_class);
	void showMultiWinsEditTools();
	void showSonLoginMenu(const QPair<QString, QString> & user_pair, SpcDataBase* chk_db = 0);
	void showHelpViewMenu();
	void showPowersToolsMenu();
	void showLoginTools(const QString & name);
	void showChangePasswdTool(const QPair<QString, QString> & user_pair);
	void showFileTools(int user_state);
	void showOpenfilemenus();
	void showManageTools();
	void showDataesManageTool();
	void showOnlyForDataesTool();
	void showPlotsMrgTools();
	void showOverallMrgTools();
	void showEngiPropertys();
	void showProjsPromotionTools();
	void showImproveTools(QToolButton * g_bar);
	void showOkCancelBar(QToolButton * firstbar);
	void showFreeConstructTool();
	void rearrangeLayoutBtns(bool lay_order);	
	void setCommandStack(QUndoStack * stack);
	QUndoStack * currentCommand();

private slots:
	void sigFromOkCancel(); 
	void setSure();
	void setFalse();

private:
	QToolButton * defineButton(QToolButton * des, QIcon icon, QString text);
	bool sure_not;
	QEventLoop * m_loop;
	QUndoStack * table_commandStack;
	MainWindow * father;
};

#endif
