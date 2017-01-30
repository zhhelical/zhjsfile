//rediscache.js
"use strict"
const redis = require('redis')
    , mysql = require('./mysqldata.js')
    , co = require('co')
    , clientDb = redis.createClient()
var dbconn = false

module.exports = {
    redis_srv: dbconn,
    construct: function() {//将来加入出错信息反馈
        var chk_fault
        dbconn = true
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
            else{
                if(replies.length)
                    cb(replies[0])
                else
                    cb([])
            }
        })
    },
    getCacheValue:function(key, cb){
        clientDb.get(key, function (err, replies) {
            if(err) cb('ERRGETVALUE')
            else
                cb(replies)
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
    searchScope:function(key, lat, lng, last_scope, cur_scope, city){
        var dis_scope = require('../mywsServer/procesures/service/distance.js')
        var people = []
        return new Promise(function (resolve, reject) {
            mysql.valueByAddr(city).then(function (repersons) {
                if(repersons.length) {
                    var js_objs = JSON.parse(repersons[0].value)
                    for (var ei in js_objs) {
                        if (ei == 0)
                            continue
                        var obj_location = js_objs[ei].location
                        var distance = dis_scope.distanceCal(obj_location.latitude, obj_location.longitude, lat, lng)
                        if (js_objs[ei].content.add_key==city.gate && distance>last_scope && distance<=cur_scope)
                            people.push(js_objs[ei])
                    }
                }
                resolve(people)
            })
        })
    }
}
