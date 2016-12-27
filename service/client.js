//clients.js
"use strict"

//const md5 = require('md5')
const sql_database = require('./mysqldata.js')
const redis_database = require('./rediscache.js')
class Client{
    constructor(keys, values, array){
        this.key = keys
        this.value = values
        array.push(this)
        this.dealDataBase(values)
    }
    dealDataBase(values) {
        c_database.newTable(this.key, values)
    }
    onTable(keys, order) {
        if(order == 'select')
            c_database.selectValue(keys)
        else if(order == 'delete')
            c_database.deleteValue(keys)
        else if(order == 'cover')
            c_database.coverValue(keys)
    }
    redisDatabase(key, handle) {

    }
    dealData(data){

    }
}
module.exports = Client
