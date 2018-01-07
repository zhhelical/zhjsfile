#ifndef TITLEBAR_H
#define TITLEBAR_H
#include<QtGui>
#include<QtCore>

class TitleBar : public QWidget
{
	Q_OBJECT
public:
	TitleBar(QWidget * parent = 0);
	~TitleBar();

signals:
	void slideToolBox();
	void showMultiWinsBar();
	void startProtecting();
	void returnHome();
	void helpViewer();
	
public slots:
	void changeTheme(const QString & theme);

private:
	QPushButton * defineButton(QPushButton * des, QIcon icon);
	QPushButton * homeButton;
	QPushButton * wtoolsButton;
	QPushButton * protectButton;
	QPushButton * toolsButton;
	QPushButton * helpButton;
	QPushButton *close;
	QLabel * themeLabel;
};

#endif
