//app.js
var app = require('express')();
//process.app = app;//方便在其他地方使用app获取配置

require('./config')(__dirname, app);//所有配置
var mode = app.get('mode');
controller(app);

var config = app.get(mode);
require('http').createServer(app).listen(config.port, function () {
    console.log('%srunning, listen:%s', config.name, config.port);
});
