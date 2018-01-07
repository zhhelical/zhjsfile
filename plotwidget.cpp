#include <QtGui>
#include "qwt_plot_canvas.h"
#include "qwt_text.h"
#include "qwt_text_label.h"
#include "plotwidget.h"
#include "plot.h"
#include "toolsplot.h"
#include "dataestableview.h"
#include "littlewidgetsview.h"
#include "littlewidgetitem.h"
#include "tablesgroup.h"
#include "tablemanagement.h"
#include "dataselectmodel.h"
#include "spcnum.h"
#include "spcdatabase.h"

PlotWidget::PlotWidget(SpcNum * val, SpcDataBase * db, QWidget * parent)
:QWidget(parent), focusing(false), last_group(0), func_btn(0), vl_layout(0), single_plot(0), multi_plot(0), histo_plot(0), gauss_plot(0), item_info(0), pos_item(0), editor_viewer(0), wl_editor(0), nested_animation(0), qml_win(0), nested_table(0), color_pad(0), back_guide(val), db_source(db)
{
	setAttribute(Qt::WA_DeleteOnClose);	
}

PlotWidget::~PlotWidget()
{}

void PlotWidget::setEnginame(const QString & name)
{
	if (!name.isEmpty())
	{
		curengi_name = name;
		QStringList engi_time = curengi_name.split(tr("，，。"));	
		QStringList all_tbls = db_source->allTables();
		QString pro_ktime(QString("%1").arg(QDateTime::fromString(engi_time.at(1)).toTime_t()));		
		QString f_time;
		foreach (QString e_str, all_tbls)
		{
			if (e_str.contains(pro_ktime))
			{
				QStringList fur_list = e_str.split(tr("，，。"));
				uint t_seconds = fur_list[2].split(tr("，")).at(0).toUInt();			
				f_time = QDateTime::fromTime_t(t_seconds).toString();
				break;
			}
		}
		QStringList engi_init;
		db_source -> getDataesFromTable("projectskeys", "name", engi_init, engi_time.at(0));
		foreach (QString e_str, engi_init)
		{
			QStringList e_list = e_str.split(tr("；")).filter(f_time);		  
			if (e_str.contains(tr("，")))
			{
				QStringList enext_list = e_list.at(0).split(tr("，"));
				engi_init[engi_init.indexOf(e_str)] = enext_list.at(1);
			}
		}
		reInitForMultiPlots(engi_init);	
	}
	else
	{
		initPlotWidgetForMulti();
		if (!curengi_name.isEmpty())	
			curengi_name.clear();
	}
}

void PlotWidget::removeCoverPad()
{
	color_pad -> close();
	color_pad -> deleteLater();
	color_pad = 0;
	focusing = false;
	bool chk_modify = false, chk_area = false, chk_ast = false, chk_dl = false, chk_dlst = false, chk_ctrl = false, chk_ctrlst = false, chk_ds = false, chk_ts = false;
	emit enableQmlBtnsForShowContents(chk_modify, chk_area, chk_ast, chk_dl, chk_dlst, chk_ctrl, chk_ctrlst, chk_ds, chk_ts, chk_ds);
}

void PlotWidget::setEndRect(const QRect & end)
{
	setFixedSize(end.size());
	anim_endrect = end;
}

void PlotWidget::setCloseBtnPtr(QPushButton * close_btn)
{
	func_btn = close_btn;
	if (func_btn)
	{
		func_btn -> setAttribute(Qt::WA_DeleteOnClose);
		connect(func_btn, SIGNAL(clicked()), func_btn, SLOT(close()));
		func_btn -> setIcon(QIcon("./images/littlewinclose.png"));
		func_btn -> setIconSize(QSize(20, 20));
		func_btn -> setFixedHeight(20);
		func_btn -> setFixedWidth(20);
		func_btn -> setGeometry(width()-20, 0, 20, 20);
		func_btn -> show();
		func_btn -> raise();
	}
}

void PlotWidget::updateCloseBtnPos()
{
	func_btn -> move(width()-20, 0);
	func_btn -> raise();
}

void PlotWidget::respondMultiDailyNewData(DataesTableView * t_daily)
{
	last_group = t_daily->model()->data(t_daily->model()->index(0, 0)).toInt();
	single_plot -> appendNewMarker();
	multi_plot -> appendNewMarker();
}

