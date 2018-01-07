#include <QtGui>
#include <QApplication>
#include "mainframe.h"

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(spc);
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QApplication app(argc, argv);
	Frame window;
	window.show();
	return app.exec();
}
