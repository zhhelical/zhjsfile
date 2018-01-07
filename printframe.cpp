#include <QtCore>
#include <QtGui>
#include "printframe.h"
#include "tablesgroup.h"

PrintFrame::PrintFrame(QWidget * parent)
: QFrame(parent)
{
	framing_table = qobject_cast<TablesGroup *>(parent);
	setAttribute(Qt::WA_DeleteOnClose);
	setFont(QFont("Microsoft YaHei", 16));
	connect(framing_table, SIGNAL(printed()), this, SLOT(close()));
	connect(this, SIGNAL(destroyed()), framing_table, SLOT(clearPdfFramePtr()));
}

PrintFrame::~PrintFrame()
{}

void PrintFrame::initPaperFrame(const QString & p_standard)
{
	paper_size = p_standard;
	generateGlassFrame(p_standard);
}

const QString & PrintFrame::showedPaperFrame()
{
	return paper_size;
}

void PrintFrame::generateGlassFrame(const QString & p_standard)
{
	QRect p_rect;
	if (p_standard.contains(tr("A3横向")))
		p_rect = QRect(0, 0, 1200, 848);
	else if (p_standard.contains(tr("A3纵向")))
		p_rect = QRect(0, 0, 848, 1200);
	else if (p_standard.contains(tr("A4横向")))
		p_rect = QRect(0, 0, 848, 600);
	else
		p_rect = QRect(0, 0, 600, 848);
	setFrameRect(p_rect);
	setGeometry(framing_table->x(), framing_table->y(), p_rect.width(), p_rect.height());
	setStyleSheet("QWidget { background-color: rgba(68, 68, 68, 75%); border-width: 1px; border-style: solid; border-radius: 5px; border-color: #555555; }");
	show();
	setFunctionBtns();	
} 

void PrintFrame::setFunctionBtns()
{
 	QPushButton * ok_button = new QPushButton(tr("确定范围"), this);
	ok_button -> setFocusPolicy(Qt::NoFocus);
	ok_button -> setStyleSheet("QPushButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
	connect(ok_button, SIGNAL(clicked()), framing_table, SLOT(exportPdfTableAction()));
	QPushButton * cancel_button = new QPushButton(tr("取消"), this);
	cancel_button -> setFixedSize(ok_button->rect().size());
	cancel_button -> setFocusPolicy(Qt::NoFocus);
	cancel_button -> setStyleSheet("QPushButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 7; padding: 3px; font-size: 10px; padding-left: 5px; padding-right: 5px; min-width: 50px; max-width: 50px; min-height: 13px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(close())); 
	ok_button -> show();
	ok_button -> setGeometry(width()/2-ok_button->width()*1.5, height()/2-ok_button->height()*0.5, ok_button->width(), ok_button->height());
	cancel_button -> show();
	cancel_button -> setGeometry(width()/2+cancel_button->width()*0.5, height()/2-cancel_button->height()*0.5, cancel_button->width(), cancel_button->height());
}

void PrintFrame::mousePressEvent(QMouseEvent * event)
{
	setStyleSheet("QWidget { background-color: rgba(0, 0, 0, 75%); border-width: 1px; border-style: solid; border-radius: 5px; border-color: #555555; }");
	press_pos = event->pos();
	event -> accept();
}

void PrintFrame::mouseMoveEvent(QMouseEvent * event)
{
	QPoint pos_delta = event->pos()-press_pos;
	QPoint topleft_pos = geometry().topLeft()+pos_delta;
	move(topleft_pos);
	event -> accept();
}

void PrintFrame::mouseReleaseEvent(QMouseEvent * event)
{
	setStyleSheet("QWidget { background-color: rgba(68, 68, 68, 75%); border-width: 1px; border-style: solid; border-radius: 5px; border-color: #555555; }");
	event -> accept();
}
