//client.js
"use strict"

const md5 = require('md5')
const c_mobil = require('mobil.js')

class Client{
    constructor(socket){
        console.log('------ socket connect with: id ${socket.id} -------')
        this.id = socket.id
        this.socket = socket
        this.user = null
        this.mobil = null
        this.mobilPos = {
            secure_key: '',
            clongitude:null,
            clatitude:null
        }
    }
    connect(){
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
        on(message, handle){
            var client = this
            this.socket.on(message, function(...args){
                console.log('[client] ${message}')
                console.log(args)
                handle.apply(client, args)
            })
        }
        emit(message, ...data){
            this.socket.emit(message, ...data)
        }
 /*       broadcast(message, ...data){
            if(!this.mobil)
                return
            this.others().forEach()
        }*/
    }
}
export.Client = Client