
const multer = require('multer')

const fs = require('fs')
const mysql = require('../../databases/mysqlpics.js')
var storage = multer.memoryStorage()
var upload = multer({ storage: storage })
//var upload = multer({dest: '/data/release/helical/uploads'})

/*storage.prototype._handleFile = function _handleFile (req, file, cb) {
    this.getDestination(req, file, function (err, path) {
        if (err) return cb(err)*/
/*        var outStream = fs.createWriteStream(path)
        file.stream.pipe(outStream)
        outStream.on('error', cb)
        outStream.on('finish', function () {
            cb(null, {
                path: path,
                size: outStream.bytesWritten
            })
        })*/
 /*       fs.open(path, 'r', function (status, fd) {
            if (status) {
                console.log(status.message)
                return
            }
            var fileSize = getFilesizeInBytes(path)
            var buffer = new Buffer(fileSize)
            fs.read(fd, buffer, 0, fileSize, 0, function (err, num) {
                var query = "INSERT INTO pictures SET ?",
                    values = {
                        file_type: 'img',
                        file_size: buffer.length,
                        file: buffer
                    }
                mysql.query(query, values, function (er, da) {
                    if(er)throw er;
                })
            })
        })
    })
}*/
/*storage.prototype._removeFile = function _removeFile (req, file, cb) {
    fs.unlink(file.path, cb)
}*/

module.exports = function(app){
    app.post('/', upload.array('photos', 3), function (req, res) {
        fs.open(req.path, 'r', function (status, fd) {
            if (status) {
                console.log(status.message)
                return
            }
            for(var i in req.files) {
                fs.read(fd, req.files[i].buffer, 0, req.files[i].size, 0, function (err, num) {
                    var values = {
                        img_desc: req.files[i].fieldname,
                        img_size: req.files[0].buffer.length,
                        img: req.files[0].buffer
                    }
                    mysql.insert_pic(values).then(function (res) {
                        console.log(storage, 'mem', i)
                        if(i == 2) {
                            storage = null
                            console.log(storage, 'clear mem', i)
                            res.json({success: true})
                        }
                    }).then(function (err) {
                        if(err != undefined) {
                            res.json({success: false})
                            return
                        }
                    })
                })
            }
        })
    })
    app.get('/', function(req, res, next){
        var form = fs.readFileSync('./form.html', {encoding: 'utf8'})
        res.send(form)
    })
}

