//downloadimgs.js
const file = require("fs")
const fetch = require('node-fetch')
const co = require('co')
const http = require("http-request")
const imginfo = require('imageinfo')
const shell = require('./shell.js')
const web_src = require('./websrc.js')
var master_dir = '../../../../master/'
    , clients_dir = '../../../clients/'
    , bxDir = `${master_dir}downloads/bx/`
    , tc58Dir = `${master_dir}downloads/tc/`
    , imgspos = `${master_dir}downloads/imgs/`
    , randomfunc = function(required){
        var rnum = Math.random()
        return Math.floor(rnum * required)
    }
    , interval_time = function(){
        return 180000+randomfunc(3)*60000
    }
    , cleanCommHtml = function(cons){
        var revar = new RegExp("<div class='viewad-main'>(.|\r|\n)*<\/div>", "g")
        var container = cons.match(revar)[0]
        return container
    }
    , reanynisType = function(name, w_site){
        var en_site = w_site.split('/'), dest = en_site[en_site.length-2], childSite = (dest=='bx' ? web_src.bxChildSite : web_src.ChildSite58)
        for(var bxi in childSite){
            var cbx_objs = childSite[bxi]
            for(var bii in cbx_objs){
                var obj_atrrs = cbx_objs[bii]
                for(var boi in obj_atrrs) {
                    if(obj_atrrs[boi] == name){
                        var real_obj = {type:bxi}
                        return real_obj
                    }
                }
            }
        }
    }
    , rd_evdir = function(destination, dirs){
        var dest = `${destination}${dirs[0]}`
        return file.readdirSync(dest)
    }
    , mv_imgedfunc = function (rd_file, path, cb) {
        var f_path = `${path}/${rd_file}`, city_dir = path.split('/'), ca_dir = city_dir[city_dir.length - 1]
        file.exists(`${imgspos}${ca_dir}`, function (exists) {
            if (exists)
                mv_imgedAct(f_path, `${imgspos}${ca_dir}/`).then(function () { cb()}).then(function (err) {
                    if(err)
                        mv_imgedAct(f_path, `${imgspos}${ca_dir}/`).then(function(){cb()}).then(function(err){
                            if(err){
                                console.log('unluckey! need mv byhand')
                                cb()
                            }
                        })
                })
            else {
                var mk_order = `mkdir ${imgspos}${ca_dir}`
                shell.shellFunc(mk_order).then(function () {
                    mv_imgedAct(f_path, `${imgspos}${ca_dir}/`).then(function () {cb()}).then(function (err) {
                        if(err)
                            mv_imgedAct(f_path, `${imgspos}${ca_dir}/`).then(function(){cb()}).then(function(err){
                                if(err){
                                    console.log('unluckey! need mv byhand')
                                    cb()
                                }
                            })
                    })
                }).then(function (err) {
                    if (err)
                        mv_imgedAct(f_path, `${imgspos}${ca_dir}/`).then(function () {cb()}).then(function (err) {
                            if(err)
                                mv_imgedAct(f_path, `${imgspos}${ca_dir}/`).then(function(){cb()}).then(function(err){
                                    if(err){
                                        console.log('unluckey! need mv byhand')
                                        cb()
                                    }
                                })
                        })
                })
            }
        })
    }
    , mv_imgedAct = function (rd_file, path) {
        var sh_order = `mv ${rd_file} ${path}/`
        return new Promise(function (resolve, reject){
            shell.shellFunc(sh_order).then(function () {
                resolve()
            }).then(function (err) {
                if (err)
                    reject(err)
            })
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
    , recurFecth = function(type, p_links, d_imgs, cb){
        if(!p_links.links.length)
            cb(d_imgs)
        else {
            chkPicsOnLink(p_links.links[0], type, function(pics){
                if(!pics){
                    p_links.links.splice(0, 1)
                    recurFecth(type, p_links, d_imgs, cb)
                }
                else {
                    var set_inter = interval_time()
                    setTimeout(function () {
                        fetchWebSite(p_links.links[0]).then(function (f_res) {
                            if (type.type != 'bxtapOlds')
                                downloadImgs(p_links.links[0], p_links.wtimes[0], f_res.toString('utf8'), function (i_res) {
                                    d_imgs.push(i_res)
                                    p_links.links.splice(0, 1)
                                    recurFecth(type, p_links, d_imgs, cb)
                                })
                            else
                                downloadOldsImgs(p_links.links[0], p_links.wtimes[0], f_res.toString('utf8'), function (i_res) {
                                    d_imgs.push(i_res)
                                    p_links.links.splice(0, 1)
                                    recurFecth(type, p_links, d_imgs, cb)
                                })
                        })
                    }, set_inter)
                }
            })
        }
    }
    , getwebImg = function(urls, time, down_arr, cb){
        if(!urls.length)
            cb('img download success')
        else {
            var sh_order = 'head -n 80 /dev/urandom | tr -dc A-Za-z0-9 | head -c 32'
            shell.shellFunc(sh_order).then(function (result) {
                var options = {url: urls[0], encoding: 'binary'}
                http.get(options, `${imgspos}imgslist/${result}`, function (err, res) {
                    if (err) {
                        console.log(err)
                        getwebImg(urls, time, down_arr, cb)
                    }
                    else {
                        var f_info = file.readFileSync(`${imgspos}imgslist/${result}`)
                        var img_info = imginfo(f_info)
                        var size = file.statSync(`${imgspos}imgslist/${result}`).size
                        var values = {
                            img_time: time,
                            img_key: '',
                            img_size: img_info.width + '?&' + img_info.height + '?&' + size,
                            img_pos: urls.length - 1,
                            img_local: '',
                            img_name: result
                        }
                        down_arr.push(values)
                        urls.splice(0, 1)
                        getwebImg(urls, time, down_arr, cb)
                    }
                })
            }).then(function (err) {
                if (err) {
                    urls.splice(0, 1)
                    getwebImg(urls, time, down_arr, cb)
                }
            })
        }
    }
    , downloadImgs = function(link, time, info, cb){
        info = cleanCommHtml(info)
        var imgs_arr = []
        revar = new RegExp("<div\\s+class='photo-item(.*?)<\/div>", "g")
        var images = info.match(revar)
        for (var imi in images) {
            var image = images[imi].match(/http:[^\s]*\)/g)
            if(image)
                imgs_arr.push(image[0].replace(/\)/g, ''))
            if (imgs_arr.length == 3)
                break
        }
        var d_objs = {http:link, imgs:[]}
        getwebImg(imgs_arr, time, d_objs.imgs, function(){cb(d_objs)})
    }
    , downloadOldsImgs = function(link, time, info, cb){
        info = cleanCommHtml(info)
        var imgs_arr = []
        revar = new RegExp("<div\\s+class='big-img-box'>(.|\r|\n)*?<ul", "g")
        var images = info.match(revar)
        if(!images) {
            revar = new RegExp("服务简介.*img(.*?)<\/div>", "g")
            var t_images = info.match(revar)
            if(t_images)
                images = t_images[0].match(/img src='http:[^\s]*img[^\s]*'/g)
        }
        else
            images = images[0].match(/http:[^\s]*img[^\s]*'/g)
        for (var imi in images) {
            imgs_arr.push(images[imi].replace(/'|img src=/g, ''))
            if (imgs_arr.length == 3)
                break
        }
        var d_objs = {http:link, imgs:[]}
        getwebImg(imgs_arr, time, d_objs.imgs, function(){cb(d_objs)})
    }
    , chkPicsOnLink = function(link, o_fs, cb){
        var objs_pos = ''
        if(o_fs.type.match('tapGates'))
            objs_pos = `${master_dir}tapGatesObjs/gatesobjs.txt`
        else if(o_fs.type.match('tapJobs'))
            objs_pos = `${master_dir}tapJobsObjs/jobsobjs.txt`
        else if(o_fs.type.match('tapServes'))
            objs_pos = `${master_dir}tapServesObjs/servesobjs.txt`
        else if(o_fs.type.match('tapRend'))
            objs_pos = `${master_dir}tapRendObjs/rendsobjs.txt`
        else
            objs_pos = `${master_dir}tapOldsObjs/oldsobjs.txt`
        file.readFile(objs_pos, function(err, res){
            if(err)
                return chkPicsOnLink(link, o_fs, cb)
            var f_obj = JSON.parse(res), pics = 0
            for(var fi in f_obj){
                if(f_obj[fi].link == link){
                    pics = f_obj[fi].pictures
                    break
                }
            }
            cb(pics)
        })
    }
    , readDirFiles = function(destination){
        file.readdir(destination, function(err, rd_dirs){
            if(err){
                console.log(err)
                return
            }
            var rd_files = function(rddirs, rdfs){
                if(!rddirs.length)
                    return
                if(!rdfs.length) {
                    rddirs.splice(0, 1)
                    return rd_files(rddirs, rdfs)
                }
                var pr_name = rdfs[0].split('links')[0]+'/'
                var re_obj = reanynisType(pr_name, destination), arr_tolimgs = []
                var f_path = `${destination}${rddirs[0]}/${rdfs[0]}`
                file.readFile(f_path, function(err, res){
                    var f_obj = JSON.parse(res)
                    recurFecth(re_obj, f_obj, arr_tolimgs, function(r_res){
                        mv_imgedfunc(f_path, `${imgspos}imgedlinks/`, function(){
                            var old_total = []
                            var of_old = file.readFileSync(`${imgspos}imgobjs.txt`)
                            if(of_old)
                                old_total = JSON.parse(of_old)
                            for(var ari in r_res)
                                old_total.push(r_res[ari])
                            file.writeFile(`${imgspos}imgobjs.txt`, JSON.stringify(old_total), function () {
                                rdfs.splice(0, 1)
                                rd_files(rdfs)
                            })
                        })
                    })
                })
            }
            var firsts = rd_evdir(destination, rd_dirs)
            rd_files(rd_dirs, firsts)
        })
    }
    , startLoopFetch = function(){
        console.log('start loop')
        fetch_switch = true
        loopInterval()
        readDirFiles(bxDir)
    }
module.exports = {
    digWebImgs: function(dlink, dtime, dinfo, dtype, cb){
        console.log(dtype)
        if (!dtype.match('tapOlds'))
            downloadImgs(dlink, dtime, dinfo, function (i_res) {
                console.log(i_res)
                cb(i_res)
            })
        else
            downloadOldsImgs(dlink, dtime, dinfo, function (i_res) {
                cb(i_res)
            })
    },
    genWebImgs: function(){
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
    stopWebAction: function(){
        fetch_start = false
        fetch_switch = false
        clearInterval(interval)
    }
}

//readDirFiles(bxDir)