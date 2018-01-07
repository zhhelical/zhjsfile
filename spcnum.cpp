#include <QtSql/QSqlTableModel>
#include "spcnum.h"
#include "plotwidget.h"
#include "dataestableview.h"
#include "spcdatabase.h"

SpcNum::SpcNum(QObject * parent)
:QObject(parent), current_plots(0), spcdata_db(0)
{
	initNewFilePara();	
}

SpcNum::~SpcNum()
{}

void SpcNum::setDatabase(SpcDataBase * db)
{
	spcdata_db = db;
}

void SpcNum::setCalculatingPlots(PlotWidget * w_pt)
{
	current_plots = w_pt;
}

void SpcNum::runSpcInitCal(DataesTableView * e_view, DataesTableView * g_view)
{
	QList<double> cal_res;
	int i_h=0 , i_v=0;
	if (e_view->model()->headerData(0, Qt::Vertical).toString().contains(tr("组")))
	{
		i_h = e_view->model()->columnCount();
		i_v = e_view->model()->rowCount();
	}
	else
	{
		i_h = e_view->model()->rowCount();
		i_v = e_view->model()->columnCount();		
	}
	QMap<int, QList<double> > tolDataes;
	QString engi_description = g_view->model()->data(g_view->model()->index(0, 0)).toString();
	for (int i = 0; i < i_v; i++)
	{
		QList<double> e_list;
		for (int j = 0; j < i_h; j++)
			e_list << e_view->model()->data(e_view->model()->index(i, j)).toDouble();
		tolDataes.insert(i, e_list);
	}
	engiParaesCal(g_view, tolDataes, cal_res);
	engi_description += "^"+g_view->model()->data(g_view->model()->index(3, 0)).toString();
	emit showCalResTable(engi_description, cal_res);
}

void SpcNum::dailyDatesCal(DataesTableView * daily_view)
{
	QList<double> daily_dataes;
	for (int i = 1; i < daily_view->model()->rowCount()-5; i++)
		daily_dataes.push_back(daily_view->model()->data(daily_view->model()->index(i, 0)).toDouble());
	num_avr = averageDataes(daily_dataes);
	daily_view->model()->setData(daily_view->model()->index(daily_view->model()->rowCount()-2, 0), num_avr);
	startAvrPtsJudge(num_avr);
	QString is_sigma = daily_view->model()->headerData(daily_view->model()->rowCount()-3, Qt::Vertical).toString();
	if (is_sigma.contains(tr("西格玛")))
	{
		num_s = devCalFromDataes(daily_dataes);
		daily_view->model()->setData(daily_view->model()->index(daily_view->model()->rowCount()-4, 0), num_s);
		startDevPtsJudge(num_s);
	}
	else
	{
		num_r = rngCalFromDataes(daily_dataes);
		daily_view->model()->setData(daily_view->model()->index(daily_view->model()->rowCount()-4, 0), num_r);
		startRngPtsJudge(num_r);
	}
	daily_view -> setCalResDataesForDaily();
	current_plots -> respondMultiDailyNewData(daily_view);
}

void SpcNum::startPtRuningJudging(double val, int type)
{
	if (type == 0)
		startAvrPtsJudge(val);
	else if (type == 1)
		startDevPtsJudge(val);
	else
		startRngPtsJudge(val);
}

void SpcNum::setEngiDescriptions(const QPair<QString, QString> & paircontent)
{
	engi_carrying << paircontent;
}

void SpcNum::setRelatedDataesByGroups(int from_num, int to_num, const QString & proj, QMap<int, QList<double> > & map_dataes)
{
	QSqlQueryModel * d_nested = new QSqlQueryModel;
	spcdata_db -> dataesByGroupsFromDb(from_num, to_num, proj, d_nested);
	for (int i = 0; i < d_nested->rowCount(); i++)
	{
		QList<double> dataes_list;
		if (proj.contains(tr("，，。dailydataes")))
		{
			QVariant dataes = d_nested->data(d_nested->index(i, 1));
			dailyDataesTransFromDB(dataes, dataes_list);
		}
		else
			dataes_list.push_back(d_nested->data(d_nested->index(i, 1)).toDouble());
		map_dataes.insert(d_nested->data(d_nested->index(i, 0)).toInt(), dataes_list);
	}
	delete d_nested;	
}

void SpcNum::cpkInitModelFromDb(int type, const QString & project, QStandardItemModel * cpk_model)
{
	QMap<int, QList<double> > tmp_cpkMap;
	cpkDataesTransFromDB(project, tmp_cpkMap);
	if (type == 0)
	{
		cpk_model -> setRowCount(1);
		cpk_model -> setColumnCount(1);
		cpk_model -> insertRows(1, tmp_cpkMap.size()+1);
		if (tmp_cpkMap.value(1).size() > 1)
			cpk_model -> insertColumns(1, tmp_cpkMap.value(1).size());
		bool tophead_copyed = false;	 
		for (int i = 1; i < tmp_cpkMap.size()+1; i++)
		{
			for (int j = 0; j < tmp_cpkMap.value(1).size(); j++)
			{
				if (!tophead_copyed && j<tmp_cpkMap.value(1).size())
					cpk_model -> setData(cpk_model->index(i, j+1), QString(tr("样本%1")).arg(j+1));
				if (j == tmp_cpkMap.value(1).size()-1)
					tophead_copyed = true;			
				cpk_model -> setData(cpk_model->index(i+1, j+1), tmp_cpkMap.value(i).at(j));
			}
			cpk_model -> setData(cpk_model->index(i+1, 0), QString(tr("样本组%1")).arg(i));
		}
	}
	else if (type == 7)
		cpkHistogramTranslation(tmp_cpkMap, cpk_model);
	else
		cpkGaussTranslation(tmp_cpkMap, cpk_model);		
}

void SpcNum::cpkHistogramTranslation(const QMap<int, QList<double> > & dataes_map, QStandardItemModel * histo_model)
{
	QList<double> tol_dataes = mergeDataesFromMap(dataes_map);
	double low = selectedGroupsMin(tol_dataes);
	double high = selectedGroupsMax(tol_dataes);
	double tol_range = high-low;
	int grp_histogram = calForHistogramGroups(tol_dataes.size());
	double unit_size = HistogramUnitSize(tol_range, grp_histogram);
	double init = HistogramUnitLowMark(low);
	histo_model -> setRowCount(grp_histogram+2);
	histo_model -> setColumnCount(3);
	histo_model -> setData(histo_model->index(0, 0), tr("组距"));
	histo_model -> setData(histo_model->index(0, 1), unit_size);
	histo_model -> setData(histo_model->index(0, 2), tr("频率"));
	histo_model -> setData(histo_model->index(1, 0), tr("组界"));
	qSort(tol_dataes);
	for (int j = 0; j < grp_histogram+1; j++)
	{
		double low_rng = init+unit_size*j;
		double up_rng = init+unit_size*(j+1);
		histo_model -> setData(histo_model->index(j+1, 1), low_rng);
		histo_model -> setData(histo_model->index(j+1, 2), freqDisForHistogram(up_rng, tol_dataes));
	}	
}

