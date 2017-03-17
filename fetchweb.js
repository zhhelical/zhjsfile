const fetch = require('node-fetch')
const file = require("fs")
const co = require('co')
const shell = require('./shell.js')
const joiner = require('./clients.js')
const txgeo = require('./txgeo.js')
const web_src = require('./websrc.js')
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
    }//promote here
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
var reCitys = function(){
    var recitys = []
    for(var ci in web_src.qgcitys) {
        if(!web_src.qgcitys[ci].bx)
            continue
        recitys.push(web_src.qgcitys[ci].bx)
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
var rePages = function(random){
    var si = 0
    for(var pi in web_src.bxChildSite) {
        if(si == random)
            return web_src.bxChildSite[pi]
        si++
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
var loopForFetch = function(f_city, f_page, rw_dir){
    var set_inter = interval_time(), plink = ''
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
            randomFetch()
        })
    }, set_inter)
}
var randomFetch = function(){
    if(!fetch_switch || !fetch_start)
        return
    var s_citys = reCitys(), o_citys = [], f_city = selectCity(s_citys, o_citys)
    var f_pages = rePages(randomfunc(5))
    var cityDir = function (city) {
        file.exists(`${master_dir}downloads/bx/${city}`, function (exists) {
            if (exists) {
                file.readdir(`${master_dir}downloads/bx/${city}`, function (f_err, files) {
                    if (f_err)
                        return
                    var f_page = selectPage(f_pages, files)
                    if (!f_page) {
                        o_citys.push(city)
                        var ff_city = selectCity(s_citys, o_citys)
                        cityDir(ff_city)
                    }
                    else
                        loopForFetch(f_city, f_page, `${master_dir}downloads/bx/${city}/`)
                })
            }
            else {
                var sh_order = `mkdir ${master_dir}downloads/bx/${city}`
                shell.shellFunc(sh_order).then(function (res) {
                    var olds = []
                    var f_page = selectPage(f_pages, olds)
                    loopForFetch(f_city, f_page, `${master_dir}downloads/bx/${city}/`)
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
    randomFetch()
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
    digWebSrc: function(from_local, gate){
        txgeo.findLocalAddr(from_local, function (c_res) {
            if(c_res != 'error') {
                var detail_addr = c_res.split('市')[0]
                if(detail_addr.match(/省/g))
                    detail_addr = detail_addr.split('省')[1]
                var dest = {}, w_site= 0
                //var not_null = () ? 'bx' : 'tc'//promote here
                for(var ni in web_src.qgcitys){
                    if(web_src.qgcitys[ni].cn==detail_addr /*&& web_src.qgcitys[ni][not_null]*/){
                        dest = web_src.qgcitys[ni]
                        break
                    }
                }
                if(!dest.cn)
                    return
                var dest_dir = `${clients_dir}downloads/`, f_page = ''
                if (w_site == 0){
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
                else{
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
                if(f_page) {
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
            }
            else {
                if(c_res) {
                    joiner.appOptErr('null', JSON.stringify(from_local), `${c_res}`, 'fetchweb.digWebSrc.txgeo.localToAddr', 'null', 'null', 'null')
                    reject(c_res)
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
