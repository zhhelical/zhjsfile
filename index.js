
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
            img_size: req.body.width+'?&'+req.body.height+'?&'+req.file.size,
            img_pos: req.body.which,
            img_where: req.body.localpath,
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
    app.get('/', function(req, res){
        var whiches = req.headers.which.split('?')
        if(req.headers.hasOwnProperty('localkey')) {
            mysql.one_pic(whiches[0], req.headers.localkey, whiches[1]).then(function (result) {
                if (!result.length)
                    res.status('304').send(result)
                else
                    res.sendFile(result[0].img_name)
            }).then(function (err) {
                if (err != undefined) {
                    res.send({success: false})
                    return
                }
            })
        }
        else{
            mysql.multi_pic(whiches[0], whiches[1]).then(function (result) {
                if (!result.length)
                    res.status('304').send(result)
                else
                    res.sendFile(result[0].img_name)
            }).then(function (err) {
                if (err != undefined) {
                    res.send({success: false})
                    return
                }
            })
        }
    })
}