void PlotWidget::deleteNewDataOnMultiDaily()
{
	last_group -= 1;
	single_plot -> deleteLastPoint();
	multi_plot -> deleteLastPoint();
}

void PlotWidget::respondChangingAct(int type)
{
	QString change_content = item_info->text();
	if (type == 0)
	{}
	else if (type == 1)
	{}
	else if (type == 2)
	{}
	else if (type == 3)
	{}
	else if (type == 4)
	{}
	else
		return;
}

void PlotWidget::setRelatedCtrlValue(const QList<double> & vals)
{
	single_plot -> setNewValueForCtrlLine(vals.at(0), vals.at(1));
	multi_plot -> setNewValueForCtrlLine(vals.at(2), vals.at(3));
}

void PlotWidget::plotShowDetailChanging(const QString & style, const QString & detail)
{
	if (style == tr("图背景"))
		emit canvasColorChanging(detail);	
}

void PlotWidget::resetRelatedPlotVars(double ratio)
{
	if (single_plot)
		single_plot -> setPlotShowingVars(single_plot->title(), ratio);
	else if (histo_plot)
		histo_plot -> setPlotShowingVars(histo_plot->title(), ratio);
	else
		gauss_plot -> setPlotShowingVars(gauss_plot->title(), ratio);
}

void PlotWidget::resetEndRect(int w_size, int h_size)
{
	if (w_size)
		anim_endrect.setWidth(w_size);
	if (h_size)
		anim_endrect.setHeight(h_size);
}

void PlotWidget::generateSingleToolplot(QStandardItemModel * cpk_source, bool h_g)
{
	if (!vl_layout)
	{
		vl_layout = new QVBoxLayout(this);
		setLayout(vl_layout);
	}
	else
	{
		vl_layout->itemAt(0)->widget()->setParent(0);
		QLayoutItem * item = vl_layout->takeAt(0);
		delete item;		
	}
	if (h_g)
	{
		histo_plot = new HistogramPlot(curengi_name, qobject_cast<QWidget *>(vl_layout), back_guide, db_source);
		histo_plot -> createHistoForInitCpks(cpk_source);
		test_histoes << histo_plot;
		vl_layout -> addWidget(histo_plot);
	}
	else
	{
		gauss_plot = new GaussPlot(curengi_name, qobject_cast<QWidget *>(vl_layout), back_guide, db_source);
		gauss_plot -> createGaussForInitCpks(cpk_source);
		test_gausses << gauss_plot;		
		vl_layout -> addWidget(gauss_plot);
	}
}

void PlotWidget::generateSingle(QStandardItem * guide_item, QStandardItem * pos_info, const QRect & size, LittleWidgetsView * p_view, LittleWidgetItem * item_view, TablesGroup * group, const QRect & orig_size)
{
	if (!orig_size.isNull())
	{
		setEndRect(orig_size);
		setFixedWidth(size.width());
		setFixedHeight(size.height());
	}
	else
		setEndRect(size);
	item_info = guide_item;
	pos_item = pos_info;
	qml_win = item_view;
	nested_table = group;
	initPlotWidgetForSingle(item_info);
	if (p_view)
	{
		QPushButton * btn_close = new QPushButton(this);	
		connect(this, SIGNAL(plotMoving(PlotWidget *, const QPoint &)), p_view, SLOT(dealMimeForPlotMove(PlotWidget *, const QPoint &)));
		connect(btn_close, SIGNAL(destroyed(QObject *)), item_view, SLOT(arrangePlotForBtnSig(QObject *)));
		setCloseBtnPtr(btn_close);
	}
}

void PlotWidget::resetRelatedToolPlot(QStandardItemModel * guide_ptr)
{
 	vl_layout->itemAt(0)->widget()->setParent(0); 
 	QLayoutItem * item = vl_layout->takeAt(0);
	delete item; 
	foreach (HistogramPlot * histo, test_histoes)
	{
		if (histo->currentCpkDataesModel() == guide_ptr)
		{
			vl_layout -> addWidget(histo);
			break;
		}
	}
	foreach (GaussPlot * gauss, test_gausses)
	{
		if (gauss->currentCpkDataesModel() == guide_ptr)
		{
			vl_layout -> addWidget(gauss);	  
			break;
		}
	}
	update();		
}

