//shell.js

var exec = require('child_process').exec
function my_exec(command, callback) {
    var proc = exec(command)
    var list = []
    proc.stdout.setEncoding('utf8');
    proc.stdout.on('data', function (chunk) {
        list.push(chunk)
    })
    proc.stdout.on('end', function () {
        callback(list.join());
    })
}
module.exports = {
    shellFunc: function(sh){
        var res = ''
        my_exec(sh, function (stdout) {
            res = stdout
        })
        return res
    }
}


