import QtQuick 1.1
import NestedWidget 1.0

Flickable{
        id: slideView
        x: 0
        y: 0
        property int show_width
        property int show_height
        property int window_width
        property int window_height
        property string cell_name
        property bool not_frozen: true
        width: show_width
        height: show_height
        contentHeight: window_height
        contentWidth: window_width
        visible: true
        function initWholeSize(w_show, h_show, w_real, h_real){
            show_width =w_show;
            show_height = h_show;
            window_width = w_real;
            window_height = h_real
        }
        function resetRectangle(new_width, new_height){
            show_width = new_width;
            show_height = new_height;
        }
        function resetCellsSize(define_width, define_height){
            window_width = define_width;
            window_height = define_height;
        }
        function frozenState(bool){
            not_frozen = bool;
        }
        function dragPointChanging(pt_x,  pt_y, wid, heig){
            if (!not_frozen)
            {
                 if (pt_x  > 0  && pt_x  < 30)
                 {
                     if (contentX  < 0)
                         return
                     contentX  -= 10.0
                 }
                 if (pt_x+wid   > show_width-30  && pt_x+wid  < show_width)
                 {
                     if (contentX  >window_width-show_width)
                         return
                    contentX += 10.0
                 }
                 if (pt_y > 0  && pt_y < 30)
                {
                     if (contentY  < 0)
                         return
                    contentY -= 10.0
                 }
                 if (pt_y+heig  > show_height-30 && pt_y+heig  < show_height)
                 {
                     if (contentY  >window_height-show_height)
                         return
                    contentY += 10.0
                 }
             }
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
            width: child_reg.width
            height: child_reg.height
            color: palette.window
            SystemPalette { id: palette }
            LittleWidgetItem{
                 id: child_reg
            }
        }
}
