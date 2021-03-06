import QtQuick 2.3

Rectangle {

	id: detect

	property int detectWidth: 400
	property int detectHeight: 200
	property Item fillAnchors: parent

	signal gotMouseShortcut(var shortcut)
	signal canceledMouseShortcut()

	anchors.fill: fillAnchors

	opacity: 0
	visible: false

	color: colour.fadein_slidein_block_bg

	// Click on background is like rejecting it
	// (this MouseArea has to come here at the top so that it can be overwritten below for the actual widget
	// (no click on actual rect should close it))
	MouseArea {
		anchors.fill: parent
		hoverEnabled: true
		acceptedButtons: Qt.LeftButton | Qt.RightButton
		onClicked: {
			if(!rect.contains(Qt.point(mouse.x,mouse.y)))
			hideMe.start()
		}
	}

	Rectangle {

		id: rect

		// position it
		x: (parent.width-width)/2
		y: (parent.height-height)/2

		// Set size
		width: detectWidth
		height: detectHeight

		// Adjust colour and look
		color: colour.fadein_slidein_bg
		border.width: 1
		border.color: colour.fadein_slidein_border
		radius: global_element_radius

		// Confirmation text
		Text {

			x: 0
			y: 20
			width: parent.width
			horizontalAlignment: Text.AlignHCenter

			color: colour.text
			font.pointSize: 10
			wrapMode: Text.WordWrap

			text: "<h2>" + qsTr("Set Mouse Shortcut") + "</h2>"

		}

		// Mousearea preventing background mousearea from catching clicks
		MouseArea {
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton | Qt.RightButton
			onClicked: {}
		}

		Rectangle {

			color: "#00000000"

			x: (parent.width-width)/2
			y: (parent.height-height)/2+10
			width: childrenRect.width
			height: 50

			Row {
				spacing: 5

				CustomComboBox {
					id: mouseshortcut_modifier
					width: (rect.width-50)/2
					model: ["----", "Ctrl", "Alt", "Shift", "Ctrl+Alt", "Ctrl+Shift", "Alt+Shift", "Ctrl+Alt+Shift"]
				}

				CustomComboBox {
					id: mouseshortcut_button
					width: (rect.width-50)/2
					model: ["Left Button", "Right Button", "Middle Button", "Wheel Up", "Wheel Down"]
				}

			}

		}


		CustomButton {

			width: 2.5*(parent.width/7)
			x: parent.width/7
			y: parent.height-height-15
			text: qsTr("Don't set")

			onClickedButton: {
				canceledMouseShortcut()
				hide()
			}

		}

		CustomButton {

			width: 2.5*(parent.width/7)
			x: 3.5*(parent.width/7)
			y: parent.height-height-15
			text: qsTr("Set Shortcut")

			onClickedButton: {

				var shortcut = ""
				if(mouseshortcut_modifier.currentIndex != 0)
					shortcut += mouseshortcut_modifier.currentText + " + "
				shortcut += mouseshortcut_button.currentText

				gotMouseShortcut(shortcut)
				hide()
			}

		}

	}

	function show() {
		showMe.start()
	}
	function hide() {
		hideMe.start()
	}

	function setShortcut(mod,but) {

		if(mod === "")
			mouseshortcut_modifier.currentIndex = 0
		else
			mouseshortcut_modifier.currentIndex = mouseshortcut_modifier.find(mod)
		mouseshortcut_button.currentIndex = mouseshortcut_button.find(but)
	}

	PropertyAnimation {
		id: hideMe
		target: detect
		property: "opacity"
		to: 0
		onStopped: {
			visible = false
		}
	}
	PropertyAnimation {
		id: showMe
		target: detect
		property: "opacity"
		to: 1
		onStarted: {
			visible = true
		}
	}

}
