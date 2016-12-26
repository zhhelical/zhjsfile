//shell.js

var sh_order = ''
    , exec = require('child_process').exec
    , co = require('co')
    , thunkify = require('thunkify')
    , my_exec = thunkify(function(command, callback) {
        var proc = exec(command)
        var list = []
        proc.stdout.setEncoding('utf8');
        proc.stdout.on('data', function (chunk) {
            list.push(chunk)
        })
        proc.stdout.on('end', function () {
            callback(list.join());
        })
     })
module.exports = {
    command: sh_order,
    shellFunc: co.wrap(function *() {
            var res = yield my_exec(sh_order)
            return res
        })()
}
/*var fs = require('fs')
    , co = require('co')
    , thunkify = require('thunkify');
var readFile = thunkify(fs.readFile);
co.wrap(function *() {
    var test1 = yield readFile('test1.txt');
    var test2 = yield readFile('test2.txt');
    var test = test1.toString() + test2.toString();
    console.log(test);
})();*/
