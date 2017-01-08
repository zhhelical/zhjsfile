
const multer = require('multer')

const fs = require('fs')
const mysql = require('../../databases/mysqlpics.js')
//var storage = multer.memoryStorage()
var upload = multer({dest: '/data/release/helical/uploads'})

module.exports = function(app){
    app.post('/', upload.single('photo'), function (req, res) {
        var values = {
            img_time: req.body.time,
            img_key: req.body.user,
            img_size: req.body.width+'?&'+req.body.height+'?&'+req.file.buffer.length,
            img_where: req.body.which,
            img: req.file.buffer
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
    app.get('/', function(req, res){
        //console.log(req.headers)
        //var form = fs.readFileSync('./form.html', {encoding: 'utf8'})
        //res.send(form)
        mysql.select_pic(req.headers.get_time, req.headers.localkey, req.headers.which).then(function (result) {
            if(!result.length)
                res.({success: 'empty'})
            else
                res.sendFile(result[0].img_name)
        }).then(function (err) {
            if(err != undefined) {
                res.send({success: false})
                return
            }
        })
    })
}

