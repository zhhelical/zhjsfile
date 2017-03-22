//mysqlpics.js
"use strict"
var mysql = require('mysql')
    , co = require('co')
    , mydb = mysql.createPool({
            host: 'localhost',
            user: 'root',
            password: '635401',
            database: 'helicaldb'
        })

module.exports = {
    insert_pic: function(obj_value) {
        return new Promise(function (resolve, reject) {
            mydb.getConnection(function (err, conn) {
                if (err)
                    reject(err, null, null)
                else {
                    conn.query("INSERT INTO pictures set ?", obj_value, function (err, results) {
                        conn.release()
                        if (err) reject(err)
                        resolve(results)
                    })
                }
            })
        })
    },
    one_pic: function(time, key_value, pos) {
        return new Promise(function (resolve, reject) {
            mydb.getConnection(function (err, conn) {
                if (err)
                    reject(err, null, null)
                else {
                    conn.query("SELECT img_name FROM pictures WHERE img_time='" + time + "' AND img_key='" + key_value + "'" + " AND img_pos='" + pos + "'", function (err, results) {
                        conn.release()
                        if (err) reject(err)
                        resolve(results)
                    })
                }
            })
        })
    },
    multi_pic: function(time, pos) {
        return new Promise(function (resolve, reject) {
            mydb.getConnection(function (err, conn) {
                if (err)
                    reject(err, null, null)
                else {
                    conn.query("SELECT img_name FROM pictures WHERE img_time='" + time + " AND img_pos='" + pos + "'", function (err, results) {
                        conn.release()
                        if (err) reject(err)
                        resolve(results)
                    })
                }
            })
        })
    }
}