#####################
#### QML SOURCES ####
#####################

SET(d "qml")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/mainwindow.qml ${d}/Shortcuts.qml ${d}/Colour.qml)

SET(d "qml/slidein")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/ThumbnailBar.qml ${d}/MetaData.qml ${d}/MainMenu.qml ${d}/SlideshowBar.qml)

SET(d "qml/mainview")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/Background.qml ${d}/QuickInfo.qml ${d}/ContextMenu.qml ${d}/ContextMenuEntry.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/SmartImage.qml ${d}/MainView.qml)

SET(d "qml/openfile")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/OpenFile.qml ${d}/BreadCrumbs.qml ${d}/UserPlaces.qml ${d}/Folders.qml ${d}/FilesView.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/EditFiles.qml ${d}/TweaksZoom.qml ${d}/Tweaks.qml ${d}/TweaksViewMode.qml ${d}/TweaksPreview.qml)

SET(d "qml/elements")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/ScrollBarHorizontal.qml ${d}/ScrollBarVertical.qml ${d}/SettingsText.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/CustomCheckBox.qml ${d}/CustomButton.qml ${d}/CustomTabView.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/CustomComboBox.qml ${d}/CustomRadioButton.qml ${d}/CustomSlider.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/CustomSpinBox.qml ${d}/CustomConfirm.qml ${d}/CustomTextEdit.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/CustomDetectShortcut.qml ${d}/CustomMouseShortcut.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/CustomExternalCommand.qml ${d}/CustomLineEdit.qml ${d}/ToolTip.qml)

SET(d "qml/fadein")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/About.qml ${d}/Scale.qml ${d}/Rename.qml ${d}/Delete.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/Wallpaper.qml ${d}/Slideshow.qml ${d}/Filter.qml ${d}/Startup.qml)

SET(d "qml/settings")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/SettingsItem.qml ${d}/TabDetailsTile.qml ${d}/TabOtherLanguageTiles.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/TabLookAndFeelBasic.qml ${d}/TabLookAndFeelAdvanced.qml ${d}/TabOtherContext.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/TabFiletypesTiles.qml ${d}/TabShortcutsCategories.qml ${d}/TabShortcutsTilesSet.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/TabShortcutsTilesAvail.qml ${d}/TabThumbnailsBasic.qml ${d}/TabThumbnailsAdvanced.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/TabShortcuts.qml ${d}/TabDetails.qml ${d}/TabOther.qml ${d}/TabFiletypes.qml)

SET(d "qml/settings2")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/SettingsManager.qml ${d}/EntryTitle.qml ${d}/EntrySetting.qml ${d}/EntryContainer.qml)
SET(d "qml/settings2/tabs")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/TabLookAndFeel.qml ${d}/TabThumbnails.qml ${d}/TabMetadata.qml)
SET(d "qml/settings2/tabs/lookandfeel")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/SortBy.qml ${d}/WindowMode.qml ${d}/TrayIcon.qml ${d}/ClosingX.qml ${d}/FitInWindow.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/OverlayColor.qml ${d}/Quickinfo.qml ${d}/Background.qml ${d}/BorderAroundImage.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/CloseOnClick.qml ${d}/Loop.qml ${d}/Transition.qml ${d}/HotEdge.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/MouseWheelSensitivity.qml ${d}/Interpolation.qml ${d}/Remember.qml ${d}/Animation.qml)
SET(d "qml/settings2/tabs/thumbnails")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/ThumbnailSize.qml ${d}/Spacing.qml ${d}/LiftUp.qml ${d}/KeepVisible.qml ${d}/Dynamic.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/CenterOn.qml ${d}/TopOrBottom.qml ${d}/Label.qml ${d}/FilenameOnly.qml ${d}/Disable.qml)
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/Cache.qml)
SET(d "qml/settings2/tabs/metadata")
SET(photoqt_SOURCES ${photoqt_SOURCES} ${d}/MouseTrigger.qml ${d}/Metadata.qml ${d}/MetadataTile.qml ${d}/FontSize.qml)
