#ifndef EXPORTSTYLEGENERATOR_H
#define EXPORTSTYLEGENERATOR_H
#include <QtCore>
#include <QtGui>

class TablesGroup;
class PrintFrame;
class ExportPdfGenerator : public QObject
{
	Q_OBJECT

public:
	ExportPdfGenerator(QObject * parent = 0);
	~ExportPdfGenerator();
	void printTableView(const QString & p_name, const QString & p_order, PrintFrame * pf_ptr);	

private:
	void printLandscapeA3(const QString & p_name, QPrinter * printer, PrintFrame * pf_ptr);
	void printPortraitA3(const QString & p_name, QPrinter * printer, PrintFrame * pf_ptr);
	void printLandscapeA4(const QString & p_name, QPrinter * printer, PrintFrame * pf_ptr);
	void printPortraitA4(const QString & p_name, QPrinter * printer, PrintFrame * pf_ptr);	
	void printRealSize(const QString & p_name, QPrinter * printer);
	void generatePaperFrame();
	TablesGroup * printting_table;
};

#endif 
