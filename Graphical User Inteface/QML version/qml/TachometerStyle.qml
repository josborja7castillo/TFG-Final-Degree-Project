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
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import QtGraphicalEffects 1.0


CircularGaugeStyle{

    id: tachometerStyle
    tickmarkStepSize: 1
    labelStepSize: 1

    //"Ticks" between values
    tickmarkInset: toPixels(0.04)
    minorTickmarkInset: tickmarkInset
    labelInset: toPixels(0.23)


    //properties
    property real xCenter: outerRadius
    property real yCenter: outerRadius
    property real needleLength: 0.75 * outerRadius - tickmarkInset * 2
    property real needleTipWidth: toPixels(0.02)
    property real needleBaseWidth: toPixels(0.06)
    property bool halfGauge: false

    function toPixels(percentage){
        return percentage * outerRadius;
    }

    function degToRad(degrees){
        return degrees * (Math.PI / 180);
    }

    function radToDeg(radians){
        return radians * (180 / Math.PI);
    }

    function paintBackground(ctx){
        if(halfGauge){
            ctx.beginPath();
            ctx.rect(0, 0, ctx.canvas.width, ctx.canvas.height / 2);
            ctx.clip();
        }

        //I think here it's background painting
        ctx.beginPath();
        ctx.fillStyle = "black";
        ctx.ellipse(0, 0, ctx.canvas.width, ctx.canvas.height);
        ctx.fill();

        //tickmarks
        ctx.beginPath();
        ctx.lineWidth = tickmarkInset;
        ctx.strokeStyle = "black";
        ctx.arc(xCenter, yCenter, outerRadius - ctx.lineWidth / 2, outerRadius - ctx.lineWidth / 2, 0, Math.PI * 2);
        ctx.stroke();

    }

    background: Canvas {
            RadialGradient{
                anchors.fill: parent
                gradient: Gradient{
                    GradientStop{position: 0.0; color: "#163A5E"}
                    GradientStop{position: 0.5; color: "#161616"}
                }
            }

            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();
                paintBackground(ctx);
                //new path for:
                ctx.beginPath();
                ctx.lineWidth = tachometerStyle.toPixels(0.08);
                ctx.strokeStyle = Qt.rgba(0.5, 0, 0, 1);
                var warningCircumference = maximumValueAngle - minimumValueAngle * 0.1;
                var startAngle = maximumValueAngle - 83;
                ctx.arc(outerRadius, outerRadius,
                                // Start the line in from the decorations, and account for the width of the line itself.
                                outerRadius - tickmarkInset - ctx.lineWidth / 2,
                                degToRad(startAngle - angleRange / 8 + angleRange * 0.015),
                                degToRad(startAngle - angleRange * 0.025), false);
                ctx.stroke();

            }
    }

    tickmark: Rectangle {
            implicitWidth: toPixels(0.02)
            antialiasing: true
            implicitHeight: toPixels(0.08)
            color: styleData.index === tachometer.maximumValue || styleData.index === tachometer.maximumValue - 1 ? Qt.rgba(0.5, 0, 0, 1) : "#c8c8c8" //modify to set max rpm
    }

    minorTickmark: Item {
        implicitWidth: 8
        implicitHeight: 6

        Rectangle {
            color: styleData.index >= (4*(tachometer.maximumValue-1) )  ? Qt.rgba(0.5, 0, 0, 1) : "#c8c8c8" //modify to set max rpm
            anchors.fill: parent
            anchors.leftMargin: 2
            anchors.rightMargin: 4
        }
    }

    tickmarkLabel: Text {
        //font.pixelSize: Math.max(6, toPixels(0.12))
        font{family: localFont.name;pixelSize: tachometerStyle.toPixels(0.14)}
        text: styleData.value
        color: styleData.index === tachometer.maximumValue || styleData.index === tachometer.maximumValue - 1 ? Qt.rgba(0.5, 0, 0, 1) : "#c8c8c8" //modify to set max rpm
        antialiasing: true
    }

    needle: Canvas {
            implicitWidth: needleBaseWidth
            implicitHeight: needleLength

            property real xCenter: width / 2 + width / 4
            property real yCenter: height / 2 + height / 4



            onPaint: {
                ///needle drawings

                var ctx = getContext("2d");
                ctx.reset();
                ctx.beginPath();
                ctx.moveTo(xCenter, height);
                ctx.lineTo(xCenter - needleBaseWidth / 2, height - needleBaseWidth / 2);
               // ctx.lineTo(xCenter - needleBaseWidth / 2, 0);
                ctx.lineTo(xCenter - needleTipWidth / 2, 0);
                ctx.lineTo(xCenter, yCenter - needleLength);
                ctx.lineTo(xCenter, 0);
                ctx.closePath();
                //ctx.fillStyle = Qt.rgba(0.247, 0.5, 0.75, 0.66); //color
               // ctx.fillStyle = Qt.rgba(0, 0, 0, 0.66); //color
                var gradient = ctx.createLinearGradient(0,0,needleLength,needleBaseWidth);
                //gradient.addColorStop(0.0,"#000000");
                gradient.addColorStop(0.5,"#FFFFFF");
                ctx.fillStyle = gradient;
                ctx.fill();

                ctx.beginPath();
                ctx.moveTo(xCenter, height)
                ctx.lineTo(width, height - needleBaseWidth / 2);
                ctx.lineTo(xCenter - needleTipWidth /2, 0);
                ctx.lineTo(xCenter, 0);
                ctx.closePath();
                //ctx.fillStyle = Qt.lighter(Qt.rgba(0.247, 0.5, 0.75, 0.66)); //color
               // var gradient = ctx.createLinearGradient(0,0,needleLength,needleBaseWidth);
                //gradient.addColorStop(0.0,"#000000");

                //gradient.addColorStop(0.5,"#FFFFFF");
                ctx.fillStyle = gradient;
                ctx.fill();
                /*ctx.beginPath();
                var gradient = ctx.createRadialGradient(0,tachometer.height, outerRadius * 0.4,Math.cos(needleRotation + 145)*needleLength,Math.sin(needleRotation + 145)*needleLength, outerRadius *0.4);
                gradient.addColorStop(0.5, "#81FFFE");   //oben
                gradient.addColorStop(0.46, "#81FFFE");   //oben
                gradient.addColorStop(0.45, "#112478");   //mitte
                gradient.addColorStop(0.33, "transparent");   //unten
                //ctx.lineWidth = 150;
                ctx.fillStyle = gradient;
                ctx.arc(xCenter, yCenter, tachometer.radius - (ctx.lineWidth / 2), angleOffset, angleOffset + angle);
                ctx.fill();*/


            }
        }


    foreground: null
}











