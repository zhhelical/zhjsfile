#include "editlongtoolsviewer.h"

#include <QtCore>
#include <QtDeclarative>
#include <QtGui>
#include "tablesgroup.h"
#include "littlewidgetsview.h"
#include "dataestableview.h"
#include "relatedtreeview.h"
#include "tablemanagement.h"
#include "tablemgmtploteditcommand.h" 
#include "tablemgmtgroupeditcommand.h"
#include "plotwidget.h"
#include "matrixwidget.h"

#include <qplatformdefs.h> // MEEGO_EDITION_HARMATTAN

#ifdef HARMATTAN_BOOSTER
#include <MDeclarativeCache>
#endif

#if defined(QMLJSDEBUGGER) && QT_VERSION < 0x040800

#include <qt_private/qdeclarativedebughelper_p.h>

#if !defined(NO_JSDEBUGGER)
#include <jsdebuggeragent.h>
#endif
#if !defined(NO_QMLOBSERVER)
#include <qdeclarativeviewobserver.h>
#endif

// Enable debugging before any QDeclarativeEngine is created
struct QmlJsDebuggingEnabler
{
	QmlJsDebuggingEnabler()
	{
		QDeclarativeDebugHelper::enableDebugging();
	}
};

// Execute code in constructor before first QDeclarativeEngine is instantiated
static QmlJsDebuggingEnabler enableDebuggingHelper;

#endif // QMLJSDEBUGGER

class EditLongToolsViewerPrivate
{
	EditLongToolsViewerPrivate(QDeclarativeView * view_) : view(view_) {}
	QString mainQmlFile;
	QDeclarativeView * view;
	friend class EditLongToolsViewer;
	QString adjustPath(const QString &path);
};

QString EditLongToolsViewerPrivate::adjustPath(const QString & path)
{
#ifdef Q_OS_MAC
	if (!QDir::isAbsolutePath(path))
		return QString::fromLatin1("%1/../Resources/%2").arg(QCoreApplication::applicationDirPath(), path);
#else
	const QString pathInInstallDir = QString::fromLatin1("%1/../%2").arg(QCoreApplication::applicationDirPath(), path);
	if (QFileInfo(pathInInstallDir).exists())
		return pathInInstallDir;
#endif
	return path;
}

EditLongToolsViewer::EditLongToolsViewer(QWidget *parent)
: QDeclarativeView(parent), d(new EditLongToolsViewerPrivate(this)), nested_window(parent), nested_item(0), m_commandStack(0), d_table(0), p_table(0), dataes_view(0), minfo_view(0), plots_tview(0), total_view(0), plots_view(0), edit_table(0)
{
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);	
	setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags(Qt::FramelessWindowHint);
	connect(engine(), SIGNAL(quit()), SLOT(close()));
	setResizeMode(QDeclarativeView::SizeRootObjectToView);
    // Qt versions prior to 4.8.0 don't have QML/JS debugging services built in
#if defined(QMLJSDEBUGGER) && QT_VERSION < 0x040800
#if !defined(NO_JSDEBUGGER)
	new QmlJSDebugger::JSDebuggerAgent(d->view->engine());
#endif
#if !defined(NO_QMLOBSERVER)
	new QmlJSDebugger::QDeclarativeViewObserver(d->view, d->view);
#endif
#endif
}

EditLongToolsViewer::~EditLongToolsViewer()
{
	delete d;
}

EditLongToolsViewer * EditLongToolsViewer::create()
{
#ifdef HARMATTAN_BOOSTER
	return new EditLongToolsViewer(MDeclarativeCache::qDeclarativeView(), 0);
#else
	return new EditLongToolsViewer();
#endif
}

