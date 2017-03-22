
const multer = require('multer')
const Bagpipe = require('bagpipe')
const path = require('path')
const imginfo = require('imageinfo')
const zlib = require('zlib')
const fs = require('fs')
const mysql = require('../../databases/mysqlpics.js')
const notify = require('../respondnotify.js')
var upload = multer({dest: '/data/release/helical/uploads'})
var bagpipe = new Bagpipe(10)

var send_gzip = function(f_path, res, cb){
    var gzipstream = zlib.createGzip()
    var file = fs.createReadStream(path.resolve(f_path))
    var img_info = imginfo(fs.readFileSync(path.resolve(f_path)))
    res.writeHead(200, {"content-encoding": "gzip", 'Content-Type':img_info.mimeType+'/'+img_info.format})
    file.pipe(gzipstream).pipe(res)
    cb('success')
}
var feedback = function(f_name, respond){
    var file = fs.readFileSync(path.resolve(f_name), 'binary')
    var img_info = imginfo(fs.readFileSync(path.resolve(f_name)))
    respond.writeHead(200, {'Content-Type':img_info.mimeType+'/'+img_info.format})
    respond.write(file, 'binary')
    respond.end()
}
var zipFeedback = function(f_name, respond){
    bagpipe.push(send_gzip, f_name, respond, function (data) {
        console.log('zipload')
    })
}

module.exports = function(app){
    app.post('/uploads/', upload.single('photo'), function (req, res) {
        var values = {
            img_time: req.body.time,
            img_key: req.body.user,
            img_size: req.body.width+'?&'+req.body.height+'?&'+req.file.size,
            img_pos: req.body.which,
            img_local: req.body.local,
            img_name: req.file.filename
        }
        console.log(req.body)
        mysql.insert_pic(values).then(function (result) {
            res.json({success: true})
        }).then(function (err) {
            if(err != undefined) {
                res.json({success: false})
                return
            }
        })
    })
    app.post('/wxnotify', function (req, res) {
        notify.dealXmlBody(req.body, function(d_res){
            res.send(d_res)
        })
    })
    app.get('/uploads/*', function(req, res){
        var picname = req.headers.filename
        if (!picname.match('/'))
            picname = '/data/release/helical/uploads/' + picname
        var accept_encoding = req.headers['accept-encoding']
        if (accept_encoding && accept_encoding.indexOf('gzip') != -1)
            zipFeedback(picname, res)
        else
            feedback(picname, res)
        return
    })
}

