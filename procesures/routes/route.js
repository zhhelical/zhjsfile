//httpssrv.js
"use strict"

var express = require('express');
var router = express.Router();

var file_ctrl = require('../controller/file-ctrl')

/**上传文件*/
router.post('/upload',file_ctrl.upload);

module.exports = router;