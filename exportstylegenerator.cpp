#include <QtCore>
#include <QtGui>
#include "exportstylegenerator.h"
#include "tablesgroup.h"
#include "printframe.h"

ExportPdfGenerator::ExportPdfGenerator(QObject * parent)
: QObject(parent)
{
	printting_table = qobject_cast<TablesGroup *>(parent);
}

ExportPdfGenerator::~ExportPdfGenerator()
{}

void ExportPdfGenerator::printTableView(const QString & p_name, const QString & p_order, PrintFrame * pf_ptr) 
{
	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
	if (p_order == tr("A3横向"))
		printLandscapeA3(p_name, &printer, pf_ptr);
	else if (p_order == tr("A3纵向"))
		printPortraitA3(p_name, &printer, pf_ptr);
	else if (p_order == tr("A4横向"))
		printLandscapeA4(p_name, &printer, pf_ptr);	
	else if (p_order == tr("A4纵向"))
		printPortraitA4(p_name, &printer, pf_ptr);	
	else
		printRealSize(p_name, &printer);
	QString file_name(p_name+".pdf");	
	QDir test_dir(tr("/home/dapang/workstation/spc-tablet/")+file_name);
	test_dir.rename(test_dir.path(), tr("/home/dapang/workstation/pdf文件/")+file_name);	
}

void ExportPdfGenerator::printLandscapeA3(const QString & p_name, QPrinter * printer, PrintFrame * pf_ptr)
{
	printer -> setPaperSize(QPrinter::A3);
	printer -> setOrientation(QPrinter::Landscape);
	printer -> setOutputFileName(QString("%1.pdf").arg(p_name));
	QPainter painter(printer);
	QRect v_rect = pf_ptr->geometry();
	QPixmap view_pix = QPixmap::grabWidget(printting_table, v_rect);
	painter.drawPixmap(printer->paperRect(), view_pix, view_pix.rect());
	painter.end();
} 

void ExportPdfGenerator::printPortraitA3(const QString & p_name, QPrinter * printer, PrintFrame * pf_ptr)
{
	printer -> setPaperSize(QPrinter::A3);
	printer -> setOrientation(QPrinter::Portrait);
	printer -> setOutputFileName(QString("%1.pdf").arg(p_name));
	QPainter painter(printer);
	QRect v_rect = pf_ptr->geometry();
	QPixmap view_pix = QPixmap::grabWidget(printting_table, v_rect);
	painter.drawPixmap(printer->paperRect(), view_pix, view_pix.rect());
	painter.end();
} 

void ExportPdfGenerator::printLandscapeA4(const QString & p_name, QPrinter * printer, PrintFrame * pf_ptr)
{
	printer -> setPaperSize(QPrinter::A4);
	printer -> setOrientation(QPrinter::Landscape);
	printer -> setOutputFileName(QString("%1.pdf").arg(p_name));
	QPainter painter(printer);
	QRect v_rect = pf_ptr->geometry();
	QPixmap view_pix = QPixmap::grabWidget(printting_table, v_rect);
	painter.drawPixmap(printer->paperRect(), view_pix, view_pix.rect());
	painter.end();
} 

void ExportPdfGenerator::printPortraitA4(const QString & p_name, QPrinter * printer, PrintFrame * pf_ptr)
{
	printer -> setPaperSize(QPrinter::A4);
	printer -> setOrientation(QPrinter::Portrait);
	printer -> setOutputFileName(QString("%1.pdf").arg(p_name));
	QPainter painter(printer);
	QRect v_rect = pf_ptr->geometry();
	QPixmap view_pix = QPixmap::grabWidget(printting_table, v_rect);
	painter.drawPixmap(printer->paperRect(), view_pix, view_pix.rect());
	painter.end();
} 

void ExportPdfGenerator::printRealSize(const QString & p_name, QPrinter * printer)
{
	QRect v_rect = printting_table->rect();  
	printer -> setPaperSize(v_rect.size(), QPrinter::Millimeter);
	printer -> setOutputFileName(QString("%1.pdf").arg(p_name));
	printer -> setFullPage(true);
	QPainter painter(printer);
	QPixmap view_pix = QPixmap::grabWidget(printting_table, v_rect);
	painter.drawPixmap(printer->paperRect(), view_pix, view_pix.rect());
	painter.end();
} 

