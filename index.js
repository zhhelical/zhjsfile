
const multer = require('multer')

const fs = require('fs')
const mysql = require('../databases/mysqldata.js')
var storage = multer.memoryStorage()
//var upload = multer({ storage: storage })
var upload = multer({dest: '/data/release/helical/uploads'})

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

modules.exports = function(app){
    app.post('/', upload.array('test_img', 3), function (req, res) {
        if (res == undefined)
            res.json({success: false})
        else
            res.json({success: true})
    })
    app.get('/form', function(req, res, next){
        var form = fs.readFileSync('./form.html', {encoding: 'utf8'})
        res.send(form)
    })
}