void PlotWidget::actionForNestedorTitle()
{
	removeCoverPad();
	editor_viewer = new QWidget(this);
	editor_viewer -> setAttribute(Qt::WA_TranslucentBackground, true);
	QGridLayout * g_layout = new QGridLayout;
	wl_editor = new QLineEdit(this);
	wl_editor -> setStyleSheet("font-family: Microsoft YaHei; color: white; border: 1px solid rgb(50, 50, 50); border-radius: 3px; background: rgb(50, 50, 50)");	
	QPushButton * ok_button = new QPushButton(tr("确定"));
	ok_button -> setFocusPolicy(Qt::NoFocus);
	ok_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 5; padding: 1px; font-size: 10px; padding-left: 2px; padding-right: 2px; min-width: 20px; max-width: 50px; min-height: 12px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");	
	connect(ok_button, SIGNAL(clicked()), this, SLOT(clickedBtnFromSure()));	
	QPushButton * cancel_button = new QPushButton(tr("取消"));
	cancel_button -> setFocusPolicy(Qt::NoFocus);
	cancel_button -> setStyleSheet("QPushButton,QToolButton{color: black; background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c); border-width: 1px; border-color: #339; border-style: solid; border-radius: 5; padding: 1px; font-size: 10px; padding-left: 2px; padding-right: 2px; min-width: 20px; max-width: 50px; min-height: 12px; max-height: 13px;} QPushButton::pressed,QToolButton::pressed{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #969B9C, stop: 0.5 #16354B, stop: 1.0 #244F76); border-color: #11505C;}");
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(clickedBtnFromCancel()));	
	QRectF t_pos;
	QString title_text;
	if (histo_plot)
	{
		t_pos = histo_plot->titleLabel()->textRect(); 
		title_text = histo_plot->title().text();
	}
	else if (gauss_plot)
	{
		t_pos = gauss_plot->titleLabel()->textRect();
		title_text = gauss_plot->title().text();
	}
	else
	{
		t_pos = single_plot->titleLabel()->textRect();
		title_text = single_plot->title().text();
	}
	g_layout -> addWidget(wl_editor, 0, 0, 1, 4);        
	g_layout -> addWidget(ok_button, 1, 1, 1, 1);  
	g_layout -> addWidget(cancel_button, 1, 2, 1, 1);	  
	g_layout->setSpacing(2);
	g_layout->setContentsMargins(0, 0, 0, 0);	
	editor_viewer -> setLayout(g_layout); 
	editor_viewer -> setGeometry(t_pos.left(), t_pos.bottom(), t_pos.width(), 40);	
	openNestedEdittor(editor_viewer, t_pos.width(), 40);		
}

void PlotWidget::nestedPlotContentAction(const QString & action)
{
 	if (histo_plot)
		histo_plot -> toggleAreaLinesShowingState(); 
	else if (gauss_plot)
	{
		if (action == tr("分区显示"))
			gauss_plot -> freshPerformLines(); 
		else
			gauss_plot -> freshCtrlLines(); 		
	}
	else
	{
		if (action == tr("分区显示"))
			single_plot -> toggleAreaLinesShowingState();	  
		else if (action == tr("控制线"))
			single_plot -> toggleCtrlLinesShowingState();	
		else if (action == tr("点图线"))
			single_plot -> showHideDotsLines();
		else if (action == tr("点序坐标") || action == tr("时序坐标"))
			single_plot -> toggleXaxisLabels();
	}  
}

bool PlotWidget::hasColorPad()
{
	if (color_pad)
		return true;
	return false;
}

bool PlotWidget::validForPlotChanging()
{
	QStandardItem * single_mode = item_info->parent();
	QPair<int, int> check_range;
	if (single_mode->text() == tr("均值图"))
	{
		single_plot -> plotXrangeForSingleMode(check_range);
	}
	return true;	
}

bool PlotWidget::isDataesEmpty()
{
	if (!curengi_name.isEmpty())
		return false;
	return true;
}

bool PlotWidget::engiCtrlType()
{
	if (multi_plot && multi_plot->title().text()==tr("西格玛图"))
		return true;
	if (single_plot && single_plot->title().text()==tr("西格玛图"))
		return true;
	return false;	
}

int PlotWidget::currentEngiSamples()
{
	return engi_samples;
}

int PlotWidget::currentEngiGroups()
{
	return last_group;
}

double PlotWidget::upAvrCtrlLimit()
{
	return single_plot->relatedUpCtrlLimit();
}
	
