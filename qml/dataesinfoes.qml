import QtQuick 1.1

Rectangle {
    id: slide
    width: 600
    height: 64
    color: "gray"
    property bool undoemptyed: false
    property bool redoemptyed: false
    property bool editting: false
    property bool textempty: false
    property bool senddata: false
    signal hideReturnlastToWin(string content)
    signal sendUndoRedo(string un_re)
    signal sendToLeftContainer(string layer, string name)
    signal sendToRightContainer(string descri, bool state)
    signal sendResourceStateToWin(string layer, bool state)
    function grayUndoImage(bool){
        undoemptyed = bool;
    }
    function grayRedoImage(bool){
        redoemptyed = bool;
    }
    function assignTextToSender(label, send_str){
        sendToLeftContainer(label, send_str);
        return true;
    }
    ListView{
        id: slideView
        contentWidth: 600
        anchors.fill: parent
        boundsBehavior: Flickable.DragOverBounds
        cacheBuffer: 0
        orientation: ListView.Horizontal
        flickableDirection: Flickable.HorizontalFlick
        contentHeight: 64
        snapMode: ListView.SnapOneItem
        model: nested_model
        delegate: slideDelegate
    }
    ListModel {
        id: nested_model
        ListElement {
            title: "撤销"
        }
        ListElement {
            title: "重做"
        }
        ListElement {
            title: "冻结滑动"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/freezing.png"
        }
        ListElement {
            title: "清空选择"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/clear.png"
            nested: [
                    ListElement {
                        name: "清空内容"
                        grayed: true
                        picture: "/home/dapang/workstation/spc-tablet/images/unselect.png"
                    },
                    ListElement {
                        name: "清空格式"
                        grayed: false
                        picture: "/home/dapang/workstation/spc-tablet/images/styleclear.png"
                    },
                ListElement {
                    name: "取消选择"
                    picture: "/home/dapang/workstation/spc-tablet/images/clearselect.png"
                }
            ]
        }
        ListElement {
            title: "存储综合"
            picture: "/home/dapang/workstation/spc-tablet/images/dsave.png"
        }
        ListElement {
            title: "删除综合"
            picture: "/home/dapang/workstation/spc-tablet/images/dremove.png"
        }
        ListElement {
            title: "综合数据"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/datainfo.png"
            nested: [
                    ListElement {
                        name: "选取数据"
                        grayed: true
                        picture: "/home/dapang/workstation/spc-tablet/images/dataselect.png"
                    },
                    ListElement {
                        name: "暂停选取"
                        grayed: false
                        picture: "/home/dapang/workstation/spc-tablet/images/selectpause1.png"
                    }
            ]
        }
        ListElement {
            title: "字体大小"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/rate.png"
            nested: [ListElement {
                    name: "放大字体"
                    picture: "/home/dapang/workstation/spc-tablet/images/large.png"
                },
                ListElement {
                    name: "缩小字体"
                    picture: "/home/dapang/workstation/spc-tablet/images/little.png"
                }
            ]
        }
        ListElement {
            title: "文字颜色"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/color.png"
            nested: [
                ListElement {
                    name: "文字"
                    l_color: "darkGreen"},
                ListElement {
                    name: "文字"
                    l_color: "green"},
                ListElement {
                    name: "文字"
                    l_color: "gray"},
                ListElement {
                    name: "文字"
                    l_color: "red"},
                ListElement {
                    name: "文字"
                    l_color: "white"},
                ListElement {
                    name: "文字"
                    l_color: "blue"},
                ListElement {
                    name: "文字"
                    l_color: "cyan"},
                ListElement {
                    name: "文字"
                    l_color: "darkMagenta"},
                ListElement {
                    name: "文字"
                    l_color: "yellow"},
                ListElement {
                    name: "文字"
                    l_color: "darkRed"},
                ListElement {
                    name: "文字"
                    l_color: "black"},
                ListElement {
                    name: "文字"
                    l_color: "magenta"}
            ]
        }
        ListElement {
            title: "文字编辑"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/normal.png"
            nested: [
                ListElement {
                    name: "左对齐"
                    picture: "/home/dapang/workstation/spc-tablet/images/left.png"
                },
                ListElement {
                    name: "居中"
                    picture: "/home/dapang/workstation/spc-tablet/images/middle.png"
                },
                ListElement {
                    name: "右对齐"
                    picture: "/home/dapang/workstation/spc-tablet/images/right.png"
                }
            ]
        }
        ListElement {
            title: "表格显示"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/showhide.png"
            nested: [
                ListElement {
                     name: "行表头"
                     picture: "/home/dapang/workstation/spc-tablet/images/row-head.png"
                    },
                  ListElement {
                     name: "列表头"
                     picture: "/home/dapang/workstation/spc-tablet/images/col-head.png"
                    },
                  ListElement {
                     name: "行显示"
                     picture: "/home/dapang/workstation/spc-tablet/images/rowshow.png"
                    },
                  ListElement {
                     name: "行隐藏"
                     picture: "/home/dapang/workstation/spc-tablet/images/rowhide.png"
                    },
                  ListElement {
                     name: "列显示"
                     picture: "/home/dapang/workstation/spc-tablet/images/colshow.png"
                    },
                  ListElement {
                     name: "列隐藏"
                     picture: "/home/dapang/workstation/spc-tablet/images/colhide.png"
             }
            ]
        }
        ListElement {
            title: "表背景色"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/tablepaint.png"
            nested: [
                ListElement {
                    name: "表格"
                    l_color: "darkGreen"},
                ListElement {
                    name: "表格"
                    l_color: "green"},
                ListElement {
                    name: "表格"
                    l_color: "gray"},
                ListElement {
                    name: "表格"
                    l_color: "red"},
                ListElement {
                    name: "表格"
                    l_color: "white"},
                ListElement {
                    name: "表格"
                    l_color: "blue"},
                ListElement {
                    name: "表格"
                    l_color: "cyan"},
                ListElement {
                    name: "表格"
                    l_color: "darkMagenta"},
                ListElement {
                    name: "表格"
                    l_color: "yellow"},
                ListElement {
                    name: "表格"
                    l_color: "darkRed"},
                ListElement {
                    name: "表格"
                    l_color: "black"},
                ListElement {
                    name: "表格"
                    l_color: "magenta"}
            ]
        }
        ListElement {
            title: "单元尺寸"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/cellsize.png"
            nested:[
                ListElement {
                    name: "行高："
                },
                ListElement {
                    name: "列宽："
                }
            ]
        }
        ListElement {
            title: "单元合并"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/mergecells.png"
            nested:[
                ListElement {
                    name: "合并单元"
                    picture: "/home/dapang/workstation/spc-tablet/images/mergecell.png"
                },
                ListElement {
                    name: "取消合并"
                    picture: "/home/dapang/workstation/spc-tablet/images/splitcell.png"
                }
            ]
        }
        ListElement {
            title: "插删行列"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/insertcolums.png"
            nested: [
                ListElement {
                    name: "插入行"
                    picture: "/home/dapang/workstation/spc-tablet/images/insertrow.png"
                },
                ListElement {
                    name: "插入列"
                    picture: "/home/dapang/workstation/spc-tablet/images/insertcol.png"
                },
                ListElement {
                    name: "删除行"
                    picture: "/home/dapang/workstation/spc-tablet/images/deleterow.png"
                },
                ListElement {
                    name: "删除列"
                    picture: "/home/dapang/workstation/spc-tablet/images/deletecol.png"
                }
            ]
        }
        ListElement {
            title: "表格框"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/tframe.png"
            nested: [
                ListElement {
                    name: "粗匣框线"
                    picture: "/home/dapang/workstation/spc-tablet/images/thick.png"
                },
                ListElement {
                    name: "外侧框线"
                    picture: "/home/dapang/workstation/spc-tablet/images/thin.png"
               },
                ListElement {
                    name: "所有框线"
                    picture: "/home/dapang/workstation/spc-tablet/images/allframe.png"
               } ,
                ListElement {
                    name: "无框线"
                    picture: "/home/dapang/workstation/spc-tablet/images/noframe.png"
               },
                ListElement {
                    name: "左框线"
                    picture: "/home/dapang/workstation/spc-tablet/images/border- left.png"
               },
                ListElement {
                    name: "右框线"
                    picture: "/home/dapang/workstation/spc-tablet/images/border-right.png"
               },
                ListElement {
                    name: "上框线"
                    picture: "/home/dapang/workstation/spc-tablet/images/border-top.png"
               },
                ListElement {
                    name: "下框线"
                    picture: "/home/dapang/workstation/spc-tablet/images/border-bottom.png"
               }
            ]
        }
        ListElement {
            title: "pdf导出"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/pdf.png"
            nested: [ListElement {
                    name: "A3横向"
                    picture: "/home/dapang/workstation/spc-tablet/images/A3landscape.png"
                },
                ListElement {
                    name: "A3纵向"
                    picture: "/home/dapang/workstation/spc-tablet/images/A3portrait.png"
                },
                ListElement {
                    name: "A4横向"
                    picture: "/home/dapang/workstation/spc-tablet/images/A4landscape.png"
                },
                ListElement {
                     name: "A4纵向"
                     picture: "/home/dapang/workstation/spc-tablet/images/A4portrait.png"
                },
                ListElement {
                     name: "实际尺寸"
                     picture: "/home/dapang/workstation/spc-tablet/images/realsize.png"
                }
            ]
        }
        ListElement {
            title: "返回"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/return.png"
        }
        ListElement {
            title: "隐藏"
            picture: "/home/dapang/workstation/spc-tablet/images/hide.png"
        }
        ListElement {
            title: "取消"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/cancel.png"
        }
    }
    Component {
        id: slideDelegate
       Row {
            Rectangle{
                height: 64
                width: 55
                border.color: "#C4C4C4"
                gradient: Gradient {
                    GradientStop { position: 0.0; color: {if(title==="冻结滑动"||title==="清空选择"||title==="综合数据"||title==="字体大小"||title==="文字编辑"||title==="表格显示"||title==="单元尺寸"||title==="单元合并"||title==="插删行列"||title==="表格框"||title==="文字颜色"||title==="表背景色"||title==="pdf导出")
                                                        collapsed===false?"#AEEEEE":"#4A708B";
                                                    else
                                                        top_ma.pressed?"#AEEEEE":"#4A708B"
                        }
                    }
                   GradientStop { position: 0.66; color: {if(title==="冻结滑动"||title==="清空选择"||title==="综合数据"||title==="字体大小"||title==="文字编辑"||title==="表格显示"||title==="单元尺寸"||title==="单元合并"||title==="插删行列"||title==="表格框"||title==="文字颜色"||title==="表背景色"||title==="pdf导出")
                               collapsed===false?"#AEEEEE":"#292929";
                           else
                               top_ma.pressed?"#AEEEEE":"#292929"
                        }
                   }
                   GradientStop { position: 1.0; color: {if(title==="冻结滑动"||title==="清空选择"||title==="综合数据"||title==="字体大小"||title==="文字编辑"||title==="表格显示"||title==="单元尺寸"||title==="单元合并"||title==="插删行列"||title==="表格框"||title==="文字颜色"||title==="表背景色"||title==="pdf导出")
                               collapsed===false?"#AEEEEE":"#080808";
                           else
                               top_ma.pressed?"#AEEEEE":"#080808"
                        }
                   }
                }
                Text {
                        text: qsTr(title)
                        anchors.fill: parent
                        anchors.bottomMargin: 2
                        verticalAlignment: Text.AlignBottom
                        horizontalAlignment: Text.AlignHCenter
                        font.family: "宋体"
                        font.pointSize: 8
                        color: "white"
                    }
                Image{
                    id: top_img
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenterOffset: -8
                    anchors.verticalCenter: parent.verticalCenter
                    width: 40
                    height: 40
                    source: {
                        if (title==="撤销")
                           undoemptyed ? "/home/dapang/workstation/spc-tablet/images/undo.png" :"/home/dapang/workstation/spc-tablet/images/undogray.png" ;
                        else if (title==="重做")
                           redoemptyed ? "/home/dapang/workstation/spc-tablet/images/redo.png" : "/home/dapang/workstation/spc-tablet/images/redogray.png";
                        else
                            picture;
                    }
                }
                MouseArea {
                    id: top_ma
                anchors.fill: parent
                onClicked: {nested_model.setProperty(index, "collapsed", !collapsed);                    
                    if (title==="撤销" || title==="重做" || title==="存储综合"|| title==="删除综合")
                        sendUndoRedo(title)
                    if (title==="冻结滑动")
                        sendToLeftContainer(title, collapsed)
                    if (title==="返回" || title==="隐藏"|| title==="取消")
                        hideReturnlastToWin(title)
                    if (title==="综合数据")
                        sendResourceStateToWin(title, collapsed)
                }
            }
    }
            Loader {
            id: subslideLoader
            visible: !collapsed
            property variant subItemModel : nested
            sourceComponent: {
                if (title === "文字颜色" || title === "表背景色")
                    collapsed ? null : subverDelegate
                else if (title === "单元尺寸")
                    collapsed ? null : subeditorDelegate
                else
                    collapsed ? null : subslideDelegate
            }
            onStatusChanged: if (status == Loader.Ready)
                             {item.model = subItemModel}
            }
 }
    }
    Component {
    id: subslideDelegate
    Row {
    property alias model : subslideRepeater.model
    Repeater {
    id: subslideRepeater
    delegate: Rectangle {
    height: 64
    width: 55
    border.color: "#C4C4C4"
    gradient: Gradient {
        GradientStop { position: 0.0; color: sub_ma.pressed?"#AEEEEE":"#4A708B" }
        GradientStop { position: 0.66; color: sub_ma.pressed?"#AEEEEE":"#292929" }
        GradientStop { position: 1.0; color: sub_ma.pressed?"#AEEEEE":"#080808" }
    }
    Image{
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenterOffset: -8
        anchors.verticalCenter: parent.verticalCenter
        width: 40
        height: 40
        source: {
            if (name==="选取数据")
                grayed ? "/home/dapang/workstation/spc-tablet/images/dataselect.png" : "/home/dapang/workstation/spc-tablet/images/dataselect1.png";
            else if (name==="暂停选取")
                grayed ? "/home/dapang/workstation/spc-tablet/images/selectpause.png" : "/home/dapang/workstation/spc-tablet/images/selectpause1.png";
            else
                picture
        }
    }
    Text {
        text: qsTr(name)
        anchors.fill: parent
        anchors.bottomMargin: 2
        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignHCenter
        font.family: "宋体"
        font.pointSize: 8
        color: "white"
    }
    MouseArea {
        id: sub_ma
        anchors.fill: parent
        onClicked: {
                if (name==="选取数据" || name==="暂停选取")
                {
                    subslideRepeater.model.setProperty(index, "grayed", !grayed);
                     if (name==="选取数据")
                        subslideRepeater.model.setProperty(index+1, "grayed", !grayed)
                     else
                        subslideRepeater.model.setProperty(index-1, "grayed", !grayed)
                    sendToRightContainer(name, grayed)
                  }
                  else
                      sendToLeftContainer("subslideRepeater", name)
            }
          }
        }
      }
    }
}
    Component {
    id: subverDelegate
    Row {
    property alias model : subslideRepeater.model
    Repeater {
    id: subslideRepeater
    delegate: Rectangle {
        height: 64
        width: 55
        border.color: "#C4C4C4"
        color: l_color
        MouseArea {
            id: r_ma
            anchors.fill: parent
            onClicked: {
                sendToLeftContainer(name, l_color)
        }
}
}
 }
}
    }
    Component {
    id: subeditorDelegate
    Column {
    property alias model : subeditorRepeater.model
    Repeater {
    id: subeditorRepeater
    delegate: Rectangle {
        height: 32
        width: 131
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#4A708B" }
            GradientStop { position: 0.66; color: "#292929" }
            GradientStop { position: 1.0; color: "#080808" }
        }
            Text {
                id: label
                height: 32
                width: 48
                text: qsTr(name)
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                font.family: "宋体"
                font.pointSize: 8
                color: "white"
            }
            Rectangle {
                width: 40
                height: 32
                anchors.leftMargin: 45
                anchors.rightMargin: 45
                anchors.topMargin: 6
                anchors.bottomMargin: 6
                anchors.fill: parent
                anchors.margins: 1
                color: "black"
                border.color: "#66ffffff"
                radius: 5
            TextInput {
                id: edit_text
                anchors.fill: parent
                anchors.leftMargin: 5
                validator: IntValidator { bottom:0; top: 2000}
                color : "white"
                focus: editting ? true : false
                enabled: senddata ? assignTextToSender(name, text) : true
                text: !textempty ? "" : ""
                activeFocusOnPress: false
                MouseArea {
                    id: text_ma
                    anchors.fill: parent
                    onClicked: {
                        editting = true;
                        textempty = true;
                        if (!parent.activeFocus) {
                            parent.forceActiveFocus();
                            parent.openSoftwareInputPanel();
                        }
                    }
                    onDoubleClicked: {
                        editting = false;
                        sendToLeftContainer(name, edit_text.text);
                        parent.closeSoftwareInputPanel();
                    }
                }
            }
    }
            Rectangle {
                id: button
                width: 38
                height: 32
                anchors.right: parent.right
                gradient: Gradient {
                    GradientStop { position: 0.0; color: nest_ma.pressed?"#AEEEEE":"#4A708B"}
                    GradientStop { position: 0.66; color: nest_ma.pressed?"#AEEEEE":"#292929"}
                    GradientStop { position: 1.0; color: nest_ma.pressed?"#AEEEEE":"#080808"}
                   }
                Image {
                    width: 32
                    height: 32
                    source: {
                    if (name === "行高：")
                        "/home/dapang/workstation/spc-tablet/images/ok.png"
                    else
                        "/home/dapang/workstation/spc-tablet/images/cancel.png"
                    }
                    MouseArea {
                    id: nest_ma
                    anchors.fill: parent
                    onClicked: {
                        editting = false;
                        edit_text.closeSoftwareInputPanel();
                    }
                    onPressed: {
                        if (name === "行高："){
                            senddata = true;
                        }
                        else{ textempty = !textempty;
                             senddata = false;
                        }
                    }
                    onReleased: {senddata = false;}
                }
             }
          }
        }
      }
    }
  }
}