void SpcNum::cpkGaussTranslation(const QMap<int, QList<double> > & dataes_map, QStandardItemModel * gauss_model)
{
	QList<double> tol_dataes = mergeDataesFromMap(dataes_map);
	double low = selectedGroupsMin(tol_dataes);
	double high = selectedGroupsMax(tol_dataes);
	double tol_range = high-low;
	int grp_histogram = calForHistogramGroups(tol_dataes.size());
	double unit_size = HistogramUnitSize(tol_range, grp_histogram);
	double init = HistogramUnitLowMark(low);
	gauss_model -> setRowCount(grp_histogram+1);
	gauss_model -> setColumnCount(3);
	gauss_model -> setData(gauss_model->index(0, 0), tr("分组"));
	gauss_model -> setData(gauss_model->index(0, 1), tr("频率"));
	gauss_model -> setData(gauss_model->index(0, 2), tr("累计频率"));
	for (int j = 0; j < grp_histogram+1; j++)
	{
		double low_rng = init+unit_size*j;
		double up_rng = init+unit_size*(j+1);
		gauss_model -> setData(gauss_model->index(j+1, 0), low_rng);
		gauss_model -> setData(gauss_model->index(j+1, 1), normFreqSum(low_rng, up_rng, tol_dataes));
		double sums = 0.0;
		if (j > 0)
			sums += gauss_model->data(gauss_model->index(j, 2)).toDouble();
		gauss_model -> setData(gauss_model->index(j+1, 2), normFreqSum(low_rng, up_rng, tol_dataes)+sums);
	}	
}

void SpcNum::selectedDataesModel(int from, int to, int type, QStandardItemModel * dataes_model, const QString & engine)
{
	QMap<int, QList<double> > tmp_selectedMap;
	setRelatedDataesByGroups(from, to, engine, tmp_selectedMap);
	if (type == 7)
	{
		dataes_model -> setRowCount(tmp_selectedMap.size()+1);
		dataes_model -> setColumnCount(tmp_selectedMap.value(0).size());
		dataes_model -> setData(dataes_model->index(0, 0), tr("测量值表"));	 
		for (int i = 0; i < tmp_selectedMap.size(); i++)
		{
			for (int j = 0; j < tmp_selectedMap.value(0).size(); j++)
				dataes_model -> setData(dataes_model->index(i+1, j), tmp_selectedMap.value(i).at(j));
		}
	}
	else if (type == 8)
	{
		QList<double> tol_dataes = mergeDataesFromMap(tmp_selectedMap);
		double low = selectedGroupsMin(tol_dataes);
		double high = selectedGroupsMax(tol_dataes);
		double tol_range = high-low;
		int grp_histogram = calForHistogramGroups(tol_dataes.size());
		double unit_size = HistogramUnitSize(tol_range, grp_histogram);
		double init = HistogramUnitLowMark(low);
		dataes_model -> setRowCount(grp_histogram+1);
		dataes_model -> setColumnCount(3);
		dataes_model -> setData(dataes_model->index(0, 0), tr("组距"));
		dataes_model -> setData(dataes_model->index(0, 1), unit_size);
		dataes_model -> setData(dataes_model->index(0, 2), tr("频率"));
		dataes_model -> setData(dataes_model->index(1, 0), tr("组界"));
		qSort(tol_dataes);
		for (int j = 0; j < grp_histogram; j++)
		{
			double low_rng = init+unit_size*j;
			double up_rng = init+unit_size*(j+1);
			dataes_model -> setData(dataes_model->index(j+1, 1), low_rng);
			dataes_model -> setData(dataes_model->index(j+1, 2), freqDisForHistogram(up_rng, tol_dataes));
		}		
	}
	else
	{
		QList<double> tol_dataes = mergeDataesFromMap(tmp_selectedMap);
		double low = selectedGroupsMin(tol_dataes);
		double high = selectedGroupsMax(tol_dataes);
		double tol_range = high-low;
		int grp_histogram = calForHistogramGroups(tol_dataes.size());
		double unit_size = HistogramUnitSize(tol_range, grp_histogram);
		double init = HistogramUnitLowMark(low);
		dataes_model -> setRowCount(grp_histogram+1);
		dataes_model -> setColumnCount(3);
		dataes_model -> setData(dataes_model->index(0, 0), tr("分组"));
		dataes_model -> setData(dataes_model->index(0, 1), tr("频率"));
		dataes_model -> setData(dataes_model->index(0, 2), tr("累计频率"));
		qSort(tol_dataes);
		for (int j = 0; j < grp_histogram; j++)
		{
			double low_rng = init+unit_size*j;
			double up_rng = init+unit_size*(j+1);
			dataes_model -> setData(dataes_model->index(j+1, 0), low_rng);
			dataes_model -> setData(dataes_model->index(j+1, 1), normFreqSum(low_rng, up_rng, tol_dataes));
			double sums = 0.0;
			if (j > 0)
				sums += dataes_model->data(dataes_model->index(j, 2)).toDouble();
			dataes_model -> setData(dataes_model->index(j+1, 2), normFreqSum(low_rng, up_rng, tol_dataes)+sums);
		}		
	}	
}

