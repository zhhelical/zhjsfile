//mysqldata.js
"use strict"
var mysql = require('mysql')
    , co = require('co')
    , database
    , queries = []
    , user_exec = function(strs) {
        return new Promise(function (resolve, reject){
            database.query("SELECT user FROM people WHERE user LIKE %"+strs[0]+"%", function(err, results) {
                if (err) reject(err)
                resolve(results)
            })
        })
    }
    , text_exec = function(strs) {
        return new Promise(function (resolve, reject){
            database.query("SELECT value FROM people WHERE user LIKE %"+strs[0]+"%", function(err, results) {
                if (err) reject(err)
                resolve(results)
            })
        })
    }
    , picture_exec = function(strs) {
        return new Promise(function (resolve, reject){
            database.query("SELECT photo FROM pictures WHERE user LIKE %"+strs[0]+"%", function(err, results) {
                if (err) reject(err)
                resolve(results)
            })
        })
    }
    , insert_exec = function(strs) {
        return new Promise(function (resolve, reject) {
            database.query("INSERT INTO people (user, value) VALUES(" + "'" + strs[0] + "'" + ", " + "'" + strs[1] + "'" + ")", function (err, results) {
                if (err) reject(err)
                resolve(results)
            })
        })
    }

module.exports = {
    private_sql: database,
    query_order: queries,
    connectDb: function() {
        database = mysql.createConnection({
            host: 'localhost',
            user: 'root',
            password: '635401',
        })
        database.connect(this.dealShutDown)
        database.on('error', this.dealShutDown)
    },
    selectUser: co(function *() {
        var res = yield user_exec(queries)
        return res
    }),
    selectText: co(function *() {
        var res = yield text_exec(queries)
        return res
    }),
    selectPicture: co(function *() {
        var res = yield picture_exec(queries)
        return res
    }),
    insertText: co(function *() {
        var res = yield insert_exec(queries)
        return res
    }),
    insertPicture: co(function *(){
/*        var user_key = JSON.stringify(objs.key)
        var user_value = JSON.stringify(objs.value)
        database.query(`INSERT INTO people (user, value) VALUES(${user_key}, ${user_value})`, function (err, results) {
            if (err) cb(err)
        })*/
    }),
    dealShutDown: function(err) {// 如果是连接断开，自动重新连接,如果是数据库问题用系统shell重启mariadb
        if(err){
            if (err.code === 'PROTOCOL_CONNECTION_LOST') {
                database.connect()
            } else
                console.error(err.stack || err)
        }
    }
}