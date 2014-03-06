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
        property real oldX: -1
        property real oldY: -1

        function move() {
            var dx = mouseX - oldX;
            var dy = mouseY - oldY;
            /*if(dx != 0.0 || dy != 0.0)*/ {
                viewer.rotateCamera(mouseX / width * 2 - 1, -mouseY / height * 2 + 1);
                oldX = mouseX;
                oldY = mouseY;
            }
        }

        onMouseXChanged: move()
        onMouseYChanged: move();
        onPressed: {
            oldX = mouseX;
            oldY = mouseY;
            viewer.startRotateCamera(mouseX / width * 2 - 1, -mouseY / height * 2 + 1);
        }
        onDoubleClicked: {
            viewer.resetCamera();
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
