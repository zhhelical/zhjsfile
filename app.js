//app.js
"use strict"

const http = require('http')
const express = require('express')

var app = express()

app.configure(function() {
    app.use(express.bodyParser());
}
app.post('/upload', function(req, res) {
    console.log(req.body)
    res.send('Post Over');
})
app.listen(9090)