void EditLongToolsViewer::setMainQmlFile(const QString & file, int q_type)
{
	d->mainQmlFile = d->adjustPath(file);
	d->view->setSource(QUrl::fromLocalFile(d->mainQmlFile));
	qml_type = q_type;
	if (q_type < 8)
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setFixedWidth(nested_window->width());
		connect(rootObject(), SIGNAL(hideReturnlastToWin(QString)), this, SIGNAL(sendStopInfoToMainWin(QString)));
		connect(rootObject(), SIGNAL(sendResourceStateToWin(QString, bool)), this, SIGNAL(relatedSelectedResource(QString, bool)));
		connect(rootObject(), SIGNAL(sendUndoRedo(QString)), this, SLOT(undoRedoActFromQml(QString)));
		connect(this, SIGNAL(ungrayForUndo(QVariant)), rootObject(), SLOT(grayUndoImage(QVariant)));
		connect(this, SIGNAL(ungrayForRedo(QVariant)), rootObject(), SLOT(grayRedoImage(QVariant)));
	}
	else		
		connect(this, SIGNAL(qmlRecSizeChanged(QVariant, QVariant)), rootObject(), SLOT(redefineRecSize(QVariant, QVariant)));	
}

void EditLongToolsViewer::setNestedPosItem(QStandardItem * p_item)
{
	nested_item = p_item;
}

void EditLongToolsViewer::addImportPath(const QString &path)
{
	d->view->engine()->addImportPath(d->adjustPath(path));
}

void EditLongToolsViewer::setOrientation(ScreenOrientation orientation)
{
#if defined(Q_OS_SYMBIAN)
    // If the version of Qt on the device is < 4.7.2, that attribute won't work
	if (orientation != ScreenOrientationAuto) 
	{
		const QStringList v = QString::fromAscii(qVersion()).split(QLatin1Char('.'));
		if (v.count() == 3 && (v.at(0).toInt() << 16 | v.at(1).toInt() << 8 | v.at(2).toInt()) < 0x040702) 
		{
			qWarning("Screen orientation locking only supported with Qt 4.7.2 and above");
			return;
		}
	}
#endif // Q_OS_SYMBIAN
	Qt::WidgetAttribute attribute;
	switch (orientation) 
	{
#if QT_VERSION < 0x040702
    // Qt < 4.7.2 does not yet have the Qt::WA_*Orientation attributes
		case ScreenOrientationLockPortrait:
			attribute = static_cast<Qt::WidgetAttribute>(128);
			break;
		case ScreenOrientationLockLandscape:
			attribute = static_cast<Qt::WidgetAttribute>(129);
			break;
		default:
			case ScreenOrientationAuto:
				attribute = static_cast<Qt::WidgetAttribute>(130);
				break;
#else // QT_VERSION < 0x040702
			case ScreenOrientationLockPortrait:
				attribute = Qt::WA_LockPortraitOrientation;
				break;
			case ScreenOrientationLockLandscape:
				attribute = Qt::WA_LockLandscapeOrientation;
				break;
		default:
			case ScreenOrientationAuto:
				attribute = Qt::WA_AutoOrientation;
				break;
#endif // QT_VERSION < 0x040702
	};
	setAttribute(attribute, true);
}

void EditLongToolsViewer::showExpanded()
{
#if defined(Q_OS_SYMBIAN) || defined(MEEGO_EDITION_HARMATTAN) || defined(Q_WS_SIMULATOR)
	d->view->showFullScreen();
#elif defined(Q_WS_MAEMO_5)
	d->view->showMaximized();
#else
	d->view->show();
#endif
}

void EditLongToolsViewer::setRelatedTreePt(int t_type, TableManagement * m_table)
{
	if (qml_type < 8)
	{
		connect(rootObject(), SIGNAL(sendToRightContainer(QString, bool)), m_table, SLOT(relatedStyleTreeActions(QString, bool)));
		connect(m_table, SIGNAL(newPlotEditCommand(TableMgmtPlotEditCmd *)), this, SLOT(pushPlotEditCommand(TableMgmtPlotEditCmd *)));
	}
	else
		connect(rootObject(), SIGNAL(selectingOnRec(bool)), m_table, SLOT(listenFreezingState(bool)));// for little rec nested m_table freezing right tree slide
	if (qml_type == 10)
		connect(rootObject(), SIGNAL(selectedContents(int)), m_table, SLOT(replyForSelectedStrData(int)));
	if (qml_type==8 || qml_type==9)
	{
		if (qml_type==8)
		{
			connect(m_table, SIGNAL(timeDefineTitle(QVariant)), rootObject(), SLOT(initTimeTitle(QVariant)));
			connect(rootObject(), SIGNAL(selectedYearMonth(int, int)), m_table, SLOT(resetDatelist(int, int)));
			connect(rootObject(), SIGNAL(endSelectedDate(int, int, int)), m_table, SLOT(endSelectionForDate(int, int, int)));
		}
		else
		{
			connect(rootObject(), SIGNAL(realWidthReturn(int)), m_table, SLOT(viewerRealWidthReset(int)));
			connect(rootObject(), SIGNAL(selectedContents(QString)), m_table, SLOT(endSelectionForNumber(QString)));
		}
		connect(rootObject(), SIGNAL(closeViewer()), m_table, SLOT(closeNestedWidget()));
	}
	if (t_type == 0)
		dataes_view = m_table;
	else if (t_type==1)
		minfo_view = m_table;
	else if (t_type>2 && t_type<6)
		plots_tview = m_table;	
	else if (t_type>5 && t_type<8)
		total_view = m_table;		
}
	
