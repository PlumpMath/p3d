import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

ApplicationWindow {
    id: window
    title: qsTr("Hello World")
    width: 640
    height: 480

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

    Rectangle {
        anchors.fill: parent
        color: "#f2f2f2"
    }


    TabView {
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
