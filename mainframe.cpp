#include <QtGui>
#include <QtCore>
#include "mainframe.h"
#include "titlebar.h"
#include "mainwindow.h"
#include "slidetoolbar.h"
#include "plotwidget.h"

Frame::Frame(QWidget * parent)
:QFrame(parent)
{
	m_mouse_down = false;
	setStyleSheet("border: 1px groove gray");
	setWindowFlags(Qt::FramelessWindowHint);
	setMouseTracking(true);        
	m_titleBar = new TitleBar(this);        
	m_content = new MainWindow(this);        
	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->addWidget(m_titleBar);
	vbox->setMargin(0);
	vbox->setSpacing(0);
	vbox->addWidget(m_content);
	connect(m_content, SIGNAL(closeSystem()), this, SLOT(close()));
	connect(m_titleBar, SIGNAL(slideToolBox()), m_content, SLOT(createMainToolBars()));
	connect(m_titleBar, SIGNAL(helpViewer()), m_content, SLOT(helpContent()));
	connect(m_titleBar, SIGNAL(showMultiWinsBar()), m_content, SLOT(usrOwnedMultiWinsTools()));
	connect(m_titleBar, SIGNAL(startProtecting()), m_content, SLOT(startProtectionMode()));
	connect(m_titleBar, SIGNAL(returnHome()), m_content, SLOT(backToNewFileState()));
	connect(m_content, SIGNAL(newTheme(const QString &)), m_titleBar, SLOT(changeTheme(const QString &)));
	setLayout(vbox);
	resize(1680, 943);
}

Frame::~Frame()
{}

MainWindow * Frame::contentWidget() const 
{ 
	return m_content; 
}

TitleBar * Frame::titleBar() const 
{ 
	return m_titleBar; 
}

void Frame::mousePressEvent(QMouseEvent *e)
{
	m_old_pos = e->pos();
	m_mouse_down = e->button() == Qt::LeftButton;
}

void Frame::mouseMoveEvent(QMouseEvent *e)
{
	int x = e->x();
	int y = e->y(); 
	if (m_mouse_down) 
	{
		int dx = x - m_old_pos.x();
		int dy = y - m_old_pos.y();            
		QRect g = geometry();
		if (left)
			g.setLeft(g.left() + dx);
		if (right)
			g.setRight(g.right() + dx);
		if (bottom)
			g.setBottom(g.bottom() + dy);
		setGeometry(g);
		m_old_pos = QPoint(!left ? e->x() : m_old_pos.x(), e->y());
	} 
	else 
	{
		QRect r = rect();
		left = qAbs(x - r.left()) <= 5;
		right = qAbs(x - r.right()) <= 5;
		bottom = qAbs(y - r.bottom()) <= 5;
		bool hor = left | right;
		if (hor && bottom) 
		{
			if (left)
				setCursor(Qt::SizeBDiagCursor);
			else
				setCursor(Qt::SizeFDiagCursor);
		} 
		else if (hor) 
			setCursor(Qt::SizeHorCursor);
		else if (bottom)
			setCursor(Qt::SizeVerCursor);
		else
			setCursor(Qt::ArrowCursor);
	}
}

void Frame::mouseReleaseEvent(QMouseEvent *e)
{
	Q_UNUSED(e);
	m_mouse_down = false;
	m_old_pos = QPoint();
}

/*void Frame::backToInitState()
{
//	if (m_content->okcancelToolSlide())
//		return;
	if (m_content->curToolBar())
		m_content -> animateForToolBar(m_content->curToolBar(), 0);
	emit newFileState();
}*/
