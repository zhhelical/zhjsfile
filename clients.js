 //clients.js
 const co = require('co')
 const mysql = require('../../../databases/mysqldata.js')
 const redis = require('../../../databases/rediscache.js')
 const shell = require('./shell.js')
 var clientsArray = []

 module.exports = {
     joinEvent: function(person, relation, sqled, cached, cb){
         for(var who in clientsArray){//for outmind offline issue
             if(clientsArray[who].key == person) {
                 clientsArray.splice(clientsArray[who], 1)
                 break
             }
         }
         var that = this
         if(sqled) {
             this.localkeyExpired(person, function (ep) {
                 if (ep) {
                     mysql.text_exec(person.localKey).then(function (vals) {
                         var sh_order = 'head -n 80 /dev/urandom | tr -dc A-Za-z0-9 | head -c 168'
                         shell.shellFunc(sh_order).then(function (result) {
                             var old_person = person
                             person.localKey = result
                             mysql.insert_exec(JSON.stringify(person), vals).then(function (sql_res) {
                                 mysql.deletePeopleRow(JSON.stringify(old_person)).then(function () {}).then(function(err){
                                     if(err)
                                         that.appOptErr(person.openid, null, err, 'joinEvent.*.mysql.deletePeopleRow', 'people', `${old_person}`, 'null')
                                 })
                                 redis.storeCache(JSON.stringify(person), vals).then(function (res) {
                                     if (cached)
                                         redis.deleteData(JSON.stringify(person)).then(function (d_res) {}).then(function(err){
                                             if(err)
                                                 that.appOptErr(person.openid, null, err, 'joinEvent.*.redis.deleteData', 'people', `${person}`, 'null')
                                         })
                                     clientsArray.push({key: person, line: relation, jointime: new Date()})
                                     cb({key: 'join success', value: result})
                                 }).then(function (err) {
                                     if (err)
                                         cb({key: 'join failed', value: ''})
                                 })
                             }).then(function (err) {
                                 if(err) {
                                     that.appOptErr(person.openid, null, err, `joinEvent.mysql.text_exec(${person.localKey}).shell.shellFunc`, 'people', `${person}`, 'null')
                                     cb({key: 'join failed', value: ''})
                                 }
                             })
                         }).then(function (err) {
                             if(err) {
                                 that.appOptErr(person.openid, null, err, `joinEvent.mysql.text_exec(${person.localKey}).shell.shellFunc`, 'people', `${person}`, 'null')
                                 cb({key: 'join failed', value: ''})
                             }
                         })
                     }).then(function (err) {
                         if(err) {
                             that.appOptErr(person.openid, null, err, `joinEvent(if(ep)).mysql.text_exec(${person.localKey})`, 'people', `${person}`, 'null')
                             cb({key: 'join failed', value: ''})
                         }
                     })
                 }
                 else {
                     if (!cached) {
                         mysql.text_exec(person.localKey).then(function (vals) {
                             redis.storeCache(JSON.stringify(person), vals).then(function (res) {
                                 cb({key: 'join success', value: person})
                                 clientsArray.push({key: person, line: relation, jointime: new Date()})
                             }).then(function (err) {
                                 if(err) {
                                     that.appOptErr(person.openid, null, err, `joinEvent.mysql.text_exec(${person.localKey})`, 'people', `${person}`, 'null')
                                     cb({key: 'join failed', value: ''})
                                 }
                             })
                         })
                     }
                     else {
                         clientsArray.push({key: person, line: relation, jointime: new Date()})
                         cb({key: 'join success', value: person.localKey})
                     }
                 }
             })
         }
         else {
             clientsArray.push({key: person, line: relation, jointime: new Date()})
             cb({key: 'join success', value: person.localKey})
         }
     },
     leaveEvent: function(relation){
         var that = this
         for(var who in clientsArray){
             if(clientsArray[who].line === relation) {
                 var person = clientsArray[who].key
                 redis.deleteData(JSON.stringify(person)).then(function (d_res) {}).then(function(err){
                     if(err)
                         that.appOptErr(person.openid, null, err, 'leaveEvent.*.redis.deleteData', 'people', `${person}`, 'null')
                 })
                 clientsArray.splice(clientsArray[who], 1)
                 break
             }
         }
     },
     findByKey: function(localkey){
         for(var who in clientsArray){
             if(clientsArray[who].key.localKey == localkey)
                 return clientsArray[who]
         }
         return null
     },
     findEvent:function(relation){
         for(var who in clientsArray){
             if(clientsArray[who].line == relation)
                 return clientsArray[who].key
         }
         return null
     },
     localkeyExpired:function(totle_key, cb){
         var cur_time = new Date()
         current_mseconds = cur_time.getTime()
         mysql.text_exec(totle_key.localKey).then(function(vals){
             vals_obj = JSON.parse(vals)
             var origin_mtime = vals_obj[0].origintime
             var minus_mtime = Math.round((current_mseconds-origin_mtime)/1000)
             if(minus_mtime > 2592000)
                 cb(true)
             else
                 cb(false)
         }).then(function(err){
             if(err)
                 that.appOptErr(totle_key.openid, null, err, `localkeyExpired.mysql.text_exec(${totle_key}, cb)`, 'people', totle_key, 'null')
         })
     },
     recordsExpired: function(recorder, cb){
         var cur_time = new Date()
         current_mseconds = cur_time.getTime()
         var time_expire = 7776000000
         var that = this
         var r_openid = JSON.parse(recorder)
         redis.getCacheValue(recorder).then(function (val) {
             var cache_objs = JSON.parse(val)
             var cache_dels = []
             for(var ti in cache_objs){
                 if(ti == 0)
                     continue
                 var obj_time = new Date(cache_objs[ti].time)
                 var rec_time = obj_time.getTime()
                 if(current_mseconds-rec_time < time_expire)
                     break
                 if(current_mseconds-rec_time >= time_expire)
                     cache_dels.push(cache_objs[ti])
             }
             if(cache_dels.length){
                 that.asynDelPics(cache_dels, r_openid.openid)
                 if(cache_dels.length == cache_objs.length-1) {
                     mysql.deletePeopleRow(recorder).then(function (d_res) {}).then(function (err) {
                         if(err)
                             that.appOptErr(r_openid.openid, null, err, 'recordsExpired.*.mysql.deletePeopleRow', 'people', val, 'null')
                     })
                     redis.storeCache(recorder, JSON.stringify(cache_objs[0])).then(function(){}).then(function(err){
                         if(err)
                             that.appOptErr(r_openid.openid, null, err, 'recordsExpired.*.redis.storeCache_if', 'people', val, JSON.stringify(cache_objs[0]))
                     })
                 }
                 else{
                     for(var di in cache_dels){
                         var d_index = cache_objs.indexOf(cache_dels[di])
                         cache_objs.splice(d_index, 1)
                     }
                     mysql.update_exec(recorder, JSON.stringify(cache_objs)).then(function(u_res){}).then(function(err){
                         if(err)
                             that.appOptErr(r_openid.openid, null, err, 'recordsExpired.*.mysql.update_exec', 'people', val, JSON.stringify(cache_objs))
                     })
                     redis.storeCache(recorder, JSON.stringify(cache_objs)).then(function(){}).then(function(err){
                         if(err)
                             that.appOptErr(r_openid.openid, null, err, 'recordsExpired.*.redis.storeCache_else', 'people', val, JSON.stringify(cache_objs))
                     })
                 }
             }
             cb(cache_dels)
         }).then(function(err){
             if(err)
                 that.appOptErr(r_openid.openid, null, err, 'recordsExpired.redis.getCacheValue', 'people', 'null', 'null')
         })
     },
     asynDelPics: function(d_array, openid){
         if(!d_array.length)
             return
         var that = this
         var img_time = d_array[0].time
         mysql.select_picsprops(img_time, '').then(function(s_res){
             if(!s_res.length){
                 d_array.splice(0, 1)
                 that.asynDelPics(d_array)
             }
             var shell_asyn = function(f_names){
                 if(!f_names.length) {
                     d_array.splice(0, 1)
                     that.asynDelPics(d_array, openid)
                 }
                 var f_name = f_names[0]
                 var sh_order = `rm -rf ${f_name}`
                 shell.shellFunc(sh_order).then(function (result) {
                     f_names.splice(0, 1)
                     shell_asyn(f_names)
                 }).then(function(err) {
                     if(err)
                         that.appOptErr(openid, null, err, `asynDelPics.shell_asyn(${f_names})`, `${f_name}`, `${f_name}`, 'null')
                 })
             }
             var filenames = []
             for(var ri in s_res)
                 filenames.push(s_res[ri].img_name)
             shell_asyn(filenames)
         }).then(function(err) {
             if(err)
                 that.appOptErr(openid, null, err, `asynDelPics.mysql.select_picsprops(${img_time}, null)`, 'pictures', `img_time=${img_time}`, 'null')
         })
     },
     appOptErr: function(w_openid, w_localkey, reason, option, w_table, opt_val, optnew_val){
         var c_openid = ''
         var that = this
         if(!w_openid){
             var cli = that.findByKey(w_localkey)
             c_openid = cli.key.openid
         }
         else
             c_openid = w_openid
         var err_obj = {
             err_reason: reason,
             err_table: w_table,
             openid: c_openid,
             err_time: (new Date().getTime()),
             old_content: opt_val,
             new_content: optnew_val,
             opt_type: option
         }
         mysql.insertErrLogs(err_obj).then(function (res) {}).then(function (res) {throw err})
     }
 }