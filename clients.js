 //clients.js

 var clientsArray = []

 module.exports = {
     people: clientsArray,
     joinEvent: function(person, relation){
         clientsArray.push({key:person, line:relation})
     },
     leaveEvent: function(relation){
         for(var who in clientsArray){
             if(clientsArray[who].line === relation) {
                 clientsArray.splice(clientsArray[who], 1)
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