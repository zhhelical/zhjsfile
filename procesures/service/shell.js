//shell.js

var sh_order = 'head -n 80 /dev/urandom | tr -dc A-Za-z0-9 | head -c 168'
    , proc = require('child_process')
    , co = require('co')
    , my_exec = function(command) {
        return new Promise(function (resolve, reject){
            proc.exec(command, function(error, data){
                if (error) reject(error);
                resolve(data);
            })
        })
     }
module.exports = {
    shellFunc: co(function *() {
                    var res = yield my_exec(sh_order)
                    return res
                })
}

