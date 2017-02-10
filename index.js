
const multer = require('multer')
const path = require('path')
const imginfo = require('imageinfo')
const zlib = require('zlib')
const fs = require('fs')
const mysql = require('../../databases/mysqlpics.js')
const notify = require('../respondnotify.js')
const body_parser = require('body-parser')
var upload = multer({dest: '/data/release/helical/uploads'})

var feedback = function(f_name, respond){
    var file = fs.readFileSync(path.resolve(f_name), 'binary')
    var img_info = imginfo(fs.readFileSync(path.resolve(f_name)))
    respond.writeHead(200, {'Content-Type':img_info.mimeType+'/'+img_info.format})
    respond.write(file, 'binary')
    respond.end()
}
var zipFeedback = function(f_name, respond){
    var gzipstream = zlib.createGzip()
    var file = fs.createReadStream(path.resolve(f_name))
    var img_info = imginfo(fs.readFileSync(path.resolve(f_name)))
    respond.writeHead(200, {"content-encoding": "gzip", 'Content-Type':img_info.mimeType+'/'+img_info.format})
    file.pipe(gzipstream).pipe(respond)
}

module.exports = function(app){
    app.post('/', upload.single('photo'), function (req, res) {
        var values = {
            img_time: req.body.time,
            img_key: req.body.user,
            img_size: req.body.width+'?&'+req.body.height+'?&'+req.file.size,
            img_pos: req.body.which,
            img_name: req.file.filename
        }
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
        this.use(body_parser.urlencoded({extended:true}))
        this.use(bodyParser.text({type: 'text/xml'}))
        notify.dealXmlBody(req.body, function(d_res){
            res.send(d_res)
        })
    })
    app.get('/uploads/*', function(req, res){
        this.use(function(req,res,next){
            var _send = res.send
            var sent = false
            res.send = function(data){
                if(sent) return
                _send.bind(res)(data)
                sent = true
            }
            next()
        })
        var accept_encoding = req.headers['accept-encoding']
        if (accept_encoding && accept_encoding.indexOf('gzip') != -1)
            zipFeedback(req.headers.filename, res)
        else
            feedback(req.headers.filename, res)
        return
    })
}

