//rediscache.js
"use strict"
const redis = require('redis')
    , mysql = require('./mysqldata.js')
    , co = require('co')
    , clientDb = redis.createClient()
var dbconn = false
var assortScopes = function(s_results, s_gate, s_lat, s_lng, slast_scope, scur_scope){
    var dis_scope = require('../mywsServer/procesures/service/distance.js')
    var arr_empty = []
    for(var si in s_results){
        var si_array = []
        var js_style = JSON.parse(s_results[si].value)
        for(var jsi in js_style){
            if (jsi == 0)
                continue
            var obj_location = js_style[jsi].location
            var distance = dis_scope.distanceCal(obj_location.latitude, obj_location.longitude, s_lat, s_lng)
            if(js_style[jsi].content.add_key==s_gate && distance>=slast_scope && distance<=scur_scope)
                si_array.push(js_style[jsi])
        }
        if(si_array.length)
            arr_empty.push(si_array)
    }
    return arr_empty
}
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
    getCacheKey:function(key){
        return new Promise(function (resolve, reject) {
            clientDb.keys(key, function (err, replies) {
                if (err) reject(err)
                else {
                    if(replies.length)
                        resolve(replies[0])
                    else
                        resolve(replies)
                }
            })
        })
    },
    getCacheValue:function(key){
        return new Promise(function (resolve, reject) {
            clientDb.get(key, function (err, replies) {
                if (err) reject(err)
                else{
                    console.log(replies, 'redis')
                    if(replies)
                            resolve(replies[0])
                    else
                        resolve(replies)
                }
            })
        })
    },
    deleteData:function(key){
        var that = this
        return new Promise(function (resolve, reject) {
            that.getCacheKey(key).then(function (res) {
                clientDb.del(key, function (err, reply) {
                    if (err) reject(err)
                    else resolve(reply)
                })
            }).then(function (err) {
                if (err)
                    reject(err)
            })
        })
    },
    searchScope:function(lat, lng, last_scope, cur_scope, city){
        var people = []
        return new Promise(function (resolve, reject) {
            mysql.valueByAddr(city).then(function (repersons) {
                if(repersons.length)
                    people = assortScopes(repersons, city.gate, lat, lng, last_scope, cur_scope)
                resolve(people)
            }).then(function(err){
                if(err)
                    reject(err)
            })
        })
    }
}