double PlotWidget::lowAvrCtrlLimit()
{
	return single_plot->relatedLowCtrlLimit();
}
	
double PlotWidget::upDevCtrlLimit()
{
	return multi_plot->relatedUpCtrlLimit();	
}
	
double PlotWidget::lowDevCtrlLimit()
{
	return multi_plot->relatedLowCtrlLimit();
}
	
double PlotWidget::upRngCtrlLimit()
{
	return multi_plot->relatedUpCtrlLimit();
}
	
double PlotWidget::lowRngCtrlLimit()
{
	return multi_plot->relatedLowCtrlLimit();
}

double PlotWidget::engiDefinedUpCtrl()
{
	return upper_engiLimit;
}
	
double PlotWidget::engiDefinedLowCtrl()
{
	return lower_engiLimit;
}
	
QWidget * PlotWidget::coverWidgetPtr()	
{
	return color_pad;
}
	
const QString & PlotWidget::currentEngi()
{
	return curengi_name;
}

const QString & PlotWidget::avrDataCurState(int num)
{
	return single_plot->relatedPtJudgedState(num);
}
	
const QString & PlotWidget::devDataCurState(int num)
{
	return multi_plot->relatedPtJudgedState(num);
}
	
const QString & PlotWidget::rngDataCurState(int num)
{
	return multi_plot->relatedPtJudgedState(num);
}

QRect & PlotWidget::endRect()
{
	return anim_endrect;
}

QStandardItem * PlotWidget::singlePlotInfo()
{
	return item_info;
}

QPushButton * PlotWidget::exsitedBtn()
{
	return func_btn;
}

QWidget * PlotWidget::respondMatchPtr(const QString & ptr_type)
{
	if (ptr_type == tr("单图"))
		return single_plot;	
	else if (ptr_type == tr("复合图"))
		return multi_plot;
	else if (ptr_type == tr("直方图"))
		return histo_plot;
	else
		return gauss_plot;
	return 0;
}

QList<double> & PlotWidget::inputtedDailyDataes()
{
	return tmp_data;
}

QList<double> & PlotWidget::avrDevJudgeStack(bool avr_dev, int type)
{
	if (avr_dev)
		return single_plot->ptJudgeStack(type);
	else
		return multi_plot->ptJudgeStack(type);
}

QList<HistogramPlot *> & PlotWidget::currentHistograms()
{
	return test_histoes;
}
	
QList<GaussPlot *> & PlotWidget::currentGausses()
{
	return test_gausses;
}

QMap<int, QString> & PlotWidget::relatedJudgedStateMap(int type)
{
	if (type == 0)
		return single_plot->relatedPtJudgedState();
	else
		return multi_plot->relatedPtJudgedState();
}

QMap<int, QList<double> > & PlotWidget::ownedEngiTolDataes()
{
	return part_records;
}

void PlotWidget::startRelatedDailyCaling(Plot * w_plot, QList<double> & vals)
{
	if (back_guide->currentPlotWidget()!=this)
		back_guide -> setCalculatingPlots(this);
	if (w_plot == single_plot)
	{
		double avr = back_guide->averageDataes(vals); 
		back_guide -> startPtRuningJudging(avr, 0);
	}
	else if (w_plot == multi_plot)
	{
		if (multi_plot->title().text() == tr("西格玛图"))
		{
			double dev = back_guide->devCalFromDataes(vals); 
			back_guide -> startPtRuningJudging(dev, 1);
		}
		else
		{
			double rng = back_guide->rngCalFromDataes(vals); 
			back_guide -> startPtRuningJudging(rng, 2);
		}		
	}
	else
		return;
}

void PlotWidget::rectangleAnimating(const QVariant & v_rec)
{
	QRect show_rect = v_rec.toRect();
	editor_viewer -> setFixedSize(show_rect.size());
	editor_viewer -> update();
}

void PlotWidget::clickedBtnFromSure()
{
	QwtText n_text(wl_editor->text());  
 	if (histo_plot)
	{
		n_text.setFont(histo_plot->title().font());
		histo_plot -> setTitle(n_text);		
	}
	else if (gauss_plot)
	{
		n_text.setFont(gauss_plot->title().font());
		gauss_plot -> setTitle(n_text);
	}
	else
	{
		n_text.setFont(single_plot->title().font());
		single_plot -> setTitle(n_text);
	}
	closeNestedViewer(editor_viewer); 
}
	
