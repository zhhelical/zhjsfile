#ifndef PRINTFRAME_H
#define PRINTFRAME_H
#include <QtCore>
#include <QtGui>

class TablesGroup;
class PrintFrame : public QFrame 
{
	Q_OBJECT

public:
	PrintFrame(QWidget * parent = 0);
	~PrintFrame();
	void initPaperFrame(const QString & p_standard);	
	const QString & showedPaperFrame();

private:
	void generateGlassFrame(const QString & p_standard);
	void setFunctionBtns();	
	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	QString paper_size;
	QPoint press_pos;
	TablesGroup * framing_table;
};

#endif  
