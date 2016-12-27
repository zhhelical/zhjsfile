 //clients.js

 var clientsArray = []

 module.exports = {
     people: clientsArray,
     leaveEvent: function(person){
         for(var who in clientsArray){
             if(clientsArray[who] === person) {
                 clientsArray.splice(who, 1)
                 break
             }
         }
     },
     joinEvent: function(person){
         clientsArray.push(person)
     },
     findEvent:function(keys){
         for(var who in clientsArray){
             if(clientsArray[who].key == keys)
                 return clientsArray[who]
         }
     }
 }