void SpcNum::dbSpcNamesTranslation(const QString & transed, QString & chinese)
{
	if (transed == "name")
		chinese = QString(tr("名称"));
	else if (transed == "password")
		chinese = QString(tr("密码"));
	else if (transed == "department")
		chinese = QString(tr("部门"));
	else if (transed == "projects")
		chinese = QString(tr("工程"));	
	else if (transed == "title")
		chinese = QString(tr("职位"));
	else if (transed == "departments")
		chinese = QString(tr("部门"));
	else if (transed == "managers")
		chinese = QString(tr("管理者"));
	else if (transed == "authoagent")
		chinese = QString(tr("授权代表"));
	else if (transed == "attribution")
		chinese = QString(tr("工程状态"));
	else if (transed == "authorizing")
		chinese = QString(tr("待授权者"));
	else if (transed == "company")
		chinese = QString(tr("公司"));
	else if (transed == "unit")
		chinese = QString(tr("测量单位"));
	else if (transed == "precision")
		chinese = QString(tr("测量精度"));
	else if (transed == "ctrlplot")
		chinese = QString(tr("控制类型"));
	else if (transed == "uplimit")
		chinese = QString(tr("上规范限"));
	else if (transed == "lowlimit")
		chinese = QString(tr("下规范限"));
	else if (transed == "grpsamles")
		chinese = QString(tr("每组样本"));
	else if (transed == "groups")
		chinese = QString(tr("样本组数"));
	else if (transed == "desnum")
		chinese = QString(tr("目标值"));
	else if (transed == "constructor")
		chinese = QString(tr("创建者"));
	else if (transed == "constructtime")
		chinese = QString(tr("创建时间"));
	else if (transed == "dataes")
		chinese = QString(tr("数据"));
	else if (transed == "groupnum")
		chinese = QString(tr("组序"));
	else if (transed == "avr")
		chinese = QString(tr("平均值"));
	else if (transed == "time")
		chinese = QString(tr("时间"));
	else if (transed == "person")
		chinese = QString(tr("工作者"));
	else if (transed == "dev")
		chinese = QString(tr("离散值"));
	else if (transed == "dsigma")
		chinese = QString(tr("标准差"));
	else if (transed == "cpk")
		chinese = QString(tr("工程CPK"));
	else if (transed == "upavr")
		chinese = QString(tr("均值上限"));
	else if (transed == "lowavr")
		chinese = QString(tr("均值下限"));
	else if (transed == "updev")
		chinese = QString(tr("西格玛上限"));
	else if (transed == "lowdev")
		chinese = QString(tr("西格玛下限"));
	else if (transed == "accept")
		chinese = QString(tr("控制状态"));
	else if (transed == "rng")
		chinese = QString(tr("极差"));
	else if (transed == "rsigma")
		chinese = QString(tr("标准差"));
	else if (transed == "uprng")
		chinese = QString(tr("极差上限"));
	else if (transed == "lowrng")
		chinese = QString(tr("极差下限"));
	else if (transed == "state")
		chinese = QString(tr("点控状态"));
	else if (transed.contains("ormal"))
	{
		if (transed.contains("unormal"))
		{
			chinese = QString(tr("准则"));
			if (transed.contains("0unormal"))
				chinese += QString("1");
			if (transed.contains("5unormal"))
			{
				if (chinese == tr("准则"))
					chinese += QString("2");
				else
					chinese += ","+QString("2");
			}
			if (transed.contains("3unormal"))
			{
				if (chinese == tr("准则"))
					chinese += QString("3");
				else
					chinese += ","+QString("3");
			}
			if (transed.contains("6unormal"))
			{
				if (chinese == tr("准则"))
					chinese += QString("4");
				else
					chinese += ","+QString("4");
			}
			if (transed.contains("1unormal"))
			{
				if (chinese == tr("准则"))
					chinese += QString("5");
				else
					chinese += ","+QString("5");
			}		
			if (transed.contains("2unormal"))
			{
				if (chinese == tr("准则"))
					chinese += QString("6");
				else
					chinese += ","+QString("6");
			}
			if (transed.contains("7unormal"))
			{
				if (chinese == tr("准则"))
					chinese += QString("7");
				else
					chinese += ","+QString("7");
			}
			if (transed.contains("4unormal"))
			{
				if (chinese == tr("准则"))
					chinese += QString("8");
				else
					chinese += ","+QString("8");
			}
			chinese += tr("失控");
		}
		else
			chinese = QString(tr("正常"));
	}
	else
		chinese = transed;
}

void SpcNum::dailyDataesTransFromDB(QVariant dataes, QList<double> & data_list)
{
	QString tmp_dataes1 = dataes.toString();
	QStringList tmp_dataes2 = tmp_dataes1.split(",", QString::SkipEmptyParts);
	for (int i = 0; i < tmp_dataes2.size(); i++)
		data_list << tmp_dataes2[i].toDouble();
}

void SpcNum::randListGenerate(QList<int> & rand_list, const QList<int> & old_list)
{
	QList<int> test_list;
	if (!old_list.isEmpty())
		test_list = old_list;
	foreach (int r, rand_list)
	{
		r = randNumberGenerate();
		while (test_list.contains(r))
			r = randNumberGenerate();
		test_list << r;	
	}  
	rand_list = test_list;
}

void SpcNum::compareTwoTimeLists(const QStringList & base_list, const QStringList & c_list, QString & compare_result)
{
	if (base_list == c_list)
	{
		compare_result = tr("时间串相同");
		return;
	}
	int base_size = base_list.size();
	int c_size = c_list.size();
	QStringList base_copys = base_list;
	QStringList c_copys = c_list;
	if (base_size > c_size)
	{
		QStringList match_list;
		foreach (QString b_time, base_list)
		{
			if (c_list.contains(b_time))
				match_list << base_copys.takeAt(base_copys.indexOf(b_time));
		}
		if (match_list.isEmpty())
		{
			compare_result = tr("时间串不相关");	
			return;
		}
		if (match_list==c_list && base_list.indexOf(match_list[0])==0 && base_list==match_list+base_copys)
			compare_result = tr("时间串包含");
		else
			compare_result = tr("时间串混杂");
	}
	else if (base_size == c_size)
		compare_result = tr("时间串不同");
	else
	{
		QStringList match_list;
		foreach (QString c_time, c_list)
		{
			if (base_list.contains(c_time))
				match_list << c_copys.takeAt(c_copys.indexOf(c_time));
		}
		if (match_list.isEmpty())
		{
			compare_result = tr("时间串不相关");	
			return;
		}		
		if (match_list==base_list && c_list.indexOf(match_list[0])==0 && c_list==match_list+c_copys)
			compare_result = tr("时间串被包含");
		else
			compare_result = tr("时间串混杂");		
	}
}

