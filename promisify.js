//define("pages/web/lib/promisify.js", function(require, module, exports, window,document,frames,self,location,navigator,localStorage,history,Caches,screen,alert,confirm,prompt,fetch,XMLHttpRequest,WebSocket,webkit,WeixinJSCore,WeixinJSBridge,Reporter){ "use strict";

module.exports = function (api) {
    return function (options) {
        for (var _len = arguments.length, params = Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
            params[_key - 1] = arguments[_key];
        }

        return new Promise(function (resolve, reject) {
            api.apply(undefined, [Object.assign({}, options, { success: resolve, fail: reject })].concat(params));
        });
    };
};
//});
