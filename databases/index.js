
const multer = require('multer')

const fs = require('fs')
const mysql = require('../../databases/mysqlpics.js')
//var storage = multer.memoryStorage()
var upload = multer({dest: '/data/release/helical/uploads'})

module.exports = function(app){
    app.post('/', upload.single('photo'), function (req, res) {
        console.log(res)
        /*var values = {
            img_time: req.body.time,
            img_key: req.body.user,
            img_size: req.file.buffer.length,
            img: req.file.buffer
        }
        mysql.insert_pic(values).then(function (result) {
            res.json({success: true})
        }).then(function (err) {
            if(err != undefined) {
                res.json({success: false})
                return
            }
        })*/
    })
    app.get('/', function(req, res){
        //console.log(req.headers)
        //var form = fs.readFileSync('./form.html', {encoding: 'utf8'})
        //res.send(form)
        var s_key = req.headers.get_time + '%' + req.headers.localkey
        mysql.select_pic(s_key).then(function (result) {
            var bufferBase64 = new Buffer( blob, 'binary' ).toString('base64')
            res.send(bufferBase64)
        }).then(function (err) {
            if(err != undefined) {
                res.send({success: false})
                return
            }
        })
    })
}

