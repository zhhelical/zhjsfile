//mysqlpics.js
"use strict"
var mysql = require('mysql')
    , co = require('co')
    , mydb = mysql.createConnection({
            host: 'localhost',
            user: 'root',
            password: '635401',
            database: 'helicaldb'
        })
    , dbconn = false


module.exports = {
    private_sql: dbconn,
    connectDb: function() {
        mydb.connect(this.dealShutDown)
        mydb.on('error', this.dealShutDown)
    },
    insert_pic: function(obj_value) {
        return new Promise(function (resolve, reject) {
            mydb.query("INSERT INTO pictures set ?", obj_value, function (err, results) {
                if (err) reject(err)
                resolve(results)
            })
        })
    },
    select_pic: function(time, key_value, pos) {
        return new Promise(function (resolve, reject){
            if(key_value != '') {
                mydb.query("SELECT img_name FROM pictures WHERE img_time='" + time + "' AND img_key='" + key_value + "'" + " AND img_pos='" + pos + "'", function (results, err) {
                    if (err) reject(err)
                    resolve(results)
                })
            }
            else{
                mydb.query("SELECT img_name FROM pictures WHERE img_time='" + time + " AND img_pos='" + pos + "'", function (results, err) {
                    if (err) reject(err)
                    resolve(results)
                })
            }
        })
    },
    dealShutDown: function(err) {// 如果是连接断开，自动重新连接,如果是数据库问题用系统shell重启mariadb
        if(err){
            if (err.code === 'PROTOCOL_CONNECTION_LOST') {
                mydb.connect()
            } else
                console.error(err.stack || err)
        }
    }
}