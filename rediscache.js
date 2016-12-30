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
                cb(replies[0])
        })
    },
    getCacheValue:function(key, cb){
        clientDb.mget(key, function (err, replies) {
            if(err) cb('ERRGETVALUE')
            else
                cb(replies)
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
    isOldDriver: function(openID, cb){
        clientDb.keys(openID, function (err, replies) {
            if(err) cb(err)
            else
                cb(replies)
        })
        if(!mysql.private_sql)
            mysql.connectDb()
        return mysql.select(openID)
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
