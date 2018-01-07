import QtQuick 1.1
import NestedWidget 1.0

Flickable{
        id: slideView
        property int show_width
        property int show_height
        property int cell_width
        property int cell_height
        property bool not_frozen: true
        width: show_width
        height: show_height
        contentHeight: cell_height
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
        function frozenState(bool){
            not_frozen = bool;
        }
        opacity: 1
        clip: true
        smooth: true
        interactive: not_frozen ? true : false
        anchors.fill: parent
        flickableDirection: Flickable.HorizontalAndVerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        Rectangle{
            objectName: "f_rec"
            anchors.fill: parent
            width: child_reg.width
            height: child_reg.height
            color: palette.window
            SystemPalette { id: palette }
            LittleWidgetItem{
            id: child_reg
        }
      }
}

