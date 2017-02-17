//txgeo.js
var http = require("http"),
    Url = require("url"),
    querystring = require('querystring')

module.exports = function(location, cb){
    var settings = {
        beforeSend: function(req){},
        complete: function(req){},
        timeout: 10,
        type: 'get',
        url: 'http://apis.map.qq.com/ws/geocoder/v1',
        dataType: 'json',
        data: {
            key: "MVKBZ-OTK3O-K44WT-SEFYD-WGR5K-I7FKC",
            location: `${location.latitude},${location.longitude}`,
            coord_type: "1",
            output: "json"
        },
        success: function (data) {
            if (data.status == 0) {
                var address = data.result.address
                return address
            } else {
                return "系统错误，请联系管理员！"
            }
        },
        error: function () {
            return "系统错误，请联系管理员！"
        }
    }

    var params = Url.parse(settings.url, true)

    var options = {
        host: params.hostname,
        port: params.port || 80,
        path: params.path+`/?location=${settings.data.location}&key=${settings.data.key}&get_poi=1`,
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
            cb(settings.success(data))
            //settings.complete(req)
        })
    }).on('error', function(e) {
        cb('error')
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
