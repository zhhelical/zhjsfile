 //master.js
 const co = require('co')
 const mysql = require('../../../databases/mysqldata.js')
 const joiners = require('./clients.js')
 const m_openid = 'oSej50L7Zswj9TVllkJ0O68FJv2s'
 var getAllSubmits = function(cb){
     mysql.masterGetting('people').then(function (result) {
         if (result && result.length)
             cb({bool:true, value:result})
         else {
             joiners.appOptErr(m_openid, null, 'empty', 'master.getAllSubmits.mysql.masterGetting', 'messages', 'null', 'null')
             cb({bool:false, value:'empty'})
         }
     }).then(function(err){
         if (err) {
             joiners.appOptErr(m_openid, null, `${err}`, 'master.getAllSubmits.mysql.masterGetting', 'people', 'null', 'null')
             cb({bool:false, value:'system wrong'})
         }
     })
 }
 var getAllMsgs = function(cb){
     mysql.masterGetting('messages').then(function (result) {
         if (result && result.length)
             cb({bool:true, value:result})
         else{
             joiners.appOptErr(m_openid, null, 'empty', 'master.getAllMsgs.mysql.masterGetting', 'messages', 'null', 'null')
             cb({bool:false, value:'empty'})
         }
     }).then(function(err){
         if (err) {
             joiners.appOptErr(m_openid, null, `${err}`, 'master.getAllMsgs.mysql.masterGetting', 'messages', 'null', 'null')
             cb({bool: false, value: 'system wrong'})
         }
     })
 }
 module.exports = {
     id_master: m_openid,
     getSuitableSubmits: function(from){
         return new Promise(function (resolve, reject){
             getAllSubmits(function(s_res){
                 if (s_res.bool){
                     var l_res = s_res.value.length-from
                     var sql_arr = s_res.value
                     var re_arr = []
                     if(l_res > 60)
                         re_arr = sql_arr.slice(from, from + 60)
                     else
                         re_arr = sql_arr.slice(from)
                     var res_arr = []
                     for(var mi in re_arr){
                         if(mi == 0)
                             continue
                         var ev_obj = {}
                         ev_obj = JSON.parse(re_arr[mi])
                         res_arr.push(ev_obj)
                     }
                     resolve(res_arr)
                 }
                 else
                     reject(null)
             })
         })
     },
     getSuitableMsgs: function(from){
         return new Promise(function (resolve, reject){
             getAllMsgs(function(m_res){
                 if (m_res.bool){
                     var l_res = m_res.value.length-from
                     var sql_arr = m_res.value
                     var re_arr = []
                     if(l_res > 60)
                         re_arr = m_res.value.slice(from, from + 60)
                     else
                         re_arr = m_res.value.slice(from)
                     var res_arr = []
                     for(var mi in re_arr){
                         var ev_obj = {}
                         ev_obj.contents = JSON.parse(re_arr[mi].contents)
                         ev_obj.time = JSON.parse(re_arr[mi].time)
                         res_arr.push(ev_obj)
                     }
                     resolve(res_arr)
                 }
                 else
                     reject(null)
             })
         })
     }
 }