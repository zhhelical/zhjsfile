 //clients.js
 const co = require('co')
 const mysql = require('../../../databases/mysqldata.js')
 const redis = require('../../../databases/rediscache.js')
 const shell = require('./shell.js')
 var clientsArray = []

 module.exports = {
     joinEvent: function(person, relation, sqled, cached, cb){
         for(var who=clientsArray.length-1; who>-1; who--){//for outmind offline issue
             if(JSON.stringify(clientsArray[who].key)==JSON.stringify(person) || JSON.stringify(JSON.stringify(person)).match(clientsArray[who].key)) {
                 if(JSON.stringify(relation) != JSON.stringify(clientsArray[who].relation)) {
                     clientsArray[who].relation = null
                     clientsArray.splice(who, 1)
                 }
             }
         }
         var that = this
         var cur_time = new Date()
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
                                     clientsArray.push({key: person, line: relation, jointime: cur_time})
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
                                 cb({key: 'join success', value: person.localKey})
                                 clientsArray.push({key: person, line: relation, jointime: cur_time})
                             }).then(function (err) {
                                 if(err) {
                                     that.appOptErr(person.openid, null, err, `joinEvent.mysql.text_exec(${person.localKey})`, 'people', `${person}`, 'null')
                                     cb({key: 'join failed', value: ''})
                                 }
                             })
                         })
                     }
                     else {
                         clientsArray.push({key: person, line: relation, jointime: cur_time})
                         cb({key: 'join success', value: person.localKey})
                     }
                 }
             })
         }
         else {
             clientsArray.push({key: person, line: relation, jointime: cur_time})
             cb({key: 'join success', value: person.localKey})
         }
         var current_mseconds = cur_time.getTime()
         that.delExpiredMsgs(person.openid, current_mseconds)
     },
     leaveEvent: function(localkey){
         var that = this
         for(var who in clientsArray){
             if(clientsArray[who].key.localKey === localkey) {
                 var person = clientsArray[who].key
                 clientsArray.splice(clientsArray[who], 1)
                 if(person.hasOwnProperty('expires_in')){
                     delete person.expires_in
                     delete person.session_key
                 }
                 redis.deleteData(JSON.stringify(person)).then(function (d_res) {}).then(function(err){
                     if(err)
                         that.appOptErr(person.openid, null, err, 'leaveEvent.*.redis.deleteData', 'people', `${person}`, 'null')
                 })
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
     updateConn: function(localkey, new_conn){
         var cli = this.findByKey(localkey)
         if(cli)
             cli.line = new_conn
     },
     localkeyExpired:function(totle_key, cb){
         var cur_time = new Date(), current_mseconds = cur_time.getTime()
         mysql.text_exec(totle_key.localKey).then(function(vals){
             if(vals.length) {
                 vals_obj = eval('('+vals+')')
                 var origin_mtime = vals_obj[0].origintime
                 var minus_mtime = Math.round((current_mseconds - origin_mtime) / 1000)
                 if (minus_mtime > 2592000)
                     cb(true)
                 else
                     cb(false)
             }
         }).then(function(err){
             if(err)
                 that.appOptErr(totle_key.openid, null, err, `localkeyExpired.mysql.text_exec(${totle_key}, cb)`, 'people', totle_key, 'null')
         })
     },
     recordsExpired: function(recorder){
         var cur_time = new Date(), current_mseconds = cur_time.getTime(), time_expire = 7776000000
         var that = this
         var r_openid = JSON.parse(recorder)
         redis.getCacheValue(recorder).then(function (val) {
             var cache_objs = eval('('+val+')')
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
                             that.appOptErr(r_openid.openid, r_openid.localKey, err, 'recordsExpired.*.mysql.deletePeopleRow', 'people', val, 'null')
                     })
                     redis.deleteData(recorder).then(function(){}).then(function(err){
                         if(err)
                             that.appOptErr(r_openid.openid, r_openid.localKey, err, 'recordsExpired.*.redis.deleteData', 'people', val, JSON.stringify(cache_objs[0]))
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
         }).then(function(err){
             if(err)
                 that.appOptErr(r_openid.openid, null, err, 'recordsExpired.redis.getCacheValue', 'people', 'null', 'null')
         })
     },
     msgAutoReply: function(messager, t_send){
         var that = this
         setTimeout(function(){
             mysql.msgsGetting(messager).then(function (res) {
                 var times = JSON.parse(res.time)
                 var t_pos = 0
                 for(var ti in times){
                     if(times[ti] == t_send){
                         t_pos = ti
                         break
                     }
                 }
                 var contents = eval('('+res.contents+')')
                 for(var ci in contents){
                     if(ci == t_pos){
                         var str_reply = '感谢您的批评指导或意见，我们会竭力优化我们对您的服务，希望您常来，再次感谢您'
                         contents[ci].reply = str_reply
                         break
                     }
                 }
                 mysql.msgsSave(messager, contents, t_send, false).then(function(s_res){}).then(function(err){
                     if (err)
                         that.appOptErr(messager, null, `${err}`, 'clients.msgAutoReply.mysql.msgsSave', 'messages', 'null', t_send)
                 })
             }).then(function (err) {
                 if(err)
                     that.appOptErr(messager, null, `${err}`, 'clients.msgAutoReply.mysql.msgsGetting', 'messages', 'null', t_send)
             })
         },1800000)
     },
     asynDelPics: function(d_array, openid){
         if(!d_array.length)
             return
         var that = this
         var img_time = d_array[0]
         if(typeof(d_array[0])=='object')
             img_time = d_array[0].time
         mysql.select_picsprops(img_time, '').then(function(s_res){
             mysql.deletePictures(img_time).then(function (d_res) {}).then(function (err) {
                 if (err)
                     joiners.appOptErr(key_id.openid, null, `${err}`, 'clients.*.mysql.deletePictures', 'pictures', img_time, 'null')
             })
             var shell_asyn = function (f_names) {
                 if (!f_names.length) {
                     d_array.splice(0, 1)
                     that.asynDelPics(d_array, openid)
                 }
                 else {
                     var f_name = f_names[0]
                     var sh_order = `rm -rf ${f_name}`
                     shell.shellFunc(sh_order).then(function (result) {
                         f_names.splice(0, 1)
                         shell_asyn(f_names)
                     }).then(function (err) {
                         if (err)
                             that.appOptErr(openid, null, err, `asynDelPics.shell_asyn(${f_names})`, `${f_name}`, `${f_name}`, 'null')
                     })
                 }
             }
             var filenames = []
             for (var ri in s_res) {
                 var rpic_name = s_res[ri].img_name
                 if (!rpic_name.match('/'))
                     rpic_name = '/data/release/helical/uploads/' + rpic_name
                 filenames.push(rpic_name)
             }
             shell_asyn(filenames)
         }).then(function(err) {
             if(err)
                 that.appOptErr(openid, null, err, `asynDelPics.mysql.select_picsprops(${img_time}, null)`, 'pictures', `img_time=${img_time}`, 'null')
         })
     },
     delExpiredMsgs: function(oid, c_time){
         mysql.msgsGetting(oid).then(function(m_res){
             if(m_res.length) {
                 var id_msgs = eval('(' + m_res.contents + ')')
                 var times = JSON.parse(m_res.time)
                 var del_arr = []
                 for (var ti in times) {
                     if ((c_time - times[ti]) >= 7776000000)
                         del_arr.push(ti)
                 }
                 if (del_arr.length) {
                     for (var i = del_arr.length - 1; i > -1; i--) {
                         for (var mi = id_msgs.length - 1; mi > -1; mi--) {
                             if (mi == i) {
                                 id_msgs.splice(mi, 1)
                                 times.splice(mi, 1)
                                 break
                             }
                         }
                     }
                     var up_msgs = JSON.stringify(id_msgs), up_times = JSON.stringify(times)
                     mysql.msgsUpdate(oid, up_msgs, up_times).then(function (u_res) {
                     }).then(function (err) {
                         if (err)
                             that.appOptErr(oid, 'null', err, `delExpiredMsgs.*.mysql.msgsUpdate(${c_time})`, 'messages', 'null', `${up_msgs}/${up_times}`)
                     })
                 }
             }
         }).then(function(err){
             if(err)
                 that.appOptErr(oid, 'null', err, `delExpiredMsgs.mysql.msgsGetting(${c_time})`, 'messages', `del_time=${c_time}`, 'null')
         })
     },
     appOptErr: function(w_openid, w_localkey, reason, option, w_table, opt_val, optnew_val){
         var c_openid = ''
         var that = this
         if(!w_openid){
             var cli = that.findByKey(w_localkey)
             if(cli)
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
         mysql.insertErrLogs(err_obj).then(function (res) {}).then(function (err) {throw err})
     }
 }