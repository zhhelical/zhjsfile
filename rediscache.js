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
        clientDb.mget(key, function (err, replies) {
            if(err) cb('ERRGETVALUE')
            else
                cb(replies[0])
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
