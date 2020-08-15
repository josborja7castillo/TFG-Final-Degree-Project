/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.5 as QControls

Window {
    id: root
    visible: true
    width: Screen.width
    height: Screen.height
    color: "#161616"
    title: qsTr("Dashboard")
    FontLoader{id:localFont; source: "/fonts/bahnschrift.ttf"}



    MouseArea{
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: popup.open()
    }

    QControls.Popup{
        id: popup
        width: 0.33 * root.width
        height: root.height
        background: Rectangle{
            implicitHeight: parent.height
            implicitWidth: parent.width
            color: "#163A5E"
            border.color: "#EEE"
            border.width: 5
        }

        Label {
            ColumnLayout{
                y: 0.05*root.height
                spacing: 0.05*root.height
                anchors.fill: parent.width
                 CheckBox{
                     id: checkFuelLvl
                     Layout.alignment: Qt.AlignLeft
                     style: CheckBoxStyle{
                         label: Text{
                             color: "silver"
                             text: "Fuel Level Displayed"
                         }
                     }
                 }
                 CheckBox{
                     id: checkFuelRate
                     style: CheckBoxStyle{
                         label: Text{
                             color: "silver"
                             text: "Fuel Rate Displayed"
                         }
                     }
                     Layout.alignment: Qt.AlignLeft
                 }
                 CheckBox{
                     id: checkThrottleDigit
                     style: CheckBoxStyle{
                         label: Text{
                             color: "silver"
                             text: "Throttle Input Displayed"
                         }
                     }
                     Layout.alignment: Qt.AlignLeft
                 }
                 RowLayout{
                     Text{
                         color: "silver"
                         text: "Max RPMs"
                     }
                     SpinBox{
                         id: maxRpmSpinBox
                         minimumValue: 6
                         maximumValue: 10
                     }
                 }
            }//end of first ColumnLayout

            ColumnLayout{
                y: 0.5 * root.height
                id: consoleColumn
                spacing: 20
                Text {
                    id: consoleTitle
                    text: qsTr("Console info")
                    color: "white"
                }
                Rectangle{
                    height: 0.4 * root.height
                    width: 0.25 * root.width
                    border.width: 2
                    border.color: "white"
                    color: "black"
                    Text{
                           id:consoleText
                           objectName: "console"
                           anchors.left: parent.left
                           anchors.leftMargin: 5
                           anchors.right: parent.right
                           anchors.rightMargin: 5
                           anchors.top: parent.top
                           anchors.topMargin: 5
                           anchors.bottom: parent.bottom
                           anchors.bottomMargin: 5
                           font.pointSize: 10
                           color: "white"
                           text: "hello"
                           property string displayText: "value"
                    }//text end
                }//Rectangle end
            }//second ColumnLayout end
        }//Label end
        enter: Transition {
            NumberAnimation{property: "opacity"; from: 0.0; to:1.0}
        }
        closePolicy: popup.CloseOnPressOutside
        exit: Transition{
            NumberAnimation{property: "opacity"; from: 1.0; to: 0.0}
        }
    }

    CircularGauge{
        x: 0.5*(root.width - width)
        y: 0.5*(root.height - height)
        z: 1
        id:tachometer
        width: 0.7*(root.width>root.height?root.height:root.width)
        height: width
        maximumValue: maxRpmSpinBox.value
        value: 5500 / 1000
        anchors.horizontalCenter: parent.Center
        anchors.verticalCenter: parent.verticalCenter
        style: TachometerStyle {}
        Rectangle{

          width: parent.width
          height: parent.height
          radius: 0.5 * width
          border.width: 3
          border.color: "silver"
          color: "transparent"
          z: 2
        }

    }//tachometer gauge end /*
/*
    Rectangle{
        id: geargauge
        x: 0.15*root.width
        y: 0.15*root.height
        width: 0.45*(root.width>root.height?root.height:root.width)
        height: width
        radius: 0.5*width
        border.width: 2
        border.color: "silver"
        z: 2
        color: ((backend.valueRpm/1000) > (tachometer.maximumValue - 1) ) ? "red" : "transparent"
    }//gear gauge rectangle end

*/
    RowLayout
    {
        id: fuelLvlLayOut
        visible: checkFuelLvl.checked
        x: 265
        y: 315
        z: 3
        Text {
            id: fuelLvlValue
            font{family: localFont.name;pixelSize: 35}
            text: 50 / 100
            color: (lvlprcnt > 10) ? "green" : Qt.rgba(0.5, 0, 0, 1)
            property real lvlprcnt: 0.85 * 100
        } //fuelLvl value
        Text {
            id: fuelLvlText
            text: qsTr("%   FUEL LEVEL")
            font{family: localFont.name;pixelSize: 15}
            color: "darkgray"
        }//fuelLvl text
    }

    RowLayout
    {
        id: fuelRateLayOut
        visible: checkFuelRate.checked
        x: 265
        y: 395
        z: 3
        Text {
            id: fuelRateValue
            font{family: localFont.name;pixelSize: 35}
            text: "10"
            color: (rate < 10) ? "green" : Qt.rgba(0.5, 0, 0, 1)
            property real rate: 0.85 * 100
        } //fuelrate value
        Text {
            id: fuelRateText
            text: qsTr("L/h   FUEL RATE")
            font{family: localFont.name;pixelSize: 15}
            color: "darkgray"
        }//fuelRate text
    }

    RowLayout
    {
        id: throttleDigitLayOut
        visible: checkThrottleDigit.checked
        x: 265
        y: 475
        z: 3
        Text {
            id: throttleValue
            font{family: localFont.name;pixelSize: 35}
            text: 30 / 100
            color: (throttle > 10) ? "green" : Qt.rgba(0.5, 0, 0, 1)
            property real throttle: 0.5 * 100
        } //throttle value
        Text {
            id: throttleText
            text: qsTr("%   Throttle Position")
            font{family: localFont.name;pixelSize: 15}
            color: "darkgray"
        }//throttle text
    }//RowLayout end

    RowLayout
    {
        x: 0.45 * root.width
        y: root.height - 40
        Text {
            id: journeyText
            text: Qt.formatDateTime(new Date(),"ddd") //ddd MMM d //h:mm AP
            font.pixelSize: 20
            font.bold: false
            color: "darkgray"
        }
        Text {
            id: dayText //ddd "MMM d" //h:mm AP
            text: Qt.formatDateTime(new Date(),"MMM d")
            font.pixelSize: 20
            font.bold: true
            color: "darkgray"
        }
        Text {
            id: timeText
            text: Qt.formatDateTime(new Date(), "hh:mm")

            font.pixelSize: 25
            font.bold: true
            color: "white"
        }
        Timer{
            id: timer
            interval: 15000
            repeat: true
            running: true

            onTriggered:
            {
                timeText.text =  Qt.formatTime(new Date(),"hh:mm")
                dayText.text = Qt.formatDateTime(new Date(),"MMM d")
                journeyText.text = Qt.formatDateTime(new Date(),"ddd")
            }
        }
    }//RowLayout of Date and time

    Text {
        x: root.width - 150
        y: 0.04*root.width
        id: tempsText
        text: qsTr("TEMPS")
        font.pixelSize: 20
        font.italic: true
        color: "white"
    }
    Rectangle{
        x: root.width - 150
        y: 0.04*root.width
        width: 150
        height: 1
        color: "steelblue"
    }

        ProgressBar{
            x: root.width - 80
            y: 150
            id: coolantBar
            value: 80 / 100
            width: 50
            Layout.fillWidth: false
            style: ProgressBarStyle{
                background: Rectangle{
                    radius: 2
                    color: "gray"
                    border.color: "lightgray"
                    border.width: 1
                    implicitWidth: 200
                    implicitHeight: 24
                }
                progress: Rectangle {
                    color: (coolantBar.value < 0.85) ? "blue" : Qt.rgba(0.5, 0, 0, 1)
                    border.color: (coolantBar.value < 0.85) ? "steelblue" : Qt.rgba(0.5, 0, 0, 1)
                }
            }
        }
        Image {
            x: root.width - (coolantBar.width + 80)
            y: 130
            z: 4
            id: coolantImage
            source: "/images/engine-temperature.png"
            width: 50
            height: 50
            fillMode: Image.PreserveAspectFit
        }

        Image {
            x: root.width - (coolantBar.width + 85)
            y: 210
            id: oilImage
            source: "/images/oil-pressure.png"
            width: 50
            height: 50
            fillMode: Image.PreserveAspectFit
        }
        ProgressBar{
            x: root.width - 80
            y: 225
            id: oilTempBar
            value: 60 / 100
            width: 50
            Layout.fillWidth: false
            style: ProgressBarStyle{
                background: Rectangle{
                    radius: 2
                    color: "gray"
                    border.color: "lightgray"
                    border.width: 1
                    implicitWidth: 200
                    implicitHeight: 24
                }
                progress: Rectangle {
                    color: (oilTempBar.value < 0.85) ? "blue" : Qt.rgba(0.5, 0, 0, 1)
                    border.color: (oilTempBar.value < 0.85) ? "steelblue" : Qt.rgba(0.5, 0, 0, 1)
                }
            }
        }

        Text {
                z: 3
                id: gearText
                font{family: localFont.name;pixelSize: 140}
                text: "5"
                color: "white"
                horizontalAlignment: Text.AlignRight
                anchors.horizontalCenter: tachometer.horizontalCenter
                anchors.top: tachometer.top
                anchors.topMargin: 0.5*(tachometer.height - 100)
                visible: false
            }
        Text {
            z: 3
            text: "x1000"
            color: "white"
            font{family: localFont.name;pixelSize: 24}
            anchors.bottom: tachometer.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenter: tachometer.horizontalCenter
        }
        Text {
            z: 3
            id: speedText
            color: "white"
            font{family: localFont.name;pixelSize: 150}
            text: "100"
            anchors.top: tachometer.top
            anchors.bottom: tachometer.bottom
            //anchors.bottomMargin: 120
            anchors.horizontalCenter: tachometer.horizontalCenter
            anchors.topMargin: 175
        }
        Text {
            z: 3
            text: "km/h"
            color: "white"
            font{family: localFont.name;pixelSize: 20}
            anchors.top: tachometer.top
            anchors.horizontalCenter: tachometer.horizontalCenter
            anchors. topMargin: 0.5*tachometer.height - 25
        }

        Rectangle{
            z: 3
            color: "#ffffff"
            width: 180; height: 1;
            anchors.horizontalCenter: tachometer.horizontalCenter
            y: 0.5 * root.height
        }
        Row{
            id: outsideTempRow
            spacing: 15
            x: 0.5 * root.width + 100
            y: root.height - 50
        }
}// Window end
