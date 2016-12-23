//service.js
"use strict"

var https=require('https');
var ws=require('ws');
var fs=require('fs');
var keypath=process.cwd()+'/server.key';//秘钥文件
var certpath=process.cwd()+'/public.pem';

var options = {
    key: fs.readFileSync(keypath),
    cert: fs.readFileSync(certpath)
};

var server=https.createServer(options, function (req, res) {
    res.writeHead(403);//403即可
    res.end("This is a  WebSockets server!\n");
}).listen(9595);

var wss = new ws.Server( { server: server } );
wss.on('connection', function(conn) {
    conn.on('message', function(message) {
        //传输内容为JSON
        var mData = JSON.parse(message);
        if(mData && mData.EVENT) {
            switch(mData.EVENT) {
                case "LOGIN":
                    // 新用户加入
                    uid = mData.USER.UID;
                    //把加入用户的信息和连接关联起来
                    conn.UserId = uid;
                    conn.GroupId = mData.PKG.GROUPID;
                    //console.log('User:{\'uid\':' + newUser.uid + ',\'nickname\':' + newUser.nick + '}coming on protocol websocket draft ' + conn.protocolVersion);
                    console.log('connected user' + wss.clients.length);
                    console.log(uid+"///"+mData.PKG.GROUPID);
                    // 把新用户的信息广播给在线用户
                    for(var i = 0; i < wss.clients.length; i++) {
                        //同组的才广播
                        if(wss.clients[i].GroupId==mData.PKG.GROUPID){
                            wss.clients[i].send("new member");
                        }
                    }
                    break;
                case "TEXT":
                    console.log(mData);
                    //用户发送内容
                    var content = mData.PKG.CONTENT;
                    //发给目标组
                    var gId = mData.PKG.GROUPID;
                    for(var i = 0; i < wss.clients.length; i++) {
                        //发送给同组的所有成员
                        if(gId==wss.clients[i].GroupId){
                            wss.clients[i].send(content);
                        }
                    }
                    break;
                default:
                    break;
            }

        } else {
            console.log(mData);
        }
    });
    conn.on('error', function() {
        console.log(Array.prototype.join.call(arguments, ", "));
    });
    conn.on('close', function() {
        console.log('someone offline')
    });

});