//txgeo.js
var http = require("http"),
    Url = require("url"),
    querystring = require('querystring')

/*module.exports = function(location, cb) {
    $.ajax({
        type: 'get',
        url: 'http://apis.map.qq.com/ws/geocoder/v1',
        dataType: 'jsonp',
        data: {
            key: "26SBZ-M6CWI-PDAGZ-5WANA-GSSZS-WCBYN",
            location: `${location.latitude},${location.longitude}`,
            get_poi: "1",
            coord_type: "1",//输入的locations的坐标类型,1 GPS坐标
            output: "jsonp"
        },
        success: function (data, textStatus) {
            if (data.status == 0) {
                var address = data.result.formatted_addresses.recommend
                cb(address)
            } else {
                cb("系统错误，请联系管理员！")
            }
        },
        error: function () {
            cb("系统错误，请联系管理员！")
        }
    })
}
 function defaults(url){
 return {
 // 如果返回false可以取消本次请求
 beforeSend: function(req){},
 complete: function(req){},
 data: '', // Object, String
 dataType: 'html',
 error: function(){},
 headers: {}, // {k:v, ...}
 statusCode: {},
 success: function(data){},
 timeout: 10,
 type: 'GET', // GET, POST
 url: url
 }
 }*/


function ajax(location, cb){
    var settings = {
        type: 'get',
        url: 'http://apis.map.qq.com/ws/geocoder/v1',
        dataType: 'jsonp',
        data: {
            key: "26SBZ-M6CWI-PDAGZ-5WANA-GSSZS-WCBYN",
            location: `${location.latitude},${location.longitude}`,
            get_poi: "1",
            coord_type: "1",
            output: "jsonp"
        },
        success: function (data, textStatus) {
            if (data.status == 0) {
                var address = data.result.formatted_addresses.recommend
                cb(address)
            } else {
                cb("系统错误，请联系管理员！")
            }
        },
        error: function () {
            cb("系统错误，请联系管理员！")
        }
    }

    var params = Url.parse(settings.url, true)

    var options = {
        host: params.hostname,
        port: params.port || 3000,
        path: params.path,
        method: settings.type
    }

    var req = http.request(options, function(res) {
        var data = ''
        res.on('data', function(chunk){
            data += chunk
        }).on('end', function(){
            if(	settings.dataType === "json" ){
                try{
                    data = JSON.parse(data);
                }catch(e){
                    data = null
                }
            }
            settings.success(data)
            settings.complete(req)
        })
    }).on('error', function(e) {
        settings.error(e)
    })

    if( typeof settings.beforeSend === "function" ){
        if ( !settings.beforeSend(req) ){
            settings.complete(req)
            req.end()
            return false
        }
    }

    if( settings.type === "POST" ){
        req.write(querystring.stringify(settings.data))
    }

    req.setTimeout(settings.timeout)
    req.end()
}

exports.ajax = ajax