void SpcNum::insertListByTimeSequence(const QStringList & insing_list, QStringList & insed_list)
{
	QList<QDateTime> dt_list;
	foreach (QString trans_str, insed_list)
	{
		if (trans_str.contains(tr("，")))
		{
			QStringList t_list = trans_str.split(tr("，"));
			dt_list << QDateTime::fromString(t_list.at(0));
		}
		else
			dt_list << QDateTime::fromString(trans_str, Qt::ISODate);
	}
	foreach (QString trans_str, insing_list)
	{
		QDateTime i_dt;
		if (trans_str.contains(tr("，")))
		{
			QStringList t_list = trans_str.split(tr("，"));
			i_dt = QDateTime::fromString(t_list.at(0));
		}
		else
			i_dt = QDateTime::fromString(trans_str, Qt::ISODate);	  
		if (!dt_list.contains(i_dt))
		{
			foreach (QDateTime dt, dt_list)
			{
				if (i_dt < dt)
				{
					if (dt_list.indexOf(dt) == 0)
					{
						insed_list.insert(0, trans_str);
						dt_list.insert(0, i_dt);
						break;
					}
					insed_list.insert(dt_list.indexOf(dt)-1, trans_str);
					dt_list.insert(dt_list.indexOf(dt)-1, i_dt);
					break;
				}
				if (i_dt > dt)
				{
					insed_list.insert(dt_list.indexOf(dt)+1, trans_str);
					dt_list.insert(dt_list.indexOf(dt)+1, i_dt);
					break;
				}				
			}
		}
	}	
}

void SpcNum::initNewFilePara()
{
	cpk_tolAvr = 0.0;
	num_avr = 0.0;
	num_s = 0.0;
	num_r = 0.0;
	data.clear();
	u_data.clear();
	tol_dataes.clear();	
	product_pair.clear();
	engi_carrying.clear();	
	current_plots = 0;
}

void SpcNum::sortItemSequenceByTime(const QList<QStandardItem *> & sort_items, QMap<int, QStandardItem *> & sorted_map)
{
	if (sort_items.isEmpty())
		return;
	QList<QDateTime> base_sorting;
	foreach (QStandardItem * s_item, sort_items)
	{
		QDateTime s_dt = QDateTime::fromString(s_item->text());
		base_sorting << s_dt;
	}
	qSort(base_sorting);	
	foreach (QStandardItem * s_item, sort_items)
	{
		QDateTime s_dt = QDateTime::fromString(s_item->text());
		sorted_map.insert(base_sorting.indexOf(s_dt)+1, s_item);
	}
}

bool SpcNum::chkStrOnlyUseLetterOrNumber(const QString & str)
{
	for (int i = 0; i < str.size(); i++)
	{
		if (!str.at(i).isLetterOrNumber())
			return false;
	}
	return true;	
}

bool SpcNum::checkPunctuationInString(const QString & str)
{
	for (int i = 0; i < str.size(); i++)
	{
		if (str.at(i).isPunct())
			return true;
	}
	return false;	
}

bool SpcNum::inputNumbersJudging(const QString & judged)
{
	if (!judged.at(0).isNumber())
		return false;
	int dots = 0;
	for (int i = 0; i < judged.size(); i++)
	{
		if (judged.at(i) == '.')
		{
			dots += 1;
			continue;
		}
		if (!judged.at(i).isNumber())
			return false;
	}
	if (dots > 1)
		return false;
	return true;
}

int SpcNum::randNumberGenerate()
{
	QTime t = QTime::currentTime();
	qsrand(t.msec()+t.second()*1000);
	return qrand();	
}

int SpcNum::findTimestrPosInStringlist(const QStringList & strs, const QString & base_str)
{
	if (strs.isEmpty())
		return 0;
	QDateTime m_dt = QDateTime::fromString(base_str);   
	QList<QDateTime> sort_list;
	foreach (QString str, strs)
	{
		QDateTime dt = QDateTime::fromString(str);
		sort_list << dt;
	}
	int i_pos = 0;
	bool inside_sorts = false;
	foreach (QDateTime dt, sort_list)
	{
		if (m_dt < dt)
		{
			inside_sorts = true;
			i_pos = sort_list.indexOf(dt);
			break;
		}
	}
	if (!inside_sorts)
		i_pos = sort_list.size();
	return i_pos;
}

double SpcNum::averageDataes(QList<double> & vals)
{
	int n = vals.size();
	double tol = 0.0;
	for(int i = 0; i < n; i++)
		tol += vals.at(i);
	return tol/n;
}

double SpcNum::devCalFromDataes(QList<double> sdataes)
{
	double avr = averageDataes(sdataes);
	double total = 0.0;
	for (int i = 0; i < sdataes.size(); i++)
		total += (sdataes.at(i)-avr)*(sdataes.at(i)-avr);
	return qSqrt(total/(sdataes.size()-1));	
}
	
double SpcNum::rngCalFromDataes(QList<double> rdataes)
{
	QList<double> temp = rdataes;
	qSort(temp);
	return temp.at(temp.size()-1)-temp.at(0);	
}

double SpcNum::getCpkTolAvr(const QString & cpk_tbl)
{
	QString sel_pos = QString("groupnum=%1").arg(1);
	return spcdata_db->dataFromTable(cpk_tbl, "avr", sel_pos).toDouble();
}

double SpcNum::getAvrVal()
{
	return num_avr;
}

double SpcNum::getDevVal()
{
	return num_s;
}

double SpcNum::getRngVal()
{
	return num_r;
}

double SpcNum::selectedGroupsMax(QList<double> & range)
{
	return maxFromList(range);	
}

double SpcNum::curGroupMax()
{
	return maxFromList(data);
}

double SpcNum::selectedGroupsMin(QList<double> & range)
{
	return minFromList(range);
}

double SpcNum::curGroupMin()
{
	return minFromList(data);
}

double SpcNum::HistogramUnitSize(double rng, int groups)
{
	return rng/groups;
}

double SpcNum::HistogramUnitLowMark(double val)
{
	return val-unit_accuracy/2;
}

double SpcNum::normalProbabilityYaxleValue(QList<double> & xs, QList<double> & ys, double xaxel)
{
	double b0 = normalProbabilityB0Para(xs, ys);
	double l_slope = normalProbabilityLineSlope(xs, ys);
	return b0 + l_slope*xaxel;
}
	
double SpcNum::normalProbabilityB0Para(QList<double> & xs, QList<double> & ys)
{
	double x_avr = listSum(xs)/xs.size();
	double y_avr = listSum(ys)/ys.size();
	double l_slope = normalProbabilityLineSlope(xs, ys);
	return y_avr - l_slope * x_avr;
}

