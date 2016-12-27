//rediscache.js
"use strict"
const redis = require('redis')
var clientDb

module.exports = {
    construct: function(key, value) {
        var chk_fault
        clientDb = redis.createClient(6379,'127.0.0.1')
        clientDb.on('error', function (err) {
            return chk_fault = err
        })
        this.storeCache(key, value)
    },
    storeCache:function(key, value){
        clientDb.set(key, value, function (err, reply) {
            console.log(reply.toString())
        })
    },
    getCache:function(key){
        var result
        client.get(key, function (err, reply) {
            if(err)
                return result
            else
                return reply.toString()
        })
    }
}