void PlotWidget::clickedBtnFromCancel()
{
	closeNestedViewer(editor_viewer);   
}

void PlotWidget::clearNestedAnimation()
{
	if (editor_viewer)
		editor_viewer -> setFixedWidth(nested_animation->endValue().toRect().width());
	else
	{
		editor_viewer -> deleteLater();
		editor_viewer = 0;
		wl_editor = 0;
	}
	nested_animation -> deleteLater();
	nested_animation = 0;
	update();	
}

void PlotWidget::mousePressEvent(QMouseEvent * event)
{
	if (!color_pad && qml_win)
	{	
		generateCoverWidget();
		qml_win -> checkSameColorInMatrix(this);
		QMapIterator<QPair<int, int>, PlotWidget *> p_map(qml_win->allCurrentPlots());
		while (p_map.hasNext())
		{
			p_map.next();
			if (p_map.value() == this)
			{
				setClueText(p_map.key().first, p_map.key().second);
				break;
			}
		}		
		bool chk_modify = true, chk_area = true, chk_arst = false, chk_dl = false, chk_dlst = false, chk_ctrl = false, chk_ctrlst = false, chk_ds = false, chk_ts = false, chk_dtst = false;
		if (single_plot)
		{
			chk_arst = single_plot->chkAreaLinesShowingState();
			chk_ctrl = true;
			chk_ctrlst  = single_plot->chkCtrlLinesShowingState();
			chk_dl = true;
			chk_dlst = single_plot->chkDotsLinesShowingState();			
			chk_ds = true;
			chk_ts = true;	
			chk_dtst = !single_plot->chkXaxisTimeNumShow();
		}
		else if (histo_plot)
			chk_arst = histo_plot->chkAreaLinesShowingState();
		else
		{
		 	chk_arst = gauss_plot->chkPerformLinesShowingState();
			chk_ctrl = true;
			chk_ctrlst  = gauss_plot->chkCtrlLinesShowingState();
		}
		emit enableQmlBtnsForShowContents(chk_modify, chk_area, chk_arst, chk_dl, chk_dlst, chk_ctrl, chk_ctrlst, chk_ds, chk_ts, chk_dtst);
	}
	focusing = !focusing;
	mv_pt = event->pos();
	event -> accept();
}

void PlotWidget::mouseMoveEvent(QMouseEvent * event)
{
	if (!color_pad)
	{
		event -> ignore();
		return;
	}
	emit plotMoving(this, mv_pt);
	event -> accept();
}
	
void PlotWidget::mouseReleaseEvent(QMouseEvent * event)
{
	if (color_pad && !focusing)
	{
		removeCoverPad();
		event -> accept();
	}
	else
		event -> ignore();	
}

void PlotWidget::paintEvent(QPaintEvent * event)
{
	if (color_pad)
	{
		QLabel * c_lbl = qobject_cast<QLabel *>(color_pad->childAt(color_pad->width()/2, color_pad->height()/2));
		color_pad -> resize(width(), height());
		c_lbl -> setGeometry((width()-c_lbl->width())/2, (height()-c_lbl->height())/2, c_lbl->width(), c_lbl->height());
	}
	QWidget::paintEvent(event);
}

void PlotWidget::initPlotWidgetForMulti()
{
	if (single_plot)
	{
		single_plot -> clearCurrentShowing();
		multi_plot -> clearCurrentShowing();
		return;
	}
	single_plot = new Plot(QwtText(tr("均值图")), db_source, this);
	vl_layout = new QVBoxLayout;
	vl_layout -> setMargin(10);
	vl_layout -> setSpacing(15);
	vl_layout -> addWidget(single_plot);
	multi_plot = new Plot(QwtText(tr("西格玛图")), db_source, this);
	vl_layout -> addWidget(multi_plot);
	setLayout(vl_layout);
}
	
