import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

ApplicationWindow {
    id: window
    title: qsTr("p3d.in")
    width: 640
    height: 480
    color: "#f2f2f2"

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                text: qsTr("Update")
                onTriggered: Qt.openUrlExternally("https://db.tt/4RRONHzp");
            }
            MenuItem {
                text: qsTr("Exit")
                onTriggered: Qt.quit();
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        property real oldX: -1
        property real oldY: -1

        function move() {
            var dx = mouseX - oldX;
            var dy = mouseY - oldY;
            if(dx != 0.0 || dy != 0.0) {
                console.log("mouse move:", dx, dy)
                viewer.rotateCamera(oldX / width * 2 - 1, oldY / height * 2 - 1,
                                    mouseX / width * 2 - 1, mouseY / height * 2 - 1);
                oldX = mouseX;
                oldY = mouseY;
            }
        }

        onMouseXChanged: move()
        onMouseYChanged: move();
        onPressed: {
            console.log("mouse press:", mouseX, mouseY)
            oldX = mouseX;
            oldY = mouseY;
        }
        onReleased: {
            console.log("mouse release:", mouseX, mouseY)
        }
    }

    Rectangle {
        visible: false
        anchors.fill: parent
        color: "#f2f2f2"
    }


    TabView {
        visible: false
        anchors.fill: parent
        Tab {
            title: "Staff picks"

            Gallery {
                anchors.fill: parent
                galleryUrl: "http://p3d.in/api/p3d_models?staffPicks=1&allUsers=1&offset=0"
            }
        }

        Tab {
            title: "Popular"

            Gallery {
                anchors.fill: parent
                galleryUrl: "http://p3d.in/api/p3d_models?popular=1&allUsers=1&offset=0"
            }
        }

        Tab {
            title: "Newest"

            Gallery {
                anchors.fill: parent
                galleryUrl: "http://p3d.in/api/p3d_models?newest=1&allUsers=1&offset=0"
            }
        }
    }

}