double SpcNum::normsinv(const double p)
{
	static const double LOW  = 0.02425;
	static const double HIGH = 0.97575;
	/* Coefficients in rational approximations. */
	static const double a[] =
	{
		-3.969683028665376e+01,
		2.209460984245205e+02,
		-2.759285104469687e+02,
		1.383577518672690e+02,
		-3.066479806614716e+01,
		2.506628277459239e+00
	};
	static const double b[] =
	{
		-5.447609879822406e+01,
		1.615858368580409e+02,
		-1.556989798598866e+02,
		6.680131188771972e+01,
		-1.328068155288572e+01
	};
	static const double c[] =
	{
		-7.784894002430293e-03,
		-3.223964580411365e-01,
		-2.400758277161838e+00,
		-2.549732539343734e+00,
		4.374664141464968e+00,
		2.938163982698783e+00
	};
	static const double d[] =
	{
		7.784695709041462e-03,
		3.224671290700398e-01,
		2.445134137142996e+00,
		3.754408661907416e+00
	};
	double q, r;
	if (p < 0 || p > 1)
		return 0.0;
	else if (p == 0)
		return INFINITY;
	else if (p == 1)
		return -INFINITY;
	else if (p < LOW)
	{
  /* Rational approximation for lower region */
		q = sqrt(-2*log(p));
		return normInvShiftedVal((((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
		((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1));
	}
	else if (p > HIGH)
	{
  /* Rational approximation for upper region */
		q  = sqrt(-2*log(1-p));
		return normInvShiftedVal(-(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
		((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1));
	}
	else
	{
  /* Rational approximation for central region */
		q = p - 0.5;
		r = q*q;
		return normInvShiftedVal((((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5])*q /
		(((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1));
	}
} 

QString SpcNum::ensureCpkGrade(const double cpk)
{
	double a_plus = 1.67;
	double a_common = 1.33;
	double b_common = 1.0;
	double c_common = 0.67;
	if (cpk > a_plus)
		return "A+";
	else if (cpk>a_common && cpk<=a_plus)
		return "A";
	else if (cpk>b_common && cpk<=a_common)
		return "B";	
	else if (cpk>c_common && cpk<=b_common)
		return "C";	
	else if (cpk <= c_common)
		return "D";	
	return "";
}

QList<double> SpcNum::ptsFrequency(QList<double> & pts)
{
	QList<double> pts_freq;
	double tol_pts = listSum(pts);
	double freq_sum = 0.0;
	for (int i = 0; i < pts.size(); i++)
	{
		freq_sum += pts.at(i)/tol_pts;
		pts_freq << freq_sum;
	}
	return pts_freq;
}

QList<double> SpcNum::mergeDataesFromMap(const QMap<int, QList<double> > & map)
{
	QMap<int, QList<double> > tmp_map = map;
	QList<double> tmp_dou;
	for (int i = 0; i < tmp_map.size(); i++)
		tmp_dou += tmp_map.value(i+1);	
	return tmp_dou;
}
			
const QList<QPair<QString, QString> > & SpcNum::engiCarryings()
{
	return engi_carrying;
}

const QList<QPair<double, double> > SpcNum::GaussPlotPair()
{
	QList<QPair<double, double> > markers_pair;
	QStringList real;
	spcdata_db -> getDataesFromTable("norproparaes", "fractile", real);
	QStringList frac;
	spcdata_db -> getDataesFromTable("norproparaes", "frequency", frac);
	for (int i = 0; i < real.size(); i++)
	{
		QPair<double, double> marker_pair;
		marker_pair.first = real.at(i).toDouble();
		marker_pair.second = frac.at(i).toDouble();
		markers_pair << marker_pair;		
	}
	return markers_pair;	
}

PlotWidget * SpcNum::currentPlotWidget()
{
	return current_plots;
}

void SpcNum::threePtsJudging(double nextPt)
{
	current_plots->avrDevJudgeStack(true, 3) << nextPt;
	if (current_plots->avrDevJudgeStack(true, 3).size() == 3)
	{
		if (threeJudging())
		{
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-1] += "1unormal";
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-1] += " endunormal";
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()] += " endunormal";
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()+1] += " endunormal";
		}
		else
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-1] += "1normal";
		current_plots->avrDevJudgeStack(true, 3).pop_front();
	}	
}

void SpcNum::fivePtsJudging(double nextPt)
{
	current_plots->avrDevJudgeStack(true, 5) << nextPt;
	if (current_plots->avrDevJudgeStack(true, 5).size() == 5)
	{
		if (fiveJudging())// changed here
		{
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-3] += "2unormal";
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-3] += " endunormal";
			for (int i = -1; i < 3; i++)
			{
				if (!current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-i].contains(" endunormal"))
					current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-i] += " endunormal";	
			}			
		}
		else
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-3] += "2normal";
		current_plots->avrDevJudgeStack(true, 5).pop_front();
	}	
}

void SpcNum::sixPtsJudging(double nextPt)
{
	current_plots->avrDevJudgeStack(true, 6) << nextPt;
	if (current_plots->avrDevJudgeStack(true, 6).size() == 6)
	{
		if (sixJudging())// changed here
		{
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-4] += "3unormal";
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-4] += " endunormal";
			for (int i = -1; i < 4; i++)
			{
				if (!current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-i].contains(" endunormal"))
					current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-i] += " endunormal";	
			}
		}
		else
			current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-4] += "3normal";
		current_plots->avrDevJudgeStack(true, 6).pop_front();
	}	
}
	
void SpcNum::eightPtsJudging(double nextPt)
{
	current_plots->avrDevJudgeStack(false, 8) << nextPt;
	if (current_plots->avrDevJudgeStack(false, 8).size() == 8)
	{
		if (eightJudging())// changed here
		{
			current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-6] += "4unormal";
			current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-6] += " endunormal";
			for (int i = -1; i < 6; i++)
			{
				if (!current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-i].contains(" endunormal"))
					current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-i] += " endunormal";	
			}
		}	
		else
			current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-6] += "4normal";
		current_plots->avrDevJudgeStack(false, 8).pop_front();
	}	
}

void SpcNum::ninePtsJudging(double nextPt, int type)
{
	if (type == 0)
	{
		current_plots->avrDevJudgeStack(true, 90) << nextPt;
		if (current_plots->avrDevJudgeStack(true, 90).size() == 9)
		{
			if (nineJudging(current_plots->avrDevJudgeStack(true, 90), 0))// changed here
			{
				current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-7] += "5unormal";
				current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-7] += " endunormal";
				for (int i = -1; i < 7; i++)
				{
					if (!current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-i].contains(" endunormal"))
						current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-i] += " endunormal";	
				}
			}
			else
				current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-7] += "5normal";
			current_plots->avrDevJudgeStack(true, 90).pop_front();
		}
	}
	else
	{
		current_plots->avrDevJudgeStack(false, 91) << nextPt;
		if (current_plots->avrDevJudgeStack(false, 91).size() == 9)
		{
			if (nineJudging(current_plots->avrDevJudgeStack(false, 91), 1))
			{
				current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-7] += "5unormal";
				current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-7] += " endunormal";
				for (int i = -1; i < 7; i++)
				{
					if (!current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-i].contains(" endunormal"))
						current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-i] += " endunormal";	
				}
			}
			else
				current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-7] += "5normal";
			current_plots->avrDevJudgeStack(false, 91).pop_front();
		}
	}	
}
	
