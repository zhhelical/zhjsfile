function formatMinuteTime(date) {
    var year = date.getFullYear()
    var month = date.getMonth() + 1
    var day = date.getDate()
    var hour = date.getHours()
    var minute = date.getMinutes()
    var second = date.getSeconds()
    return [year, month, day].map(formatNumber).join('/') + ' ' + [hour, minute, second].map(formatNumber).join(':')
}

function formatFullTime(date) {
    var year = date.getFullYear()
    var month = date.getMonth() + 1
    var day = date.getDate()
    var hour = date.getHours()
    var minute = date.getMinutes()
    return `${year}年${month}月${day}日${hour}时${minute}分`
}

function formatDayTime(date) {
    var month = date.getMonth() + 1
    var day = date.getDate()
    return [month, day].map(formatNumber).join('月') + '日'
}

function formatNumber(n) {
  n = n.toString()
  return n[1] ? n : '0' + n
}

module.exports = {
    formatYmdTime: formatFullTime,
    formatDayTime: formatDayTime,
    formatMinuteTime: formatMinuteTime
}
