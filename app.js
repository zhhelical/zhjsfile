//app.js
var app = require('express')()
var fs = require('fs')
var compress = require('compression')
app.use(compress())
var controller = require('./controller/index')
controller(app)
var ssl_keys = {
    key: fs.readFileSync(process.cwd()+'/2_www.helicalzh.la.key'),
    cert: fs.readFileSync(process.cwd()+'/1_www.helicalzh.la_bundle.crt')
}

require('https').createServer(ssl_keys, app).listen(3000, function () {
    console.log('%srunning, listen:%s', 'nodesrv', 3000)
})