void SpcNum::fourteenPtsJudging(double nextPt, int type)
{
	if (type == 0)
	{
		current_plots->avrDevJudgeStack(true, 140) << nextPt;
		if (current_plots->avrDevJudgeStack(true, 140).size() == 14)
		{
			if (fourteenJudging(current_plots->avrDevJudgeStack(true, 140)))
			{
				current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-12] += "6unormal";
				current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-12] += " endunormal";
				for (int i = -1; i < 12; i++)
				{
					if (!current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-i].contains(" endunormal"))
						current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-i] += " endunormal";	
				}
			}
			else
			{
				current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-12] += "6normal";
				if (!current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-12].contains("unormal"))
					current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()-12] += " endnormal";
			}
			current_plots->avrDevJudgeStack(true, 140).pop_front();
		}
	}
	else
	{
		current_plots->avrDevJudgeStack(false, 141) << nextPt;
		if (current_plots->avrDevJudgeStack(false, 141).size() == 14)
		{
			if (fourteenJudging(current_plots->avrDevJudgeStack(false, 141)))
			{
				current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-12] += "6unormal";
				current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-12] += " endunormal";
				for (int i = -1; i < 12; i++)
				{
					if (!current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-i].contains(" endunormal"))
						current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-i] += " endunormal";	
				}
			}
			else
				current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-12] += "6normal";
			current_plots->avrDevJudgeStack(false, 141).pop_front();
		}
	}	
}

void SpcNum::fifteenPtsJudging(double nextPt)
{
	current_plots->avrDevJudgeStack(false, 15) << nextPt;
	if (current_plots->avrDevJudgeStack(false, 15).size() == 15)
	{
		if (fifteenJudging())
		{
			current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-13] += "7unormal";
			current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-13] += " endunormal";
			for (int i = -1; i < 13; i++)
			{
				if (!current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-i].contains(" endunormal"))
					current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-i] += " endunormal";	
			}
		}
		else
		{
			current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-13] += "7normal";
			if (!current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-13].contains("unormal"))
				current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()-13] += " endnormal";
		}
		current_plots->avrDevJudgeStack(false, 15).pop_front();
	}	
}

bool SpcNum::oneOutLimit(double judVal, int type)// end correct
{
	if (type == 0)
	{
		if (judVal>current_plots->upAvrCtrlLimit() || judVal<current_plots->lowAvrCtrlLimit())
			return true;
	}
	else if (type == 1)
	{
		if (judVal>current_plots->upDevCtrlLimit() || judVal<current_plots->lowDevCtrlLimit())
			return true;
	}
	else
	{
		if (judVal>current_plots->upRngCtrlLimit() || judVal<current_plots->lowRngCtrlLimit())
			return true;
	}
	return false;
}

bool SpcNum::threeJudging()// end correct
{
	QList<double> tempData = current_plots->avrDevJudgeStack(true, 3);
	qSort(tempData);
	double edge = (current_plots->upAvrCtrlLimit()-current_plots->lowAvrCtrlLimit())/6;
	if (tempData.at(1)>current_plots->upAvrCtrlLimit()-edge || tempData.at(1)<current_plots->lowAvrCtrlLimit()+edge)
		return true;
	return false;
}

bool SpcNum::fiveJudging()//end correct
{
	QList<double> tempData = current_plots->avrDevJudgeStack(true, 5);
	qSort(tempData);
	double centor = (current_plots->upAvrCtrlLimit()+current_plots->lowAvrCtrlLimit())/2;
	double edge = (current_plots->upAvrCtrlLimit()-current_plots->lowAvrCtrlLimit())/6;
	double upper = centor + edge;
	double lower = centor - edge;
	if (tempData.at(1)>upper || tempData.at(3)<lower)
		return true;
	return false;	
}

bool SpcNum::sixJudging()//end correct
{
	QList<double> tempList = current_plots->avrDevJudgeStack(true, 6);
	qSort(tempList);
	if (current_plots->avrDevJudgeStack(true, 6) == tempList)
		return true;
	else
	{
		qSort(tempList.begin(), tempList.end(), qGreater<double>());
		if (current_plots->avrDevJudgeStack(true, 6) == tempList)
			return true;	
	}
	return false;	
}

bool SpcNum::eightJudging()//end correct
{
	QList<double> tempData = current_plots->avrDevJudgeStack(false, 8);
	qSort(tempData);
	double centor = (current_plots->upDevCtrlLimit()+current_plots->lowDevCtrlLimit())/2;
	double edge = (current_plots->upDevCtrlLimit()-current_plots->lowDevCtrlLimit())/6;
	double upper = centor + edge;
	double lower = centor - edge;
	if (tempData.at(0)<centor && tempData.at(7)>centor)
	{
		foreach (double d_chk, tempData)
		{
			if (d_chk>=lower && d_chk<=upper)
				return false;
		}
		return true;	
	}
	return false;
}

bool SpcNum::nineJudging(QList<double> & judDataes, int type)// end correct
{
	QList<double> tempList = judDataes;
	qSort(tempList);
	double centor = 0.0;
	if (type == 0)
		centor = (current_plots->upAvrCtrlLimit()+current_plots->lowAvrCtrlLimit())/2;
	else
		centor = (current_plots->upDevCtrlLimit()+current_plots->lowDevCtrlLimit())/2;
	if (tempList.at(0)>centor || tempList.back()<centor)
		return true;	
	return false;		
}

bool SpcNum::fourteenJudging(QList<double> & judDataes)// end correct
{
	int i = 1;
	if (judDataes.at(0) >= judDataes.at(1))
	{
		foreach (double d_chk, judDataes)
		{
			if (judDataes.indexOf(d_chk) == judDataes.size()-1)
				break;
			if ((d_chk-judDataes.at(judDataes.indexOf(d_chk)+1))*i >= 0)
				i *= -1;
			else
				return false;
		}
	}
	if (judDataes.at(0) <= judDataes.at(1))
	{
		foreach (double d_chk, judDataes)
		{
			if (judDataes.indexOf(d_chk) == judDataes.size()-1)
				break;		  
			if ((d_chk-judDataes.at(judDataes.indexOf(d_chk)+1))*i <= 0)
				i *= -1;
			else
				return false;
		}		
	}
	return true;
}

