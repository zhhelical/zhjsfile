#include "helpcontents.h"
#include "littlewidgetitem.h"

HelpContents::HelpContents(QWidget * parent)
:QWidget(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setFixedWidth(1825);
	setFixedHeight(862);
	QPalette p(palette());
	p.setColor(QPalette::Background, Qt::darkGray);
	setAutoFillBackground(true);
	setPalette(p);
	initContents();
}

HelpContents::~HelpContents()
{}

void HelpContents::initContents()
{
 	QDir related_dir("/home/dapang/workstation/spc-tablet/images/help-images");
	QFileInfoList qfi_list = related_dir.entryInfoList(QDir::Files);
	foreach (QFileInfo fi, qfi_list) 
	{
		QPixmap * pix = new QPixmap;
		pix -> load(fi.filePath());
		pix_hash.insert(fi.fileName(), pix);
	}
	QStringList ps_names = pix_hash.keys();
	QString p_first = ps_names.filter("page1.").at(0);
	QString p_second = ps_names.filter("page2.").at(0);
	QString p_third = ps_names.filter("page3.").at(0);
	QHBoxLayout * h_layout = new QHBoxLayout;
	h_layout -> setSpacing(10);
	QLabel * p1_label = new QLabel;
	p1_label -> setPixmap(*pix_hash.value(p_first));
	QLabel * p2_label = new QLabel;
	p2_label -> setPixmap(*pix_hash.value(p_second));	
	QLabel * p3_label = new QLabel;
	p3_label -> setPixmap(*pix_hash.value(p_third));	
	h_layout -> addWidget(p1_label);
	h_layout -> addWidget(p2_label);
	h_layout -> addWidget(p3_label);
	setLayout(h_layout);
}
