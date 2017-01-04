//app.js
var app = require('express')();
var fs = require('fs')();
//process.app = app;//方便在其他地方使用app获取配置

require('./config')(__dirname, app);//所有配置
var mode = app.get('mode');
var controller = require('controller/index');
controller(app);
var ssl_keys = {
    key: fs.readFileSync(process.cwd()+'/2_www.helicalzh.la.key'),
    cert: fs.readFileSync(process.cwd()+'/1_www.helicalzh.la_bundle.crt')
}
var config = app.get(mode);
require('https').createServer(ssl_keys, app).listen(config.port, function () {
    console.log('%srunning, listen:%s', config.name, config.port);
});