void PlotWidget::initPlotWidgetForSingle(QStandardItem * s_item)
{
	DataSelectModel * infoes_model = qobject_cast<DataSelectModel *>(s_item->model());
	QStandardItem * single_mode = s_item->parent();
	vl_layout = new QVBoxLayout(this);
	vl_layout->setMargin(0);
	vl_layout->setSpacing(0);
	QStandardItem * tkey_item = infoes_model->findTimeParentItem(s_item);
	QString et_stamp = QString("%1").arg(QDateTime::fromString(tkey_item->child(0)->child(0)->text()).toTime_t());
	if (single_mode->text() == tr("均值图") || single_mode->text()==tr("西格玛图") || single_mode->text()==tr("极差图"))
	{
		single_plot = new Plot(QwtText(single_mode->text()), db_source, this);
		single_plot -> setPlotDeployment(et_stamp, s_item);
		vl_layout -> addWidget(single_plot);		
	}
	else if (s_item->text()==tr("直方图") || single_mode->text()==tr("直方图") || s_item->text() == tr("正态概率纸") || single_mode->text()==tr("正态概率纸"))
	{
		QString time_lbl;
		QString tbl_lbl;
		QList<QStandardItem *> all_lines;		
		if (s_item->text()==tr("直方图") || s_item->text() == tr("正态概率纸"))
		{	
			QString cpk_find(tr("完成版本"));
			infoes_model -> findItemsSingleLine(s_item, all_lines, cpk_find);
			time_lbl = QString("%1").arg(QDateTime::fromString(all_lines.at(0)->child(0)->child(0)->text()).toTime_t());
			tbl_lbl = tr("，，。cpkdataes");
		}
		else
		{
			time_lbl = et_stamp;
			tbl_lbl = tr("，，。dailydataes");
		}
		QString tbl_name;
		QStringList db_tbls = db_source->allTables();		
		foreach (QString tbl, db_tbls)
		{
			if (tbl.contains(tbl_lbl) && tbl.contains(time_lbl))
			{
				tbl_name = tbl;
				break;
			}
		}
		tbl_name = tbl_name+tr("，。，")+QDateTime::fromString(tkey_item->child(0)->child(0)->text()).toString(Qt::ISODate);		
		if (s_item->text()==tr("直方图") || s_item->text()==tr("正态概率纸"))
		{
			if (s_item->text() == tr("直方图"))
			{
				histo_plot = new HistogramPlot(tbl_name, this, back_guide, db_source);
				histo_plot -> createHistoForItem(s_item);
				vl_layout -> addWidget(histo_plot);
			}
			else
			{
				gauss_plot = new GaussPlot(tbl_name, this, back_guide, db_source);
				gauss_plot -> createPlotFromItem(s_item);
				vl_layout -> addWidget(gauss_plot);
			}
		}
		else
		{			
			if (single_mode->text() == tr("直方图"))
			{
				histo_plot = new HistogramPlot(tbl_name, this, back_guide, db_source);
				histo_plot -> createHistoForItem(s_item);
				vl_layout -> addWidget(histo_plot);
			}
			else
			{
				gauss_plot = new GaussPlot(tbl_name, this, back_guide, db_source);
				gauss_plot -> createPlotFromItem(s_item);
				vl_layout -> addWidget(gauss_plot);
			}			
		}
	}	
	if (single_plot)
	{
		if (nested_table)
			single_plot->canvas()->setStyleSheet("background-color: #008B8B");
		connect(this, SIGNAL(canvasColorChanging(const QString &)), single_plot, SLOT(setCanvasColor(const QString &)));
	}
	else if (histo_plot)
	{

	}
	else
	{}
	setLayout(vl_layout);
}

void PlotWidget::reInitForMultiPlots(QStringList & relist)
{
	if (!single_plot)
		initPlotWidgetForMulti();
	upper_engiLimit = relist.at(4).toDouble();
	lower_engiLimit = relist.at(5).toDouble();
	engi_samples = relist.at(6).toDouble();
	engi_groups = relist.at(7).toDouble();
	QString find_ktime = QString("%1").arg(QDateTime::fromString(curengi_name.split(tr("，，。")).at(1)).toTime_t());
	QString daily_table;
	QStringList t_all = db_source->allTables();
	foreach (QString tbl, t_all)
	{
		if (tbl.contains(tr("，，。dailydataes")) && tbl.contains(find_ktime))
		{
			daily_table = tbl;
			break;
		}
	}
	if (!daily_table.isEmpty())
		initEngiDailyPartDataesRecord(daily_table);
	if (relist.at(3) == tr("西格玛图"))
	{
		QString dev_str = curengi_name+","+"dev";
		multi_plot -> resetForMultiPlots(dev_str);
		QString avr_str = curengi_name+","+"devavr";
		single_plot -> resetForMultiPlots(avr_str);
	}
	else if (relist.at(3) == tr("极差图"))
	{
		QString rng_str = curengi_name+","+"rng";
		multi_plot -> resetForMultiPlots(rng_str);
		QString avr_str = curengi_name+","+"rngavr";
		single_plot -> resetForMultiPlots(avr_str);
	}
}

