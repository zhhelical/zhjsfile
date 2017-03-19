const fetch = require('node-fetch')
const file = require("fs")
const co = require('co')
const shell = require('./shell.js')
const joiner = require('./clients.js')
const txgeo = require('./txgeo.js')
const web_src = require('./websrc.js')
//const generatorlinks = require("./readdledfs.js")
var master_dir = '../../../master/'
    , clients_dir = '../../../clients/'
    , webhead = 'http://'
    , bxsite = 'baixing.com/'
    , site58 = '58.com/'
    , fetch_start = false
    , fetch_switch = false
    , interval
var fetchInterval = function(){
    return 18000000
}
var aDayInterval = function(){
    return 86400000
}
var startInterval = function(starter){
    var order_time = starter.getTime()
    var curDayEnd = new Date()
    curDayEnd.setHours(23, 59, 59, 999)
    var end_time = curDayEnd.getTime()
    return end_time-order_time
}
var loopInterval = function(){
    var curLoopEnd = fetchInterval()
    setTimeout(function(){
        fetch_switch = false
    }, curLoopEnd)
}
var randomfunc = function(required){
        var rnum = Math.random()
        return Math.floor(rnum * required)
    }
var randomsite = function(){
        var r_site = randomfunc(2)
        if(r_site == 0)
            return 'bx'
        else
            return '58'
    }
var matchSelectedPage = function(page, f_arr){
    var absolute = ''
    for(var fp in page) {
        if(fp != 'cn') {
            absolute = page[fp].split('/')[0]
            break
        }
    }
    for(var fi in f_arr){
        if(f_arr[fi].match(absolute)) {
            f_arr.splice(fi, 1)
            return true
        }
    }
    return null
}
var interval_time = function(){
    return 180000+randomfunc(3)*60000
}
var fetchWebSite = function(page_link, rw_pos, cb){
    fetch(page_link.page).then(function(res){
        return res.buffer()
    }).then(function(buffer) {
        var fExist = function() {
            var f_names = page_link.page.split('/')
            var f_name = f_names[f_names.length-2]
            file.exists(`${rw_pos}`, function (exists) {
                if (exists) {
                    file.writeFile(`${rw_pos}${f_name}.txt`, buffer.toString('utf8'), function () {
                        console.log(`finished ${rw_pos} ${page_link.city} ${page_link.page}`)
                        cb()
                    })
                }
                else {
                    var sh_order = `mkdir ${rw_pos}`
                    shell.shellFunc(sh_order).then(function (result) {
                        file.writeFile(`${rw_pos}${f_name}.txt`, buffer.toString('utf8'), function () {
                            console.log(`finished ${rw_pos} ${page_link.city} ${page_link.page}`)
                            cb()
                        })
                    }).then(function (err) {
                        if (err)
                            fExist()
                    })
                }
            })
        }
        fExist()
    }).catch(function(err) {
        console.log(err)
    })
}
var reCitys = function(site){
    var recitys = []
    if(!site) {
        for (var ci in web_src.qgcitys) {
            if (!web_src.qgcitys[ci].bx)
                continue
            recitys.push(web_src.qgcitys[ci].bx)
        }
    }
    else{
        for (var ci in web_src.qgcitys) {
            if (!web_src.qgcitys[ci].tc)
                continue
            recitys.push(web_src.qgcitys[ci].tc)
        }
    }
    return recitys
}
var selectCity = function(citys, lasts){
        var r_city = randomfunc(citys.length), found = false
        for(var li in lasts){
            if(lasts[li] == citys[r_city]){
                found = true
                citys.splice(r_city, 1)
                break
            }
        }
        if(found)
            selectCity(citys, lasts)
        else
            return citys[r_city]
    }
