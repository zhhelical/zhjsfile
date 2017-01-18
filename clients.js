 //clients.js
 const co = require('co')
 const mysql = require('../../../databases/mysqldata.js')
 const redis = require('../../../databases/rediscache.js')
 const shell = require('./shell.js')
 var clientsArray = []

 module.exports = {
     joinEvent: function(person, relation, sqled, cached, cb){
         var old_keys = JSON.parse(person)
         for(var who in clientsArray){//for outmind offline issue
             if(clientsArray[who].key == old_keys.localKey) {
                 clientsArray.splice(clientsArray[who], 1)
                 break
             }
         }
         if(sqled) {
             this.localkeyExpired(person, function (ep) {
                 if (ep) {
                     mysql.text_exec(person).then(function (vals) {
                         var sh_order = 'head -n 80 /dev/urandom | tr -dc A-Za-z0-9 | head -c 168'
                         shell.shellFunc(sh_order).then(function (result) {
                             old_keys.localKey = result
                             mysql.insert_exec(JSON.stringify(old_keys), vals).then(function (sql_res) {
                                 mysql.deleteData(person, function () {})
                                 redis.storeCache(JSON.stringify(old_keys), vals).then(function (res) {
                                     if (cached)
                                         redis.deleteData(person, function (d_res) {})
                                     clientsArray.push({key: result, line: relation, jointime: new Date()})
                                     cb({key: 'join success', value: result})
                                 }).then(function (err) {
                                     if (typeof(err) != 'undefined')
                                         cb({key: 'join failed', value: ''})
                                 })
                             }).then(function (err) {
                                 if (typeof(err) != 'undefined')
                                     cb({key: 'join failed', value: ''})
                             })
                         }).then(function (err) {
                             cb({key: 'join failed', value: ''})
                         })
                     })
                 }
                 else {
                     if (!cached) {
                         mysql.text_exec(person).then(function (vals) {
                             redis.storeCache(person, vals).then(function (res) {
                                 cb({key: 'join success', value: person})
                                 clientsArray.push({key: person, line: relation, jointime: new Date()})
                             }).then(function (err) {
                                 if (typeof(err) != 'undefined')
                                     cb({key: 'join failed', value: ''})
                             })
                         })
                     }
                     else {
                         clientsArray.push({key: person, line: relation, jointime: new Date()})
                         cb({key: 'join success', value: person})
                     }
                 }
             })
         }
         else {
             clientsArray.push({key: person, line: relation, jointime: new Date()})
             cb({key: 'join success', value: person})
         }
     },
     leaveEvent: function(relation){
         for(var who in clientsArray){
             if(clientsArray[who].line === relation) {
                 var person = JSON.parse(clientsArray[who].key)
                 redis.getCacheKey(`*${person.localKey}*`, function(key){
                     redis.deleteData(key, function (d_res) {})
                 })
                 clientsArray.splice(clientsArray[who], 1)
                 console.log(clientsArray)
                 break
             }
         }
     },
     findEvent:function(relation){
         for(var who in clientsArray){
             if(clientsArray[who].line == relation)
                 return clientsArray[who].key
         }
         return null
     },
     localkeyExpired:function(local_key, cb){
         var cur_time = new Date()
         current_mseconds = cur_time.getTime()
         mysql.text_exec(local_key).then(function(vals){
             vals_obj = JSON.parse(vals)
             var origin_mtime = vals_obj[0].origintime
             var minus_mtime = (current_mseconds-origin_mtime)/1000
             if(minus_mtime > 2592000)
                 cb(true)
             else
                 cb(false)
         }).then(function(err){cb(err)})
     }
 }