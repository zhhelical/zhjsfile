import QtQuick 1.1
import NestedWidget 1.0

Flickable{
        id: slideView
        x: 0
        y: 0
        property int show_width
        property int show_height
        property int cell_width
        property int cell_heigh
        width: show_width
        height: show_height
        contentHeight: cell_heigh
        contentWidth: cell_width
        visible: true
        function initWholeSize(e_width, e_height, define_width, define_height){
            show_width = e_width;
            show_height = e_height;
            cell_width = define_width;
            cell_heigh = define_height;
        }
        function resetRectangle(new_width, new_height){
            show_width = new_width;
            show_height = new_height;
        }
        function resetCellsSize(define_width, define_height){
            cell_width = define_width;
            cell_height = define_height;
        }
        opacity: 1
        clip: true
        smooth: true
        interactive: true
        anchors.fill: parent
        flickableDirection: Flickable.HorizontalAndVerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        Rectangle{
            objectName: "f_rec"
            width: child_reg.width
            height: child_reg.height
            color: palette.window
            SystemPalette { id: palette }
            LittleWidgetItem{
            id: child_reg
        }
    }
}

