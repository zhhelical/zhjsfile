//distance.js
var pk = 180 / 3.1415926536  

function distanceCal(lat_a, lng_a, lat_b, lng_b){
  var a1 = lat_a / pk  
  var a2 = lng_a / pk  
  var b1 = lat_b / pk  
  var b2 = lng_b / pk  
  var t1 = Math.cos(a1) * Math.cos(a2) * Math.cos(b1) * Math.cos(b2)  
  var t2 = Math.cos(a1) * Math.sin(a2) * Math.cos(b1) * Math.sin(b2)  
  var t3 = Math.sin(a1) * Math.sin(b1)  
  var tt = Math.acos(t1 + t2 + t3)  
  return 6378137 * tt
}
var EARTH_RADIUS = 6378.137;//地球半径
function rad(d)
{
  return d * Math.PI / 180.0;
}

module.exports = {
  distanceCal: distanceCal
}
