
var fs = require('fs');
var path = require('path');
var    formidable = require('formidable');
var util = require('util');
var fs = require('fs');
var path = require('path');

var setting = require('../config/setting.json');




/**
 * 上传
 */
exports.upload = function(req,res){

    var form = new formidable.IncomingForm();
    form.encoding = 'utf-8';

    //如果需要临时文件保持原始文件扩展名，设置为true
    form.keepExtensions = false;

    //文件大小限制，默认2MB
    form.maxFieldsSize = 2 * 1024 * 1024;


    //图片存放目录
    var imageDir = setting.image_dir;

    //上传临时目录
    var tmpDir = setting.tmp_dir;

    form.uploadDir = tmpDir;//目录需要已存在
    /**
     * fields 表单中的其他属性
     * files  文件集合
     */
    form.parse(req, function(err, fields, files) {

        //图片完整路径
        var imagePath = path.resolve(imageDir, files.file.name);


        //将临时目录中的图片移动到图片存放目录下
        fs.rename(files.file.path,imagePath,function(err){
            if(err) {
                res.json({'success':false,'msg':err});
            }else{
                var image_url = setting.image_url+'/'+files.file.name;
                res.json({'success':true,'msg':'上传成功！','image_url':image_url});
                //res.json({'success':true,'msg':'上传成功！','image_url':image_url,'fields':util.inspect({fields: fields, files: files})});
            }
        });
    });
}




/**
 * 下载
 */
exports.download = function(req,res){
    var filename = req.params.filename;
    var dir = setting.file_dir;
    var file_path = path.resolve(dir,filename);
    fs.exists(file_path,function(exists) {
        if(!exists) {
            res.json({'success':false,'msg':'文件不存在！'});
        }else{
            res.download(file_path,function(err){
                if(err) {
                    res.json({'success':false,'msg':err});
                }
            });

        }
    });
}
