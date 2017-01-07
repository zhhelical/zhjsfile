
const multer = require('multer')

const fs = require('fs')
const mysql = require('../../databases/mysqlpics.js')
var storage = multer.memoryStorage()
var upload = multer({ storage: storage })

module.exports = function(app){
    app.post('/', upload.single('photo'), function (req, res) {
        fs.open(req.path, 'r', function (status, fd) {
            if (status) {
                console.log(status.message)
                return
            }
            fs.read(fd, req.file.buffer, 0, req.file.size, 0, function (err, num) {
                var values = {
                    img_desc: req.body.user,
                    img_size: req.file.buffer.length,
                    img: req.file.buffer
                }
                mysql.insert_pic(values).then(function (result) {
                    console.log(storage, 'mem')
                    storage = null
                    console.log(storage, 'clear mem')
                    res.json({success: true})
                }).then(function (err) {
                    if(err != undefined) {
                        res.json({success: false})
                        return
                    }
                })
            })
        })
    })
    app.get('/', function(req, res){
        //console.log(req.headers)
        //var form = fs.readFileSync('./form.html', {encoding: 'utf8'})submit_time + '?1' + '&' + key_str
        //res.send(form)
        var s_key = req.headers.time + '%' + req.headers.user
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

