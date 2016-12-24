//datastore.js
"use strict"
const mysql = require('mysql')

class Mysql{
    constructor(host, user, passwd, db, port){
        mysql.createConnection({
            host: host,
            user: user,
            password: passwd,
            database: db,
            port: port
        })
    }
    select(strs) {

    }
    insert(strs) {

    }
}
exports.Mysql = Mysql