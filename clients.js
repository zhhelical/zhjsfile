 //clients.js

 var clientsArray = []

 module.exports = {
     joinEvent: function(person, relation){
         clientsArray.push({key:person, line:relation})
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
     }
 }