/*
DashboardGaugeStyle {
    id: tachometerStyle
    tickmarkStepSize: 1
    labelStepSize: 1
    needleLength: toPixels(0.85)
    needleBaseWidth: toPixels(0.02)
    needleTipWidth: toPixels(0.08)

    tickmark: Rectangle {
        implicitWidth: toPixels(0.02)
        antialiasing: true
        implicitHeight: toPixels(0.08)
        color: styleData.index === tachometer.maximumValue || styleData.index === tachometer.maximumValue - 1 ? Qt.rgba(0.5, 0, 0, 1) : "#c8c8c8" //modify to set max rpm
    }

    minorTickmark: Item {
        implicitWidth: 8
        implicitHeight: 6

        Rectangle {
            color: styleData.index >= (4*(tachometer.maximumValue-1) )  ? Qt.rgba(0.5, 0, 0, 1) : "#c8c8c8" //modify to set max rpm
            anchors.fill: parent
            anchors.leftMargin: 2
            anchors.rightMargin: 4
        }
    }

    tickmarkLabel: Text {
        //font.pixelSize: Math.max(6, toPixels(0.12))
        font{family: localFont.name;pixelSize: tachometerStyle.toPixels(0.14)}
        text: styleData.value
        color: styleData.index === tachometer.maximumValue || styleData.index === tachometer.maximumValue - 1 ? Qt.rgba(0.5, 0, 0, 1) : "#c8c8c8" //modify to set max rpm
        antialiasing: true
    }


    background: Canvas {
       RadialGradient{
            anchors.fill: parent
            gradient: Gradient{
                GradientStop{position: 0.0; color: "#163A5E"}
                GradientStop{position: 0.5; color: "#161616"}
            }
        }
        OpacityMask{
            anchors.fill: tachometer
            smooth: true
            visible: true
            source: tachometer
            maskSource: tachometer
            invert: false
        }
        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            paintBackground(ctx);
            ctx.beginPath();
            ctx.lineWidth = tachometerStyle.toPixels(0.08);
            ctx.strokeStyle = Qt.rgba(0.5, 0, 0, 1);
            var warningCircumference = maximumValueAngle - minimumValueAngle * 0.1;
            var startAngle = maximumValueAngle - 83;
            //var startAngle = (tachometer.maximumValue);
            ctx.arc(outerRadius, outerRadius,
                // Start the line in from the decorations, and account for the width of the line itself.
                outerRadius - tickmarkInset - ctx.lineWidth / 2,
                degToRad(startAngle - angleRange / 8 + angleRange * 0.015),
                degToRad(startAngle - angleRange * 0.025), false);
            ctx.stroke();


        }

       /* Text {
            id: rpmText
            font.pixelSize: tachometerStyle.toPixels(0.3)
            text: rpmInt
            color: "white"
            horizontalAlignment: Text.AlignRight
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.verticalCenter
            anchors.topMargin: 20

            readonly property int rpmInt: valueSource.rpm
        }*/
      /*  Text {
            id: gearText
            font{family: localFont.name;pixelSize: tachometerStyle.toPixels(0.3)}
            text: gearInt
            color: "white"
            horizontalAlignment: Text.AlignRight
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.verticalCenter
            anchors.bottomMargin: 40

            readonly property string gearInt: valueSource.gear
        }
        Text {
            text: "x1000"
            color: "white"
            font{family: localFont.name;pixelSize: tachometerStyle.toPixels(0.1)}
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
           // anchors.leftMargin: -80
        }
     /*   Text {
            text: "RPM"
            color: "white"
            font{family: localFont.name;pixelSize: tachometerStyle.toPixels(0.1)}
            anchors.top: gearText.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }
*/
    /*    Text {
            id: speedText
            color: "white"
            font{family: localFont.name;pixelSize: tachometerStyle.toPixels(0.4)}
            text: speedInt
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 120
            anchors.horizontalCenter: parent.horizontalCenter
           // anchors.leftMargin: -80

           readonly property int speedInt: valueSource.kph
        }
        Text {
            text: "km/h"
            color: "white"
            font{family: localFont.name;pixelSize: tachometerStyle.toPixels(0.1)}
            anchors.top: speedText.bottom
            anchors.horizontalCenter: parent.horizontalCenter   //--> to be implemented
        }
        Rectangle{
            color: "#ffffff"
            y:200; width: 120; height: 1;
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.Center
        }


    }
}
*/

