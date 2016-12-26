//shell.js

var process = require('child_process');
//直接调用命令
exports.createRandom = function (sh){process.exec(sh,
    function (error, stdout, stderr) {
        if (error !== null){
            console.log('exec error: ' + error)
            return null
        }
        return stdout
    })
}