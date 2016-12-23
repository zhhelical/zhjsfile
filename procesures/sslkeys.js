//ssls.js
"use strict"

const fs = require('fs')

exports.ssls = {
    key: fs.readFileSync(process.cwd()+'/2_www.helicalzh.la.key'),
    cert: fs.readFileSync(process.cwd()+'/1_www.helicalzh.la_bundle.crt')
}
