define("pages/web/lib/wxsocket.io.js", function(require, module, exports, window,document,frames,self,location,navigator,localStorage,history,Caches,screen,alert,confirm,prompt,fetch,XMLHttpRequest,WebSocket,webkit,WeixinJSCore,WeixinJSBridge,Reporter){ 'use strict';

var _slicedToArray = function () { function sliceIterator(arr, i) { var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"]) _i["return"](); } finally { if (_d) throw _e; } } return _arr; } return function (arr, i) { if (Array.isArray(arr)) { return arr; } else if (Symbol.iterator in Object(arr)) { return sliceIterator(arr, i); } else { throw new TypeError("Invalid attempt to destructure non-iterable instance"); } }; }();

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

function _toConsumableArray(arr) { if (Array.isArray(arr)) { for (var i = 0, arr2 = Array(arr.length); i < arr.length; i++) { arr2[i] = arr[i]; } return arr2; } else { return Array.from(arr); } }

function _toArray(arr) { return Array.isArray(arr) ? arr : Array.from(arr); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var emitter = require('./emitter.js');

/** socket.io 协议常量 */
var packets = {
    open: 0 // non-ws
    , close: 1 // non-ws
    , ping: 2,
    pong: 3,
    message: 4,
    upgrade: 5,
    noop: 6
};
var events = {
    CONNECT: 0,
    DISCONNECT: 1,
    EVENT: 2,
    ACK: 3,
    ERROR: 4,
    BINARY_EVENT: 5,
    BINARY_ACK: 6
};

var PING_CHECK_INTERVAL = 2000;

var WxSocketIO = function () {
    function WxSocketIO() {
        _classCallCheck(this, WxSocketIO);
    }

    _createClass(WxSocketIO, [{
        key: 'connect',
        value: function connect(url) {
            var _this = this;

            return new Promise(function (resolve, reject) {
                wx.onSocketOpen(function (response) {
                    _this.isConnected = true;
                    //this.ping();
                    resolve(response);
                });
                wx.onSocketError(function (error) {
                    if (_this.isConnected) {
                        _this.fire('error', error);
                    } else {
                        reject(error);
                    }
                });
                wx.onSocketMessage(function (message) {
                    return _this._handleMessage(message);
                });
                wx.onSocketClose(function (result) {
                    if (_this.isConnected) {
                        _this.fire('error', new Error("The websocket was closed by server"));
                    } else {
                        _this.fire('close');
                    }
                    _this.isConnected = false;
                    _this.destory();
                });
                wx.connectSocket({
                    url: url + '/?EIO=3&transport=websocket'
                });
            });
        }
    }, {
        key: 'ping',
        value: function ping() {
            var _this2 = this;

            setTimeout(function () {
                if (!_this2.isConnected) return;
                wx.sendSocketMessage({
                    data: [packets.ping, 'probe'].join('')
                });
            }, PING_CHECK_INTERVAL);
        }
    }, {
        key: 'close',
        value: function close() {
            var _this3 = this;

            return new Promise(function (resolve, reject) {
                if (_this3.isConnected) {
                    _this3.isConnected = false;
                    wx.onSocketClose(resolve);
                    wx.closeSocket();
                } else {
                    reject(new Error('socket is not connected'));
                }
            });
        }
    }, {
        key: 'emit',
        value: function emit(type) {
            for (var _len = arguments.length, params = Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
                params[_key - 1] = arguments[_key];
            }

            var data = [type].concat(params);
            wx.sendSocketMessage({
                data: [packets.message, events.EVENT, JSON.stringify(data)].join("")
            });
        }
    }, {
        key: 'destory',
        value: function destory() {
            this.removeAllListeners();
        }
    }, {
        key: '_handleMessage',
        value: function _handleMessage(_ref) {
            var data = _ref.data;

            var _$exec = /^(\d)(\d?)(.*)$/.exec(data),
                _$exec2 = _slicedToArray(_$exec, 4),
                match = _$exec2[0],
                packet = _$exec2[1],
                event = _$exec2[2],
                content = _$exec2[3];

            if (+event === events.EVENT) {
                switch (+packet) {
                    case packets.message:
                        var pack = void 0;
                        try {
                            pack = JSON.parse(content);
                        } catch (error) {
                            console.error('解析 ws 数据包失败：');
                            console.error(error);
                        }

                        var _pack = pack,
                            _pack2 = _toArray(_pack),
                            type = _pack2[0],
                            _params = _pack2.slice(1);

                        this.fire.apply(this, [type].concat(_toConsumableArray(_params)));
                        break;
                }
            } else if (+packet == packets.pong) {
                this.ping();
            }
        }
    }]);

    return WxSocketIO;
}();

;

emitter.setup(WxSocketIO.prototype);

module.exports = WxSocketIO;
});
