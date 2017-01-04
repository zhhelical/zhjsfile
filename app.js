//app.js
"use strict"

var app = require('express')();
var fs = require('fs');
var http = require('http');
var https = require('https');
var privateKey  = fs.readFileSync('./2_www.helicalzh.la.key', 'utf8'),
var certificate = fs.readFileSync('./1_www.helicalzh.la_bundle.crt', 'utf8');
var credentials = {key: privateKey, cert: certificate};

var httpServer = http.createServer(app);
var httpsServer = https.createServer(credentials, app);
var PORT = 9090
var SSLPORT = 443

httpServer.listen(PORT, function() {
    console.log('HTTP Server is running on: http://localhost:%s', PORT);
})
httpsServer.listen(SSLPORT, function() {
    console.log('HTTPS Server is running on: https://localhost:%s', SSLPORT);
})

// Welcome
app.get('/', function(req, res) {
    console.log(req)
    if(req.protocol === 'https') {
        res.status(200).send('Welcome to Safety Land!');
    }
    else {
        res.status(200).send('Welcome!');
    }
})
app.post('/upload', function(req, res) {
    console.log(req.body.name);
    console.log(req.body.email);
    res.send('Post Over');
})
app.listen(9090)