void EditLongToolsViewer::setPlotsContainerPt(LittleWidgetsView * p_container)
{
	if (!views_undos.contains(p_container))
	{
		m_commandStack = new QUndoStack(this);
		sigSlotLinkForViewStack(p_container, m_commandStack);
		views_undos.insert(p_container, m_commandStack);
	}
	else
		resetStateForViewsRedos(p_container);
	plots_view = p_container;	
}

void EditLongToolsViewer::setEditingTable(TablesGroup * editting)
{
	if (!m_commandStack)
	{
		m_commandStack = new QUndoStack(this);
		sigSlotLinkForTgrpStack(editting, m_commandStack);
		tables_undos.insert(editting, m_commandStack);
	}
	else
		resetStateForTablesRedos(editting);
	edit_table = editting;	
}

void EditLongToolsViewer::setOtherEditingTable(QWidget * editting)
{
	d_table = qobject_cast<DataesTableView *>(editting);
	p_table = qobject_cast<RelatedTreeView *>(editting);
	if (d_table)
	{
		connect(rootObject(), SIGNAL(selectedContents(int)), d_table, SLOT(replyForSelectedStrs(int)));
//		connect(rootObject(), SIGNAL(selectingOnRec(bool)), d_table, SLOT(listenFreezingState(bool)));
	}
	else
	{
		connect(rootObject(), SIGNAL(selectedContents(int)), p_table, SLOT(replyForSelectedStrs(int)));
		connect(rootObject(), SIGNAL(selectingOnRec(bool)), p_table, SLOT(listenFreezingState(bool)));
	}
}

void EditLongToolsViewer::setNewStrList(const QStringList new_list)
{
	rootContext() -> setContextProperty("StringesList", QVariant::fromValue(new_list));
	qml_type += 1;	
}

int EditLongToolsViewer::qmlDelegateType()
{
	return qml_type;
}

int EditLongToolsViewer::currentStringPos()
{
	return str_pos;
}

QStandardItem * EditLongToolsViewer::currentNesttingItem()
{
	return nested_item;
}

QUndoStack * EditLongToolsViewer::commandStack()
{
	return m_commandStack;
}

QHash<LittleWidgetsView *, QUndoStack *> & EditLongToolsViewer::undoStackHash()
{
	return views_undos;
}

void EditLongToolsViewer::killSelf()
{
	close();
	emit killMyself(this);
}

void EditLongToolsViewer::rectangleAnimating(const QVariant & rec)
{
	emit qmlRecSizeChanged(rec.toRect().width(), rec.toRect().height());
}

void EditLongToolsViewer::setSelectedPos(int pos)
{
	str_pos = pos;
}

void EditLongToolsViewer::pushGroupEditCommand(TableMgmtGroupEditCmd * comm)
{
	m_commandStack -> push(comm);
}

void EditLongToolsViewer::undoChanged(bool c_undo)
{
	emit ungrayForUndo(c_undo);
}
	
void EditLongToolsViewer::redoChanged(bool c_redo)
{
	emit ungrayForRedo(c_redo);
}

void EditLongToolsViewer::undoRedoActFromQml(QString func)
{
	if (func == tr("撤销"))
		m_commandStack -> undo();
	else if (func == tr("重做"))
		m_commandStack -> redo();			
	else
	{
		if (func.contains(tr("存储")))
			emit sendSaveSigToWin(true);
		else
			emit sendSaveSigToWin(false);
	}	
}

