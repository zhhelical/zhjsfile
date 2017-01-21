//payunits.js

const md5_cal = require('md5')
const xml2js = require('xml2js')
const shell = require('./shell.js')
const appid = 'wxf9a75ea1c3517fbe'
const mch_id = '1434433402'
const sucure_key = '91myTzcGyPr7tkvGb7S3C3DUrxCKxEiO'
const h_ip = '123.207.24.35'
const notify_url = 'http://helicalzh.la/wxnotify/index.jsp'
const web_host = 'https://api.mch.weixin.qq.com'
const web_path = '/pay/unifiedorder'
var sign_random = function(cb){
    var linux_random = 'head /dev/urandom | tr -dc A-Za-z0-9 | head -c 32'
    shell.shellFunc(linux_random).then(function (result){
        cb(result)
    }).then(function(err){
        cb('err for sign random')
    })
}
var signFunc = function(body, cb){
    sign_random(function(res){
        if(res != 'err for sign random'){
            var js_body = JSON.stringify(body)
            var stringA = `appid=${appid}&body=helical&mch_id=${mch_id}nonce_str=${res}`
            var sign = md5_cal(`${stringA}&${sucure_key}`).toUpperCase()
            cb({random:res, sign:sign})
        }
        else
            cb({random:'err for sign', sign:''})
    })

}

module.exports = {
    randomStrs: function(cb){
        sign_random(function(r_res){
            cb(r_res)
        })
    },
    options: function(order, h_goods, cny, cb){
        signFunc(h_goods, function(r_sign){
            if(r_sign.random != 'err for sign') {
                //var jsbody = {appid: appid, mch_id: mch_id, body: h_goods, nonce_str: r_sign.random, notify_url: notify_url, out_trade_no: order, spbill_create_ip: h_ip, total_fee: cny, trade_type: 'JSAPI', sign:r_sign.sign}
                //var xmlbuild = new xml2js.Builder()
                var xmlbody = `<xml>
                    <appid>${appid}</appid>
                    <mch_id>${mch_id}</mch_id>
                    <body>test</body>
                    <nonce_str>${r_sign.random}</nonce_str>
                    <sign>${r_sign.sign}</sign>
                <xml>`
                //var xmlbody = xmlbuild.buildObject(jsbody)
                var options = {
                    paysign: r_sign.sign,
                    urlobj: {host: web_host, path: web_path, method: 'POST', data: xmlbody, headers:{"Connection":"Keep-Alive", "Content-Type":'application/xml;charset=utf-8', "Content-length":xmlbody.length}}
                }
                cb({value: options})
            }
            else
                cb({value: 'err for option'})
        })
    },
    helicalOrder: function(p_id, o_time){
        var h_order = `${p_id}${o_time}`
        return h_order
    },
    helicalGoods: function(type, price){
        var gen_goods = {"goods_detail":[{
                    "goods_id":type,
                    "good_name":'helical',
                    "quantity":1,
                    "price":price}]
        }
        return gen_goods
    }
}