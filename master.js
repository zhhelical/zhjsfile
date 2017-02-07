 //master.js
 const co = require('co')
 const mysql = require('../../../databases/mysqldata.js')
 const joiners = require('./clients.js')
 const m_openid = 'oSej50L7Zswj9TVllkJ0O68FJv2s'
 var getAllSubmits = function(cb){
     mysql.masterGetting('people').then(function (result) {
         if (result && result.length)
             cb({bool:true, value:result})
         else
             cb({bool:false, value:'empty'})
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
         else
             cb({bool:false, value:'empty'})
     }).then(function(err){
         if (err) {
             joiners.appOptErr(m_openid, null, `${err}`, 'master.getAllMsgs.mysql.masterGetting', 'messages', 'null', 'null')
             cb({bool:false, value:'system wrong'})
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
                     var res_arr = []
                     if(l_res > 60)
                         res_arr = s_res.slice(from, from+60)
                     else
                         res_arr = s_res.slice(from)
                     resolve(res_arr)
                 }
                 else{
                     var error = null
                     if(s_res.value != 'empty')
                         error = 'saved system wrong'
                     reject(error)
                 }
             })
         })
     },
     getSuitableMsgs: function(from){
         return new Promise(function (resolve, reject){
             getAllMsgs(function(m_res){
                 if (m_res.bool){
                     var l_res = m_res.value.length-from
                     var res_arr = []
                     if(l_res > 60)
                         res_arr = m_res.slice(from, from+60)
                     else
                         res_arr = m_res.slice(from)
                     resolve(res_arr)
                 }
                 else{
                     var error = null
                     if(m_res.value != 'empty')
                         error = 'saved system wrong'
                     reject(error)
                 }
             })
         })
     }
 }