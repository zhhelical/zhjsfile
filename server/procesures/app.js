//app.js
"use strict"

const p_server = require('service/service.js')
const p_config = require('config.js')
var server = new p_server.Server()
server.init(p_config.config.rootPath)
server.start()