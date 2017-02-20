//mysqldata.js
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
    newComerTest:function(strs){
        return new Promise(function (resolve, reject) {
            mydb.getConnection(function (err, conn) {
                if (err)
                    reject(err, null, null)
                else {
                    conn.query("SELECT user FROM people WHERE user LIKE '%" + strs + "%'", function (err, results) {
                        conn.release()
                        if (err) reject(err)
                        if (results.length) resolve(results[0].user)
                        else resolve([])
                    })
                }
            })
        })
    },
    text_exec: function(strs) {
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    conn.query("SELECT value FROM people WHERE user LIKE '%"+strs+"%'", function(err, results, feilds) {
                        conn.release()
                        if (err) reject(err)
                        if(results.length) resolve(results[0].value)
                        else resolve([])
                    })
                }
            })
        })
    },
    insert_exec: function(strs1, strs2) {
        var rep_contents = strs2.replace(/\\n/g, "\\\\n")
            .replace(/\'/g, "\\'")
            .replace(/\`/g, "\\`")
            .replace(/\\r/g, "\\\\r")
        return new Promise(function (resolve, reject) {
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    conn.query("INSERT INTO people (user, value) VALUES(" + "'" + strs1 + "'" + ", " + "'" + rep_contents + "'" + ")", function (err, results) {
                        conn.release()
                        if (err) reject(err)
                        resolve(results)
                    })
                }
            })
        })
    },
    insert_pays: function(ins_obj) {
        return new Promise(function (resolve, reject) {
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    conn.query("INSERT INTO payment (openid, transaction_id, out_trade_no, time_end, total_fee, sign, nonce_str, bank_type) VALUES(" + "'" + ins_obj.openid + "'" + ", " + "'" + ins_obj.transaction_id + "'" + ", " + "'" + ins_obj.out_trade_no + "'" + ", " + "'" + ins_obj.time_end + "'" + ", " + "'" + ins_obj.total_fee + "'" + ", " + "'" + ins_obj.sign + "'" + ", " + "'" + ins_obj.nonce_str + "'" + ", " + "'" + ins_obj.bank_type + "'" + ")", function (err, results) {
                        conn.release()
                        if (err) reject(err)
                        resolve(results)
                    })
                }
            })
        })
    },
    update_exec: function(strs1, strs2) {
        var rep_contents = strs2.replace(/\\n/g, "\\\\n")
            .replace(/\'/g, "\\'")
            .replace(/\`/g, "\\`")
            .replace(/\\r/g, "\\\\r")
        return new Promise(function (resolve, reject) {
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    conn.query("UPDATE people SET value='" + rep_contents + "' WHERE user='" + strs1 + "'", function (err, results) {
                        conn.release()
                        if (err) reject(err)
                        resolve(results)
                    })
                }
            })
        })
    },
    valueByAddr:function(addr){
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    var city = addr.city, gate = addr.gate
                    conn.query("SELECT * FROM people WHERE value LIKE '%"+city+"%' AND value LIKE '%"+gate+"%'", function(err, results, feilds) {
                        conn.release()
                        if (err) reject(err)
                        if(results.length) resolve(results)
                        else resolve([])
                    })
                }
            })
        })
    },
    select_picsprops: function(time, key_value) {
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    if(key_value != '') {
                        conn.query("SELECT * FROM pictures WHERE img_time='" + time + "' AND img_key='" + key_value + "'", function (err, results, feilds) {
                            conn.release()
                            if (err) reject(err)
                            if (results.length) resolve(results)
                            else resolve([])
                        })
                    }
                    else{
                        conn.query("SELECT * FROM pictures WHERE img_time='" + time + "'", function (err, results, feilds) {
                            conn.release()
                            if (err) reject(err)
                            if (results.length) resolve(results)
                            else resolve([])
                        })
                    }
                }
            })
        })
    },
    deletePeopleRow:function(user){
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    var city = addr.city, gate = addr.gate
                    conn.query("DELETE FROM people WHERE user='"+user+"'", function(err, results, feilds) {
                        conn.release()
                        if (err) reject(err)
                        if(results.length) resolve(results)
                        else resolve([])
                    })
                }
            })
        })
    },
    deletePictures:function(time){
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    conn.query("DELETE FROM pictures WHERE img_time='"+time+"'", function(err, results, feilds) {
                        conn.release()
                        if (err) reject(err)
                        if(results.length) resolve(results)
                        else resolve([])
                    })
                }
            })
        })
    },
    msgsSave: function(openid, contents, t_send, new_old){
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    conn.query("SELECT * FROM messages WHERE openid='" + openid + "'", function (err, results) {
                        if (err) reject(err+'?mydb.msgsSave.conn.query_select')
                        if(results.length) {
                            var old_contents = eval('('+results[0].contents+')')
                            var arr_time = JSON.parse(results[0].time)
                            if(new_old) {
                                arr_time.push(t_send)
                                old_contents.push(contents)
                            }
                            else
                                old_contents = contents
                            var s_contents = JSON.stringify(old_contents)
                            var rep_contents = s_contents.replace(/\\n/g, "\\\\n")
                                .replace(/\'/g, "\\'")
                                .replace(/\`/g, "\\`")
                                .replace(/\\r/g, "\\\\r")
                            conn.query("UPDATE messages SET contents='" + rep_contents + "',time='" + JSON.stringify(arr_time) + "' WHERE openid='" + openid + "'", function (err) {
                                if (err) reject(err+'?mydb.msgsSave.conn.query_update')
                                resolve('success save msgs')
                                conn.release()
                            })
                        }
                        else {
                            var time = [t_send]
                            var s_contents = JSON.stringify([contents])
                            var ins_contents = s_contents.replace(/\\n/g, "\\\\n")
                                .replace(/\'/g, "\\'")
                                .replace(/\`/g, "\\`")
                                .replace(/\\r/g, "\\\\r")
                            var q_str = "INSERT INTO messages (openid, contents, time) VALUES(" + "'" + openid + "'" + ", " + "'" + ins_contents + "'" + ", " + "'" + JSON.stringify(time) + "')"
                            conn.query(q_str, function (err) {
                                if (err) reject(err+'?mydb.msgsSave.conn.query_insert')
                                resolve('success save msgs')
                                conn.release()
                            })
                        }
                    })
                }
            })
        })
    },
    msgsGetting: function(openid){
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    conn.query("SELECT * FROM messages WHERE openid='" + openid + "'", function (err, results) {
                        if (err) reject(err+'?mydb.msgsGetting.conn.query_select')
                        if(results.length)  resolve(results[0])
                        else resolve([])
                    })
                }
            })
        })
    },
    msgsUpdate: function(openid, updates, times){
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    conn.query("UPDATE messages SET contents='" + updates + "',time='" + times + "' WHERE openid='" + openid + "'", function (err) {
                        if (err) reject(err)
                        resolve('success save msgs')
                        conn.release()
                    })
                }
            })
        })
    },
    loggersSave: function(openid){
        var that = this
        mydb.getConnection(function(err,conn){
            if(err) {
                var err_obj = {
                    err_reason: err,
                    err_table: 'loggers',
                    openid: openid,
                    err_time: (new Date().getTime()),
                    old_content: 'null',
                    new_content: 'null',
                    opt_type: 'mydb.getConnection'
                }
                that.insertErrLogs(err_obj).then(function (res) {}).then(function (err) {throw err})
            }
            else{
                conn.query("SELECT * FROM loggers WHERE openid='" + openid + "'", function (err, results, feilds) {
                    if (err) {
                        var err_obj = {
                            err_reason: err,
                            err_table: 'loggers',
                            openid: openid,
                            err_time: (new Date().getTime()),
                            old_content: 'null',
                            new_content: 'null',
                            opt_type: 'mydb.getConnection.conn.query'
                        }
                        that.insertErrLogs(err_obj).then(function (res) {}).then(function (err) {throw err})
                    }
                    if (results.length) {
                        var arr_time = JSON.parse(results[0].time)
                        arr_time.push(new Date().getTime())
                        var times = results[0].times + 1
                        conn.query("UPDATE loggers SET times='" + times + "',time='" + JSON.stringify(arr_time) + "' WHERE openid='" + openid + "'", function (err) {
                            if (err) {
                                var err_obj = {
                                    err_reason: err,
                                    err_table: 'loggers',
                                    openid: openid,
                                    err_time: (new Date().getTime()),
                                    old_content: 'null',
                                    new_content: 'null',
                                    opt_type: 'mydb.getConnection.conn.query_update'
                                }
                                that.insertErrLogs(err_obj).then(function (res) {}).then(function (err) {throw err})
                            }
                            conn.release()
                        })
                    }
                    else {
                        var time = [new Date().getTime()]
                        var times = 1
                        conn.query("INSERT INTO loggers (openid, times, time) VALUES(" + "'" + openid + "'" + ", " + "'" + times + "'" + ", " + "'" + JSON.stringify(time) + "')", function (err) {
                            if (err) {
                                var err_obj = {
                                    err_reason: err,
                                    err_table: 'loggers',
                                    openid: openid,
                                    err_time: (new Date().getTime()),
                                    old_content: 'null',
                                    new_content: 'null',
                                    opt_type: 'mydb.getConnection.conn.query_insert'
                                }
                                that.insertErrLogs(err_obj).then(function (res) {}).then(function (err) {throw err})
                            }
                            conn.release()
                        })
                    }
                })
            }
        })
    },
    masterGetting: function(table) {
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    var s_str = `SELECT * FROM ${table}`
                    conn.query(s_str, function(err, results) {
                        conn.release()
                        if (err) reject(err)
                        if(results.length) resolve(results)
                        else resolve([])
                    })
                }
            })
        })
    },
    masterDelete: function(time, local, addr) {
        var that = this
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    var s_str = `SELECT * FROM people WHERE value LIKE '%${time}%' AND value LIKE '%${local}%' AND value LIKE '%${addr}%'`
                    conn.query(s_str, function(err, results) {
                        conn.release()
                        if (err) reject(err)
                        if(results.length) {
                            var d_vals = eval('('+results[0].value+')')
                            for(var dvi in d_vals){
                                if(d_vals[dvi].time==time && JSON.stringify(d_vals[dvi].location)==local && d_vals[dvi].address==addr){
                                    d_vals.splice(dvi, 1)
                                    break
                                }
                            }
                            var new_dvals = JSON.stringify(d_vals)
                            that.update_exec(results[0].user, new_dvals).then(function(){
                                var res_arr = []
                                res_arr.push(results[0].user)
                                res_arr.push(new_dvals)
                                resolve(res_arr)
                            }).then(function(err){
                                if(err)
                                    reject('err for masterDelete.that.update_exec')
                            })
                        }
                        else resolve([])
                    })
                }
            })
        })
    },
    insertErrLogs: function(err_obj){
        return new Promise(function (resolve, reject){
            mydb.getConnection(function(err,conn){
                if(err)
                    reject(err,null,null)
                else{
                    conn.query("INSERT INTO errlog (err_reason, err_table, openid, err_time, old_content, new_content, opt_type) VALUES(" + "'" + err_obj.err_reason + "'" + ", " + "'" + err_obj.err_table + "'" + ", " + "'" + err_obj.openid + "'" + ", " + "'" + err_obj.err_time + "'" + ", " + "'" + err_obj.old_content + "'" + ", " + "'" + err_obj.new_content + "'" + ", " + "'" + err_obj.opt_type + "'" + ")", function (err){
                        conn.release()
                        if (err) reject(err)
                    })
                }
            })
        })
    }
}