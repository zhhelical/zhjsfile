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
    storeCache:function(key, value){
        if(!key || !value)
            return
        clientDb.set(key, value, function (err, reply) {
            console.log(reply.toString())
        })
    },
    getCacheKey:function(key){
        clientDb.keys(`*${key}*`, function (err, replies) {
            if(err) throw err
            else
                return replies.toString()
        })
    },
    getCacheValue:function(key){
        clientDb.mget(`*${key}*`, function (err, replies) {
            if(err) throw err
            else
                return replies.toString()
        })
    },
    deleteClientData:function(key){
        var redis_key = this.getCache(key)
        clientDb.delete(key, function (err, reply) {
            console.log(reply.toString())
        })
    },
    isOldDriver: function(openID){
        if(!mysql.private_sql)
            mysql.connectDb()
        return mysql.select(openID)
    }
}
