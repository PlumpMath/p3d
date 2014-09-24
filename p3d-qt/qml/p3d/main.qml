import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0

import p3d.p3dviewer 1.0

ApplicationWindow {
    id: window
    title: qsTr("p3d.in")
    width: 960
    height: 640
    color: "#f2f2f2"

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                text: qsTr("Open...")
                onTriggered: fileDialog.open();
            }
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

    FileDialog {
        id: fileDialog
        title: "Please choose a file"
        onAccepted: {
            console.log("You chose: " + fileDialog.fileUrl)
            viewer.loadModel(fileDialog.fileUrl)
        }
        onRejected: {
            console.log("Canceled")
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton |  Qt.MiddleButton
        property real oldX: -1
        property real oldY: -1

        function rotate(mx, my) {
            var dx = mx - oldX;
            var dy = my - oldY;
            /*if(dx != 0.0 || dy != 0.0)*/ {
                viewer.rotateCamera(mx / width * 2 - 1, -my / height * 2 + 1);
                oldX = mx;
                oldY = my;
            }
        }

        function zoom(mx, my) {
            var dx = mx - oldX;
            var dy = my - oldY;
            var dist = dy / height;
            viewer.zoomCamera(dist * 5);
            oldX = mx;
            oldY = my;
        }

        function pan(mx, my) {
            var dx = mx - oldX;
            var dy = my - oldY;
            viewer.panCamera(dx / height * 0.41, dy / height * 0.41);
            oldX = mx;
            oldY = my;
        }

        onPositionChanged: {
            if(mouse.buttons & Qt.LeftButton) {
                rotate(mouse.x, mouse.y);
            } else if(mouse.buttons & Qt.MiddleButton) {
                zoom(mouse.x, mouse.y);
            } else if(mouse.buttons & Qt.RightButton) {
                pan(mouse.x, mouse.y);
            }
        }
        onPressed: {
            oldX = mouseX;
            oldY = mouseY;
            if(mouse.buttons & Qt.LeftButton) {
                viewer.startRotateCamera(mouseX / width * 2 - 1, -mouseY / height * 2 + 1);
            }
        }
        onDoubleClicked: {
            viewer.resetCamera();
        }
        onWheel: {
            oldX = mouseX;
            oldY = mouseY;
            viewer.zoomCamera(wheel.angleDelta.y * 0.01);
        }
    }

    Rectangle {
        visible: viewer.modelState === Viewer.MS_READY
        width: 100
        height: 32
        color: "#888"

        Text {
            anchors.centerIn: parent
            text: "< BACK"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("clearModel")
                viewer.clearModel()
            }
        }
    }

    Rectangle {
        visible: viewer.modelState === Viewer.MS_NONE
        anchors.fill: parent
        color: "#f2f2f2"
    }


    TabView {
        visible: viewer.modelState === Viewer.MS_NONE
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
