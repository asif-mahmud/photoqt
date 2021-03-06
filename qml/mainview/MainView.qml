import QtQuick 2.3
import QtQuick.Controls.Styles 1.2
import QtQuick.Controls 1.2

import "../elements"

Item {

	// This property is set to true in the displayImage() function in the ThumbnailBar element and ensures that PhotoQT first loads
	// the main image before loading the thumbnails (they are blocked in the meantime using a Timer)
	// This property is set to 'false' again once the main image was loaded
	property bool amLoadingImage: false

	id: item

	// Position item
	x: settings.borderAroundImg + metaData.nonFloatWidth
	y: (settings.thumbnailKeepVisible && settings.thumbnailposition == "Top" ? settings.borderAroundImg+thumbnailBar.height : settings.borderAroundImg)
	width: background.width - 2*settings.borderAroundImg - metaData.nonFloatWidth
	height: (settings.thumbnailKeepVisible ? background.height-thumbnailBar.height+thumbnailbarheight_addon/2 : background.height)-2*settings.borderAroundImg

	function noFilterResultsFound() {
		noresultsfound.visible = true;
		image.loadImage("", false)
	}

	// Set image
	function loadImage(path, animated) {

		setOverrideCursor()

		verboseMessage("Display::setImage()", path)

		// LOAD IMAGE
		image.loadImage(path, animated)

		// Hide 'nothing loaded' message and arrows
		nofileloaded.visible = false
		metadataarrow.visible = false
		mainmenuarrow.visible = false
		noresultsfound.visible = false

		// Update metadata
		metaData.setData(getmetadata.getExiv2(path))

		restoreOverrideCursor()

	}

	function clear() {
		image.loadImage("", false)
		nofileloaded.visible = true
	}

	function getImageRect() {
		var w = image.width*image.scale
		var h = image.height*image.scale
		var x = (background.width-w)/2
		var y = (background.height-h)/2
		return [x,y,w,h]
	}

	SmartImage {

		id: image

		fadeduration: settings.transition*150
		zoomduration: 150
		zoomstep: 0.3
		clip: true
		fitinwindow: settings.fitInWindow
		interpolationNearestNeighbourThreshold: settings.interpolationNearestNeighbourThreshold
		interpolationNearestNeighbourUpscale: settings.interpolationNearestNeighbourUpscale

		onImageIsReady: amLoadingImage = false

	}

	function getClosingX_x() { return rect.x; }
	function getClosingX_height() { return rect.height; }

	// Rectangle holding the closing x top right
	Rectangle {

		id: rect

		visible: (!slideshowRunning && !settings.hidex) || (slideshowRunning && !settings.slideShowHideQuickinfo)

		// Position it
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.rightMargin: settings.fancyX ? -settings.borderAroundImg : -settings.borderAroundImg+5
		anchors.topMargin: settings.fancyX ? -settings.borderAroundImg : -settings.borderAroundImg+5

		// Width depends on type of 'x'
		width: (settings.fancyX ? 3 : 1.5)*settings.closeXsize
		height: (settings.fancyX ? 3 : 1.5)*settings.closeXsize

		// Invisible rectangle
		color: "#00000000"

		// Normal 'x'
		Text {

			id: txt_x

			visible: !settings.fancyX
			anchors.fill: parent

			horizontalAlignment: Qt.AlignRight
			verticalAlignment: Qt.AlignTop

			font.pointSize: settings.closeXsize*1.5
			font.bold: true
			color: colour.quickinfo_text
			text: "x"

		}

		// Fancy 'x'
		Image {

			id: img_x

			visible: settings.fancyX
			anchors.right: parent.right
			anchors.top: parent.top

			source: "qrc:/img/closingx.png"
			sourceSize: Qt.size(3*settings.closeXsize,3*settings.closeXsize)

		}

		// Click on either one of them
		MouseArea {
			anchors.fill: parent
			cursorShape: Qt.PointingHandCursor
			acceptedButtons: Qt.LeftButton | Qt.RightButton
			onClicked: {
				if (mouse.button == Qt.RightButton) {
					softblocked = 1
					contextmenuClosingX.popup()
				} else {
					if(settings.trayicon)
						hideToSystemTray()
					else
						quitPhotoQt()
				}
			}
		}

		// The actual context menu
		ContextMenu {

			id: contextmenuClosingX

			MenuItem {
				text: qsTr("Hide") + " 'x'"
				onTriggered: {
					settings.hidex = true;
					rect.visible = false;
				}
			}
		}
	}

	// This label is displayed at startup, informing the user how to start
	Text {

		id: nofileloaded

		anchors.fill: item
		anchors.rightMargin: Math.max(metadataarrow.width,mainmenuarrow.width)+25
		anchors.leftMargin: Math.max(metadataarrow.width,mainmenuarrow.width)+25

		verticalAlignment: Qt.AlignVCenter
		horizontalAlignment: Qt.AlignHCenter

		color: colour.bg_label
		font.pointSize: 50
		font.bold: true
		wrapMode: Text.WordWrap

		visible: false	// Set to true from function below trigegred by onComplete in mainwindow.qml

		text: qsTr("Open a file to begin")

	}

	// Arrow pointing to metadata widget
	Image {
		id: metadataarrow
		visible: false	// Set to true from function below trigegred by onComplete in mainwindow.qml
		x: 0
		y: metaData.y+metaData.height/2-height/2
		source: "qrc:/img/mainview/arrowleft.png"
		width: 150
		height: 60
	}

	// Arrow pointing to mainmenu widget
	Image {
		id: mainmenuarrow
		visible: false	// Set to true from function below trigegred by onComplete in mainwindow.qml
		x: background.width-width-5
		y: mainmenu.y+mainmenu.height/2-height/2
		source: "qrc:/img/mainview/arrowright.png"
		width: 150
		height: 60
	}

	Text {

		id: noresultsfound

		anchors.fill: item
		visible: false

		verticalAlignment: Qt.AlignVCenter
		horizontalAlignment: Qt.AlignHCenter

		color: colour.bg_label
		font.pointSize: 50
		font.bold: true
		wrapMode: Text.WordWrap

		text: qsTr("No results found...")

	}


	CustomConfirm {
		id: rotateconfirm
		header: qsTr("Rotate Image?")
		description: qsTr("The Exif data of this image says, that this image is supposed to be rotated.") + "<br><br>" + qsTr("Do you want to apply the rotation?")
		confirmbuttontext: qsTr("Yes, do it")
		rejectbuttontext: qsTr("No, don't")
		showDontAskAgain: true
		onAccepted: {
			// 1 = Do nothing
			// 2 = Horizontally Flipped
			if(metaData.orientation == 2) {
				flipHorizontal()
			// 3 = Rotated by 180 degrees
			} else if(metaData.orientation == 3) {
				rotateRight()
				rotateRight()
			// 4 = Rotated by 180 degrees and flipped horizontally
			} else if(metaData.orientation == 4) {
				rotateRight()
				rotateRight()
				flipHorizontal()
			// 5 = Rotated by 270 degrees and flipped horizontally
			} else if(metaData.orientation == 5) {
				rotateRight()
				flipHorizontal()
			// 6 = Rotated by 270 degrees
			} else if(metaData.orientation == 6)
				rotateRight()
			// 7 = Flipped Horizontally and Rotated by 90 degrees
			else if(metaData.orientation == 7) {
				rotateLeft()
				flipHorizontal()
			// 8 = Rotated by 90 degrees
			} else if(metaData.orientation == 8)
				rotateLeft()

			if(alwaysDoThis)
				settings.exifrotation = "Always"
		}

		onRejected: {
			if(alwaysDoThis)
				settings.exifrotation = "Never"
		}
	}

	function displayIdleAndNothingLoadedMessage() {
		nofileloaded.visible = true
		metadataarrow.visible = true
		mainmenuarrow.visible = true
	}

	function zoomIn(towardsCenter) {
		image.zoomIn(towardsCenter)
	}

	// Can be called from outside for zooming
	function zoomOut(towardsCenter) {
		image.zoomOut(towardsCenter)
	}

	// Zoom to actual size
	function zoomActual() {
		image.zoomActual()
	}

	// Zoom to 250%
	function zoom250() {
		image.zoom250()
	}

	// Zoom to 500%
	function zoom500() {
		image.zoom500()
	}

	// Zoom to 1000%
	function zoom1000() {
		image.zoom1000()
	}

	// Reset zoom
	function resetZoom() {
		image.resetZoom()
	}

	// Rotate image to the left
	function rotateLeft() {
		image.rotateLeft()
	}

	// Rotate image to the right
	function rotateRight() {
		image.rotateRight()
	}

	// Reset the rotation
	function resetRotation() {
		image.resetRotation()
	}

	// Mirror horizontally
	function mirrorHorizontal() {
		image.mirrorHorizontal()
	}

	// Mirror vertically
	function mirrorVertical() {
		image.mirrorVertical()
	}

	// Reset mirroring
	function resetMirror() {
		image.resetMirror()
	}

	function windowHasBeenResized() {
		image.windowHasBeenResized()
	}

	function getSourceSize() {
		return image.getCurrentSourceSize()
	}

	function isZoomed() {
		return image.isZoomed()
	}

	function setInteractiveMode(enabled) {
		image.setInteractiveMode(enabled)
	}

	function analyseClick(pos) {
		if(!image.clickInsideImage(pos) && settings.closeongrey
				&& (!thumbnailBar.clickOnThumbnailBar(pos) || thumbnailBar.y < 0 || thumbnailBar.y > background.height)
				&& (!mainmenu.clickInMainMenu(pos) || mainmenu.opacity != 1)
				&& (!metaData.clickInMetaData(pos) || metaData.opacity != 1)
				&& !blocked)
			quitPhotoQt()
	}

}
