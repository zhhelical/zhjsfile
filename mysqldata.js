//mysqldata.js
"use strict"
const mysql = require('mysql')
var database

module.exports = {
    private_sql: database,
    connectDb: function() {
        database = mysql.createConnection({
            host: 'localhost',
            user: 'root',
            password: '635401',
        })
        database.connect(this.dealShutDown)
        database.on('error', this.dealShutDown)
    },
    /*createTbl: function(){
        database.query("CREATE TABLE people(id int,user varchar(255),value varchar(3500)) IF NOT EXISTS people", function(err,result){
            if(err){throw err}
            else{console.log("创建表成功")}
        })
        database.query("CREATE TABLE pictures(id int,user varchar(255),blob photo) IF NOT EXISTS pictures", function(err,result){
            if(err){throw err}
            else{console.log("创建表成功")}
        })
    },*/
    selectUser: function(strs, cb) {
        database.query(`SELECT user FROM people WHERE user=${strs}`, function(err, results) {
            if (err) throw err
            cb(results)
        })
    },
    selectText: function(strs, cb) {
        database.query(`SELECT value FROM people WHERE user=${strs}`, function(err, results) {
            if (err) throw err
            cb(results)
        })
    },
    selectPicture: function(strs, cb) {
        database.query(`SELECT value FROM pictures WHERE user=${strs}`, function(err, results) {
            if (err) throw err
            cb(results)
        })
    },
    insertText: function(objs, cb) {
        var user_key = JSON.stringify(objs.key)
        var user_value = JSON.stringify(objs.value)
        database.query(`INSERT INTO people (user, value) VALUES(${user_key}, ${user_value})`, function (err, results) {
            if (err) cb(err)
        })
    },
    insertPicture: function(objs, cb){
/*        var user_key = JSON.stringify(objs.key)
        var user_value = JSON.stringify(objs.value)
        database.query(`INSERT INTO people (user, value) VALUES(${user_key}, ${user_value})`, function (err, results) {
            if (err) cb(err)
        })*/
    }
    dealShutDown: function(err) {// 如果是连接断开，自动重新连接,如果是数据库问题用系统shell重启mariadb
        if(err){
            if (err.code === 'PROTOCOL_CONNECTION_LOST') {
                database.connect()
            } else
                console.error(err.stack || err)
        }
    }
}