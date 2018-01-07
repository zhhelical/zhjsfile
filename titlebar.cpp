#include "titlebar.h"

TitleBar::TitleBar(QWidget * parent)
:QWidget(parent)
{
	setStyleSheet("QWidget { border: none; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #63B8FF, stop: 0.08 #8DEEEE, stop: 0.29999 #87CEEB, stop: 0.4 #79CDCD, stop: 0.9 #7EC0EE, stop: 1 #5CACEE);}");                 
	homeButton = new QPushButton(this);   
	homeButton -> setStyleSheet("QPushButton:pressed { background-color: #AEEEEE; }");
	connect(homeButton, SIGNAL(clicked()), this, SIGNAL(returnHome()));
	protectButton = new QPushButton(this);
	protectButton -> setStyleSheet("QPushButton:pressed {background-color: #AEEEEE; }");
	connect(protectButton, SIGNAL(clicked()), this, SIGNAL(startProtecting()));
	wtoolsButton = new QPushButton(this);
	wtoolsButton -> setStyleSheet("QPushButton:pressed {background-color: #AEEEEE; }");
	connect(wtoolsButton, SIGNAL(clicked()), this, SIGNAL(showMultiWinsBar()));
	themeLabel = new QLabel(tr("工程"), this);
	themeLabel -> setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	toolsButton = new QPushButton(this);      
	toolsButton -> setStyleSheet("QPushButton:pressed { background-color: #AEEEEE; }");
	connect(toolsButton, SIGNAL(clicked()), this, SIGNAL(slideToolBox()));
	helpButton = new QPushButton(this);   
	helpButton -> setStyleSheet("QPushButton:pressed { background-color: #AEEEEE; }");
	connect(helpButton, SIGNAL(clicked()), this, SIGNAL(helpViewer())); 
	close= new QPushButton(this);                  
	close->setStyleSheet("QPushButton:pressed { background-color: #AEEEEE; }"); 
      	connect(close, SIGNAL(clicked()), parent, SLOT(close()));    
 	QHBoxLayout *hbox = new QHBoxLayout(this); 
	hbox->addWidget(defineButton(homeButton, QIcon(":/images/home.png")));
	hbox->addWidget(defineButton(protectButton, QIcon(":/images/protect.png"))); 
	hbox->addWidget(defineButton(wtoolsButton, QIcon(":/images/multiwins.png")));       
	hbox->addWidget(themeLabel);
	hbox->addWidget(defineButton(toolsButton, QIcon(":/images/toolbox.png")));
	hbox->addWidget(defineButton(helpButton, QIcon(":/images/help.png")));
	hbox->addWidget(defineButton(close, QIcon(":/images/exit.png")));         
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);         
}

TitleBar::~TitleBar()
{}

QPushButton * TitleBar::defineButton(QPushButton * des, QIcon icon)
{
	des -> setIcon(icon);
	des -> setIconSize(QSize(50, 50));
	des -> setFixedHeight(50);
	des -> setFixedWidth(50);
	return des;	
}

void TitleBar::changeTheme(const QString & theme)
{
	themeLabel -> setText(theme);	
	themeLabel -> repaint();
}
