import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

RadioButton {

	// there can be an icon displayed as part of the label
	property string icon: ""

	property string indicatorColourEnabled: colour.radio_check_indicator_color
	property string indicatorColourDisabled: colour.radio_check_indicator_color_disabled
	property string indicatorBackgroundColourEnabled: colour.radio_check_indicator_bg_color
	property string indicatorBackgroundColourDisabled: colour.radio_check_indicator_bg_color_disabled
	property int fontsize: 10
	property string textColour: colour.text
	property string tooltip: text

	style: RadioButtonStyle {
		indicator: Rectangle {
			implicitWidth: 1.6*fontsize
			implicitHeight: 1.6*fontsize
			radius: 0.9*fontsize
			color: control.enabled ? indicatorBackgroundColourEnabled : indicatorBackgroundColourDisabled
			Behavior on color { ColorAnimation { duration: 150; } }
			Rectangle {
				anchors.fill: parent
				visible: control.checked
				color: control.enabled ? indicatorColourEnabled : indicatorColourDisabled
				radius: 0.9*fontsize
				anchors.margins: 0.4*fontsize
			}
		}
		label: Rectangle {
			color: "#00000000"
			implicitWidth: childrenRect.width
			implicitHeight: childrenRect.height
			Image {
				id: img
				x: 0
				y: 0
				width: (icon != "") ? 1.6*fontsize : 0
				height: (icon != "") ? 1.6*fontsize : 0
				source: icon
				visible: (icon != "")
			}
			Text {
				id: txt
				x: (icon != "") ? 1.8*fontsize : 0
				y: 0
				color: control.enabled ? textColour : colour.text_disabled
				Behavior on color { ColorAnimation { duration: 150; } }
				height: 1.6*fontsize
				font.pointSize: fontsize
				text: control.text
			}
		}
	}

	ToolTip {
		text: parent.tooltip
		anchors.fill: parent
		cursorShape: Qt.PointingHandCursor
		onClicked: parent.checked = true
	}

}
