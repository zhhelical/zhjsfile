define("pages/web/lib/emitter.js", function(require, module, exports, window,document,frames,self,location,navigator,localStorage,history,Caches,screen,alert,confirm,prompt,fetch,XMLHttpRequest,WebSocket,webkit,WeixinJSCore,WeixinJSBridge,Reporter){ 'use strict';

var _slicedToArray = function () { function sliceIterator(arr, i) { var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"]) _i["return"](); } finally { if (_d) throw _e; } } return _arr; } return function (arr, i) { if (Array.isArray(arr)) { return arr; } else if (Symbol.iterator in Object(arr)) { return sliceIterator(arr, i); } else { throw new TypeError("Invalid attempt to destructure non-iterable instance"); } }; }();

module.exports = {
    setup: function setup(target) {
        var listeners = [];

        Object.assign(target, {
            on: function on(type, handle) {
                if (typeof handle == 'function') {
                    listeners.push([type, handle]);
                }
            },
            fire: function fire(type) {
                for (var _len = arguments.length, params = Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
                    params[_key - 1] = arguments[_key];
                }

                listeners.forEach(function (_ref) {
                    var _ref2 = _slicedToArray(_ref, 2),
                        listenType = _ref2[0],
                        handle = _ref2[1];

                    return type == listenType && handle.apply(undefined, params);
                });
            },
            removeAllListeners: function removeAllListeners() {
                listeners = [];
            }
        });
    }
};
});
