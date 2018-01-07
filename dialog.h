#ifndef DIALOG_H
#define DIALOG_H
#include <QtGui>

class MainWindow;
class TableManagement;
class DataesTableView;
class RelatedTreeView;
class SpcDataBase;
class DiaLog : public QDialog
{
	Q_OBJECT
public:
	DiaLog(QWidget * parent = 0);
	~DiaLog();
	void initWinDialog(MainWindow * guider, const QString & information, int type);
	void initSaveStrDefineDialog(MainWindow * guider, SpcDataBase * db, const QString & passwd);
	void showForCollisionDbsDataes(const QHash<QString, QString> & projs, const QHash<QString, QString> & managers, MainWindow * p_mw);
	bool chkCollideDbsState();

signals:
	void sendSaveFileName(const QString & s_name);

private slots:
	void onSure();
	void onCancel();
	void judgeInputTextForSave();
	void judgeInputTextForExport();
	void chkSaveDbsCollideDataes();
	void animatedSize(const QVariant & v_size);
	void resetLabelColor();
	void killAnimation();
	void killMyself();

private:
	void initHintDialog(const QString & info);
	void initWarningDialog(const QString & info, int type);
	void initErrorDialog(const QString & info);
	void defineDialogLabelSize(QLabel * dest_label);
	void openMeOnAnimation();	
	void closeMeOnAnimation();
	bool r_chk;
	bool sure;
	bool unsure;
	QString inputor;
	QPropertyAnimation * dialog_animation;
	QModelIndex pos_index;
	MainWindow * w_inspector;
	SpcDataBase * search_db;
};

#endif
