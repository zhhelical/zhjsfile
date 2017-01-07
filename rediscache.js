//rediscache.js
"use strict"
const redis = require('redis')
    , mysql = require('./mysqldata.js')
    , co = require('co')
    , clientDb = redis.createClient()
    , dbconn = false

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
            else
                cb(replies[0])
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
    /*cacheSqlComp: function(caches, sqls){
        var comped_array = []
        for(var cache in caches){
            if(cache == 0) {
                comped_array.push(caches[cache])
                continue
            }
            var time_cache = caches[cache].time
            for(var sql in sqls){
                if(JSON.stringify(time_cache) == JSON.stringify(sqls[sql].time))
                    continue
                if(sql == 0)
                    continue
                comped_array.push(caches[cache])
            }
        }
        return JSON.stringify(comped_array)
    },*/
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
    },
    searchScope:function(key, lat, lng, scope, city, cb){
        var dis_scope = require('./distance.js')
        var people = []
        clientDb.keys(key, function (err, rekey) {
            if(err) throw err
            clientDb.get(rekey[0], function (err, reval) {
                if(err) cb('ERRGETVALUE')
                var readdr = JSON.parse(reval[0]).addr
                mysql.valueByAddr(readdr).then(function(repersons){
                    for(var ei in repersons){
                        if(ei == 0)
                            continue
                        var obj_location = repersons[ei].location
                        if(dis_scope(obj_location.latitude, obj_location.longitude, lat, lng) < scope)
                            people.push(repersons[ei])
                    }
                    cb(people)
                })
            })
        })
    }
}