void EditLongToolsViewer::replyPlotPositionChanging(QStandardItem * old_pos, QStandardItem * new_pos, PlotWidget * mv_plot, PlotWidget * move_to)
{
	TableMgmtPlotEditCmd * push_new = 0;
	if (!new_pos->text().isEmpty())
	{
		QString mv_pos(tr("移动位置"));
		push_new = new TableMgmtPlotEditCmd(plots_tview, old_pos, new_pos, mv_pos, plots_view, mv_plot, move_to);
	}
	else
	{
		QString row_matrix = tr("拆分列");
		push_new = new TableMgmtPlotEditCmd(plots_tview, old_pos, new_pos, row_matrix, plots_view, mv_plot, 0);
	}
	pushPlotEditCommand(push_new);
}

void EditLongToolsViewer::unredoForMatrixChange(LittleWidgetsView * qml, int old_rc, int new_rc, bool row_col, const QString & undo_order, const QHash<QPair<int, int>, int> & hide_widgets, const QList<PlotWidget *> & hide_plots)
{
	TableMgmtPlotEditCmd * push_new = new TableMgmtPlotEditCmd(undo_order, plots_tview, qml, old_rc, new_rc, row_col, hide_widgets, hide_plots);
	pushPlotEditCommand(push_new);	
}

void EditLongToolsViewer::pushPlotEditCommand(TableMgmtPlotEditCmd * comm)
{
	m_commandStack -> push(comm);
}

void EditLongToolsViewer::resetStateForViewsRedos(LittleWidgetsView * trans_view)
{
	m_commandStack = views_undos.value(trans_view);
	emit ungrayForRedo(m_commandStack->canRedo());
	emit ungrayForUndo(m_commandStack->canUndo());
}

void EditLongToolsViewer::resetStateForTablesRedos(TablesGroup * t_trans)
{
	if (tables_undos.contains(t_trans))
		m_commandStack = tables_undos.value(t_trans);
	else
	{
		m_commandStack = new QUndoStack(this);
		sigSlotLinkForTgrpStack(t_trans, m_commandStack);
		tables_undos.insert(t_trans, m_commandStack);
	}
	emit ungrayForRedo(m_commandStack->canRedo());
	emit ungrayForUndo(m_commandStack->canUndo());
}

void EditLongToolsViewer::sigSlotLinkForViewStack(LittleWidgetsView * view, QUndoStack * stack)
{
	connect(stack, SIGNAL(canRedoChanged(bool)), this, SLOT(redoChanged(bool)));
	connect(stack, SIGNAL(canUndoChanged(bool)), this, SLOT(undoChanged(bool)));	
	connect(rootObject(), SIGNAL(sendToLeftContainer(QString, QString)), view, SLOT(relatedWorkFromQmlEditor(QString, QString)));
	connect(view, SIGNAL(plotPositionChanging(QStandardItem *, QStandardItem *, PlotWidget *, PlotWidget *)), this, SLOT(replyPlotPositionChanging(QStandardItem *, QStandardItem *, PlotWidget *, PlotWidget *)));
	connect(view, SIGNAL(sendPlotShowToQmlBtns(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)), rootObject(), SLOT(btnsEnable(QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant)));	
	connect(view, SIGNAL(unredoOrderForMatrixChange(LittleWidgetsView *, int, int, bool, const QString &, const QHash<QPair<int, int>, int> &, const QList<PlotWidget *> &)), this, SLOT(unredoForMatrixChange(LittleWidgetsView *, int, int, bool, const QString &, const QHash<QPair<int, int>, int> &, const QList<PlotWidget *> &)));	
}
	
void EditLongToolsViewer::sigSlotLinkForTgrpStack(TablesGroup * grp, QUndoStack * stack)
{
	connect(stack, SIGNAL(canRedoChanged(bool)), this, SLOT(redoChanged(bool)));
	connect(stack, SIGNAL(canUndoChanged(bool)), this, SLOT(undoChanged(bool)));	
	connect(rootObject(), SIGNAL(sendToLeftContainer(QString, QString)), grp, SLOT(relatedWorkFromQmlEditor(QString, QString)));
	connect(grp, SIGNAL(tablegroupCmd(TableMgmtGroupEditCmd *)), this, SLOT(pushGroupEditCommand(TableMgmtGroupEditCmd *)));
}

