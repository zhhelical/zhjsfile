 //clients.js
 const co = require('co')
 var clientsArray = []

 module.exports = {
     joinEvent: function(person, relation){
         clientsArray.push({key:person, line:relation, jointime:new Date()})
     },
     leaveEvent: function(relation){
         console.log(clientsArray)
         for(var who in clientsArray){
             if(clientsArray[who].line === relation) {
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
     memoryCacheChk:function(time){
         return new Promise(function (resolve, reject){
             var cur_mseconds = time.getTime()
             var openids = []
             try {
                 for (var ec in clientsArray) {
                     var cli_time = clientsArray[ec].jointime.getTime()
                     if ((cur_mseconds - cli_time) / 1000 > 7200) {
                         openids.push(clientsArray[ec].key)
                         clientsArray.splice(clientsArray[ec], 1)
                     }
                 }
             }catch(e) reject(e)
             resolve(openids)
         })
     }
 }