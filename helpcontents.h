#ifndef HELPCONTENTS_H
#define HELPCONTENTS_H
#include <QtCore>
#include <QtGui>

class LittleWidgetItem;
class HelpContents : public QWidget
{
	Q_OBJECT

public:
	HelpContents(QWidget * parent = 0);
	~HelpContents();
	
private:
  	void initContents();
	QHash<QString, QPixmap *> pix_hash;
	LittleWidgetItem * qml_win;
};

#endif 
 