void PlotWidget::initEngiDailyPartDataesRecord(const QString & description)
{
	last_group = db_source->lastDataFromTable(description, "groupnum").toInt();
	if (last_group <= 100)
	{
		for (int i = 0; i < last_group; i++)
		{
			QString sel_pos = QString("groupnum=%1").arg(i+1);
			back_guide->dailyDataesTransFromDB(db_source->dataFromTable(description, "dataes", sel_pos), part_records[i+1]);
		}
	}
	else
	{
		for (int i = last_group-100; i < last_group; i++)
		{
			QString sel_pos = QString("groupnum=%1").arg(i+1);
			back_guide->dailyDataesTransFromDB(db_source->dataFromTable(description, "dataes", sel_pos), part_records[i+1]);
		}	
	}
}

void PlotWidget::chkPlotShowingState()
{
	if (histo_plot)
	{

	}
	else if (gauss_plot)
	{

	}
	else
	{

	}	
}

void PlotWidget::generateCoverWidget()
{
	color_pad = new QWidget(this);
	color_pad -> setGeometry(0, 0, this->width(), this->height());
	color_pad -> setStyleSheet("QWidget { background-color: rgba(0, 0, 0, 75%); border-width: 1px; border-style: solid; border-radius: 5px; border-color: #555555; } QWidget:hover { background-color: rgba(68, 68, 68, 75%);}");
	color_pad -> show();	
}

void PlotWidget::setClueText(int row, int col)
{
	QString label_text = tr("第")+QString("%1").arg(row+1)+tr("行")+tr("，")+tr("第")+QString("%1").arg(col+1)+tr("列");	
	QLabel * clue_pos =new QLabel(label_text, color_pad);
	clue_pos -> setAttribute(Qt::WA_TranslucentBackground);
	clue_pos -> setStyleSheet("QLabel{color: white}");	
	clue_pos -> setGeometry((width()-clue_pos->width())/2, (height()-clue_pos->height())/2, clue_pos->width(), clue_pos->height());
	clue_pos -> show();		
}

void PlotWidget::openNestedEdittor(QWidget * nesting, int end_width, int end_height)
{
	nested_animation = new QPropertyAnimation(this);
	connect(nested_animation, SIGNAL(valueChanged(const QVariant &)), this, SLOT(rectangleAnimating(const QVariant &)));
	connect(nested_animation, SIGNAL(finished()), this, SLOT(clearNestedAnimation()));
	nested_animation -> setTargetObject(nesting);
	nested_animation -> setPropertyName("geometry");
	nested_animation->setDuration(500);
	QRect nested_startRect(QRect(nesting->frameGeometry().left(), nesting->frameGeometry().top(), 0, 0));
	QRect nested_endRect(QRect(nesting->frameGeometry().left(), nesting->frameGeometry().top(), end_width, end_height));
	nested_animation->setStartValue(nested_startRect);
	nested_animation->setEndValue(nested_endRect);
	nested_animation -> start();
	nesting -> show();  
}

void PlotWidget::closeNestedViewer(QWidget * nested)
{
 	nested_animation = new QPropertyAnimation(this);
	connect(nested_animation, SIGNAL(valueChanged(const QVariant &)), this, SLOT(rectangleAnimating(const QVariant &)));
	connect(nested_animation, SIGNAL(finished()), this, SLOT(clearNestedAnimation()));
	nested_animation -> setTargetObject(nested);
	nested_animation -> setPropertyName("geometry");
	nested_animation->setDuration(500);
	QRect nested_startRect(QRect(nested->frameGeometry().topLeft().x(), nested->frameGeometry().topLeft().y(), nested->width(), nested->height()));
	QRect nested_endRect(QRect(nested->frameGeometry().topLeft().x(), nested->frameGeometry().topLeft().y(), 0, 0));
	nested_animation->setStartValue(nested_startRect);
	nested_animation->setEndValue(nested_endRect);
	nested_animation -> start(); 
}