
var buf = require('buffer');
var fs = require('fs');
var path = require('path');
var util = require('util');
var gm = require('gm').subClass({ imageMagick: true });
var uuid = require('uuid');

function createDir(pathArgs) {
    if (pathArgs[0]) {
        var dir = path.join(app.get('targetDir'), path.join.apply(path, pathArgs));
        var exists = fs.existsSync(dir);
        if (!exists)
            fs.mkdirSync(dir);
    }
}

function createPath(pathArgs, ext) {
    var args = [app.get('targetDir')];
    pathArgs[pathArgs.length - 1] = uuid.v1().replace(/-/g, '');
    args.push.apply(args, pathArgs);
    var filePath = path.join.apply(path, args) + '.' + ext;
    return fs.existsSync(filePath) ? createPath(pathArgs, ext) : filePath;
}

modules.exports = function(app){
    var contentTypes = app.get('contentType');
    app.post('/', function (req, res) {
        var ext = req.body.ext;
        var buffer = req.body.buffer;
        if (!(ext && buffer && contentTypes[ext]))
            return res.json({success: false});

        var pathArgs = req.body.folder.replace(/\n/g, '');
        if (pathArgs)
            pathArgs = pathArgs.substr(1).split('/');
        else
            pathArgs = [''];

        createDir(pathArgs);

        pathArgs.push('');
        var filePath = createPath(pathArgs, ext);
        gm(new buf.Buffer(JSON.parse(buffer))).write(filePath, function (err) {
            if (err)
                return res.json({success: false});

            res.json({success: true, data: util.format('\\%s.%s', pathArgs.join('\\'), ext)});
        });
    });
    app.get('/:filename', function (req, res) {
        var filePath = path.join(targetDir, req.params.filename);
        fs.exists(filePath, function (exists) {
            res.sendfile(exists ? filePath : path.join(targetDir, config.default));
        });
    });
}

