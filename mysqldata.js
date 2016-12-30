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
            //database: 'nodejs',
            port: 3306
        })
        database.connect(this.dealShutDown)
        database.on('error', this.dealShutDown)
    },
    createTbl: function(table_name){
        database.query(`CREATE TABLE ${table_name}`, function(err, rows, fields) {
            if (err) throw err;
            console.log('The solution is: ', rows[0].solution);
        })
        database.connect(function(err){
            if(err){
                console.log("链接失败");
                throw(err)
            }
            else{
                console.log("链接成功");
                database.query("CREATE TABLE people(id int,user varchar(255),value varchar(3500))", function(err,result){
                    if(err){throw err}
                    else{console.log("创建表成功")}
                })
                database.query("CREATE TABLE pictures(id int,user varchar(255),blob photo)", function(err,result){
                    if(err){throw err}
                    else{console.log("创建表成功")}
                })
            }
        })
    },
    selectUser: function(strs, cb) {
        database.query(`SELECT user FROM people WHERE user=${strs}`, function(err, rows, fields) {
            if (err) throw err;
            cb('The solution is: ', rows[0].solution);
        })
    },
    selectValue: function(strs, cb) {
        database.query(`SELECT value FROM people WHERE user=${strs}`, function(err, rows, fields) {
            if (err) throw err;
            cb('The solution is: ', rows[0].solution);
        })
    },
    insert: function(strs) {
        var row_info = '0, strs[1], strs[2]'
        if (strs[0] == 'image') {
        }
        else {
            database.query(`INSERT INTO people VALUES(${row_info})`, function (err, rows, fields) {
                if (err) throw err;
                console.log('The solution is: ', rows[0].solution);
            })
        }
    },
    dealShutDown: function(err) {// 如果是连接断开，自动重新连接,如果是数据库问题用系统shell重启mariadb
        if(err){
            if (err.code === 'PROTOCOL_CONNECTION_LOST') {
                database.connect()
            } else
                console.error(err.stack || err)
        }
    }
}