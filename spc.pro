QT += core gui
QT += sql
QT += declarative
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app
TARGET = spc-qwt6
INCLUDEPATH += .

# Input
RESOURCES = spc.qrc
HEADERS += 	qwt_global.h \
		qwt_math.h \
		qwt_legend_itemmanager.h \
		qwt_interval.h \
		qwt_plot.h \
		qwt_plot_layout.h \
		qwt_scale_widget.h \
		qwt_scale_draw.h \
		qwt_abstract_scale_draw.h \
		qwt_scale_div.h \
		qwt_scale_engine.h \
		qwt_text_label.h \
		qwt_legend.h \
		qwt_dyngrid_layout.h \
		qwt_plot_canvas.h \
		qwt_painter.h \
		qwt_color_map.h \
		qwt_text_engine.h \
		qwt_legend_item.h \
		qwt_null_paintdevice.h \
		qwt_clipper.h \
		qwt_symbol.h \
		qwt_point_polar.h \
		qwt_plot_marker.h \
		qwt_plot_curve.h \
		qwt_plot_seriesitem.h \
		qwt_series_data.h \
		qwt_point_3d.h \
		qwt_curve_fitter.h \
		qwt_spline.h \
		qwt_plot_grid.h \
		qwt_plot_dict.h \
		qwt_text.h \
		qwt_plot_item.h \
		qwt_scale_map.h \
		mainframe.h \
		titlebar.h \
		mainwindow.h \
		slidetoolbar.h \
		editlongtoolsviewer.h \
		spcnum.h \
		spcdatabase.h \
		helpcontents.h \
		matrixwidget.h \
		plotwidget.h \
		littlewidgetitem.h \
		littlewidgetsview.h \
		engitreeview.h \
		tablemanagement.h \
		dataselectmodel.h \
		tablesgroup.h \
		printframe.h \
		exportstylegenerator.h \
		dataestableview.h \
		relatedtreeview.h \
		nestedmodel.h \
		dialog.h \
		plot.h \
		uncontinueaxelmarkers.h \
		relatedtreecommand.h \
		dataestableeditcommand.h \
		tablemgmtploteditcommand.h \
		tablemgmtgroupeditcommand.h \
		ptsymbol.h \
		qwt_column_symbol.h \
		qwt_plot_histogram.h \
		toolsplot.h
SOURCES +=	main.cpp \
		qwt_math.cpp \
		qwt_plot_axis.cpp \
		qwt_interval.cpp \
		qwt_plot.cpp \
		qwt_plot_layout.cpp \
		qwt_scale_widget.cpp \
		qwt_text.cpp \
		qwt_plot_item.cpp \
		qwt_scale_map.cpp \
		qwt_scale_draw.cpp \
		qwt_abstract_scale_draw.cpp \
		qwt_scale_div.cpp \
		qwt_scale_engine.cpp \
		qwt_text_label.cpp \
		qwt_legend.cpp \
		qwt_dyngrid_layout.cpp \
		qwt_plot_canvas.cpp \
		qwt_painter.cpp \
		qwt_color_map.cpp \
		qwt_text_engine.cpp \
		qwt_legend_item.cpp \
		qwt_null_paintdevice.cpp \
		qwt_clipper.cpp \
		qwt_symbol.cpp \
		qwt_point_polar.cpp \
		qwt_plot_marker.cpp \
		qwt_plot_curve.cpp \
		qwt_plot_seriesitem.cpp \
		qwt_series_data.cpp \
		qwt_point_3d.cpp \
		qwt_curve_fitter.cpp \
		qwt_spline.cpp \
		qwt_plot_grid.cpp \
		qwt_plot_dict.cpp \
		qwt_plot_xml.cpp \
		mainframe.cpp \
		titlebar.cpp \
		mainwindow.cpp \
		slidetoolbar.cpp \
		editlongtoolsviewer.cpp \
		spcnum.cpp \
		spcdatabase.cpp  \
		helpcontents.cpp \
		matrixwidget.cpp \
		plotwidget.cpp \
		engitreeview.cpp \
		tablemanagement.cpp \
		dataselectmodel.cpp \	
		tablesgroup.cpp \
		printframe.cpp \
		exportstylegenerator.cpp \
		dataestableview.cpp \
		relatedtreeview.cpp \
		nestedmodel.cpp \
		dialog.cpp \
		plot.cpp \
		uncontinueaxelmarkers.cpp \
		littlewidgetitem.cpp \
		littlewidgetsview.cpp \
		relatedtreecommand.cpp \
		dataestableeditcommand.cpp \
		tablemgmtploteditcommand.cpp \
		tablemgmtgroupeditcommand.cpp \
		ptsymbol.cpp \
		qwt_column_symbol.cpp \
		qwt_plot_histogram.cpp \
		toolsplot.cpp
#LIBS += /usr/lib64/libmaliit.so.0
#LIBS += /usr/lib64/maliit/plugins/libmaliit-keyboard-plugin.so
#LIBS += /usr/lib64/libmaliit-plugins.so
#LIBS += /usr/lib64/maliit/plugins/libfcitx-maliit.so
