//txgeo.js

module.exports = function(location, cb) {
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