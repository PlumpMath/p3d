import QtQuick 2.1
import QtQuick.Controls 1.1
import "gallery.js" as Js
import QtQuick.Window 2.1

Rectangle {
    id: gallery
    property real dpiScaleFactor: (Screen.pixelDensity * 25.4 / 141.0 - 1) * 0.64 + 1

    color: "#f2f2f2"
    property url galleryUrl

    function fetch() {
        dataModel.clear();
        busy.running = true;
        Js.json(galleryUrl, function(err, data) {
            if(!err) {
                busy.running = false;
                for(var i = 0, il = data.p3d_models.length; i < il; ++i) {
                    dataModel.append(data.p3d_models[i]);
                }
            }
        });
    }

    ListModel {
        id: dataModel
    }

    BusyIndicator {
        id: busy
        anchors.centerIn: parent
    }

    Timer {
        id: refetch
        interval: 800
        running: false
        onTriggered: {
            if(!busy.running) {
                fetch();
            }
        }
    }

    GridView {
        id: modelList
        anchors.fill: parent
        clip: true
        model: dataModel
        cellWidth: width / Math.round(width / (224 * dpiScaleFactor))
        cellHeight: cellWidth

        onContentYChanged: {
            if(contentY < -100 * dpiScaleFactor) {
                if(!refetch.running) {
                    refetch.start();
                }
            } else {
                refetch.stop();
            }
        }

        delegate: Item {
            width: modelList.cellWidth
            height: modelList.cellHeight

            BorderImage {
                width: parent.width
                anchors.top: parent.top
                anchors.bottom: info.top
                source: "gallery_item_top.png"
                border.left: 10
                border.right: 10
                border.top: 15

                Image {
                    id: thumb
                    anchors.top: parent.top
                    anchors.topMargin: 9
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: height
                    source: 'http://p3d.in' + thumbnail
                    BusyIndicator {
                        visible: parent.status === Image.Loading
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            console.log("Click: " + name);
                            Qt.openUrlExternally('http://p3d.in/' + shortid);
                        }
                    }
                }
            }

            BorderImage {
                id: info
                width: parent.width
                height: textColumn.height + 12 * dpiScaleFactor
                anchors.bottom: footer.top
                source: "gallery_item_center.png"
                border.top: 2
                border.left: 10
                border.right: 10

                Column {
                    id: textColumn
                    anchors.left: parent.left
                    anchors.leftMargin: 12 * dpiScaleFactor
                    anchors.right: parent.right
                    anchors.rightMargin: 12 * dpiScaleFactor
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        font.pointSize: 13
                        width: parent.width
                        color: "#04a4d6"
                        text: name
                        elide: Text.ElideRight
                    }

                    Row {
                        width: parent.width

                        Text {
                            font.pointSize: 11
                            text: "by "
                            color: "#232323"
                        }

                        Text {
                            font.pointSize: 11
                            text: owner
                            elide: Text.ElideRight
                            color: "#04a4d6"
                        }
                    }

                }
            }

            BorderImage {
                id: footer
                width: parent.width
                height: footerRow.height + 8 * dpiScaleFactor
                anchors.bottom: parent.bottom
                source: "gallery_item_bottom.png"
                border.left: 10
                border.right: 10
                border.bottom: 15

                Row {
                    id: footerRow
                    height: 28 * dpiScaleFactor
                    anchors.left: parent.left
                    anchors.leftMargin: 12 * dpiScaleFactor

                    Image {
                        width: 26 * dpiScaleFactor
                        height: 26 * dpiScaleFactor
                        anchors.verticalCenter: parent.verticalCenter
                        source: "icon_views.png"
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        font.pointSize: 11
                        text: hits
                        color: "#232323"
                    }
                }
            }


        }
    }

    Component.onCompleted: fetch()

}
