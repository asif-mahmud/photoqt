import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../elements"

Rectangle {
	id: hovprev_but
	y: 10
	width: select.width+20
	height: parent.height-20
	color: "#00000000"

	// Select which group of images to display
	CustomComboBox {
		id: select
		y: (parent.height-height)/2
		width: 200
		backgroundColor: "#313131"
		radius: 5
		showBorder: false
		currentIndex: settingssession.value("OpenFileTypesDisplayed",0)
		onCurrentIndexChanged: {
			settingssession.setValue("OpenFileTypesDisplayed",currentIndex)
			openfile_top.loadCurrentDirectory(openfile_top.currentlyLoadedDir)
		}
		model: [qsTr("All Supported images"), "Qt " + qsTr("images"), "GraphicsMagick " + qsTr("images"), "LibRaw " + qsTr("images")]
	}

	function getFileTypeSelection() {
		return select.currentIndex
	}
	function setFileTypeSelection(i) {
		select.currentIndex = i
	}
}
