#ifndef MAINFRAME_H
#define MAINFRAME_H
#include <QtGui>
#include <QtCore>

class MainWindow;
class TitleBar;
class SlideBar;
class Frame : public QFrame
{
	Q_OBJECT
public:
	Frame(QWidget * parent = 0);
	~Frame();
	MainWindow * contentWidget() const;     
	TitleBar *titleBar() const;   
	void mousePressEvent(QMouseEvent *e);     
	void mouseMoveEvent(QMouseEvent *e);    
	void mouseReleaseEvent(QMouseEvent *e);

/*
private slots:
	void backToInitState();*/
     
private:
	TitleBar * m_titleBar;
	MainWindow * m_content;
	QPoint m_old_pos;
	bool m_mouse_down;
	bool left, right, bottom;
};

#endif
