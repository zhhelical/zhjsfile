//rediscache.js
"use strict"
const redis = require('redis')
const mysql = require('./mysqldata.js')
var clientDb

module.exports = {
    redis_srv: clientDb,
    construct: function(key, value) {//将来加入出错信息反馈
        var chk_fault
        clientDb = redis.createClient()
        clientDb.on('error', function (err) {
            return chk_fault = err
        })
        this.storeCache(key, value)
    },
    storeCache:function(key, value, cb){
        if(!key || !value)
            return
        clientDb.set(key, value, function (err, reply) {
            if(err) cb('ERRSTORE')
            else cb(reply)
        })
    },
    getCacheKey:function(key, cb){
        clientDb.keys(key, function (err, replies) {
            if(err) cb('ERRGETKEY')
            else
                cb(replies[0].toString())
        })
    },
    getCacheValue:function(key, cb){
        clientDb.mget(key, function (err, replies) {
            if(err) cb('ERRGETVALUE')
            else
                cb(replies[0].toString())
        })
    },
    deleteData:function(key, cb){
        this.getCacheKey(key, function(res){
            if(res!='ERRGETKEY'){
                clientDb.delete(key, function (err, reply) {
                    if(err) cb('ERRDELETE')
                    else cb(reply)
                })
            }
            else
                cb('ERRGETKEY')
        })
    },
    cacheSqlComp: function(caches, sqls){
        if(caches == sqls)
            return sqls
        var obj_caches = JSON.parse(caches)
        var obj_sqls = JSON.parse(sqls)
        var comped_array = obj_sqls
        for(var cache in obj_caches){
            var time_cache = obj_caches[cache].time
            for(var sql in obj_sqls){
                if(time_cache == obj_sqls[sql].time)
                    break
                comped_array.push(time_cache)
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
    }
}
