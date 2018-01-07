import QtQuick 1.1

Rectangle{
    id: root
    property int current: 0
    property int show_width: 30
    property int show_height
    width: show_width
    height: show_height
    border.color: "gray"
    border.width: 2
    radius: 10
    color: "black"
    signal selectingOnRec(bool pressed)
    signal selectedContents(int content)
    function redefineRecSize(width, height)
    {
        show_width = width;
        show_height = height
    }
    ListView{
        id: slideView
        width: parent.width
        height: parent.height
        snapMode: ListView.SnapOneItem
        model: StringesList
        delegate: Column {
            width: show_width-6
            Text{
                text: modelData
                color: "white"
                font.pointSize : 14
                font.family: "Microsoft YaHei"
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        selectedContents(index);
                        parent.color = "#9ACD32"
                    }
                    onPressed: {selectingOnRec(false)}
                    onReleased: {selectingOnRec(true)}
                }
            }
        }
        highlight: Rectangle{
            visible: StringesList.count>4 ? true : false
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#4A708B" }
                GradientStop { position: 0.66; color: "#292929" }
                GradientStop { position: 1.0; color: "#4A708B" }
            }
            width: show_width
        }
        currentIndex: root.current
        onCurrentIndexChanged: root.current = currentIndex
        preferredHighlightBegin: 0
        preferredHighlightEnd: show_height
        highlightRangeMode: ListView.StrictlyEnforceRange
        focus: true
    }
}
