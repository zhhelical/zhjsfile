//payunits.js

const md5_cal = require('md5')
const shell = require('./shell.js')
const appid = 'wxf9a75ea1c3517fbe'
const mch_id = '1434433402'
const sucure_key = '91myTzcGyPr7tkvGb7S3C3DUrxCKxEiO'
const h_ip = '123.207.24.35'
const notify_url = 'http://helicalzh.la/wxnotify/index.jsp'
const web_url = 'https://api.mch.weixin.qq.com/pay/unifiedorder'
var sign_random = function(cb){
    var linux_random = 'head /dev/urandom | tr -dc A-Za-z0-9 | head -c 32'
    shell.shellFunc(linux_random).then(function (result){
        cb(result)
    }).then(function(err){
        cb('err for sign random')
    })
}
var signFunc = function(cb){
    sign_random(function(res){
        if(res != 'err for sign random'){
            var js_body = JSON.stringify(body)
            var stringA = `appid=${appid}&body=helical&mch_id=${mch_id}nonce_str=${res}`
            var sign = md5_cal(`${stringA}&${sucure_key}`).toUpperCase()
            cb(res, sign)
        }
        else
            cb('err for sign', '')
    })

}

module.exports = {
    randomStrs: function(cb){
        sign_random(function(r_res){
            cb(r_res)
        })
    },
    options: function(order, h_goods, cny, cb){
        signFunc(function(random, sign){
            if(random != 'err for sign') {
                var options = {
                    paysign: sign,
                    urlobj: {url:`${web_url}?appid=${appid}&mch_id=${mch_id}&nonce_str=${random}&sign=${sign}&body=${h_goods}&out_trade_no=${order}&total_fee=${cny}&spbill_create_ip${h_ip}&notify_url=${notify_url}&trade_type=JSAPI`}
                }
                cb(options)
            }
            else
                cb('err for option')
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