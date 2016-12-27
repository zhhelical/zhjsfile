//mysqldata.js
"use strict"
const mysql = require('mysql')
var database

module.exports = {
    connectDb: function() {
        database = mysql.createConnection({
            host: 'localhost,
            user: 'nodejs',
            password: 'nodejs',
            database: 'nodejs',
            port: 3306
        })
        database.connect(this.dealShutDown)
        database.on('error', this.dealShutDown)
    },
    createTbl: function(){
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
    select: function(strs) {
        database.query('SELECT 1 + 1 AS solution', function(err, rows, fields) {
            if (err) throw err;
            console.log('The solution is: ', rows[0].solution);
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
    dealShutDown: function(err) {// 如果是连接断开，自动重新连接
        if(err){
            if (err.code === 'PROTOCOL_CONNECTION_LOST') {
                database.connect()
            } else
                console.error(err.stack || err)
        }
    }
}