var rePages = function(random, site){
    var si = 0
    if(!site) {
        for (var pi in web_src.bxChildSite) {
            if (si == random)
                return web_src.bxChildSite[pi]
            si++
        }
    }
    else{
        for (var pi in web_src.ChildSite58) {
            if (si == random)
                return web_src.ChildSite58[pi]
            si++
        }
    }
}
var selectPage = function(pages, old_pages){
    var r_pages = randomfunc(pages.length)
    if(!matchSelectedPage(pages[r_pages], old_pages))
        return pages[r_pages]
    if(!old_pages.length)
        return null
    selectPage(pages, old_pages)
}
var loopForFetch = function(f_city, f_page, rw_dir, s_site){
    var set_inter = interval_time(), plink = ''
    console.log(set_inter)
    setTimeout(function () {
        for (var fp in f_page) {
            if (fp != 'cn') {
                plink = f_page[fp]
                break
            }
        }
        var page_link = webhead + `${f_city}.` + bxsite + plink
        var link_obj = {city: f_city, page: page_link}
        fetchWebSite(link_obj, rw_dir, function () {
            console.log(`finished ${f_city} ${page_link}`)
            randomFetch(s_site)
        })
    }, set_inter)
}
var randomFetch = function(r_site){
    if(!fetch_switch || !fetch_start)
        return
    var s_citys = reCitys(r_site), o_citys = [], f_city = selectCity(s_citys, o_citys)
    var f_pages = rePages(randomfunc(5), r_site)
    var cityDir = function (city) {
        var f_root = r_site ? `${master_dir}downloads/tc/` : `${master_dir}downloads/bx/`
        //console.log(f_city)
        file.exists(`${f_root}${city}`, function (exists) {
            console.log(exists)
            if (exists) {
                file.readdir(`${f_root}${city}`, function (f_err, files) {
                    if (f_err)
                        return
                    var f_page = selectPage(f_pages, files)
                    if (!f_page) {
                        o_citys.push(city)
                        var ff_city = selectCity(s_citys, o_citys)
                        cityDir(ff_city)
                    }
                    else
                        loopForFetch(f_city, f_page, `${f_root}${city}/`, r_site)
                })
            }
            else {
                var sh_order = `mkdir ${f_root}${city}`
                shell.shellFunc(sh_order).then(function (res) {
                    var olds = []
                    var f_page = selectPage(f_pages, olds)
                    loopForFetch(f_city, f_page, `${f_root}${city}/`, r_site)
                }).then(function (err) {
                    if (err)
                        cityDir(city)
                })
            }
        })
    }
    cityDir(f_city)
}
var startLoopFetch = function(){
    console.log('start loop')
    fetch_switch = true
    loopInterval()
    randomFetch(0)
    //randomFetch(1)
}
var findClientsFiles = function(w_dir){
    return file.readdirSync(w_dir)
}
var chkClientsFiles = function(s_file, c_files){
    for(var ci in c_files){
        if(c_files[ci] == s_file)
            return true
    }
    return null
}
var matchCityForClient = function(city){
    for(var ci in web_src.qgcitys){
        if(web_src.qgcitys[ci].cn == city.cn)
            return web_src.qgcitys[ci]
    }
    return null
}
var findSrcForClient = function(city){
    var bxed_citys = findClientsFiles(`${master_dir}downloads/bx`), tced_citys = findClientsFiles(`${master_dir}downloads/tc`)
    for(var ci in bxed_citys){
        if(bxed_citys[ci] == city.bxname){
            city.bx = true
            break
        }
    }
    for(var ci in tced_citys){
        if(tced_citys[ci] == city.tcname){
            city.tc = true
            break
        }
    }
}
module.exports = {
    webIndexesFetch: function(){
        fetch_start = true
        var set_interval = startInterval(new Date())
        setTimeout(function(){
            startLoopFetch()
            var aDayLoop = aDayInterval()
            interval = setInterval(function(){
                startLoopFetch()
            }, aDayLoop)
        }, set_interval)
    },
    digWebSrc: function(from_local, gate, cb){
        txgeo.findLocalAddr(from_local, function (c_res) {
            if(c_res != 'error') {
                var detail_addr = c_res.split('市')[0]
                if(detail_addr.match(/省/g))
                    detail_addr = detail_addr.split('省')[1]
                var dest = matchCityForClient(detail_addr)
                if(!dest.cn)
                    return cb(null)
                var m_city = {bxname:dest.bx, bx:false, tcname:dest.tc, tc:false}
                findSrcForClient(m_city)
                var dest_dir = `${clients_dir}downloads/`, fbx_page = '', ftc_page = '', fast_res = false
                if(m_city.bx){
                    fast_res = true
                    //here add fetch others
                    cb(fast_res)
                }
                if(m_city.tc){
                    //here add fetch others
                    if(!fast_res) {
                        fast_res = true
                        cb(fast_res)
                    }
                }
                if (!m_city.bx){
                    var bx_city = dest_dir+`bx/${dest.bx}/`
                    var city_existed = findClientsFiles(`${dest_dir}bx/`)
                    if(gate == 'tapGates') {
                        if(chkClientsFiles(dest.bx, city_existed))
                            f_page = selectPage(web_src.bxChildSite.bxtapGates, findClientsFiles(bx_city))
                        else
                            f_page = selectPage(web_src.bxChildSite.bxtapGates, [])
                    }
                    else if(gate == 'tapJobs'){
                        if(chkClientsFiles(dest.bx, city_existed))
                            f_page = selectPage(web_src.bxChildSite.bxtapJobs, findClientsFiles(bx_city))
                        else
                            f_page = selectPage(web_src.bxChildSite.bxtapJobs, [])
                    }
                    else if(gate == 'tapServes'){
                        if(chkClientsFiles(dest.bx, city_existed))
                            f_page = selectPage(web_src.bxChildSite.bxtapServes, findClientsFiles(bx_city))
                        else
                            f_page = selectPage(web_src.bxChildSite.bxtapServes, [])
                    }
                    else if(gate == 'tapRend'){
                        if(chkClientsFiles(dest.bx, city_existed))
                            f_page = selectPage(web_src.bxChildSite.bxtapRend, findClientsFiles(bx_city))
                        else
                            f_page = selectPage(web_src.bxChildSite.bxtapRend, [])
                    }
                    else{
                        if(chkClientsFiles(dest.bx, city_existed))
                            f_page = selectPage(web_src.bxChildSite.bxtapOlds, findClientsFiles(bx_city))
                        else
                            f_page = selectPage(web_src.bxChildSite.bxtapOlds, [])
                    }
                    dest_dir = bx_city
                }
                if(fbx_page) {
                    var plink = ''
                    for (var fp in f_page) {
                        if (fp != 'cn') {
                            plink = f_page[fp]
                            break
                        }
                    }
                    var page_link = webhead + `${dest.bx}.` + bxsite + plink
                    var link_obj = {city: dest.bx, page: page_link}
                    fetchWebSite(link_obj, dest_dir, function () {
                        console.log(`finished download ${plink} ${page_link}`)
                    })
                }
                if(!m_city.tc){
                    var tc_city = dest_dir+`tc/${dest.tc}/`
                    var city_existed = findClientsFiles(`${dest_dir}tc/`)
                    if(gate == 'tapGates') {
                        if(chkClientsFiles(dest.tc, city_existed))
                            f_page = selectPage(web_src.ChildSite58.tapGates58, findClientsFiles(tc_city))
                        else
                            f_page = selectPage(web_src.ChildSite58.tapGates58, [])
                    }
                    else if(gate == 'tapJobs'){
                        if(chkClientsFiles(dest.tc, city_existed))
                            f_page = selectPage(web_src.ChildSite58.tapJobs58, findClientsFiles(tc_city))
                        else
                            f_page = selectPage(web_src.ChildSite58.tapJobs58, [])
                    }
                    else if(gate == 'tapServes'){
                        if(chkClientsFiles(dest.tc, city_existed))
                            f_page = selectPage(web_src.ChildSite58.tapServes58, findClientsFiles(tc_city))
                        else
                            f_page = selectPage(web_src.ChildSite58.tapServes58, [])
                    }
                    else if(gate == 'tapRend'){
                        if(chkClientsFiles(dest.tc, city_existed))
                            f_page = selectPage(web_src.ChildSite58.tapRend58, findClientsFiles(tc_city))
                        else
                            f_page = selectPage(web_src.ChildSite58.tapRend58, [])
                    }
                    else{
                        if(chkClientsFiles(dest.tc, city_existed))
                            f_page = selectPage(web_src.ChildSite58.tapOlds58, findClientsFiles(tc_city))
                        else
                            f_page = selectPage(web_src.ChildSite58.tapOlds58, [])
                    }
                    dest_dir = tc_city
                }
                if(ftc_page) {
                    var plink = ''
                    for (var fp in f_page) {
                        if (fp != 'cn') {
                            plink = f_page[fp]
                            break
                        }
                    }
                    var page_link = webhead + `${dest.bx}.` + bxsite + plink
                    var link_obj = {city: dest.bx, page: page_link}
                    fetchWebSite(link_obj, dest_dir, function () {
                        console.log(`finished download ${plink} ${page_link}`)
                    })
                }
                if(!fast_res)
                    cb(null)
            }
            else {
                if(c_res) {
                    joiner.appOptErr('null', JSON.stringify(from_local), `${c_res}`, 'fetchweb.digWebSrc.txgeo.localToAddr', 'null', 'null', 'null')
                    cb(null)
                }
            }
        })
    },
    stopWebAction: function(){
        fetch_start = false
        fetch_switch = false
        clearInterval(interval)
    }
}

