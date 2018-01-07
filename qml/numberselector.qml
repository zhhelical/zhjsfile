import QtQuick 1.1

Rectangle {
    id: root
    width: show_width
    height: show_height
    border.color: "gray"
    border.width: 2
    radius: 5
    color: "black"
    property int current: 0
    property int rec_current: 0
    property int show_width
    property int show_height
    property double move_percent
    signal selectingOnRec(bool pressed)
    signal realWidthReturn(int new_width)
    signal selectedContents(string content)
    signal closeViewer()
    function redefineRecSize(width, height)
    {
        show_width = width;
        show_height = height
    }
    function defineModelListPos(percent)
    {
        current = slideView.count*percent;
        slideView.positionViewAtIndex(current, ListView.Visible)
    }
    Rectangle
    {
        id: sliderBar
        x: 20
        width: 16
        height: show_height
        radius: 2
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#4A708B" }
            GradientStop { position: 0.66; color: "#292929" }
            GradientStop { position: 1.0; color: "#4A708B" }
        }
        smooth: true
        Rectangle
        {
            id: slider
            x: -12
            y: show_height * slideView.contentY / slideView.contentHeight
            width: 40
            height: 30
            radius: 3
            border.color: "gray"
            gradient: Gradient {
                GradientStop { position: 0.0; color: "lightgray" }
                GradientStop { position: 1.0; color: "gray" }
            }
            smooth: true
            MouseArea
            {
                anchors.fill: parent
                preventStealing: true
                drag.target: parent
                drag.axis: Drag.YAxis
                drag.minimumY: 0
                drag.maximumY: sliderBar.height - 20
                onPressed: {selectingOnRec(false)}
                onReleased: {selectingOnRec(true)}
                onPositionChanged: {
                    move_percent = parent.y / (sliderBar.height-20);
                    defineModelListPos(move_percent )
                }
            }
        }
    }
    ListView{
        id: slideView
        x: 80
        width: show_width-80
        height: show_height
        snapMode: ListView.SnapOneItem
        model: StringesList
        delegate: Column {
           width: show_width-80
           Text{
               text: modelData
               color: "white"
               font.pointSize : 20
               font.family: "Microsoft YaHei"
               MouseArea{
                   anchors.fill: parent
                   onClicked: {
                       selectedContents(parent.text);
                       parent.color = "#9ACD32"
                    }
                    onPressed: {selectingOnRec(false)}
                    onReleased: {selectingOnRec(true)}
             }
         }
     }
      currentIndex: root.current
      onCurrentIndexChanged: root.current = currentIndex
      preferredHighlightBegin: 0
      preferredHighlightEnd: show_height
      highlightRangeMode: ListView.StrictlyEnforceRange
      flickableDirection: Flickable.VerticalFlick
      boundsBehavior: Flickable.StopAtBounds
      focus: true
   }
}

