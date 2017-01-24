//payunits.js

const shell = require('../mywsServer/procesures/service/shell.js')
const crypto = require('crypto')
const appid = 'wxf9a75ea1c3517fbe'
const bank_type = 'CCB_DEBIT'
const mch_id = '1434433402'
//const secure_key = '513AA201C70F1EDE03AD9F4A802745C6'

var sign_random = function(cb){
    var linux_random = 'head /dev/urandom | tr -dc A-Za-z0-9 | head -c 32'
    shell.shellFunc(linux_random).then(function (result){
        cb(result)
    }).then(function(err){
        cb('err for sign random')
    })
}
var notifyResFunc = function(accept_obj, cb){
    sign_random(function(res){
        if(res != 'err for sign random'){
            var stringA = `appid=${appid}&bank_type=${bank_type}&cash_fee=${accept_obj.total_fee*100}&mch_id=${mch_id}&nonce_str=${res}&openid=${accept_obj.open_id}&out_trade_no=${accept_obj.out_trade_no}&result_code=${s_id}&time_end=${accept_obj.end_time}&total_fee=${accept_obj.total_fee}&trade_type=JSAPI&transaction_id=${accept_obj.transaction_id}`
            var sign = crypto.createHash('md5').update(`${stringA}&key=${secure_key}`, 'utf-8').digest('hex').toUpperCase()
            cb({random:res, csStrs:stringA, sign:sign})//&sign=${notify_url}
        }
        else
            cb({random:'err for sign', sign:''})
    })
}

module.exports = {
    responseObj: [],
    forNotify: function(obj, cb){
        var endtime = new Date()
        var et_str = `${endtime.getFullYear()}${endtime.getMonth()+1}${endtime.getDate()}${endtime.getHours()}${endtime.getMinutes()}${endtime.getSeconds()}`
        trans_obj[0].end_time = et_str
        notifyResFunc(obj, function(n_res){
            if(n_res.random != 'err for sign') {
                var xml_body = `<xml>
                    <appid><![CDATA[${appid}]]></appid>
                    <bank_type><![CDATA[${bank_type}]]></bank_type>
                    <cash_fee><![CDATA[${obj.total_fee*100}]]></cash_fee>
                    <mch_id><![CDATA[${mch_id}]]></mch_id>
                    <nonce_str><![CDATA[${n_res.random}]]></nonce_str>
                    <openid><![CDATA[${obj.open_id}]]></openid>
                    <out_trade_no><![CDATA[${obj.out_trade_no}]]></out_trade_no>
                    <result_code><![CDATA[SUCCESS]]></result_code>
                    <return_code><![CDATA[SUCCESS]]></return_code>
                    <sign><![CDATA[${n_res.sign}]]></sign>
                    <time_end><![CDATA[${et_str}]]></time_end>
                    <total_fee>${obj.total_fee}</total_fee>
                    <trade_type><![CDATA[JSAPI]]></trade_type>
                    <transaction_id><![CDATA[${obj.transaction_id}]]></transaction_id>
                    </xml>`
                cb(xml_body)
            }
            else
                cb('err for respond')
        })
    },
    findRespondObj: function(info){
        var objs = this.responseObj
        var info_array = info.split('-')
        for(var ni in objs) {
            if (objs[ni].openid==info_array[0] && objs[ni].time==info_array[2])
                return objs[ni]
        }
        return null
    }
}