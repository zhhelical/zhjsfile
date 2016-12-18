//service.js
"use strict"

const path = require('path')
const http = require('http')
const SocketIO = require('socket.io')
const p_client = require('client.js')
const key_path = path.dirname(require.main.filename)
const UTF8 = {encoding: 'utf-8'}

class server{
    constructor(){
        this.port = null
        this.http = null
        this.io = null
    }
    init(path){
        this.port = process.env.PORT
        this.http = http.createServer()
        this.io = SocketIO(this.http, {path})
        this.io.on("connection", socket => {
            const client = new p_client.Client(socket)
            client.connect()
        })
    }
    start(){
        this.http.listen(this.port)
        console.log('------ server started, listen ${this.port} -------')
    }
}
export.Server = Server