#include <QtGui>
#include <QtCore>
#include "matrixwidget.h"
#include "littlewidgetitem.h"

MatrixWidget::MatrixWidget(const QRect & size, LittleWidgetItem * nested_qml, QWidget * parent)
:QWidget(parent), clue_pos(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setFixedWidth(size.width());
	setFixedHeight(size.height());
	base_size = size;
	QPalette p(palette());
	p.setColor(QPalette::Background, Qt::darkGray);
	setAutoFillBackground(true);
	setPalette(p);
	qml_win = nested_qml;
}

MatrixWidget::~MatrixWidget()
{}

void MatrixWidget::clearPosLabel()
{
	clue_pos -> deleteLater();
	clue_pos = 0;
}

void MatrixWidget::resetToDefaultBackGround()
{
	if (palette().color(QPalette::Background) != Qt::darkGray)	
		const_cast<QPalette &>(palette()).setColor(QPalette::Background, Qt::darkGray);
	if (clue_pos)
		clearPosLabel();
}

void MatrixWidget::resetOrigRectSize(int new_w, int new_h)
{
	if (new_w)
		base_size.setWidth(new_w);
	if (new_h)
		base_size.setHeight(new_h);
}

const QRect & MatrixWidget::originSize()
{
	return base_size;
}

void MatrixWidget::mousePressEvent(QMouseEvent * event)
{
	if (palette().color(QPalette::Background) != Qt::darkGreen)
	{
		const_cast<QPalette &>(palette()).setColor(QPalette::Background, Qt::darkGreen);
		QMap<QPair<int, int>, MatrixWidget *> in_map = qml_win->allCurrentDefaults();
		QMapIterator<QPair<int, int>, MatrixWidget *> d_map(in_map);
		while (d_map.hasNext())
		{
			d_map.next();
			if (d_map.value() == this)
			{
				setClueText(d_map.key().first, d_map.key().second);
				break;
			}
		}		
		qml_win -> checkSameColorInMatrix(this);
	}
	event -> accept();
}

void MatrixWidget::paintEvent(QPaintEvent * event)
{
	if (clue_pos)
		clue_pos -> setGeometry((width()-clue_pos->width())/2, (height()-clue_pos->height())/2, clue_pos->width(), clue_pos->height());
	QWidget::paintEvent(event);		
}

void MatrixWidget::setClueText(int row, int col)
{
	QString label_text = tr("第")+QString("%1").arg(row+1)+tr("行")+tr("，")+tr("第")+QString("%1").arg(col+1)+tr("列");
	clue_pos =new QLabel(label_text, this);
	clue_pos -> setGeometry((width()-clue_pos->width())/2, (height()-clue_pos->height())/2, clue_pos->width(), clue_pos->height());
	clue_pos -> show();	
}


