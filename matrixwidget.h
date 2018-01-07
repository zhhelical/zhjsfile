#ifndef MATRIXWIDGET_H
#define MATRIXWIDGET_H
#include <QtGui>
#include <QtCore>

class LittleWidgetItem;
class MatrixWidget : public QWidget
{
	Q_OBJECT
public:
	MatrixWidget(const QRect & size, LittleWidgetItem * nested_qml, QWidget * parent = 0);
	~MatrixWidget();
	void clearPosLabel();
	void resetToDefaultBackGround();
	void resetOrigRectSize(int new_w, int new_h);	
	const QRect & originSize();

protected:
	void mousePressEvent(QMouseEvent * event);
	void paintEvent(QPaintEvent * event);	

private:
	void setClueText(int row, int col);	
	LittleWidgetItem * qml_win;
	QLabel * clue_pos;
	QRect base_size;
};

#endif
