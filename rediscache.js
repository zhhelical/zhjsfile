//rediscache.js
"use strict"
const redis = require('redis')
    , mysql = require('./mysqldata.js')
    , co = require('co')
var clientDb

module.exports = {
    redis_srv: clientDb,
    construct: function() {//将来加入出错信息反馈
        var chk_fault
        clientDb = redis.createClient()
        clientDb.on('error', function (err) {
            return chk_fault = err
        })
    },
    storeCache: function(key, value) {
        return new Promise(function (resolve, reject){
            clientDb.set(key, value, function (err, reply) {
                if (err) reject(err)
                resolve(reply)
            })
        })
    },
    getCacheKey:function(key, cb){
        clientDb.keys(key, function (err, replies) {
            if(err) cb('ERRGETKEY')
            else
                cb(replies[0])
        })
    },
    getCacheValue:function(key, cb){
        clientDb.get(key, function (err, replies) {
            if(err) cb('ERRGETVALUE')
            else
                cb(replies[0])
        })
    },
    deleteData:function(key, cb){
        this.getCacheKey(key, function(res){
            if(res!='ERRGETKEY'){
                clientDb.del(key, function (err, reply) {
                    if(err) cb('ERRDELETE')
                    else cb(reply)
                })
            }
            else
                cb('ERRGETKEY')
        })
    },
    cacheSqlComp: function(caches, sqls){
        var comped_array = sqls
        for(var cache in caches){
            var time_cache = caches[cache].time
            for(var sql in sqls){
                if(time_cache == sqls[sql].time)
                    continue
                else if(sqls[sql].time == 'test'){
                    sqls.splice(sqls[sql], 1)
                    comped_array.push(caches[cache])
                    continue
                }
                comped_array.push(caches[cache])
            }
        }
        return JSON.stringify(comped_array)
    },
    batStoreSql:function(cb) {
        if (!mysql.private_sql)
            mysql.connectDb()
        this.getCacheKey('*', function (res) {
            if (res == 'ERRGETKEY') cb('ERRBATSTORE')
            else {
                for (var ei in res) {
                    mysql.insert(res[ei])
                }
            }
        })
    },
    clearMemory:function(openids){
        for(var ai in openids) {
            this.getCacheKey(openids[ai], function(key){
                this.getCacheValue(key, function(value){
                    mysql.insert_exec(key, value).then(this.deleteData(key, function(res){}))
                })
            })
        }
    }
}