bool SpcNum::fifteenJudging()// end correct
{
	QList<double> tempData = current_plots->avrDevJudgeStack(false, 15);
	qSort(tempData);
	double centor = (current_plots->upDevCtrlLimit()+current_plots->lowDevCtrlLimit())/2;
	double edge = (current_plots->upDevCtrlLimit()-current_plots->lowDevCtrlLimit())/6;
	double upper = centor + edge;
	double lower = centor - edge;
	if (tempData.at(0)>lower && tempData.at(0)<centor && tempData.at(14)>centor && tempData.at(14)<upper)
		return true;
	return false;	
}

int SpcNum::calForHistogramGroups(int groupnum)
{
	return qRound(qSqrt(groupnum));
}

int SpcNum::freqDisForHistogram(double up, QList<double> & range)
{
	QList<double> copy_list;
	foreach (double val, range)
	{
		if (val <= up)
		{
			copy_list.push_back(val);
			range.removeOne(val);
		}
		else
			break;
	}
	return copy_list.size();
}

int SpcNum::freqDisForGauss(double low, double up, QList<double> & range)
{
	QList<double> copy_list;
	foreach (double val, range)
	{
		if (val >= low && val < up)
			copy_list.push_back(val);
	}
	return copy_list.size();	
}

double SpcNum::dataesTolavrCal(QMap<int, QList<double> > & tolDataes)
{
	QMap<int, QList<double> > tempMap = tolDataes;
	QList<int> keyNumbers = tolDataes.keys();
	int n = keyNumbers.size();
	double tolVal = 0.0;
	for (int i = 0; i < n; i++)
	{
		QList<double> tempList = tempMap.value(keyNumbers.at(i));
		tolVal += averageDataes(tempList);
	}
	return tolVal/double(n);
}
			
double SpcNum::devFromAnyGroups(QMap<int, QList<double> > & keyDataes)
{
	QList<int> curkeys = keyDataes.keys();
	double tmp_s = 0.0;
	for (int i = 0; i < curkeys.size(); i++)
		tmp_s += devCalFromDataes(keyDataes.value(curkeys.at(i)));
	return tmp_s/double(curkeys.size());	
}

double SpcNum::rngFromAnyGroups(QMap<int, QList<double> > & keyDataes)
{
	QList<int> curkeys = keyDataes.keys();
	double tmp_r = 0.0;
	for (int i = 0; i < curkeys.size(); i++)
		tmp_r += rngCalFromDataes(keyDataes.value(curkeys.at(i)));
	return tmp_r/double(curkeys.size());
}

double SpcNum::calForAvrLowLimit(DataesTableView * g_view)
{
	bool if_S = g_view->model()->data(g_view->model()->index(3, 0)).toString().contains(tr("西格玛"));
	if (if_S)
	{
		QString sel_pos = QString("groupnum=%1").arg(g_view->model()->data(g_view->model()->index(6, 0)).toString());
		return cpk_tolAvr-spcdata_db->dataFromTable("spcparaes", "A3", sel_pos).toDouble()*num_s;
	}
	else
	{
		QString sel_pos = QString("groupnum=%1").arg(g_view->model()->data(g_view->model()->index(6, 0)).toString());
		return cpk_tolAvr-spcdata_db->dataFromTable("spcparaes", "A2", sel_pos).toDouble()*num_r;
	}
}
	
double SpcNum::calForAvrUpLimit(QMap<int, QList<double> > & keyDataes, DataesTableView * g_view)
{
	cpk_tolAvr = dataesTolavrCal(keyDataes);
	bool if_S = g_view->model()->data(g_view->model()->index(3, 0)).toString().contains(tr("西格玛"));
	if (if_S)
	{
		QString sel_pos = QString("groupnum=%1").arg(g_view->model()->data(g_view->model()->index(6, 0)).toString());
		return cpk_tolAvr+spcdata_db->dataFromTable("spcparaes", "A3", sel_pos).toDouble()*num_s;
	}
	else
	{
		QString sel_pos = QString("groupnum=%1").arg(g_view->model()->data(g_view->model()->index(6, 0)).toString());
		return cpk_tolAvr+spcdata_db->dataFromTable("spcparaes", "A2", sel_pos).toDouble()*num_r;
	}
}
	
double SpcNum::calForDevLowLimit(DataesTableView * g_view)
{
	QString sel_pos = QString("groupnum=%1").arg(g_view->model()->data(g_view->model()->index(6, 0)).toString());
	return spcdata_db->dataFromTable("spcparaes", "B3", sel_pos).toDouble()*num_s;
}
	
double SpcNum::calForDevUpLimit(QMap<int, QList<double> > & keyDataes, DataesTableView * g_view)
{
	num_s = devFromAnyGroups(keyDataes);
	QString sel_pos = QString("groupnum=%1").arg(g_view->model()->data(g_view->model()->index(6, 0)).toString());
	return spcdata_db->dataFromTable("spcparaes", "B4", sel_pos).toDouble()*num_s;
}
	
double SpcNum::calForRngLowLimit(DataesTableView * g_view)
{
	QString sel_pos = QString("groupnum=%1").arg(g_view->model()->data(g_view->model()->index(6, 0)).toString());
	return spcdata_db->dataFromTable("spcparaes", "D3", sel_pos).toDouble()*num_r;
}
	
double SpcNum::calForRngUpLimit(QMap<int, QList<double> > & keyDataes, DataesTableView * g_view)
{
	num_r = rngFromAnyGroups(keyDataes);
	QString sel_pos = QString("groupnum=%1").arg(g_view->model()->data(g_view->model()->index(6, 0)).toString());
	return spcdata_db->dataFromTable("spcparaes", "D4", sel_pos).toDouble()*num_r;
}

double SpcNum::calForLinearRegressorLxx(QList<double> & xs)
{
	double list_squaresum = listSquareSum(xs);
	double list_sum = listSum(xs);
	return list_squaresum - list_sum*list_sum/xs.size();
}
	
double SpcNum::calForLinearRegressorLyy(QList<double> & ys)
{
	double list_squaresum = listSquareSum(ys);
	double list_sum = listSum(ys);
	return list_squaresum - list_sum*list_sum/ys.size();
}
	
