//mysqldata.js
"use strict"
const mysql = require('mysql')
var database

class Mysql{
    constructor(host, user, passwd, db, port){
        this.connectDb(host, user, passwd, db, port)
    }
    connectDb(host, user, passwd, db, port) {
        database = mysql.createConnection({
            host: host,
            user: user,
            password: passwd,
            database: db,
            port: port
        })
        database.connect(this.dealShutDown)
        database.on('error', this.dealShutDown)
    }
    select(strs) {

    }
    insert(strs) {

    }
    dealShutDown(err) {// 如果是连接断开，自动重新连接
        if(err){
            if (err.code === 'PROTOCOL_CONNECTION_LOST') {
                connect()
            } else
                console.error(err.stack || err)
        }
    }
}
module.exports = Mysql