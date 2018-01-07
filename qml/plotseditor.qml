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
    property bool modify_enabled: false
    property bool area_enabled: false
    property bool dl_enabled: false
    property bool ctrl_enabled: false
    property bool ds_enabled: false
    property bool ts_enabled: false
    property bool dts_init: false
    property bool related_press: false
    signal hideReturnlastToWin(string content)
    signal sendUndoRedo(string un_re)
    signal sendToLeftContainer(string label, string con)
    signal sendToRightContainer(string label, bool state)
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
    function btnsEnable(modify, area, area_state, dl, dl_state, ctrl, ctrl_state, ds, ts, dts_state){
        modify_enabled = modify;
        area_enabled = area;
        dl_enabled = dl;
        ctrl_enabled = ctrl;
        ds_enabled = ds;
        ts_enabled = ts;
        dts_init = dts_state;
        nested_model.get(7).nested.setProperty(1, "areaed", area_state)
        nested_model.get(7).nested.setProperty(2, "ctrled", ctrl_state)
        nested_model.get(7).nested.setProperty(3, "dotlined", dl_state)
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
            title: "放大显示"
            picture: "/home/dapang/workstation/spc-tablet/images/zoom+.png"
        }
        ListElement {
            title: "缩小显示"
            picture: "/home/dapang/workstation/spc-tablet/images/zoom-.png"
        }
        ListElement {
            title: "存储图"
            picture: "/home/dapang/workstation/spc-tablet/images/dsave.png"
        }
        ListElement {
            title: "删除图"
            picture: "/home/dapang/workstation/spc-tablet/images/dremove.png"
        }
        ListElement {
            title: "图数据源"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/plotres.png"
            nested: [ListElement {
                    name: "编制新图"
                    grayed: true
                    picture: "/home/dapang/workstation/spc-tablet/images/mplots1.png"
                },
                ListElement {
                    name: "取消编制"
                    grayed: false
                    picture: "/home/dapang/workstation/spc-tablet/images/pdelete1.png"
                }
            ]
        }
        ListElement {
            title: "显示格式"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/pstyle.png"
            nested: [
                ListElement {
                    name: "修改标题"
                    grayed: true
                    picture: "/home/dapang/workstation/spc-tablet/images/discription.png"
                },
                ListElement {
                    name: "分区显示"
                    areaed: true
                    picture: "/home/dapang/workstation/spc-tablet/images/areaes.png"
                },
                ListElement {
                    name: "控制线"
                    ctrled: true
                    picture: "/home/dapang/workstation/spc-tablet/images/lcontrol.png"
                },
                ListElement {
                    name: "点图线"
                    dotlined: true
                    picture: "/home/dapang/workstation/spc-tablet/images/ptline.png"
                },
                ListElement {
                    name: "点序坐标"
                    grayed: true
                    picture: "/home/dapang/workstation/spc-tablet/images/numbers.png"
                },
                ListElement {
                    name: "时序坐标"
                    grayed: true
                    picture: "/home/dapang/workstation/spc-tablet/images/ltime.png"
                }
            ]
        }
        ListElement {
            title: "单元尺寸"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/viewscale.png"
            nested:[
                ListElement {
                    name: "图宽："
                },
                ListElement {
                    name: "图高："
                }
            ]
        }
        ListElement {
            title: "图排列"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/pdistribution.png"
            nested:[
                ListElement {
                    name: "行数："
                },
                ListElement {
                    name: "列数："
                }
            ]
        }
        ListElement {
            title: "合并列"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/col-merge.png"
            nested:[
                ListElement {
                    name: "列从："
                },
                ListElement {
                    name: "列到："
                }
            ]
        }
        ListElement {
            title: "拆分列"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/col-split.png"
        }
        ListElement {
            title: "图背景色"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/backcolor.png"
            nested: [
                ListElement {
                    name: "图背景"
                    l_color: "darkGreen"},
                ListElement {
                    name: "图背景"
                    l_color: "SteelBlue"},
                ListElement {
                    name: "图背景"
                    l_color: "SlateGrey"}
            ]
        }
        ListElement {
            title: "移动图"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/plot-move.png"
        }
        ListElement {
            title: "导出图"
            collapsed: true
            picture: "/home/dapang/workstation/spc-tablet/images/pexport.png"
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
                    GradientStop { position: 0.0; color: {if(title==="图数据源"||title==="显示格式"||title==="单元尺寸"||title==="图排列"||title==="合并列"||title==="图背景色"||title==="移动图")
                                                        collapsed===false?"#AEEEEE":"#4A708B";
                                                    else
                                                        top_ma.pressed?"#AEEEEE":"#4A708B"
                        }
                    }
                   GradientStop { position: 0.66; color: {if(title==="图数据源"||title==="显示格式"||title==="单元尺寸"||title==="图排列"||title==="合并列"||title==="图背景色"||title==="移动图")
                               collapsed===false?"#AEEEEE":"#292929";
                           else
                               top_ma.pressed?"#AEEEEE":"#292929"
                        }
                   }
                   GradientStop { position: 1.0; color: {if(title==="图数据源"||title==="显示格式"||title==="单元尺寸"||title==="图排列"||title==="合并列"||title==="图背景色"||title==="移动图")
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
                    if (title==="撤销" || title==="重做" || title==="存储图" || title==="删除图")
                        sendUndoRedo(title)
                    if (title==="返回"|| title==="隐藏" || title==="取消")
                        hideReturnlastToWin(title);
                    if (title==="放大显示" || title==="缩小显示")
                    {
                        if (title==="放大显示")
                            sendToLeftContainer(title, true)
                        else
                            sendToLeftContainer(title, false)
                    }
                    if (title==="图数据源" || title==="显示格式" || title==="图背景色")
                        sendResourceStateToWin(title, collapsed);
                    if (title==="拆分列" || title==="移动图" || title==="导出图")
                        sendToLeftContainer(title, collapsed)
                }
            }
    }
            Loader {
            id: subslideLoader
            visible: !collapsed
            property variant subItemModel : nested
            sourceComponent: {             
               if (title === "图背景色")
                    collapsed ? null : subverDelegate
                else if (title === "合并列" || title === "单元尺寸" || title === "图排列")
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
        GradientStop { position: 0.0; color: sub_ma.pressed&&related_press ? "#AEEEEE" : "#4A708B" }
        GradientStop { position: 0.66; color: sub_ma.pressed&&related_press ? "#AEEEEE" : "#292929" }
        GradientStop { position: 1.0; color: sub_ma.pressed&&related_press ? "#AEEEEE" : "#080808" }
    }
    Image{
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenterOffset: -8
        anchors.verticalCenter: parent.verticalCenter
        width: 40
        height: 40
        source: {
            if (name==="编制新图")
                grayed ? "/home/dapang/workstation/spc-tablet/images/mplots.png" : "/home/dapang/workstation/spc-tablet/images/mplots1.png";
            else if (name==="取消编制")
                grayed ? "/home/dapang/workstation/spc-tablet/images/pdelete.png" : "/home/dapang/workstation/spc-tablet/images/pdelete1.png";
            else if (name==="修改标题")
                modify_enabled ? "/home/dapang/workstation/spc-tablet/images/discription.png" : "/home/dapang/workstation/spc-tablet/images/discription2.png";
            else if (name==="分区显示")
                areaed&&area_enabled ? "/home/dapang/workstation/spc-tablet/images/areaes.png" : "/home/dapang/workstation/spc-tablet/images/areaes2.png"
            else if (name==="控制线")
                ctrled&&ctrl_enabled  ? "/home/dapang/workstation/spc-tablet/images/lcontrol.png" : "/home/dapang/workstation/spc-tablet/images/lcontrol2.png"
            else if (name==="点图线")
                dotlined&&dl_enabled ? "/home/dapang/workstation/spc-tablet/images/ptline.png" : "/home/dapang/workstation/spc-tablet/images/ptline2.png"
            else if (name==="点序坐标")
                dts_init&&ds_enabled ? "/home/dapang/workstation/spc-tablet/images/numbers.png" : "/home/dapang/workstation/spc-tablet/images/numbers2.png";
            else if (name==="时序坐标")
                grayed&&!dts_init&&ts_enabled ? "/home/dapang/workstation/spc-tablet/images/ltime.png" : "/home/dapang/workstation/spc-tablet/images/ltime2.png";
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
        onPressed: {
            if ((name==="修改标题"&&!modify_enabled) || (name==="分区显示"&&!area_enabled) || (name==="点图线"&&!dl_enabled) || (name==="控制线"&&!ctrl_enabled) || (name==="点序坐标"&&!ds_enabled) || (name==="时序坐标"&&!ts_enabled))
                related_press = false
            else
                related_press = true
        }
        onClicked: {
            if (name==="编制新图" || name==="取消编制" || (name==="分区显示"&&area_enabled) || (name==="点图线"&&dl_enabled) || (name==="控制线"&&ctrl_enabled) || (name==="点序坐标"&&ds_enabled) || (name==="时序坐标"&&ts_enabled))
            {
                if (name==="编制新图" || name==="取消编制" || name==="点序坐标" || name==="时序坐标")
                {
                    subslideRepeater.model.setProperty(index, "grayed", !grayed);
                    if (name==="编制新图" || name==="取消编制")
                    {
                        if (name==="编制新图")
                            subslideRepeater.model.setProperty(index+1, "grayed", !grayed);
                        else
                            subslideRepeater.model.setProperty(index-1, "grayed", !grayed);
                        sendToRightContainer(name, grayed);
                        return
                    }
                    else if (name==="点序坐标")
                    {
                        dts_init = grayed;
                        subslideRepeater.model.setProperty(index+1, "grayed", !grayed)
                    }
                    else
                    {
                        dts_init = !grayed;
                        subslideRepeater.model.setProperty(index-1, "grayed", !grayed);
                    }
                 }
                else if (name==="分区显示")
                    subslideRepeater.model.setProperty(index, "areaed", !areaed);
                else if (name==="控制线")
                    subslideRepeater.model.setProperty(index, "ctrled", !ctrled);
                else if (name==="点图线")
                    subslideRepeater.model.setProperty(index, "dotlined", !dotlined);
                sendToLeftContainer("subslideRepeater", name)
            }
            else
            {
                subslideRepeater.model.setProperty(index, "grayed", !grayed);
                sendToLeftContainer("subslideRepeater", name)
            }
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
        gradient: Gradient {
            GradientStop { position: 0.0; color: r_ma.pressed?"#AEEEEE": l_color }
            GradientStop { position: 0.66; color: r_ma.pressed?"#AEEEEE": l_color }
            GradientStop { position: 1.0; color: r_ma.pressed?"#AEEEEE": l_color }
        }
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
                    if (name === "图宽：" ||  name === "行数：" ||  name === "列从：")
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
                        if (name === "图宽：")
                        {
                            senddata = true
                            textempty = false
                         }
                       else if (name === "行数：")
                        {
                            senddata = true
                            textempty = false
                         }
                        else if (name === "列从：")
                         {
                             senddata = true
                             textempty = false
                          }
                        else
                        { textempty = false;
                            senddata = false
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