double SpcNum::calForLinearRegressorLxy(QList<double> & xs, QList<double> & ys)
{
	double list_xsum = listSum(xs);
	double list_ysum = listSum(ys);
	double total_xy = 0.0;
	for (int i = 0; i < xs.size(); i++)
		total_xy += xs.at(i)*ys.at(i);
	return total_xy - list_xsum*list_ysum/xs.size();
}

double SpcNum::normalProbabilityLineSlope(QList<double> & xs, QList<double> & ys)
{
	double Lxx = calForLinearRegressorLxx(xs);
	double Lxy = calForLinearRegressorLxy(xs, ys);
	return Lxy / Lxx;
}

double SpcNum::normFreqSum(double low, double up, QList<double> & range)
{
	double tols = range.size();
	double nums = freqDisForGauss(low, up, range);
	return nums/tols;
}

void SpcNum::engiParaesCal(DataesTableView * key_view, QMap<int, QList<double> > & tolDataes, QList<double> & calings)
{
	double upper_sPara = 0.0, lower_sPara = 0.0, upper_avrPara = 0.0, lower_avrPara = 0.0, upper_rPara = 0.0, lower_rPara = 0.0;
	bool if_S = key_view->model()->data(key_view->model()->index(3, 0)).toString().contains(tr("西格玛"));
	if (if_S)
	{
		upper_sPara = calForDevUpLimit(tolDataes, key_view);
		lower_sPara = calForDevLowLimit(key_view);
		upper_avrPara = calForAvrUpLimit(tolDataes, key_view);
		lower_avrPara = calForAvrLowLimit(key_view);
		QString sel_pos = QString("groupnum=%1").arg(key_view->model()->data(key_view->model()->index(6, 0)).toString());
		double s_sigma = num_s/spcdata_db->dataFromTable("spcparaes", "c4", sel_pos).toDouble();
		double Zu = (key_view->model()->data(key_view->model()->index(4, 0)).toDouble()-cpk_tolAvr)/s_sigma;
		double Zl = (cpk_tolAvr-key_view->model()->data(key_view->model()->index(5, 0)).toDouble())/s_sigma;
		double s_cpk = qMin(Zu, Zl)/3;
		calings << s_cpk << s_sigma << cpk_tolAvr << upper_avrPara << lower_avrPara << num_s << upper_sPara << lower_sPara;
	}
	else
	{
		upper_rPara = calForRngUpLimit(tolDataes, key_view);
		lower_rPara = calForRngLowLimit(key_view);
		upper_avrPara = calForAvrUpLimit(tolDataes, key_view);
		lower_avrPara = calForAvrLowLimit(key_view);
		QString sel_pos = QString("groupnum=%1").arg(key_view->model()->data(key_view->model()->index(6, 0)).toString());
		double r_sigma = num_r/spcdata_db->dataFromTable("spcparaes", "d2", sel_pos).toDouble();
		double Zu = (key_view->model()->data(key_view->model()->index(4, 0)).toDouble()-cpk_tolAvr)/r_sigma;
		double Zl = (key_view->model()->data(key_view->model()->index(5, 0)).toDouble())/r_sigma;
		double r_cpk = qMin(Zu, Zl)/3;
		calings << r_cpk << r_sigma << cpk_tolAvr << upper_avrPara << lower_avrPara << num_r << upper_rPara << lower_rPara;
	}
//	current_plots -> setRelatedCtrlValue(new_ctrls);
}

void SpcNum::startAvrPtsJudge(double avr)
{
	if (oneOutLimit(avr, 0))
		current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()+1] += "0unormal";
	else
		current_plots->relatedJudgedStateMap(0)[current_plots->currentEngiGroups()+1] += "0normal";	
	threePtsJudging(avr);
	fivePtsJudging(avr);
	sixPtsJudging(avr);
	ninePtsJudging(avr, 0);
	fourteenPtsJudging(avr, 0);	
}
	
void SpcNum::startDevPtsJudge(double dev)
{
	if (oneOutLimit(dev, 1))
		current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()+1] += "0unormal";
	else
		current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()+1] += "0normal";
	eightPtsJudging(dev);//? add it to avr?
	ninePtsJudging(dev, 1);
	fourteenPtsJudging(dev, 1);
	fifteenPtsJudging(dev);
}
	
void SpcNum::startRngPtsJudge(double rng)
{
	if (oneOutLimit(rng, 2))
		current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()+1] += "0unormal";
	else
		current_plots->relatedJudgedStateMap(1)[current_plots->currentEngiGroups()+1] += "0normal";
}

void SpcNum::cpkDataesTransFromDB(const QString & pro_name, QMap<int, QList<double> > & stored_map)
{
	QStringList pos_times = pro_name.split(tr("，。，"));
	QString time_row = pos_times.at(1);
	QString real_row("time="+time_row);		
	QString tmp_dataes1 = spcdata_db->dataFromTable(pos_times.at(0), "dataes", real_row).toString();
	QStringList tmp_dataes2 = tmp_dataes1.split(";", QString::SkipEmptyParts);
	tmp_dataes2.pop_back();
	for (int i = 0; i < tmp_dataes2.size(); i++)
	{
		QList<double> tmp_dataes3;
		tmp_dataes3.clear();
		QString tmp_dataes4 = tmp_dataes2.at(i);
		QStringList tmp_dataes5 = tmp_dataes4.split(",", QString::SkipEmptyParts);;
		for (int j = 0; j < tmp_dataes5.size(); j++)
			tmp_dataes3 << tmp_dataes5[j].toDouble();
		stored_map.insert(i+1, tmp_dataes3);
	}
}

double SpcNum::maxFromList(QList<double> & list)
{
	QList<double> tempList = list;
	qSort(tempList);	
	return tempList.at(tempList.size()-1);
}

double SpcNum::minFromList(QList<double> & list)
{
	QList<double> tempList = list;
	qSort(tempList);	
	return tempList.at(0);	
}

double SpcNum::listSum(QList<double> & list)
{
	double total = 0.0;
	for (int i = 0; i < list.size(); i++)
		total += list.at(i);
	return total;
}

double SpcNum::listSquareSum(QList<double> & list)
{
	double total = 0.0;
	for (int i = 0; i < list.size(); i++)
		total += list.at(i)*list.at(i);
	return total;	
}

double SpcNum::normInvShiftedVal(double norminv)
{
	double base_origin = -4.9;
	return norminv - base_origin;
}

