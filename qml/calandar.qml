import QtQuick 1.1

Rectangle {
    width: show_width
    height: show_height
    border.color: "gray"
    border.width: 2
    radius: 10
    color: "black"
    property int show_width
    property int show_height
    property string time_title
    property int current_year
    property int current_month
    property int current_day
    property bool btn_disappear: false
    signal selectingOnRec(bool pressed)
    signal selectedYearMonth(int year, int month)
    signal endSelectedDate(int year, int month, int day)
    signal closeViewer()
    function initTimeTitle(title_str)
    {
        time_title = title_str
    }
    function initTimeArrange(from, to)
    {

    }
    function redefineRecSize(width, height)
    {
        show_width = width;
        show_height = height
    }
Row {
    anchors.fill: parent
    anchors.leftMargin: 5
    Item {
        width: 60
        height: 100
        Rectangle {
            y: 2
            width: 44
            height: 22
            radius: 11
            visible: !btn_disappear
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#4A708B" }
                GradientStop { position: 0.66; color: "#292929" }
                GradientStop { position: 1.0; color: "#4A708B" }
            }
            Text {
                color: "white"
                anchors.centerIn: parent
                text: "确定"
            }
            MouseArea {
                id: sure_ma
                anchors.fill: parent
                onClicked: {
                    btn_disappear = true;
                    endSelectedDate(current_year, current_month, current_day);
                    closeViewer()
                    }
                  }
          }
    Text {
        y: 40
        width: 60
        height: 20
        color: "white"
        text: time_title
      }
    Rectangle {
        y: 76
        width: 44
        height: 22
        radius: 11
        visible: !btn_disappear
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#4A708B" }
            GradientStop { position: 0.66; color: "#292929" }
            GradientStop { position: 1.0; color: "#4A708B" }
        }
        Text {
            color: "white"
            anchors.centerIn: parent
            text: "取消"
        }
        MouseArea {
            id: cancel_ma
            anchors.fill: parent
            onClicked: {
                btn_disappear = true;
                closeViewer()
                }
              }
      }
    }
    ListView {
        width: 45
        height: 100
        snapMode: ListView.SnapToItem
        model: YearsList
        delegate:
            Column {
                    Text {
                        x: 5
                    width: 45
                    height: 20
                    color: "white"
                    text: modelData
                }
            }
        MouseArea{
            anchors.fill: parent
            onPressed: {selectingOnRec(false)}
        }
        onMovementEnded: {
            selectingOnRec(true);
            current_year = Math.round(contentY/20);
            selectedYearMonth(current_year, current_month)
        }
      }
    Text {
        y: 40
        width: 16
        height: 20
        color: "white"
        text: "年"
      } 
    ListView {
        width: 20
        height: 100
        contentWidth: 20
        smooth: true
        contentHeight: 400
        snapMode: ListView.SnapToItem
        model: MonthsList
        delegate:
            Column {
                    Text {
                        x: 5
                    width: 20
                    height: 20
                    color: "white"
                    text: modelData
                }
            }
        MouseArea{
            anchors.fill: parent
            onPressed: {selectingOnRec(false)}
        }
        onMovementEnded: {
            selectingOnRec(true);
            current_month = Math.round(contentY/20);
            selectedYearMonth(current_year, current_month)
        }
       }
    Text {
        y: 40
        width: 16
        height: 20
        color: "white"
        text: "月"
      }
    ListView {
        width: 20
        height: 100
        snapMode: ListView.SnapOneItem
        model: DaysList
        delegate:
            Column {
                    Text {
                        x: 5
                    width: 20
                    height: 20
                    color: "white"
                    text: modelData
                }
            }
        MouseArea{
            anchors.fill: parent
            onPressed: {selectingOnRec(false)}
        }
        onMovementEnded: {
            selectingOnRec(true);
            current_day = Math.round(contentY/20)
        }
        }
    Text {
        y: 40
        width: 16
        height: 20
        color: "white"
        text: "日"
      }
    }
}

