//websource.js

const fetch = require('node-fetch')
const co = require('co')
const http = require("http")
const fs = require("fs")
const s_random = require("./shell.js")
const joiners = require('./clients.js')
const txgeo = require('./txgeo.js')
var dis_scope = require('./distance.js')
var webhead = 'http://'
    , bxsite = 'baixing.com/'
    , bxChildSite = {bxtapGates:[{'bxgate0':'kuaijifuwu/', cn:'会计'},
        {'bxgate1':'lvshifuwu/', cn:'律师'},
        {'bxgate2':'baoxianfuwu/', cn:'保险'},
        {'bxgate3':'yinshuapenghui/', cn:'印刷包装'},
        {'bxgate4':'penhuizhaopai/', cn:'喷绘招牌'},
        {'bxgate5':'guanggaomeiti/', cn:'媒体广告'},
        {'bxgate6':'sheji/', cn:'设计'},
        {'bxgate7':'wangzhanjianshe/', cn:'网站架设'},
        {'bxgate8':'wangluobuxian/', cn:'网络布线'},
        {'bxgate9':'qingdian/', cn:'庆典相关'},
        {'bxgate10':'zulin/', cn:'租赁'},
        {'bxgate11':'bangongweixiu/', cn:'设备维修'},
        {'bxgate12':'gongyeshebei/', cn:'工业设备'},
        {'bxgate13':'kuaidifuwu/', cn:'快递'},
        {'bxgate14':'nongye/', cn:'农林牧'},
        {'bxgate15':'wupinpifa/', cn:'批发'},
        {'bxgate16':'lipinfuwu/', cn:'礼品定制'},
        {'bxgate17':'canyinmeishi/', cn:'餐饮'},
        {'bxgate18':'daiyunyingtg/', cn:'托管/代运'},
        {'bxgate19':'fanyifuwu/', cn:'翻译'},
        {'bxgate20':'suji/', cn:'速记'},
        {'bxgate21':'jianzhuweixiu/', cn:'建筑维修'}],
        bxtapJobs:[{'bxjob0':'gongren/'},
            {'bxjob1':'jixie/'},
            {'bxjob2':'dianzi/'},
            {'bxjob3':'siji/'},
            {'bxjob4':'songhuoyuan/'},
            {'bxjob5':'chushi/'},
            {'bxjob6':'fuwuyuan/'},
            {'bxjob7':'dianyuan/'},
            {'bxjob8':'qichemeirong/'},
            {'bxjob9':'xiaoshou/'},
            {'bxjob10':'fangdichan/'},
            {'bxjob11':'wenmi/'},
            {'bxjob12':'renshi/'},
            {'bxjob13':'kefu/'},
            {'bxjob14':'bangyong/'},
            {'bxjob15':'baojian/'},
            {'bxjob16':'meirongshi/'},
            {'bxjob17':'jianshen/'},
            {'bxjob18':'kuaiji/'},
            {'bxjob19':'jinrong/'},
            {'bxjob20':'baoxianzhaopin/'},
            {'bxjob21':'chengxuyuan/'},
            {'bxjob22':'meigong/'},
            {'bxjob23':'taobaojob/'},
            {'bxjob24':'laoshi/'},
            {'bxjob25':'daoyou/'},
            {'bxjob26':'yinshiyule/'},
            {'bxjob27':'yisheng/'},
            {'bxjob28':'caigou/'},
            {'bxjob29':'shichang/'},
            {'bxjob30':'ktvjiuba/'},
            {'bxjob31':'fanyi/'},
            {'bxjob32':'falv/'},
            {'bxjob33':'nonglinmuyu/'},
            {'bxjob34':'chuguolaowu/'},
            {'bxjob35':'qitazhaopin/'}],
        bxtapServes:[{'bxserve0':'baomu/', cn:'保姆/月嫂'},
            {'bxserve1':'canyin/', cn:'生活配送'},
            {'bxserve2':'wupinhuishou/', cn:'回收'},
            {'bxserve3':'baojieqingxi/', cn:'清洗保洁'},
            {'bxserve4':'xiyihuli/', cn:'洗衣护理'},
            {'bxserve5':'banjia/', cn:'搬家'},
            {'bxserve6':'jiadianweixiu/', cn:'家电维修'},
            {'bxserve7':'fangwuweixiu/', cn:'房屋维护'},
            {'bxserve8':'weixiu/', cn:'管道维修'},
            {'bxserve9':'kaisuo/', cn:'开锁修锁'},
            {'bxserve10':'diannaoweixiu/', cn:'电脑维修'},
            {'bxserve11':'jiadianweixiu/', cn:'家电维修'},
            {'bxserve12':'jiajuweixiu/', cn:'家具维修'},
            {'bxserve13':'jiatingzhuangxiu/', cn:'装修'},
            {'bxserve14':'ruanzhuang/', cn:'装饰'},
            {'bxserve15':'jiancaizhuangshi/', cn:'建材服务'},
            {'bxserve16':'zhuangxiu/', cn:'工装'}],
        bxtapRend:[{'bxrend0':'zhengzu/'},
            {'bxrend1':'ershoufang/'},
            {'bxrend2':'duanzu/'},
            {'bxrend3':'jingyingzhuanrang/'},
            {'bxrend4':'shangpuchushou/'},
            {'bxrend5':'shangpu/'},
            {'bxrend6':'shoufang/'},
            {'bxrend7':'changfang/'}],
        bxtapOlds:[{'bxold0':'shouji/'},
            {'bxold1':'yinger/'},
            {'bxold2':'diannao/'},
            {'bxold3':'dianqi/'},
            {'bxold4':'bijiben/'},
            {'bxold5':'jiaju/'},
            {'bxold6':'pingbandiannao/'},
            {'bxold7':'fushi/'},
            {'bxold8':'shumachanpin/'},
            {'bxold9':'menpiao/'},
            {'bxold10':'zhaoxiangji/'},
            {'bxold11':'riyongpin/'},
            {'bxold12':'shoujipeijian/'},
            {'bxold13':'yundongqicai/'},
            {'bxold14':'shoucang/'},
            {'bxold15':'ershouzixingche/'},
            {'bxold16':'bangongyongpin/'},
            {'bxold17':'bangongjiaju/'},
            {'bxold18':'gongyeshebei/'},
            {'bxold19':'yueqi/'},
            {'bxold20':'qishipenjing/'},
            {'bxold21':'nongchanpin/'},
            {'bxold22':'quanxinshangjia/'},
            {'bxold23':'qitazhuanrang/'},
            {'bxold24':'xuniwupin/'}]}
    , site58 = '58.com/'
    , ChildSite58 = {tapGates58:[{'58gate0':'shangbiaozhli/', cn:'专利商标'},
        {'58gate1':'lvshi/', cn:'法律'},
        {'58gate2':'caishui/', cn:'财务会计'},
        {'58gate3':'danbaobaoxiantouzi/', cn:'担保投资'},
        {'58gate4':'baoxianfuwu/', cn:'保险'},
        {'58gate5':'zixunzhongjie/', cn:'咨询'},
        {'58gate6':'huoyun/', cn:'货运物流'},
        {'58gate7':'kuaidi/', cn:'快递'},
        {'58gate8':'wuliu/', cn:'专线货运'},
        {'58gate9':'huojiasheb/', cn:'货架'},
        {'58gate10':'chuanmei/', cn:'传媒广告'},
        {'58gate11':'pingmian/', cn:'设计'},
        {'58gate12':'yinshua/', cn:'包装印刷'},
        {'58gate13':'penhui/', cn:'喷绘招铭牌'},
        {'58gate14':'allzhika/', cn:'制卡'},
        {'58gate15':'lipindingzhi/', cn:'礼品定制'},
        {'58gate16':'fanyi/', cn:'翻译速记'},
        {'58gate17':'wangzhan/', cn:'网站架设'},
        {'58gate18':'xitong/', cn:'网络布线'},
        {'58gate19':'appkaifa/', cn:'app研发'},
        {'58gate20':'wangzhantg/', cn:'网络营销'},
        {'58gate21':'jixieweixiu/', cn:'机械设备维护'},
        {'58gate22':'bgsbwx/', cn:'办公设备维护'},
        {'58gate23':'jianzhuweixiu/', cn:'建筑维护'},
        {'58gate24':'zulin/', cn:'租赁'},
        {'58gate25':'nongjiale/', cn:'农家经营'},
        {'58gate26':'shaokaoy/', cn:'烧烤小吃'},
        {'58gate27':'dujia/', cn:'度假娱乐'},
        {'58gate28':'wenquandujiacun/', cn:'温泉'},
        {'58gate29':'caizhai/', cn:'采摘'},
        {'58gate30':'piaoliuq/', cn:'漂流'},
        {'58gate31':'zuliaoam/', cn:'按摩足疗'},
        {'58gate32':'xiyuzx/', cn:'洗浴温泉'},
        {'58gate33':'jianshen/', cn:'运动健身'},
        {'58gate34':'yulecs/', cn:'台球厅'},
        {'58gate35':'zhuoyouba/', cn:'桌游'},
        {'58gate36':'chaguanxx/', cn:'茶馆'},
        {'58gate37':'kafeit/', cn:'咖啡厅'},
        {'58gate38':'diysgf/', cn:'DIY坊'},
        {'58gate39':'ktv/', cn:'KTV'},
        {'58gate40':'jiuba/', cn:'酒吧夜店'},
        {'58gate41':'wangba/', cn:'网吧'}],
        tapJobs58:[{'58job0':'kefu/'},
            {'58job1':'renli/'},
            {'58job2':'zplvyoujiudian/'},
            {'58job3':'zpjiudian/'},
            {'58job4':'jiudianzp/'},
            {'58job5':'chaoshishangye/'},
            {'58job6':'meirongjianshen/'},
            {'58job7':'zpanmo/'},
            {'58job8':'zpjianshen/'},
            {'58job9':'zpshengchankaifa/'},
            {'58job10':'zpshengchan/'},
            {'58job11':'zpqiche/'},
            {'58job12':'zpfangchanjianzhu/'},
            {'58job13':'zpwuye/'},
            {'58job14':'zpfangchan/'},
            {'58job15':'jiazhengbaojiexin/'},
            {'58job16':'siji/'},
            {'58job17':'zpshangwumaoyi/'},
            {'58job18':'zpwuliucangchu/'},
            {'58job19':'zptaobao/'},
            {'58job20':'zpmeishu/'},
            {'58job21':'shichang/'},
            {'58job22':'zpwentiyingshi/'},
            {'58job23':'zhuanye/'},
            {'58job24':'zpcaiwushenji/'},
            {'58job25':'zpfalvzixun/'},
            {'58job26':'fanyizhaopin/'},
            {'58job27':'zpxiezuochuban/'},
            {'58job28':'tech/'},
            {'58job29':'zpjixieyiqi/'},
            {'58job30':'zpjixie/'},
            {'58job31':'jinrongtouzi/'},
            {'58job32':'zpjinrongbaoxian/'},
            {'58job33':'zpyiyuanyiliao/'},
            {'58job34':'zpzhiyao/'},
            {'58job35':'xiaofeipin/'},
            {'58job36':'huanbao/'},
            {'58job37':'huagonggy/'},
            {'58job38':'zhikonganfang/'},
            {'58job39':'zpguanli/'},
            {'58job40':'nonglinmuyu/'},
            {'58job41':'dianzi/'}],
        tapServes58:[{'58serve0':'banjia/', cn:'搬家'},
            {'58serve1':'baomu/', cn:'月嫂保姆'},
            {'58serve2':'baojie/', cn:'清洗保洁'},
            {'58serve3':'huishou/', cn:'回收'},
            {'58serve4':'shutong/', cn:'疏通清洗'},
            {'58serve5':'songshui/', cn:'生活配送'},
            {'58serve6':'kaisuo/', cn:'门锁相关'},
            {'58serve7':'lipinxianhua/', cn:'鲜花植被'},
            {'58serve8':'ganxi/', cn:'衣物护理'},
            {'58serve9':'dianqi/', cn:'家电维修'},
            {'58serve10':'weixiu/', cn:'电脑维修'},
            {'58serve11':'shoujiweixiu/', cn:'手机维修'},
            {'58serve12':'kongtiao/', cn:'空调维修相关'},
            {'58serve13':'shumaweixiu/', cn:'数码维修'},
            {'58serve14':'jiajuweixiu/', cn:'家具维修'}],
        tapRend58:[{'58rend0':'chuzu/'}],
        tapOlds58:[{'58old0':'shouji/'},
            {'58old1':'diannao/'},
            {'58old2':'jiadian/'},
            {'58old3':'ershoujiaju/'},
            {'58old4':'danche/'},
            {'58old5':'zixingche/'},
            {'58old6':'gongcheng/'},
            {'58old7':'zhuangzaiji/'},
            {'58old8':'newchache/'},
            {'58old9':'tuituji/'},
            {'58old10':'yaluji1/'},
            {'58old11':'pingdiji/'},
            {'58old12':'diaoche/'},
            {'58old13':'qzj/'},
            {'58old14':'diaokeji/'},
            {'58old15':'jiagongzhongxin/'},
            {'58old16':'jichuang/'},
            {'58old17':'chongchuang/'},
            {'58old18':'nchechuang/'},
            {'58old19':'jianbanji/'},
            {'58old20':'zhewanji/'},
            {'58old21':'mochuang/'},
            {'58old22':'xichuang/'},
            {'58old23':'qitajichuangshebei/'},
            {'58old24':'chufangshebei/'},
            {'58old25':'jiudianshebei/'},
            {'58old26':'xiaochiche/'},
            {'58old27':'lengdongshebei/'},
            {'58old28':'qiepianji/'},
            {'58old29':'youzhashebei/'},
            {'58old30':'yinliaojiagongshebei/'},
            {'58old31':'hongbeishebei/'},
            {'58old32':'roujiagongshebei/'},
            {'58old33':'shipinjiaobanshebei/'},
            {'58old34':'guoshujiagongshebei/'},
            {'58old35':'qitashipinjiagongjixie/'},
            {'58old36':'jiguangdabiaoji/'},
            {'58old37':'xiezhenji/'},
            {'58old38':'yinshuaji/'},
            {'58old39':'jiguangqiegeji/'},
            {'58old40':'qitayinshuashebei/'},
            {'58old41':'fenglfaadianshebei/'},
            {'58old42':'nfadianji/'},
            {'58old43':'fadianjizu/'},
            {'58old44':'chaiyoufadianshebei/'},
            {'58old45':'bianyaqiq/'},
            {'58old46':'kongyaji/'},
            {'58old47':'qiyoufadianshebei/'},
            {'58old48':'qitafadianshebei/'},
            {'58old49':'zhusuji/'},
            {'58old50':'fensuiji/'},
            {'58old51':'fengkouji/'},
            {'58old52':'chengxingji/'},
            {'58old53':'hanji/'},
            {'58old54':'qitasuliaoshebei/'},
            {'58old55':'lixinji/'},
            {'58old56':'fanyingfu/'},
            {'58old57':'ganzaoji/'},
            {'58old58':'jiaobanjis/'},
            {'58old59':'guolvqi/'},
            {'58old60':'qitahuagongshebei/'},
            {'58old61':'fenxiy/'},
            {'58old62':'jianceyi/'},
            {'58old63':'dibangg/'},
            {'58old64':'qitayiqiyibiao/'},
            {'58old65':'yumijiagong/'},
            {'58old66':'dabaoji/'},
            {'58old67':'shougeji/'},
            {'58old68':'qitanongyongjixie/'}]}
    , qgcitys = [{bx:'beijing', tc:'bj', cn:'北京'},
        {bx:'shanghai', tc:'sh', cn:'上海'},
        {bx:'tianjin', tc:'tj', cn:'天津'},
        {bx:'chongqing', tc:'cq', cn:'重庆'},
        {bx:'hebei', tc:'', cn:'河北'},
        {bx:'handan', tc:'hd', cn:'邯郸'},
        {bx:'shijiazhuang', tc:'sjz', cn:'石家庄'},
        {bx:'baoding', tc:'bd', cn:'保定'},
        {bx:'zhangjiakou', tc:'zjk', cn:'张家口'},
        {bx:'', tc:'dingzhou', cn:'定州'},
        {bx:'', tc:'gt', cn:'馆陶'},
        {bx:'', tc:'zhangbei', cn:'张北'},
        {bx:'', tc:'zx', cn:'赵县'},
        {bx:'', tc:'zd', cn:'正定'},
        {bx:'', tc:'linyixian', cn:'临猗'},
        {bx:'', tc:'qingxu', cn:'清徐'},
        {bx:'anshan', tc:'as', cn:'鞍山'},
        {bx:'fushun', tc:'fushun', cn:'抚顺'},
        {bx:'benxi', tc:'', cn:'本溪'},
        {bx:'dandong', tc:'dandong', cn:'丹东'},
        {bx:'jinzhou', tc:'jinzhou', cn:'锦州'},
        {bx:'yingkou', tc:'yk', cn:'营口'},
        {bx:'fuxin', tc:'', cn:'阜新'},
        {bx:'liaoyang', tc:'liaoyang', cn:'辽阳'},
        {bx:'chaoyang', tc:'cy', cn:'朝阳'},
        {bx:'panjin', tc:'pj', cn:'盘锦'},
        {bx:'huludao', tc:'hld', cn:'葫芦岛'},
        {bx:'jilinn', tc:'', cn:'吉林'},
        {bx:'changchun', tc:'cc', cn:'长春'},
        {bx:'jilin', tc:'jl', cn:'吉林'},
        {bx:'yanbian', tc:'yanbian', cn:'延边'},
        {bx:'siping', tc:'sp', cn:'四平'},
        {bx:'tonghua', tc:'th', cn:'通化'},
        {bx:'baicheng', tc:'bc', cn:'白城'},
        {bx:'liaoyuan', tc:'liaoyuan', cn:'辽源'},
        {bx:'songyuan', tc:'songyuan', cn:'松原'},
        {bx:'baishan', tc:'baishan', cn:'白山'},
        {bx:'heilongjiang', tc:'', cn:'黑龙江'},
        {bx:'jixi', tc:'jixi', cn:'鸡西'},
        {bx:'qitaihe', tc:'qth', cn:'七台河'},
        {bx:'hegang', tc:'hegang', cn:'鹤岗'},
        {bx:'shuangyashan', tc:'sys', cn:'双鸭山'},
        {bx:'yichun', tc:'yich', cn:'伊春'},
        {bx:'haerbin', tc:'hrb', cn:'哈尔滨'},
        {bx:'qiqihaer', tc:'qqhr', cn:'齐齐哈尔'},
        {bx:'mudanjiang', tc:'mdj', cn:'牡丹江'},
        {bx:'jiamusi', tc:'jms', cn:'佳木斯'},
        {bx:'heihe', tc:'heihe', cn:'黑河'},
        {bx:'suihua', tc:'suihua', cn:'绥化'},
        {bx:'daqing', tc:'dq', cn:'大庆'},
        {bx:'daxinganling', tc:'dxal', cn:'大兴安岭'},
        {bx:'jiangsu', tc:'', cn:'江苏'},
        {bx:'nanjing', tc:'nj', cn:'南京'},
        {bx:'wuxi', tc:'wx', cn:'无锡'},
        {bx:'zhenjiang', tc:'zj', cn:'镇江'},
        {bx:'suzhou', tc:'su', cn:'苏州'},
        {bx:'nantong', tc:'nt', cn:'南通'},
        {bx:'yangzhou', tc:'yz', cn:'扬州'},
        {bx:'yancheng', tc:'yancheng', cn:'盐城'},
        {bx:'xuzhou', tc:'xz', cn:'徐州'},
        {bx:'huaian', tc:'ha', cn:'淮安'},
        {bx:'lianyungang', tc:'lyg', cn:'连云港'},
        {bx:'', tc:'qidong', cn:'启东'},
        {bx:'', tc:'liyang', cn:'溧阳'},
        {bx:'', tc:'haimen', cn:'海门'},
        {bx:'', tc:'donghai', cn:'东海'},
        {bx:'', tc:'yangzhong', cn:'扬中'},
        {bx:'', tc:'xinghuashi', cn:'兴化'},
        {bx:'', tc:'xinyishi', cn:'新沂'},
        {bx:'', tc:'taixing', cn:'泰兴'},
        {bx:'', tc:'rudong', cn:'如东'},
        {bx:'', tc:'pizhou', cn:'邳州'},
        {bx:'', tc:'xzpeixian', cn:'沛县'},
        {bx:'', tc:'jingjiang', cn:'靖江'},
        {bx:'', tc:'jianhu', cn:'建湖'},
        {bx:'', tc:'haian', cn:'海安'},
        {bx:'', tc:'dongtai', cn:'东台'},
        {bx:'', tc:'danyang', cn:'丹阳'},
        {bx:'changzhou', tc:'cz', cn:'常州'},
        {bx:'tz', tc:'taizhou', cn:'泰州'},
        {bx:'suqian', tc:'suqian', cn:'宿迁'},
        {bx:'kunshan', tc:'', cn:'昆山'},
        {bx:'changshu', tc:'', cn:'常熟'},
        {bx:'zhangjiagang', tc:'', cn:'张家港'},
        {bx:'taicang', tc:'', cn:'太仓'},
        {bx:'', tc:'dafeng', cn:'大丰'},
        {bx:'', tc:'rugao', cn:'如皋'},
        {bx:'zhejiang', tc:'', cn:'浙江'},
        {bx:'', tc:'yueqingcity', cn:'乐清'},
        {bx:'', tc:'ruiancity', cn:'瑞安'},
        {bx:'', tc:'yiwu', cn:'义乌'},
        {bx:'', tc:'yuyao', cn:'余姚'},
        {bx:'', tc:'zhuji', cn:'诸暨'},
        {bx:'', tc:'xiangshanxian', cn:'象山'},
        {bx:'', tc:'wenling', cn:'温岭'},
        {bx:'', tc:'tongxiang', cn:'桐乡'},
        {bx:'', tc:'cixi', cn:'慈溪'},
        {bx:'', tc:'changxing', cn:'长兴'},
        {bx:'', tc:'jiashanx', cn:'嘉善'},
        {bx:'', tc:'haining', cn:'海宁'},
        {bx:'', tc:'deqing', cn:'德清'},
        {bx:'chengde', tc:'chengde', cn:'承德'},
        {bx:'tangshan', tc:'ts', cn:'唐山'},
        {bx:'langfang', tc:'lf', cn:'廊坊'},
        {bx:'cangzhou', tc:'cangzhou', cn:'沧州'},
        {bx:'hengshui', tc:'hs', cn:'衡水'},
        {bx:'xingtai', tc:'xt', cn:'邢台'},
        {bx:'qinhuangdao', tc:'qhd', cn:'秦皇岛'},
        {bx:'shanxi', tc:'', cn:'山西'},
        {bx:'shuozhou', tc:'shuozhou', cn:'朔州'},
        {bx:'xinzhou', tc:'xinzhou', cn:'忻州'},
        {bx:'taiyuan', tc:'ty', cn:'太原'},
        {bx:'datong', tc:'dt', cn:'大同'},
        {bx:'yangquan', tc:'yq', cn:'阳泉'},
        {bx:'jinzhong', tc:'jz', cn:'晋中'},
        {bx:'changzhi', tc:'changzhi', cn:'长治'},
        {bx:'jincheng', tc:'jincheng', cn:'晋城'},
        {bx:'linfen', tc:'linfen', cn:'临汾'},
        {bx:'lvliang', tc:'lvliang', cn:'吕梁'},
        {bx:'yuncheng', tc:'yuncheng', cn:'运城'},
        {bx:'neimenggu', tc:'', cn:'内蒙古'},
        {bx:'hulunbeier', tc:'hlbe', cn:'呼伦贝尔'},
        {bx:'huhehaote', tc:'hu', cn:'呼和浩特'},
        {bx:'baotou', tc:'bt', cn:'包头'},
        {bx:'wuhai', tc:'wuhai', cn:'乌海'},
        {bx:'wulanchabu', tc:'wlcb', cn:'乌兰察布'},
        {bx:'tongliao', tc:'tongliao', cn:'通辽'},
        {bx:'chifeng', tc:'chifeng', cn:'赤峰'},
        {bx:'eerduosi', tc:'erds', cn:'鄂尔多斯'},
        {bx:'bayannaoer', tc:'bycem', cn:'巴彦淖尔'},
        {bx:'xilinguole', tc:'xl', cn:'锡林郭勒'},
        {bx:'xingan', tc:'xam', cn:'兴安'},
        {bx:'alashan', tc:'alsm', cn:'阿拉善'},
        {bx:'liaoning', tc:'', cn:'辽宁'},
        {bx:'shenyang', tc:'sy', cn:'沈阳'},
        {bx:'tieling', tc:'tl', cn:'铁岭'},
        {bx:'dalian', tc:'dl', cn:'大连'},
        {bx:'', tc:'yxx', cn:'永新'},
        {bx:'', tc:'pld', cn:'庄河'},
        {bx:'', tc:'wfd', cn:'瓦房店'},
        {bx:'quzhou', tc:'quzhou', cn:'衢州'},
        {bx:'hangzhou', tc:'hz', cn:'杭州'},
        {bx:'huzhou', tc:'huzhou', cn:'湖州'},
        {bx:'jiaxing', tc:'jx', cn:'嘉兴'},
        {bx:'ningbo', tc:'nb', cn:'宁波'},
        {bx:'shaoxing', tc:'sx', cn:'绍兴'},
        {bx:'taizhou', tc:'tz', cn:'台州'},
        {bx:'wenzhou', tc:'wz', cn:'温州'},
        {bx:'lishui', tc:'lishui', cn:'丽水'},
        {bx:'jinhua', tc:'jh', cn:'金华'},
        {bx:'zhoushan', tc:'zhoushan', cn:'舟山'},
        {bx:'anhui', tc:'', cn:'安徽'},
        {bx:'chuzhou', tc:'chuzhou', cn:'滁州'},
        {bx:'fuyang', tc:'fy', cn:'阜阳'},
        {bx:'hefei', tc:'hf', cn:'合肥'},
        {bx:'bengbu', tc:'bengbu', cn:'蚌埠'},
        {bx:'wuhu', tc:'wuhu', cn:'芜湖'},
        {bx:'huainan', tc:'hn', cn:'淮南'},
        {bx:'maanshan', tc:'mas', cn:'马鞍山'},
        {bx:'anqing', tc:'anqing', cn:'安庆'},
        {bx:'sz', tc:'suzhou', cn:'宿州'},
        {bx:'bozhou', tc:'bozhou', cn:'亳州'},
        {bx:'huangshan', tc:'huangshan', cn:'黄山'},
        {bx:'huaibei', tc:'huaibei', cn:'淮北'},
        {bx:'tongling', tc:'tongling', cn:'铜陵'},
        {bx:'xuancheng', tc:'xuancheng', cn:'宣城'},
        {bx:'luan', tc:'la', cn:'六安'},
        {bx:'', tc:'hexian', cn:'和县'},
        {bx:'', tc:'hq', cn:'霍邱'},
        {bx:'', tc:'tongcheng', cn:'桐城'},
        {bx:'', tc:'ningguo', cn:'宁国'},
        {bx:'', tc:'tianchang', cn:'天长'},
        {bx:'chaohu', tc:'ch', cn:'巢湖'},
        {bx:'chizhou', tc:'chizhou', cn:'池州'},
        {bx:'shandong', tc:'', cn:'山东'},
        {bx:'taian', tc:'ta', cn:'泰安'},
        {bx:'heze', tc:'heze', cn:'菏泽'},
        {bx:'jinan', tc:'jn', cn:'济南'},
        {bx:'qingdao', tc:'qd', cn:'青岛'},
        {bx:'zibo', tc:'zb', cn:'淄博'},
        {bx:'dezhou', tc:'dz', cn:'德州'},
        {bx:'yantai', tc:'yt', cn:'烟台'},
        {bx:'weifang', tc:'wf', cn:'潍坊'},
        {bx:'jining', tc:'jining', cn:'济宁'},
        {bx:'weihai', tc:'weihai', cn:'威海'},
        {bx:'linyi', tc:'linyi', cn:'临沂'},
        {bx:'binzhou', tc:'binzhou', cn:'滨州'},
        {bx:'dongying', tc:'dy', cn:'东营'},
        {bx:'zaozhuang', tc:'zaozhuang', cn:'枣庄'},
        {bx:'rizhao', tc:'rizhao', cn:'日照'},
        {bx:'', tc:'zhangqiu', cn:'章丘'},
        {bx:'', tc:'zc', cn:'诸城'},
        {bx:'', tc:'shouguang', cn:'寿光'},
        {bx:'', tc:'shuyang', cn:'沭阳'},
        {bx:'laiwu', tc:'lw', cn:'莱芜'},
        {bx:'liaocheng', tc:'lc', cn:'聊城'},
        {bx:'henan', tc:'', cn:'河南'},
        {bx:'shangqiu', tc:'sq', cn:'商丘'},
        {bx:'zhengzhou', tc:'zz', cn:'郑州'},
        {bx:'anyang', tc:'ay', cn:'安阳'},
        {bx:'xinxiang', tc:'xx', cn:'新乡'},
        {bx:'xuchang', tc:'xc', cn:'许昌'},
        {bx:'pingdingshan', tc:'pds', cn:'平顶山'},
        {bx:'xinyang', tc:'xy', cn:'信阳'},
        {bx:'nanyang', tc:'ny', cn:'南阳'},
        {bx:'kaifeng', tc:'kaifeng', cn:'开封'},
        {bx:'luoyang', tc:'luoyang', cn:'洛阳'},
        {bx:'jiaozuo', tc:'jiaozuo', cn:'焦作'},
        {bx:'hebi', tc:'hb', cn:'鹤壁'},
        {bx:'puyang', tc:'puyang', cn:'濮阳'},
        {bx:'zhoukou', tc:'zk', cn:'周口'},
        {bx:'luohe', tc:'luohe', cn:'漯河'},
        {bx:'zhumadian', tc:'zmd', cn:'驻马店'},
        {bx:'sanmenxia', tc:'smx', cn:'三门峡'},
        {bx:'jiyuan', tc:'jiyuan', cn:'济源'},
        {bx:'hunan', tc:'', cn:'湖南'},
        {bx:'yueyang', tc:'yy', cn:'岳阳'},
        {bx:'changsha', tc:'cs', cn:'长沙'},
        {bx:'xiangtan', tc:'xiangtan', cn:'湘潭'},
        {bx:'zhuzhou', tc:'zhuzhou', cn:'株洲'},
        {bx:'hengyang', tc:'hy', cn:'衡阳'},
        {bx:'chenzhou', tc:'chenzhou', cn:'郴州'},
        {bx:'changde', tc:'changde', cn:'常德'},
        {bx:'yiyang', tc:'yiyang', cn:'益阳'},
        {bx:'loudi', tc:'ld', cn:'娄底'},
        {bx:'shaoyang', tc:'shaoyang', cn:'邵阳'},
        {bx:'xiangxi', tc:'xiangxi', cn:'湘西'},
        {bx:'zhangjiajie', tc:'zjj', cn:'张家界'},
        {bx:'huaihua', tc:'hh', cn:'怀化'},
        {bx:'yongzhou', tc:'yongzhou', cn:'永州'},
        {bx:'hubei', tc:'', cn:'湖北'},
        {bx:'wuhan', tc:'wh', cn:'武汉'},
        {bx:'xiantao', tc:'xiantao', cn:'仙桃'},
        {bx:'tianmen', tc:'tm', cn:'天门'},
        {bx:'qianjiang', tc:'qianjiang', cn:'潜江'},
        {bx:'xiangfan', tc:'xf', cn:'襄阳'},
        {bx:'ezhou', tc:'ez', cn:'鄂州'},
        {bx:'xiaogan', tc:'xiaogan', cn:'孝感'},
        {bx:'huanggang', tc:'hg', cn:'黄冈'},
        {bx:'huangshi', tc:'hshi', cn:'黄石'},
        {bx:'xianning', tc:'xianning', cn:'咸宁'},
        {bx:'jingzhou', tc:'jingzhou', cn:'荆州'},
        {bx:'yichang', tc:'yc', cn:'宜昌'},
        {bx:'shiyan', tc:'shiyan', cn:'十堰'},
        {bx:'suizhou', tc:'suizhou', cn:'随州'},
        {bx:'jingmen', tc:'jingmen', cn:'荆门'},
        {bx:'enshi', tc:'es', cn:'恩施'},
        {bx:'shennongjia', tc:'snj', cn:'神农架'},
        {bx:'', tc:'mg', cn:'明港'},
        {bx:'', tc:'yanling', cn:'鄢陵'},
        {bx:'', tc:'yuzhou', cn:'禹州'},
        {bx:'', tc:'changge', cn:'长葛'},
        {bx:'', tc:'yidou', cn:'宜都'},
        {bx:'jiangxi', tc:'', cn:'江西'},
        {bx:'yingtan', tc:'yingtan', cn:'鹰潭'},
        {bx:'xinyu', tc:'xinyu', cn:'新余'},
        {bx:'nanchang', tc:'nc', cn:'南昌'},
        {bx:'jiujiang', tc:'jj', cn:'九江'},
        {bx:'shangrao', tc:'sr', cn:'上饶'},
        {bx:'fz', tc:'fuzhou', cn:'抚州'},
        {bx:'yc', tc:'yichun', cn:'宜春'},
        {bx:'jian', tc:'ja', cn:'吉安'},
        {bx:'ganzhou', tc:'ganzhou', cn:'赣州'},
        {bx:'jingdezhen', tc:'jdz', cn:'景德镇'},
        {bx:'pingxiang', tc:'px', cn:'萍乡'},
        {bx:'fujian', tc:'', cn:'福建'},
        {bx:'fuzhou', tc:'fz', cn:'福州'},
        {bx:'xiamen', tc:'xm', cn:'厦门'},
        {bx:'ningde', tc:'nd', cn:'宁德'},
        {bx:'putian', tc:'pt', cn:'莆田'},
        {bx:'quanzhou', tc:'qz', cn:'泉州'},
        {bx:'zhangzhou', tc:'zhangzhou', cn:'漳州'},
        {bx:'longyan', tc:'ly', cn:'龙岩'},
        {bx:'sanming', tc:'sm', cn:'三明'},
        {bx:'nanping', tc:'np', cn:'南平'},
        {bx:'guangdong', tc:'', cn:'广东'},
        {bx:'guangzhou', tc:'gz', cn:'广州'},
        {bx:'shanwei', tc:'sw', cn:'汕尾'},
        {bx:'yangjiang', tc:'yj', cn:'阳江'},
        {bx:'jieyang', tc:'jy', cn:'揭阳'},
        {bx:'maoming', tc:'mm', cn:'茂名'},
        {bx:'jiangmen', tc:'jm', cn:'江门'},
        {bx:'shaoguan', tc:'sg', cn:'韶关'},
        {bx:'huizhou', tc:'huizhou', cn:'惠州'},
        {bx:'meizhou', tc:'mz', cn:'梅州'},
        {bx:'shantou', tc:'st', cn:'汕头'},
        {bx:'shenzhen', tc:'sz', cn:'深圳'},
        {bx:'zhuhai', tc:'zh', cn:'珠海'},
        {bx:'foshan', tc:'fs', cn:'佛山'},
        {bx:'zhaoqing', tc:'zq', cn:'肇庆'},
        {bx:'zhanjiang', tc:'zhanjiang', cn:'湛江'},
        {bx:'zhongshan', tc:'zs', cn:'中山'},
        {bx:'heyuan', tc:'heyuan', cn:'河源'},
        {bx:'qingyuan', tc:'qingyuan', cn:'清远'},
        {bx:'yunfu', tc:'yf', cn:'云浮'},
        {bx:'chaozhou', tc:'chaozhou', cn:'潮州'},
        {bx:'', tc:'wuyishan', cn:'武夷山'},
        {bx:'', tc:'shishi', cn:'石狮'},
        {bx:'', tc:'jinjiangshi', cn:'晋江'},
        {bx:'', tc:'nananshi', cn:'南安'},
        {bx:'dongguan', tc:'dg', cn:'东莞'},
        {bx:'', tc:'taishan', cn:'台山'},
        {bx:'', tc:'yangchun', cn:'阳春'},
        {bx:'', tc:'sd', cn:'顺德'},
        {bx:'', tc:'huidong', cn:'惠东'},
        {bx:'', tc:'boluo', cn:'博罗'},
        {bx:'guangxi', tc:'', cn:'广西'},
        {bx:'laibin', tc:'lb', cn:'来宾'},
        {bx:'hezhou', tc:'hezhou', cn:'贺州'},
        {bx:'chongzuo', tc:'chongzuo', cn:'崇左'},
        {bx:'yl', tc:'yulin', cn:'玉林'},
        {bx:'fangchenggang', tc:'fcg', cn:'防城港'},
        {bx:'nanning', tc:'nn', cn:'南宁'},
        {bx:'liuzhou', tc:'liuzhou', cn:'柳州'},
        {bx:'guilin', tc:'gl', cn:'桂林'},
        {bx:'wuzhou', tc:'wuzhou', cn:'梧州'},
        {bx:'guigang', tc:'gg', cn:'贵港'},
        {bx:'bose', tc:'baise', cn:'百色'},
        {bx:'qinzhou', tc:'qinzhou', cn:'钦州'},
        {bx:'hechi', tc:'hc', cn:'河池'},
        {bx:'beihai', tc:'bh', cn:'北海'},
        {bx:'hannan', tc:'', cn:'海南'},
        {bx:'sanya', tc:'sanya', cn:'三亚'},
        {bx:'danzhou', tc:'danzhou', cn:'儋州'},
        {bx:'dongfang', tc:'df', cn:'东方'},
        {bx:'wenchang', tc:'wenchang', cn:'文昌'},
        {bx:'qionghai', tc:'qh', cn:'琼海'},
        {bx:'wuzhishan', tc:'wzs', cn:'五指山'},
        {bx:'wanning', tc:'wanning', cn:'万宁'},
        {bx:'haikou', tc:'haikou', cn:'海口'},
        {bx:'baisha', tc:'baisha', cn:'白沙'},
        {bx:'sansha', tc:'sansha', cn:'三沙'},
        {bx:'baoting', tc:'baoting', cn:'保亭'},
        {bx:'changjiang', tc:'', cn:'昌江'},
        {bx:'chengmai', tc:'cm', cn:'澄迈'},
        {bx:'dingan', tc:'da', cn:'定安'},
        {bx:'ledong', tc:'', cn:'乐东'},
        {bx:'lingao', tc:'', cn:'临高'},
        {bx:'lingshui', tc:'lingshui', cn:'陵水'},
        {bx:'qiongzhong', tc:'qiongzhong', cn:'琼中'},
        {bx:'tunchang', tc:'tunchang', cn:'屯昌'},
        {bx:'sichuan', tc:'', cn:'四川'},
        {bx:'chengdu', tc:'cd', cn:'成都'},
        {bx:'meishan', tc:'ms', cn:'眉山'},
        {bx:'ziyang', tc:'zy', cn:'资阳'},
        {bx:'panzhihua', tc:'panzhihua', cn:'攀枝花'},
        {bx:'zigong', tc:'zg', cn:'自贡'},
        {bx:'mianyang', tc:'mianyang', cn:'绵阳'},
        {bx:'nanchong', tc:'nanchong', cn:'南充'},
        {bx:'dazhou', tc:'dazhou', cn:'达州'},
        {bx:'suining', tc:'', cn:'遂宁'},
        {bx:'guangan', tc:'ga', cn:'广安'},
        {bx:'bazhong', tc:'bazhong', cn:'巴中'},
        {bx:'luzhou', tc:'luzhou', cn:'泸州'},
        {bx:'yibin', tc:'yb', cn:'宜宾'},
        {bx:'neijiang', tc:'scnj', cn:'内江'},
        {bx:'leshan', tc:'ls', cn:'乐山'},
        {bx:'liangshan', tc:'liangshan', cn:'凉山'},
        {bx:'yaan', tc:'ya', cn:'雅安'},
        {bx:'ganzi', tc:'ganzi', cn:'甘孜'},
        {bx:'aba', tc:'ab', cn:'阿坝'},
        {bx:'deyang', tc:'deyang', cn:'德阳'},
        {bx:'guangyuan', tc:'guangyuan', cn:'广元'},
        {bx:'guizhou', tc:'', cn:'贵州'},
        {bx:'guiyang', tc:'gy', cn:'贵阳'},
        {bx:'zunyi', tc:'zunyi', cn:'遵义'},
        {bx:'anshun', tc:'anshun', cn:'安顺'},
        {bx:'qiannan', tc:'qn', cn:'黔南'},
        {bx:'qiandongnan', tc:'qdn', cn:'黔东南'},
        {bx:'tongren', tc:'tr', cn:'铜仁'},
        {bx:'bijie', tc:'bijie', cn:'毕节'},
        {bx:'liupanshui', tc:'lps', cn:'六盘水'},
        {bx:'qianxinan', tc:'qxn', cn:'黔西南'},
        {bx:'yunnan', tc:'', cn:'云南'},
        {bx:'xishuangbanna', tc:'bn', cn:'西双版纳'},
        {bx:'dehong', tc:'dh', cn:'德宏'},
        {bx:'zhaotong', tc:'zt', cn:'昭通'},
        {bx:'kunming', tc:'km', cn:'昆明'},
        {bx:'dali', tc:'dali', cn:'大理'},
        {bx:'honghe', tc:'honghe', cn:'红河'},
        {bx:'qujing', tc:'qj', cn:'曲靖'},
        {bx:'baoshan', tc:'bs', cn:'保山'},
        {bx:'wenshan', tc:'ws', cn:'文山'},
        {bx:'yuxi', tc:'yx', cn:'玉溪'},
        {bx:'chuxiong', tc:'cx', cn:'楚雄'},
        {bx:'puer', tc:'pe', cn:'普洱'},
        {bx:'lincang', tc:'lincang', cn:'临沧'},
        {bx:'nujiang', tc:'nujiang', cn:'怒江'},
        {bx:'diqing', tc:'diqing', cn:'迪庆'},
        {bx:'lijiang', tc:'lj', cn:'丽江'},
        {bx:'xizang', tc:'', cn:'西藏'},
        {bx:'lasa', tc:'lasa', cn:'拉萨'},
        {bx:'rikaze', tc:'rkz', cn:'日喀则'},
        {bx:'shannan', tc:'sn', cn:'山南'},
        {bx:'linzhi', tc:'linzhi', cn:'林芝'},
        {bx:'changdu', tc:'changdu', cn:'昌都'},
        {bx:'naqu', tc:'nq', cn:'那曲'},
        {bx:'ali', tc:'al', cn:'阿里'},
        {bx:'', tc:'rituxian', cn:'日土'},
        {bx:'', tc:'gaizexian', cn:'改则'},
        {bx:'', tc:'hlr', cn:'海拉尔'},
        {bx:'shaanxi', tc:'', cn:'陕西'},
        {bx:'xian', tc:'xa', cn:'西安'},
        {bx:'xianyang', tc:'xianyang', cn:'咸阳'},
        {bx:'yanan', tc:'yanan', cn:'延安'},
        {bx:'yulin', tc:'yl', cn:'榆林'},
        {bx:'weinan', tc:'wn', cn:'渭南'},
        {bx:'shangluo', tc:'sl', cn:'商洛'},
        {bx:'ankang', tc:'ankang', cn:'安康'},
        {bx:'hanzhong', tc:'hanzhong', cn:'汉中'},
        {bx:'baoji', tc:'baoji', cn:'宝鸡'},
        {bx:'tongchuan', tc:'tc', cn:'铜川'},
        {bx:'gansu', tc:'', cn:'甘肃'},
        {bx:'longnan', tc:'ln', cn:'陇南'},
        {bx:'wuwei', tc:'wuwei', cn:'武威'},
        {bx:'jiayuguan', tc:'jyg', cn:'嘉峪关'},
        {bx:'linxia', tc:'linxia', cn:'临夏'},
        {bx:'lanzhou', tc:'lz', cn:'兰州'},
        {bx:'dingxi', tc:'dx', cn:'定西'},
        {bx:'pingliang', tc:'pl', cn:'平凉'},
        {bx:'qingyang', tc:'qingyang', cn:'庆阳'},
        {bx:'jinchang', tc:'jinchang', cn:'金昌'},
        {bx:'zhangye', tc:'zhangye', cn:'张掖'},
        {bx:'jiuquan', tc:'jq', cn:'酒泉'},
        {bx:'tianshui', tc:'tianshui', cn:'天水'},
        {bx:'gannan', tc:'gn', cn:'甘南'},
        {bx:'baiyin', tc:'by', cn:'白银'},
        {bx:'qinghai', tc:'', cn:'青海'},
        {bx:'haibei', tc:'haibei', cn:'海北'},
        {bx:'xining', tc:'xn', cn:'西宁'},
        {bx:'haidong', tc:'haidong', cn:'海东'},
        {bx:'huangnan', tc:'huangnan', cn:'黄南'},
        {bx:'guoluo', tc:'guoluo', cn:'果洛'},
        {bx:'yushu', tc:'ys', cn:'玉树'},
        {bx:'haixi', tc:'hx', cn:'海西'},
        {bx:'hainan', tc:'hainan', cn:'海南'},
        {bx:'ningxia', tc:'', cn:'宁夏'},
        {bx:'zhongwei', tc:'zw', cn:'中卫'},
        {bx:'yinchuan', tc:'yinchuan', cn:'银川'},
        {bx:'shizuishan', tc:'szs', cn:'石嘴山'},
        {bx:'wuzhong', tc:'wuzhong', cn:'吴忠'},
        {bx:'guyuan', tc:'guyuan', cn:'固原'},
        {bx:'xinjiang', tc:'', cn:'新疆'},
        {bx:'yili', tc:'yili', cn:'伊犁'},
        {bx:'tacheng', tc:'tac', cn:'塔城'},
        {bx:'hami', tc:'hami', cn:'哈密'},
        {bx:'hetian', tc:'ht', cn:'和田'},
        {bx:'aletai', tc:'alt', cn:'阿勒泰'},
        {bx:'boertala', tc:'betl', cn:'博尔塔拉'},
        {bx:'kelamayi', tc:'klmy', cn:'克拉玛依'},
        {bx:'wulumuqi', tc:'xj', cn:'乌鲁木齐'},
        {bx:'shihezi', tc:'shz', cn:'石河子'},
        {bx:'changji', tc:'changji', cn:'昌吉'},
        {bx:'changji', tc:'kl', cn:'垦利'},
        {bx:'tulufan', tc:'tlf', cn:'吐鲁番'},
        {bx:'bayinguoleng', tc:'bygl', cn:'巴音郭楞'},
        {bx:'akesu', tc:'aks', cn:'阿克苏'},
        {bx:'', tc:'ks', cn:'喀什'},
        {bx:'', tc:'kzls', cn:'克孜勒苏'},
        {bx:'', tc:'ale', cn:'阿拉尔'},
        {bx:'', tc:'wjq', cn:'五家渠'},
        {bx:'', tc:'tmsk', cn:'图木舒克'},
        {bx:'', tc:'kel', cn:'库尔勒'}]
    , randomfunc = function(required){
        var rnum = Math.random()
        return rnum * required
    }
    , randomsite = function(){
        var r_site = randomfunc(2)
        if(r_site == 0)
            return 'bx'
        else
            return '58'
    }
    , localkeyForweb = function(p_local, cb){
        var sh_order = 'head -n 80 /dev/urandom | tr -dc A-Za-z0-9 | head -c 168'
        s_random.shellFunc(sh_order).then(function (result) {
            cb(result)
        }).then(function (err) {
            if (err) {
                joiners.appOptErr(p_local, null, `${err}`, 'websource.localkeyForweb.s_random.shellFunc', 'null', 'null', 'null')
                cb('localKey failed')
            }
        })
    }
    , getwebImg = function(urls, local, time, cb){
        if(!urls.length) {
            cb('img download success')
            return
        }
        http.get(urls[0], function(res){
            var imgData = ""
            res.setEncoding("binary"); //一定要设置response的编码为binary否则会下载下来的图片打不开
            res.on("data", function(chunk){
                imgData += chunk;
            })
            res.on("end", function(){
                var sh_order = 'head -n 80 /dev/urandom | tr -dc A-Za-z0-9 | head -c 32'
                s_random.shellFunc(sh_order).then(function (result) {
                    fs.writeFile(`/data/release/helical/uploads/${result}`, imgData, "binary", function(err){
                        if(err){
                            joiners.appOptErr(url, null, `${err}`, 'websource.getwebImg.fs.writeFile', 'null', url, 'null')
                            urls.splice(0, 1)
                            getwebImg(urls)
                        }
                        else{
                            var values = {
                                img_time: time,
                                img_key: local,
                                img_size: '',//req.body.width+'?&'+req.body.height+'?&'+req.file.size
                                img_pos: urls.length-1,
                                img_local: '',
                                img_name: result
                            }
                            mysql.insert_pic(values).then(function (mres) {
                                urls.splice(0, 1)
                                getwebImg(urls)
                            }).then(function (err) {
                                if(err != undefined) {
                                    urls.splice(0, 1)
                                    getwebImg(urls)
                                }
                            })
                        }
                    })
                }).then(function (err) {
                    if (err) {
                        joiners.appOptErr(url, null, `${err}`, 'websource.getwebImg', 'null', url, 'null')
                        urls.splice(0, 1)
                        getwebImg(urls)
                    }
                })

            })
        })
    }
    , findCityObj = function(addr){
        for(var oi in qgcitys) {
            if(addr.match(qgcitys[oi].cn))
                return qgcitys[oi]
        }
        return null
    }
    , reanynisAddr = function(addr){
        return new Promise(function (resolve, reject) {
            txgeo.addrToLocal(addr, function (c_res) {
                if (c_res != 'error')
                    resolve(c_res)
                else {
                    joiners.appOptErr(null, addr, `${c_res}`, 'websource.reanynisAddr.txgeo', 'web', addr, 'null')
                    reject(c_res)
                }
            })
        })
    }
    , getWebChildren = function(site, gate){
        var c_select = []
        if(site == 'bx'){
            for (var ci in bxChildSite) {
                if (ci.match(gate)) {
                    c_select = bxChildSite[ci]
                    break
                }
            }
        }
        else{
            for (var ci in ChildSite58) {
                if (ci.match(gate)) {
                    c_select = ChildSite58[ci]
                    break
                }
            }
        }
        return c_select
    }
    , getChildPages = function(bx58, p_page, p_url){
        var c_pages = p_page.match(/"http:\/\/(.*?)jump(.*?)"/g)
        if(bx58) {
            p_url = p_url.split('//')
            var revar = new RegExp("<a href='http:\/\/"+`${p_url[1]}`+"(.*?)'", "g")
            c_pages = p_page.match(revar)
        }
        return c_pages
    }
    , analyseIndexPage = function(info, gate, bx58, r_num, cb){
        var revar = new RegExp("<li data-aid=(.*?)<\/li>", "g"), names = [], times = [], i_link = []
        if(!bx58) {
            if (gate == 'tapGates')
                revar = new RegExp("<div(.*?)infolist(.*?)class(.*?)cleft(.*?)>(.|\r|\n)*?<\/table>", "g")
            else if (gate == 'tapJobs')
                revar = new RegExp("<dd(.*?)title(.*?)<\/dd>", "g")
            else if (gate == 'tapServes')
                revar = new RegExp("<div(.*?)id=\"infolist\"(.*?)class=\"cleft\"(.*?)\>(.|\r|\n)*?<\/table>", "g")
            else if (gate == 'tapRend')
                revar = new RegExp("<li (.|\r|\n)*?<\/li>", "g")
            else
                revar = new RegExp("<table(.*?)class=\"tbimg(.|\r|\n)*?<\/table>", "g")
        }
        var dds = info.value.match(revar)
        if(bx58){
            if(!dds){
                revar = new RegExp("<div\\s+class='waterfall-glist-container'>(.*?)<div\\s+class='waterfall-loader'>", "g")
                dds = info.value.match(revar)[0].match(/<div(.*?)class='waterdrop(.*?)><div(.*?)class='waterdrop/g)
                for (var ii in dds) {
                    var ddsi = dds[ii].match(/http:\/\/[^\s]*html/g)[0]
                    if (ddsi) {
                        i_link.push(ddsi)
                        if (gate == 'tapGates' || gate == 'tapJobs') {
                            var name = ddsi[0].match(/class='waterfall-title'>[\u4e00-\u9fa5|a-zA-Z0-9]+</g)[0].match(/>[\u4e00-\u9fa5|a-zA-Z0-9]+</g)[0].replace(/<|>/g, '')
                            names.push(name)
                        }
                        if (ii > r_num)
                            break
                    }
                }
            }
            else {
                revar = new RegExp("<div class='ad-item-detail'>(.*?)http(.*?)<\/time>", "g")
                for (var ii in dds) {
                    var ddsi = dds[ii].match(revar), link = '', time = ''
                    if (ddsi) {
                        link = ddsi[0].match(/http:\/\/[^\s]*\'/g)[0].replace(/'/g, '')
                        if (link) {
                            i_link.push(link)
                            time = ddsi[0].match(/<time>(.*?)<\/time>/g)[0].replace(/[^\u4e00-\u9fa5|0-9|\s]*/g, '')
                            times.push(time)
                            if (gate == 'tapGates' || gate == 'tapJobs') {
                                var name = ddsi[0].match(/>[\u4e00-\u9fa5]+</g)[0].replace(/<|>/g, '')
                                names.push(name)
                            }
                            if (ii > r_num)
                                break
                        }
                    }
                }
            }
        }
        else{
            if(gate=='tapGates' || gate=='tapServes'){
                revar = new RegExp("<td(.*?)class=\"t\">(.|\r|\n)*?<\/td>", "g")
                var ddsi = dds[0].match(revar)
                for (var ii in ddsi) {
                    var linkings = ddsi[ii].match(/<a href='http:\/\/(.|\r|\n)*?<p class="seller">/g)
                    if(linkings) {
                        var link = linkings[0].match(/http:\/\/[^\s]*\'/g)[0].replace(/'/g, '')
                        i_link.push(link)
                        var name = ddsi[ii].match(/<p class="seller">(.|\r|\n)*?\/p>/g)[0].match(/>[\u4e00-\u9fa5]+</g)[0].replace(/<|>/g, '')
                        names.push(name)
                        if(ii > r_num)
                            break
                    }
                }
            }
            else if (gate == 'tapJobs'){
                for (var ii in dds) {
                    var ddsi = dds[ii].match(/title=\"[\u4e00-\u9fa5]*\">[\u4e00-\u9fa5]{1}/g), link = '', name = ''
                    if(ddsi) {
                        link = dds[ii].match(/http:\/\/[^\s]*\'/g)[0].replace(/'/g, '')
                        i_link.push(link)
                        name = ddsi[0].split('>')[0].replace(/[^\u4e00-\u9fa5]*/g, '')
                        names.push(name)
                        if(ii > r_num)
                            break
                    }
                }
            }
            else if (gate == 'tapRend'){
                revar = new RegExp("<ul class=\"listUl\">(.|\r|\n)*?<\/ul>", "g")
                dds = info.value.match(revar)
                revar = new RegExp("<li(.|\r|\n)*?<\/li>", "g")
                var ddsi = dds[0].match(revar)
                for (var ii in ddsi) {
                    var linkings = ddsi[ii].match(/<div(.*?)class=\"des\">(.|\r|\n)*?<\/a>/g)
                    if(linkings) {
                        var link = linkings[0].match(/http:\/\/[^\s]*\"/g)[0].replace(/"/g, '')
                        i_link.push(link)
                        if(ii > r_num)
                            break
                    }
                }
            }
            else{
                revar = new RegExp("<tr(.*?)sortid=(.*?)>(.|\r|\n)*?<\/tr>", "g")
                var ddsi = dds[0].match(revar)
                for (var ii in ddsi) {
                    var linkings = ddsi[ii].match(/<td(.*?)class=\"img\">(.|\r|\n)*?<\/td>/g)
                    if(linkings) {
                        var link = linkings[0].match(/http:\/\/[^\s]*\'/g)[0].replace(/'/g, '')
                        i_link.push(link)
                        if(ii > r_num)
                            break
                    }
                }
            }
        }
        var web_searcheds = []
        var recur_link = function(l_list){
            if(!l_list.length) {
                cb(web_searcheds)
                return
            }
            fetchWebSite(l_list[0]).then(function(res) {
                if (gate == 'tapGates'){
                    var c_obj = {city:info.city, type: info.type, name:names[0], value: res}
                    analyseWebForGate(c_obj, info.latitude, info.longitude, info.lscope, info.scope, bx58, function (g_res) {
                        if (g_res)
                            web_searcheds.push(g_res)
                        l_list.splice(0, 1)
                        names.splice(0, 1)
                        recur_link(l_list)
                    })
                }
                else if(gate == 'tapJobs') {
                    var c_obj = {city:info.city, company:names[0], value:res}
                    analyseWebForJob(c_obj, info.latitude, info.longitude, info.lscope, info.scope, bx58, function(g_res){
                        if(g_res)
                            web_searcheds.push(g_res)
                        l_list.splice(0, 1)
                        names.splice(0, 1)
                        recur_link(l_list)
                    })
                }
                else if(gate == 'tapServes') {
                    var c_obj = {city:info.city, type: info.type, value:res}
                    analyseWebForServe(c_obj, info.latitude, info.longitude, info.lscope, info.scope, bx58, function(g_res){
                        if(g_res)
                            web_searcheds.push(g_res)
                        l_list.splice(0, 1)
                        recur_link(l_list)
                    })
                }
                else if(gate == 'tapRend') {
                    var c_obj = {city:info.city, value:res}
                    analyseWebForRend(c_obj, info.latitude, info.longitude, info.lscope, info.scope, bx58, function(g_res){
                        if(g_res)
                            web_searcheds.push(g_res)
                        l_list.splice(0, 1)
                        recur_link(l_list)
                    })
                }
                else {
                    var c_obj = {city:info.city, value:res}
                    analyseWebForOld(c_obj, info.latitude, info.longitude, info.lscope, info.scope, bx58, function(g_res){
                        if(g_res)
                            web_searcheds.push(g_res)
                        l_list.splice(0, 1)
                        recur_link(l_list)
                    })
                }
            }).then(function(err){
                if(err){
                    joiners.appOptErr(l_list[0], 'null', `${err}`, 'websource.analyseIndexPage.fetchWebSite', 'null', 'null', 'null')
                    l_list.splice(0, 1)
                    recur_link(l_list)
                }
            })
        }
        recur_link(i_link)
    }
    , analyseWebForGate = function(info, s_lat, s_lng, slast_scope, scur_scope, bx58, cb){
        var arri_obj = {time:'', location:{longitude:'', latitude:''}, pictures:0, content:{add_key:gate, add_value:[
            {id:'gate0', name:info.type}, {id:'gate1', name:''}, {id:'gate2', contents:''}, {id:'gate4', contents:''}
        ]}, address:''}
        var revar = new RegExp("<div\\s+class='viewad-meta2\\s+new-fuwu-version'>(.*?)<div\\s+id='newContactModal'\\s+class='modal modal-load\\s+disable-setpos\\s+hide'>", "g")
        var f_match = []
        if(bx58) {
            f_match = info.value.match(revar)
            if(f_match[0].match(/所在地/g))
                arri_obj.address = info.city+f_match[0].match(/所在地(.*?)<\/a>/g)[0].replace(/<[^\u4e00-\u9fa5]*>/g, '').replace(/<|>/g, '').split('所在地')[1]
            else
                arri_obj.address = info.city+f_match[0].match(/服务范围(.*?)<\/a>/g)[0].replace(/<[^\u4e00-\u9fa5]*>/g, '').replace(/<|>/g, '').split('服务范围')[1]
        }
        else{
            revar = new RegExp("<div\\s+class=\"newinfo\"(.|\r|\n)*?<span\\s+class=\"adr\">(.*?)\/span>", "g")
            arri_obj.address = info.city+info.value.match(revar)[0].replace(/[^\u4e00-\u9fa5|0-9|\：|\:|\-]*/g, '')
        }
        if(arri_obj.address == '')
            return cb(null)
        reanynisAddr(arri_obj.address).then(function (a_res) {
            var distance = dis_scope.distanceCal(a_res.latitude, a_res.longitude, s_lat, s_lng)
            if (distance >= slast_scope && distance <= scur_scope) {
                arri_obj.location.longitude = a_res.lng
                arri_obj.location.latitude = a_res.lat
                var imgs_arr = []
                if(bx58){
                    revar = new RegExp("首次发布于(.*?)<\/span>", "g")
                    arri_obj.time = f_match[0].match(revar)[0].split('>')[1].replace(/&nbsp;/g, '').replace(/[^\u4e00-\u9fa5|:|0-9]+/g, '')
                    if(f_match[0].match(/服务内容/g))
                        arri_obj.content.add_value[2].contents += f_match[0].match(/服务内容(.*?)<label>/g)[0].replace(/target='_blank'/g, ' ').replace(/[^\u4e00-\u9fa5|\s|：]+/g, '')+'\n'
                    if(f_match[0].match(/所在地/g))
                        arri_obj.content.add_value[2].contents += f_match[0].match(/所在地(.*?)<label>/g)[0].replace(/target='_blank'/g, ' ').replace(/[^\u4e00-\u9fa5|\s|：]+/g, '')+'\n'
                    if(f_match[0].match(/服务范围/g))
                        arri_obj.content.add_value[2].contents += f_match[0].match(/服务范围(.*?)<label>/g)[0].replace(/target='_blank'/g, ' ').replace(/[^\u4e00-\u9fa5|\s|：]+/g, '')+'\n'
                    if(f_match[0].match(/联系人/g))
                        arri_obj.content.add_value[2].contents += f_match[0].match(/联系人(.*?)<label>/g)[0].replace(/target='_blank'/g, ' ').replace(/[^\u4e00-\u9fa5|\s|：]+/g, '')
                    revar = new RegExp("<strong>[0-9]+<\/strong>", "g")
                    arri_obj.content.add_value[3].contents = '联系电话：'+info.value.match(revar)[0].replace(/<.*?>/g, '')
                    revar = new RegExp("<div\\s+class='photo-item(.*?)<\/div>", "g")
                    var images = info.value.match(revar)
                    for (var imi in images) {
                        var image = images[imi].match(/http:[^\s]*\)/g)[0].replace(/\)/g, '')
                        imgs_arr.push(image)
                        if (imgs_arr.length == 3)
                            break
                    }
                }
                else {
                    revar = new RegExp("发布日期(.*?)<\/li>", "g")
                    arri_obj.time = info.value.match(revar)[0].split('>')[1].split('<')[0]
                    revar = new RegExp("sphone.*[\r\n]*?", "g")
                    arri_obj.content.add_value[3].contents = '联系电话：'+info.value.match(revar)[0].replace(/[^0-9]+/g, "").replace(/'/g, "")
                    revar = new RegExp("<article\\s+class=\"description_con\">(.|\r|\n)*?<\/script>", "g")
                    var re_intro = info.value.match(revar)[0].match(/>[\u4e00-\u9fa5|\u3002|\uff1f|\uff01|\uff0c|\u3001|\uff1b|\uff1a|\u201c|\u201d|\u2018|\u2019|\uff08|\uff09|\u300a|\u300b|\u3008|\u3009|\u3010|\u3011|\u300e|\u300f|\u300c|\u300d|\ufe43|\ufe44|\u3014|\u3015|\u2026|\u2014|\uff5e|\ufe4f|\uffe5|0-9]+</g)
                    for (var ri in re_intro) {
                        arri_obj.content.add_value[2].contents += re_intro[ri].replace(/>|</g, '')
                        if (ri != re_intro.length-1)
                            arri_obj.content.add_value[2].contents += '\n'
                    }
                    revar = new RegExp("<div\\s+id=\"img_player1\">(.|\r|\n)*?<\/div>", "g")
                    var images = info.value.match(revar)[0].match(/http:\/\/pic[^\s]*"/g)
                    for (var imi in images) {
                        var image = images[imi].replace(/"/g, '')
                        imgs_arr.push(image)
                        if (imgs_arr.length == 3)
                            break
                    }
                }
                arri_obj.pictures = imgs_arr.length
                cb(arri_obj)
            }
            else
                cb(null)
        }).then(function (err) {
            if (err) {
                joiners.appOptErr('null', 'null', `${err}`, 'websource.analyseWebForGate.reanynisAddr', 'null', 'null', 'null')
                cb(null)
            }
        })
    }
    , analyseWebForJob = function(info, s_lat, s_lng, slast_scope, scur_scope, bx58, cb){/*no finished*/
        var arri_obj = {time:'', location:{longitude:'', latitude:''}, pictures:0, content:{add_key:gate, add_value:[
            {id:'gate0', name:info.company}, {id:'gate1', contents:''}, {id:'gate2', contents:''}, {id:'gate4', contents:''}
        ]}, address:''}
        var revar = new RegExp(">工作地点：(.*?)\/span>", "g")
        if(bx58)
            arri_obj.address = info.city+info.value.match(revar)[0].split('工作地点：')[1].replace(/[^\u4e00-\u9fa5-|0-9]*/g, '')
        else{
            revar = new RegExp(">工作地址(.*?)(\/span>|\/p>)(.*?)(\/span>|\/p>)", "g")
            arri_obj.address = info.city+info.value.match(revar)[0].split('工作地址')[1].replace(/[^\u4e00-\u9fa5-|0-9]*/g, '')
        }
        if(arri_obj.address == '')
            return cb(null)
        reanynisAddr(arri_obj.address).then(function (a_res) {
            var distance = dis_scope.distanceCal(a_res.latitude, a_res.longitude, s_lat, s_lng)
            if (distance >= slast_scope && distance <= scur_scope) {
                arri_obj.location.longitude = a_res.lng
                arri_obj.location.latitude = a_res.lat
                var imgs_arr = []
                if(bx58){
                    revar = new RegExp("首次发布于(.*?)<\/span>", "g")
                    arri_obj.time = info.value.match(revar)[0].split('>')[1].replace(/&nbsp;/g, '').replace(/[^\u4e00-\u9fa5|:|0-9]+/g, '')
                    revar = new RegExp(">职位类别：(.*?)\/span>", "g")
                    arri_obj.content.add_value[2].contents = info.value.match(revar)[0].split('职位类别：')[1].replace(/[^\u4e00-\u9fa5-|0-9]*/g, '')+'\n'
                    revar = new RegExp(">招聘人数：(.*?)\/span>", "g")
                    arri_obj.content.add_value[2].contents += info.value.match(revar)[0].split('招聘人数：')[1].replace(/[^\u4e00-\u9fa5-|0-9]*/g, '')+'\n'
                    revar = new RegExp(">学历要求：(.*?)\/span>", "g")
                    var sc_info = info.value.match(revar)
                    if(sc_info)
                        arri_obj.content.add_value[2].contents += sc_info[0].split('学历要求：')[1].replace(/[^\u4e00-\u9fa5-|0-9]*/g, '')+'\n'
                    revar = new RegExp(">年龄：(.*?)'>", "g")
                    var p_old = info.value.match(revar)
                    if(p_old)
                        arri_obj.content.add_value[2].contents += p_old[0].replace(/[^\u4e00-\u9fa5-|0-9|\d-]*/g, "")
                    revar = new RegExp("<strong>[0-9]*<\/strong>", "g")
                    arri_obj.content.add_value[3].contents = '联系电话：'+info.match(revar)[0].replace(/<.*?>/g, "")
                    revar = new RegExp(">职位描述(.|\r\n)*hide'>", "g")
                    arri_obj.content.add_value[1].contents = info.value.split('职位描述')[1].replace(/<br \/>/g, '\n').replace(/<.*>|&zwnj;/g, '')
                }
                else{
                    if(info.value.match(/>薪资待遇：</g)) {
                        revar = new RegExp("<div(.*?)class=\"posinfo\".*<span(.*?)id=\"ck1\">", "g")
                        var re_poses = info.value.match(revar)[0].match(/<li(.*?)<\/li>/g)
                        for(var poi in re_poses) {
                            arri_obj.content.add_value[2].contents += re_poses[poi].replace(/<a(.*?)\/a>/g, '').replace(/<span>+/g, ' ').replace(/<[^\u4e00-\u9fa5]+>/g, '')
                            if(poi != re_poses.length-1)
                                arri_obj.content.add_value[2].contents += '\n'
                        }
                        revar = new RegExp("<div(.*?)class=\"headline\"(.*?)id=\"gongsi_h\">.*<(.*?)class=\"videoup(.*?)>", "g")
                        arri_obj.content.add_value[1].contents = info.value.match(revar)[0].replace(/<br>/g, "\n").replace(/<[^\u4e00-\u9fa5]+>|公司介绍/g, "")
                    }
                    else{
                        arri_obj.content.add_value[1].contents = info.value.replace(/<br>/g, "\n").match(/公司介绍<\/h2>(.|\r|\n)*?\/p>/g)[0].replace(/<[^\u4e00-\u9fa5]+>|公司介绍|公司视频：/g, "")
                        var head_tabs = info.value.match(/<div(.*?)<\/div>/g)
                        for(var ti in head_tabs){
                            if(head_tabs[ti].match(/pos_salary/g))
                                arri_obj.content.add_value[1].contents += head_tabs[ti].match(/pos_salary(.*?)\/span>/g)pos_base_condition item_condition
                            if(head_tabs[ti].match(/pos_name/g))
                                arri_obj.content.add_value[1].contents += head_tabs[ti].match(/pos_name(.*?)\/span>/g)
                        }
                        arri_obj.content.add_value[1].contents = '薪资待遇：'+info.value.match(/<span\\s+class=\"pos_salary\">(.*?)<\/span>/g, "\n")[0].replace(/<.*>/g, '')
                    }
                }
                cb(arri_obj)
            }
            else
                cb(null)
        }).then(function (err) {
            if (err) {
                joiners.appOptErr('null', 'null', `${err}`, 'websource.analyseWebForJob.reanynisAddr', 'null', 'null', 'null')
                cb(null)
            }
        })
    }
    , analyseWebForServe = function(info, s_lat, s_lng, slast_scope, scur_scope, bx58, cb){
        var arri_obj = {time:'', location:{longitude:'', latitude:''}, pictures:0, content:{add_key:gate, add_value:[
            {id:'gate0', name:info.type}, {id:'gate1', contents:''}, {id:'gate2', contents:''}, {id:'gate4', contents:''}
        ]}, address:''}
        var revar = new RegExp("所在地：(.*?)\/div>", "g")
        if(bx58)
            arri_obj.address = info.value.match(revar)[0].split('：')[1].replace(/[^\u4e00-\u9fa5]*/g, '')
        else {
            revar = new RegExp("<.*adr.*>-.*\/.*>", "g")
            arri_obj.address = info.value.match(revar)[0].replace(/[^\u4e00-\u9fa5]*/g, '')
        }
        if(arri_obj.address == '')
            return cb(null)
        reanynisAddr(arri_obj.address).then(function (a_res) {
            var distance = dis_scope.distanceCal(a_res.latitude, a_res.longitude, s_lat, s_lng)
            if (distance >= slast_scope && distance <= scur_scope) {
                arri_obj.location.longitude = a_res.lng
                arri_obj.location.latitude = a_res.lat
                var imgs_arr = []
                if(bx58){
                    revar = new RegExp("服务范围：(.*?)\/div>", "g")
                    arri_obj.content.add_value[1].name = '服务范围：'+info.value.match(revar)[0].split('：')[1].replace(/\s+/g, ' ').replace(/[^\u4e00-\u9fa5-\s]/g, '')
                    revar = new RegExp("服务简介(.|\r|\n)*?\/section>", "g")
                    arri_obj.content.add_value[2].contents = info.value.match(revar)[0].split('服务简介')[1].replace(/(\r\n)*/g, '').replace(/<br \/>/g, '\n').replace(/<[^\u4e00-\u9fa5]*>|&zwnj;|&nbsp;/g, '')
                    revar = new RegExp("<strong>[0-9]*<\/strong>", "g")
                    arri_obj.content.add_value[3].contents = '联系电话：'+info.value.match(revar)[0].replace(/<.*?>/g, "")
                    revar = new RegExp("<div\\s+class='photo-item(.*?)<\/div>", "g")
                    var images = info.value.match(revar)
                    for (var imi in images) {
                        var image = images[imi].match(/http:[^\s]*\)/g)[0].replace(/\)/g, '')
                        imgs_arr.push(image)
                        if (imgs_arr.length == 3)
                            break
                    }
                }
                else{
                    revar = new RegExp("<.*发布日期.*\/.*>", "g")
                    arri_obj.time = info.value.match(revar)[0].split('>')[1].split('<')[0]
                    revar = new RegExp("sphone.*[\r\n]*", "g")
                    arri_obj.content.add_value[3].contents = '联系电话：'+info.value.match(revar)[0].split("'")[1].split("'")[0]
                    revar = new RegExp("服务区域(.|\r|\n)*?\/div>", "g")
                    arri_obj.content.add_value[1].name = info.match(revar)[0].replace(/\s+/g, ' ').replace(/[^\u4e00-\u9fa5-\s]/g, '')
                    revar = new RegExp("<article(.|\r|\n)*\/article>","g")
                    arri_obj.content.add_value[2].contents = info.match(revar)[0].replace(/<br \/>/g, '\n').replace(/<[^\u4e00-\u9fa5]*>|&zwnj;|&nbsp;/g, '')
                    revar = new RegExp("img src=\"http:.*\/div>", "g")
                    var images = info.value.match(revar)[0].split('img src=')
                    for (var imi in images) {
                        if (images[imi].match('http:')) {
                            var image = images[imi].match(/http:[^\s]*\s/g)[0].replace(/"/g, '').replace(/\s/g, '')
                            imgs_arr.push(image)
                        }
                        if (imgs_arr.length == 3)
                            break
                    }
                }
                arri_obj.pictures = imgs_arr.length
                cb(arri_obj)
            }
            else
                cb(null)
        }).then(function (err) {
            if (err) {
                joiners.appOptErr('null', 'null', `${err}`, 'websource.analyseWebForServe.reanynisAddr', 'null', 'null', 'null')
                cb(null)
            }
        })
    }
    , analyseWebForRend = function(info, s_lat, s_lng, slast_scope, scur_scope, bx58, cb){//no finished
        var arri_obj = {time:'', location:{longitude:'', latitude:''}, pictures:0, content:{add_key:gate, add_value:[
            {id:'gate0', name:info.type}, {id:'gate1', contents:''}, {id:'gate2', contents:''}, {id:'gate4', contents:''}, {id:'gate5', contents:''}
        ]}, address:''}
        var revar = new RegExp("地址：(.*?)\/section>", "g")
        if(bx58)
            arri_obj.address = info.value.match(revar)[0].replace(/\s+|&zwnj;+|&nbsp;+/g, ' ').replace(/[^\u4e00-\u9fa5|\s|\：]*/g, '').replace(/\s+/g, ' ')
        else {
            revar = new RegExp("详细地址：(.*?)\/span>(.|\r|\n)*?\/span>", "g")
            arri_obj.address = info.value.match(revar)[0].split('详细地址：')[1].replace(/(\r\n\s+)*/g, '').replace(/<[^\u4e00-\u9fa5]*>|&zwnj;|&nbsp;/g, '')
        }
        if(arri_obj.address == '')
            return cb(null)
        reanynisAddr(arri_obj.address).then(function (a_res) {
            var distance = dis_scope.distanceCal(a_res.latitude, a_res.longitude, s_lat, s_lng)
            if (distance >= slast_scope && distance <= scur_scope) {
                arri_obj.location.longitude = a_res.lng
                arri_obj.location.latitude = a_res.lat
                var imgs_arr = []
                if(bx58){
                    revar = new RegExp("房型：</label>(.*?)\/label>", "g")
                    if(info.nobj > 2) {
                        revar = new RegExp("面积(.*?)class='meta-面积'(.*?)\/span>", "g")
                        arri_obj.content.add_value[0].name = '面积'+info.value.match(revar)[0].match(/>[\u4e00-\u9fa5|0-9|\s]+</g)[0].replace(/<|>/g, '')
                    }
                    else
                        arri_obj.content.add_value[0].name  = info.value.match(revar)[0].match(/>[\u4e00-\u9fa5|0-9|\s]+</g)[0].replace(/<|>/g, '')
                    revar = new RegExp("<span\\s+class='meta-价格'>(.*?)\/span>", "g")
                    arri_obj.content.add_value[1].name = info.value.match(revar)[0].match(/>[\u4e00-\u9fa5|0-9]*</g)[0].replace(/<|>/g, '')
                    revar = new RegExp("房屋配置：</label>(.*?)\/label>", "g")
                    if(info.nobj == 0){
                        revar = new RegExp("房屋配置：</label>(.*?)\/label>", "g")
                        arri_obj.content.add_value[2].contents = info.value.match(revar)[0].split('房屋配置：')[1].replace(/<[^\u4e00-\u9fa5]*>/g, '').replace(/<.*?>/g, '')
                    }
                    revar = new RegExp("viewad-text(.|\r|\n)*?\/div>", "g")
                    arri_obj.content.add_value[3].contents = info.value.match(revar)[0].replace(/​&zwnj;|[^\u4e00-\u9fa5|\u3002|\uff1f|\uff01|\uff0c|\u3001|\uff1b|\uff1a|\u201c|\u201d|\u2018|\u2019|\uff08|\uff09|\u300a|\u300b|\u3008|\u3009|\u3010|\u3011|\u300e|\u300f|\u300c|\u300d|\ufe43|\ufe44|\u3014|\u3015|\u2026|\u2014|\uff5e|\ufe4f|\uffe5|0-9|\s]*/g, '')
                    revar = new RegExp("<strong>[0-9]*<\/strong>", "g")
                    arri_obj.content.add_value[4].contents = info.value.match(revar)[0].replace(/<.*?>/g, "")
                    revar = new RegExp("<div\\s+class='photo-item(.*?)<\/div>", "g")
                    var images = info.value.match(revar)
                    for (var imi in images) {
                        var image = images[imi].match(/http:[^\s]*\)/g)[0].replace(/\)/g, '')
                        imgs_arr.push(image)
                        if (imgs_arr.length == 3)
                            break
                    }
                }
                else{
                    revar = new RegExp("房屋类型：(.*?)\/span>(.|\r|\n)*?\/span>", "g")
                    arri_obj.content.add_value[0].name = info.value.match(revar)[0].split('房屋类型：')[1].replace(/\s+|&zwnj;|&nbsp;/g, ' ').replace(/[^\u4e00-\u9fa5-\0-9-\s]*/g, '').replace(/\&|\//g, '')
                    revar = new RegExp("house-pay-way(.|\r|\n)*?\/div>", "g")
                    var rend_info = info.value.match(revar)[0].split('class=')
                    arri_obj.content.add_value[1].name = rend_info[2].match(/>(.*?)</g)[0].replace(/>|</g, '')+rend_info.value[2].match(/>(.*?)</g)[1].replace(/>|<|\s/g, '')+' '+rend_info.value[3].replace(/[^\u4e00-\u9fa5]*/g, '')
                    revar = new RegExp("house-word-introduce(.|\r\n)*?\/div>", "g")
                    arri_obj.content.add_value[2].contents = info.value.match(revar)[0].replace(/&nbsp;+/g, ' ').replace(/[^\u4e00-\u9fa5\s]*/g, '').replace(/\s+/g, ' ')
                    revar = new RegExp("朝向楼层：(.*?)\/span>(.*?)\/span>", "g")
                    var desc_info = info.value.match(revar)[0]
                    arri_obj.content.add_value[3].contents = desc_info.replace(/&nbsp;+/g, ' ').replace(/<span>|<\/span>/g, '').replace(/\s+/g, ' ')+'\n'
                    revar = new RegExp("所在小区：<\/span>(.|\r|\n)*?\/span>", "g")
                    desc_info = info.value.match(revar)[0]
                    arri_obj.content.add_value[3].contents += '所在小区：'+desc_info.split('<\/span>')[1].replace(/[^\u4e00-\u9fa5]*/g, '')+'\n'
                    revar = new RegExp("所属区域：<\/span>(.|\r|\n)*?\/span>", "g")
                    desc_info = info.value.match(revar)[0]
                    arri_obj.content.add_value[3].contents += '所属区域：'+desc_info.split('<\/span>')[1].replace(/[^\u4e00-\u9fa5]*/g, '')+'\n'
                    arri_obj.content.add_value[3].contents += '详细地址：'+arri_obj.address
                    revar = new RegExp("phone-num(.*?)/em>","g")
                    arri_obj.content.add_value[4].contents = info.value.match(revar)[0].replace(/[^0-9]+/g, '').replace(/-|"/g, '')
                    revar = new RegExp("<div\\s+class=\"big-pic-list pr\">(.|\r|\n)*?<\/div>", "g")
                    var images = res.value.match(revar)[0].match(/<li(.|\r|\n)*?<\/li>/g)
                    for (var imi in images) {
                        if (images[imi].match('http:')) {
                            var image = images[imi].match(/http:[^\s]*\s/g)[0].replace(/"|>|\s/g, '')
                            imgs_arr.push(image)
                        }
                        if (imgs_arr.length == 3)
                            break
                    }
                }
                arri_obj.pictures = imgs_arr.length
                cb(arri_obj)
            }
            else
                cb(null)
        }).then(function (err) {
            if (err) {
                joiners.appOptErr('null', 'null', `${err}`, 'websource.analyseWebForServe.reanynisAddr', 'null', 'null', 'null')
                cb(null)
            }
        })
    }
    , analyseWebForOld = function(info, s_lat, s_lng, slast_scope, scur_scope, bx58, cb){
        var arri_obj = {time:'', location:{longitude:'', latitude:''}, pictures:0, content:{add_key:gate, add_value:[
            {id:'gate0', name:info.name}, {id:'gate1', contents:''}, {id:'gate2', contents:''}, {id:'gate4', contents:''}
        ]}, address:''}
        var revar = new RegExp("地址：</label>(.*?)\/li>", "g")
        if(bx58)
            arri_obj.address = info.value.match(revar)[0].replace(/[^\u4e00-\u9fa5|-\：]*/g, "")
        else {
            revar = new RegExp("区域：</div>(.|\r|\n)*?\/div>", "g")
            arri_obj.address = info.value.match(revar)[0].replace(/[^\u4e00-\u9fa5|-\：]*/g, "")
        }
        if(arri_obj.address == '')
            return cb(null)
        reanynisAddr(arri_obj.address).then(function (a_res) {
            var distance = dis_scope.distanceCal(a_res.latitude, a_res.longitude, s_lat, s_lng)
            if (distance >= slast_scope && distance <= scur_scope) {
                arri_obj.location.longitude = a_res.lng
                arri_obj.location.latitude = a_res.lat
                var imgs_arr = []
                if(bx58){
                    revar = new RegExp("<label>价格(.*?)\/span>", "g")
                    arri_obj.content.add_value[1].name = info.value.match(revar)[0].replace(/<(.*?)>/g, "").replace(/(.*?)：/g, "")
                    revar = new RegExp("TA的故事(.*?)<div\\s+class='viewad-text-hide'>", "g")
                    arri_obj.content.add_value[2].contents = info.value.match(revar)[0].replace(/<\/div>+/g, "\n").replace(/<[^\u4e00-\u9fa5]+>|[​&zwnj;]+|TA的故事/g, "")
                    revar = new RegExp("<strong>[0-9]+<\/strong>", "g")
                    arri_obj.content.add_value[3].contents = info.value.match(revar)[0].replace(/<.*?>/g, "")
                    revar = new RegExp("<div\\s+class='big-img-box'>(.|\r|\n)*?<ul", "g")
                    var images = info.value.match(revar)[0].match(/http:[^\s]*img[^\s]*'/g)
                    for (var imi in images) {
                        imgs_arr.push(images[imi].replace(/'/g, ''))
                        if (imgs_arr.length == 3)
                            break
                    }
                }
                else{
                    if(info.value.match(/物品详情<\/a>/g))
                        revar = new RegExp("价格：<\/div>(.|\r|\n)*?<\/div>", "g")
                    else
                        revar = new RegExp("<div\\s+class=\"price\">(.|\r|\n)*?<\/em>", "g")
                    arri_obj.content.add_value[1].name = info.value.match(revar)[0].replace(/\r|\n|\s+|[\u4e00-\u9fa5]+》|".*"|-+/g, "").replace(/<[^\u4e00-\u9fa5|0-9]+>/g, "").split('：')[1].replace(/<.*[\u4e00-\u9fa5]+>/g, "")
                    revar = new RegExp("<div\\s+class=\"descriptionBox(.|\r\n)*?<\/article>", "g")
                    arri_obj.content.add_value[2].contents = info.value.match(revar)[0].replace(/\r|\n|\s+/g, "").replace(/<\/span>/g, "\n").replace(/<[^\u4e00-\u9fa5]+>/g, "")
                    if(info.value.match(/物品详情<\/a>/g))
                        revar = new RegExp("<span\\s+id=\"t_phone\"(.|\r|\n)*?<\/span>","g")
                    else
                        revar = new RegExp("<span\\s+class=\"phone-num-open\">(.*?)\/span>","g")
                    arri_obj.content.add_value[3].contents = info.value.match(revar)[0].replace(/\r|\n|\s+|".*"/g, "").replace(/[^0-9]+/g, '')
                    revar = new RegExp("<div\\s+class=\"descriptionImg\">(.|\r|\n)*?<\/div>", "g")
                    var images = info.value.match(revar)[0].match(/http:\/\/pic[^\s]*(\s|")/g)
                    for (var imi in images) {
                        var image = images[imi].replace(/\s|"/g, '')
                        imgs_arr.push(image)
                        if (imgs_arr.length == 3)
                            break
                    }
                }
                arri_obj.pictures = imgs_arr.length
                cb(arri_obj)
            }
            else
                cb(null)
        }).then(function (err) {
            if (err) {
                joiners.appOptErr('null', 'null', `${err}`, 'websource.analyseWebForServe.reanynisAddr', 'null', 'null', 'null')
                cb(null)
            }
        })
    }
    , fetchWebSite = function(page_link){
        return new Promise(function (resolve, reject) {
            fetch(page_link).then(function (res) {
                return res.buffer()
            }).then(function (buffer) {
                resolve(buffer.toString('utf8'))
            }).catch(function (err) {
                reject(err)
                console.log(err)
            })
        })
    }
module.exports = {
    netGets: function(local, location, gate) {
        var server = http.createServer(function(req, res){}).listen(50082)
        return new Promise(function (resolve, reject) {
            var web_csite = findCityObj(local)
            if(!web_csite.bx && !web_csite.tc)
                reject(null)
            var f_site = webhead
            f_site += web_csite.bx ? `${web_csite.bx}.${bxsite}` : `${web_csite.tc}.${site58}`
            var site = randomsite()
            if(site == 'bx'){
                if(!web_csite.bx)
                    site = '58'
            }
            else{
                if(!web_csite.tc)
                    site = 'bx'
            }
            var web_list = getWebChildren(site, gate)
            var s_gates = randomfunc(web_list.length), s_randoms = []
            for(var si=0; si<s_gates; si++){
                var sii = randomfunc(web_list.length)
                if(s_randoms.length && s_randoms[s_randoms.length-1]!=sii)
                    s_randoms.push(sii)
                else {
                    if(s_randoms.length && s_randoms[s_randoms.length-1]==sii)
                        si -= 1
                    if(!s_randoms.length)
                        s_randoms.push(sii)
                }
            }
            var web_indexes = [], total_fetches = []
            var recurFetch = function(r_list) {
                if(!r_list.length) {
                    if(!web_indexes.length)
                        return reject(null)
                    var recur_indexes = function(w_list){
                        if(!w_list.length) {
                            resolve(total_fetches)
                            return
                        }
                        var f_info = {latitude:location.latitude, longitude:location.longitude, lscope:location.old_scope, scope:location.scope, value:w_list[0]}
                        if(gate=='tapGates' || gate=='tapServes')
                            f_info.type = web_list[w_list[0].num].cn
                        if(gate=='tapRend')
                            f_info.nobj = w_list[0].num
                        analyseIndexPage(f_info, gate, site, 5, function(reses){
                            for(var oi in reses)
                                total_fetches.push(reses[oi])
                            w_list.splice(0, 1)
                            recur_indexes(w_list)
                        })
                    }
                    recur_indexes(web_indexes)
                }
                var f_page = ''
                if(site == 'bx'){
                    if(gate == 'tapGates')
                        f_page = web_list[r_list[0]][`bxgate${r_list[0]}`]
                    else if(gate == 'tapJobs')
                        f_page = web_list[r_list[0]][`bxjob${r_list[0]}`]
                    else if(gate == 'tapServes')
                        f_page = web_list[r_list[0]][`bxserve${r_list[0]}`]
                    else if(gate == 'tapRend')
                        f_page = web_list[r_list[0]][`bxrend${r_list[0]}`]
                    else
                        f_page = web_list[r_list[0]][`bxold${r_list[0]}`]
                }
                else{
                    if(gate == 'tapGates')
                        f_page = web_list[r_list[0]][`58gate${r_list[0]}`]
                    else if(gate == 'tapJobs')
                        f_page = web_list[r_list[0]][`58job${r_list[0]}`]
                    else if(gate == 'tapServes')
                        f_page = web_list[r_list[0]][`58serve${r_list[0]}`]
                    else if(gate == 'tapRend')
                        f_page = web_list[r_list[0]][`58rend${r_list[0]}`]
                    else
                        f_page = web_list[r_list[0]][`58old${r_list[0]}`]
                }
                f_page = f_site + f_page
                fetchWebSite(f_page).then(function (p_info) {
                    if(gate=='tapGates' || gate=='tapServes' || gate=='tapRend') {
                        var i_obj = {num:r_list[0], value:p_info}
                        web_indexes.push(i_obj)
                    }
                    else
                        web_indexes.push(p_info)
                    r_list.splice(0, 1)
                    recurFetch(r_list)
                }).then(function (err) {
                    if (err) {
                        joiners.appOptErr(JSON.stringify(web_list[s_randoms[0]]), 'null', `${err}`, 'websource.netGets.fetchWebSite', 'null', 'null', 'null')
                        r_list.splice(0, 1)
                        recurFetch(r_list)
                    }
                })
            }
            recurFetch(s_randoms)
        })
    }
}

