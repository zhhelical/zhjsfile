//clients.js
"use strict"

//const md5 = require('md5')
const c_database = require('./datastore.js')

class Client{
    constructor(socket){
        if(socket && socket.EVENT) {
            switch(socket.EVENT) {
                case "LOGIN":
                    // 新用户加入
                    uid = mData.USER.UID;
                    //把加入用户的信息和连接关联起来
                    conn.UserId = uid;
                    conn.GroupId = mData.PKG.GROUPID;
                    console.log('connected user' + wss_server.clients.length);
                    console.log(uid+"///"+mData.PKG.GROUPID);
                    break;
                case "TEXT":
                    console.log(socket);
                    //用户发送内容
                    var content = socket.PKG.CONTENT;
                    //发给目标组
                    var gId = socket.PKG.GROUPID;
                    break;
                default:
                    break;
            }

        }
        else
            console.log(socket);
        this.id = socket.id
        this.socket = socket
        this.user = null
        this.mobil = null
        this.mobilPos = {
            secure_key: '',
            clongitude:null,
            clatitude:null,
            submitting_info: null,
            submitted_info: null
        }
    }
    connect() {
        this.on('hello', packet => {
            const user = packet.user
            this.user = user
        this.emit('hi', {
            message: 'welcome',
            secure_key: '' //?
            })
        })
        this.on('handup', packet => {
            if(this.mobil)
                this.mobil.removeClient(this)
            let mobil = this.mobil = p_mobil.Mobil.findWhich() || p_mobil.Mobil.create()
            mobil.addClient(this)
            console.log('new mobil user join success!')
        })
        this.on('disconnect', packet => {
            if(this.mobil){
                this.mobil.removeClient(this)
                this.mobil = null
            }
            this.socket.disconnect()
            this.socket = null
            this.user = null
            this.mobilPos = null
        })
    }
    on(message, handle) {
        var client = this
        this.socket.on(message, function(args){
            console.log('[client] ${message}')
            console.log(args)
            handle.apply(client, args)
        })
    }
    checkNewUser(key, handle) {

    }
    emit(message, data){
            this.socket.emit(message, data)
    }
}
exports.Client = Client