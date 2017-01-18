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
        mydb.getConnection(function(err,conn) {
            if(err)
                cb(err,null,null)
            else {
                return new Promise(function (resolve, reject) {
                    conn.query("INSERT INTO pictures set ?", obj_value, function (err, results) {
                        conn.release()
                        if (err) reject(err)
                        resolve(results)
                    })
                })
            }
        })
    },
    select_pic: function(time, key_value, pos) {
        mydb.getConnection(function(err,conn) {
            if(err)
                cb(err,null,null)
            else {
                return new Promise(function (resolve, reject) {
                    if (key_value != '') {
                        mydb.query("SELECT img_name FROM pictures WHERE img_time='" + time + "' AND img_key='" + key_value + "'" + " AND img_pos='" + pos + "'", function (results, err) {
                            if (err) reject(err)
                            resolve(results)
                        })
                    }
                    else {
                        mydb.query("SELECT img_name FROM pictures WHERE img_time='" + time + " AND img_pos='" + pos + "'", function (results, err) {
                            if (err) reject(err)
                            resolve(results)
                        })
                    }
                })
            }
        }
    }
}