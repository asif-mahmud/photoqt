#include "mainwindow.h"

MainWindow::MainWindow(bool verbose, QWindow *parent) : QQuickView(parent) {

	connect(this, SIGNAL(statusChanged(QQuickView::Status)), this, SLOT(loadStatus(QQuickView::Status)));

#ifdef Q_OS_WIN
	QtWin::enableBlurBehindWindow(this);
#endif

	// Settings and variables
	settingsPerSession = new SettingsSession;
	settingsPermanent = new Settings;
	fileformats = new FileFormats(verbose);
	variables = new Variables;
	shortcuts = new Shortcuts;
	touch = new TouchHandler;

	variables->verbose = verbose;

	overrideCursorHowOftenSet = 0;

	this->setMinimumSize(QSize(800,600));

	// Add image providers
	this->engine()->addImageProvider("thumb",new ImageProviderThumbnail);
	this->engine()->addImageProvider("full",new ImageProviderFull);
	this->engine()->addImageProvider("icon",new ImageProviderIcon);

	// Add settings access to QML
	qmlRegisterType<Settings>("Settings", 1, 0, "Settings");
	qmlRegisterType<FileFormats>("FileFormats", 1, 0, "FileFormats");
	qmlRegisterType<SettingsSession>("SettingsSession", 1, 0, "SettingsSession");
	qmlRegisterType<GetMetaData>("GetMetaData", 1, 0, "GetMetaData");
	qmlRegisterType<GetAndDoStuff>("GetAndDoStuff", 1, 0, "GetAndDoStuff");
	qmlRegisterType<ThumbnailManagement>("ThumbnailManagement", 1, 0, "ThumbnailManagement");
	qmlRegisterType<ToolTip>("ToolTip", 1, 0, "ToolTip");
	qmlRegisterType<ShortcutsNotifier>("ShortcutsNotifier", 1, 0, "ShortcutsNotifier");
	qmlRegisterType<Colour>("Colour", 1, 0, "Colour");

	// Load QML
	this->setSource(QUrl("qrc:/qml/mainwindow.qml"));
	this->setColor(QColor(Qt::transparent));

	// Get object (for signals and stuff)
	object = this->rootObject();

	// Class to load a new directory
	loadDir = new LoadDir(verbose);

	// Scrolled view
	connect(object, SIGNAL(thumbScrolled(int)), this, SLOT(handleThumbnails(int)));


	connect(object, SIGNAL(reloadDirectory(QString,QString)), this, SLOT(handleOpenFileEvent(QString,QString)));
	connect(object, SIGNAL(loadMoreThumbnails()), this, SLOT(loadMoreThumbnails()));
	connect(object, SIGNAL(didntLoadThisThumbnail(int)), this, SLOT(didntLoadThisThumbnail(int)));
	connect(object, SIGNAL(setOverrideCursor()), this, SLOT(setOverrideCursor()));
	connect(object, SIGNAL(restoreOverrideCursor()), this, SLOT(restoreOverrideCursor()));
	connect(object, SIGNAL(stopThumbnails()), this, SLOT(stopThumbnails()));
	connect(object, SIGNAL(reloadThumbnails()), this, SLOT(reloadThumbnails()));

	connect(object, SIGNAL(verboseMessage(QString,QString)), this, SLOT(qmlVerboseMessage(QString,QString)));

	// Hide/Quit window
	connect(object, SIGNAL(hideToSystemTray()), this, SLOT(hideToSystemTray()));
	connect(object, SIGNAL(quitPhotoQt()), this, SLOT(quitPhotoQt()));

	// React to some settings...
	connect(settingsPermanent, SIGNAL(trayiconChanged(int)), this, SLOT(showTrayIcon()));
	connect(settingsPermanent, SIGNAL(trayiconChanged(int)), this, SLOT(hideTrayIcon()));
	connect(settingsPermanent, SIGNAL(windowmodeChanged(bool)), this, SLOT(updateWindowGeometry()));
	connect(settingsPermanent, SIGNAL(windowDecorationChanged(bool)), this, SLOT(updateWindowGeometry()));

	connect(this, SIGNAL(xChanged(int)), this, SLOT(updateWindowXandY()));
	connect(this, SIGNAL(yChanged(int)), this, SLOT(updateWindowXandY()));

	// Pass on touchevent
	connect(touch, SIGNAL(receivedTouchEvent(QPointF,QPointF,qint64,int,QStringList)), this, SLOT(passOnTouchEvent(QPointF,QPointF,qint64,int,QStringList)));
	connect(touch, SIGNAL(setImageInteractiveMode(bool)), this, SLOT(setImageInteractiveMode(bool)));

	showTrayIcon();

	// We need to call this with a little delay, as the automatic restoration of the window geometry at startup when window mode
	// is enabled doesn't update the actualy window x/y (and thus PhotoQt might be detected on the wrong screen which messes up
	// calculations involving local cursor coordinates (e.g., for 'close on click on grey'))
	QTimer::singleShot(100,this, SLOT(updateWindowXandY()));

}

// Open a new file
void MainWindow::handleOpenFileEvent(QString filename, QString filter) {

// On Windows, we have to remove all three '/' after 'file:', on Linux, we need to leave one of them
#ifdef Q_OS_WIN
	if(filename.startsWith("file:///"))
		filename = filename.remove(0,8);
#else
	if(filename.startsWith("file://"))
		filename = filename.remove(0,7);
#endif

	if(filename.trimmed() == "") {
		QMetaObject::invokeMethod(object, "openFile");
		return;
	}

	variables->keepLoadingThumbnails = true;

	setOverrideCursor();

	if(variables->verbose)
		LOG << CURDATE << "handleOpenFileEvent(): Handle response to request to open new file" << NL;

	// Decode filename
	QByteArray usethis = QByteArray::fromPercentEncoding(filename.trimmed().toUtf8());

	// Store filter
	variables->openfileFilter = filter;


	QString file = "";

	// Check return file
	file = usethis;

	QMetaObject::invokeMethod(object, "alsoIgnoreSystemShortcuts",
				  Q_ARG(QVariant, false));

	// Save current directory
	variables->currentDir = QFileInfo(file).absolutePath();

	// Clear loaded thumbnails
	variables->loadedThumbnails.clear();

	// Load direcgtory
	QFileInfoList l = loadDir->loadDir(file,variables->openfileFilter);
	if(l.isEmpty()) {
		QMetaObject::invokeMethod(object, "noResultsFromFilter");
		restoreOverrideCursor();
		return;
	}
	if(!l.contains(QFileInfo(file)))
		file = l.at(0).filePath();

	// Get and store length
	int l_length = l.length();
	settingsPerSession->setValue("countTot",l_length);

	// Convert QFileInfoList into QStringList and store it
	QStringList ll;
	for(int i = 0; i < l_length; ++i)
		ll.append(l.at(i).absoluteFilePath());
	settingsPerSession->setValue("allFileList",ll);

	// Get and store current position
	int curPos = l.indexOf(QFileInfo(file));
	settingsPerSession->setValue("curPos",curPos);

	// Setiup thumbnail model
	QMetaObject::invokeMethod(object, "setupModel",
		Q_ARG(QVariant, ll),
		Q_ARG(QVariant, curPos));

	// Display current postiion in main image view
	QMetaObject::invokeMethod(object, "displayImage",
				  Q_ARG(QVariant, curPos));

	QVariant centerPos = curPos;
	if(!QMetaObject::invokeMethod(object, "getCenterPos",
				  Q_RETURN_ARG(QVariant, centerPos)))
		std::cerr << CURDATE <<  "handleOpenFileEvent(): ERROR: couldn't get center pos!" << NL;

	// And handle the thumbnails
	handleThumbnails(centerPos.toInt());

	restoreOverrideCursor();

}

// Thumbnail handling (centerPos is image currently displayed in the visible center of thumbnail bar)
void MainWindow::handleThumbnails(int centerPos) {

	if(variables->verbose)
		LOG << CURDATE << "handleThumbnails(): New thumbnail center pos: " << centerPos << NL;

	// Get some settings for later use
	int thumbSize = settingsPermanent->thumbnailsize;
	int thumbSpacing = settingsPermanent->thumbnailSpacingBetween;
	int dynamicSmartNormal = settingsPermanent->thumbnailDynamic;

	// Get total and center pos
	int countTot = settingsPerSession->value("countTot").toInt();
	currentCenter = centerPos;

	// Generate how many to each side
	int numberToOneSide = (this->width()/(thumbSize+thumbSpacing))/2;

	// Load full directory
	if(dynamicSmartNormal == 0) numberToOneSide = qMax(currentCenter,countTot-currentCenter);
	int maxLoad = numberToOneSide;
	if(dynamicSmartNormal == 2) maxLoad = qMax(currentCenter,countTot-currentCenter);

	loadThumbnailsInThisOrder.clear();
	smartLoadThumbnailsInThisOrder.clear();

	if(!variables->loadedThumbnails.contains(currentCenter)) loadThumbnailsInThisOrder.append(currentCenter);

	// Load thumbnails in this order
	for(int i = 1; i <= maxLoad+3; ++i) {
		if(i <= numberToOneSide+3) {
			if((currentCenter-i) >= 0 && !variables->loadedThumbnails.contains(currentCenter-i))
				loadThumbnailsInThisOrder.append(currentCenter-i);
			if(currentCenter+i < countTot && !variables->loadedThumbnails.contains(currentCenter+i))
				loadThumbnailsInThisOrder.append(currentCenter+i);
		} else {
			if((currentCenter-i) >= 0 && !variables->loadedThumbnails.contains(currentCenter-i))
				smartLoadThumbnailsInThisOrder.append(currentCenter-i);
			if(currentCenter+i < countTot && !variables->loadedThumbnails.contains(currentCenter+i))
				smartLoadThumbnailsInThisOrder.append(currentCenter+i);
		}
	}

	loadMoreThumbnails();

}

void MainWindow::loadMoreThumbnails() {

	if(variables->verbose)
		LOG << CURDATE << "loadMoreThumbnails(): Continue loading thumbnails?" << NL;

	if(settingsPermanent->thumbnailFilenameInstead || !variables->keepLoadingThumbnails) return;

	if(loadThumbnailsInThisOrder.length() == 0 && smartLoadThumbnailsInThisOrder.length() == 0) return;

	if(loadThumbnailsInThisOrder.length() != 0) {

		int load = loadThumbnailsInThisOrder.first();

		if(variables->loadedThumbnails.contains(load)) {
			loadThumbnailsInThisOrder.removeFirst();
			return loadMoreThumbnails();
		}

		if(variables->verbose)
			LOG << CURDATE << "loadMoreThumbnails(): Yes, please (visible)! Load #" << load << NL;

		loadThumbnailsInThisOrder.removeFirst();

		QMetaObject::invokeMethod(object, "reloadImage",
					  Q_ARG(QVariant, load),
					  Q_ARG(QVariant, false));
		variables->loadedThumbnails.append(load);

	} else {

		int load = smartLoadThumbnailsInThisOrder.first();

		if(variables->loadedThumbnails.contains(load)) {
			smartLoadThumbnailsInThisOrder.removeFirst();
			return loadMoreThumbnails();
		}

		if(variables->verbose)
			LOG << CURDATE << "loadMoreThumbnails(): Yes, please (invisible, smart)! Load #" << load << NL;

		smartLoadThumbnailsInThisOrder.removeFirst();

		QMetaObject::invokeMethod(object, "reloadImage",
					  Q_ARG(QVariant, load),
					  Q_ARG(QVariant, true));
		variables->loadedThumbnails.append(load);
	}

}

// This one was tried to be preloaded smartly, but didn't exist yet -> nothing done
void MainWindow::didntLoadThisThumbnail(int pos) {
	if(variables->verbose)
		LOG << CURDATE << "didntLoadThisThumbnail(): Thumbnail #" << pos << " not loaded smartly..." << NL;
	variables->loadedThumbnails.removeAt(variables->loadedThumbnails.indexOf(pos));
}

// These are used to communicate key combos to the qml interface (for shortcuts, lineedits, etc.)
void MainWindow::detectedKeyCombo(QString combo) {
	if(variables->verbose)
		LOG << CURDATE << "detectedKeyCombo(): " << combo.toStdString() << NL;
	QMetaObject::invokeMethod(object, "detectedKeyCombo",
				  Q_ARG(QVariant, combo));
}

// Catch wheel events
void MainWindow::wheelEvent(QWheelEvent *e) {

	if(variables->verbose)
		LOG << CURDATE << "wheelEvent()" << NL;

	if(e->angleDelta().y() < 0) {

		if(!object->property("blocked").toBool()) {

			// Wheel direction changed -> start counting at beginning
			if(variables->wheelcounter >= 0 && settingsPermanent->mouseWheelSensitivity > 1) {
				variables->wheelcounter = -1;
				return;
			// Same direction, but haven't reached counter yet
			} else if(variables->wheelcounter*-1 < settingsPermanent->mouseWheelSensitivity-1 && settingsPermanent->mouseWheelSensitivity > 1) {
				--variables->wheelcounter;
				return;
			}

		}

		// We got here? Great, so reset counter (i.e., next event starts at beginning again)
		variables->wheelcounter = 0;

		if(variables->verbose)
			LOG << CURDATE << "wheelEvent(): Wheel down" << NL;

		QMetaObject::invokeMethod(object,"mouseWheelEvent",
								  Q_ARG(QVariant, "Wheel Down"));

	} else if(e->angleDelta().y() > 0) {

		if(!object->property("blocked").toBool()) {

			// Wheel direction changed -> start counting at beginning
			if(variables->wheelcounter <= 0 && settingsPermanent->mouseWheelSensitivity > 1) {
				variables->wheelcounter = 1;
				return;
			// Same direction, but haven't reached counter yet
			} else if(variables->wheelcounter < settingsPermanent->mouseWheelSensitivity-1 && settingsPermanent->mouseWheelSensitivity > 1) {
				++variables->wheelcounter;
				return;
			}

		}

		// We got here? Great, so reset counter (i.e., next event starts at beginning again)
		variables->wheelcounter = 0;

		if(variables->verbose)
			LOG << CURDATE << "wheelEvent(): Wheel up" << NL;

		QMetaObject::invokeMethod(object,"mouseWheelEvent",
								  Q_ARG(QVariant, "Wheel Up"));

	}

	QQuickView::wheelEvent(e);

}

// Catch mouse events (ignored when mouse moved when button pressed)
void MainWindow::mousePressEvent(QMouseEvent *e) {

	if(variables->verbose)
		LOG << CURDATE << "mousePressEvent()" << NL;

	mouseCombo = "";
	mouseOrigPoint = e->pos();
	mouseDx = 0;
	mouseDy = 0;

	if(e->button() == Qt::RightButton)
		mouseCombo = "Right Button";
	else if(e->button() == Qt::MiddleButton)
		mouseCombo = "Middle Button";
	else if(e->button() == Qt::LeftButton)
		mouseCombo = "Left Button";

	if(variables->verbose)
		LOG << CURDATE << "mousePressEvent(): mouseCombo = " << mouseCombo.toStdString() << NL;

	QQuickView::mousePressEvent(e);

}
void MainWindow::mouseReleaseEvent(QMouseEvent *e) {

	if(variables->verbose)
		LOG << CURDATE << "mouseReleaseEvent()" << NL;

	QQuickView::mouseReleaseEvent(e);

	QMetaObject::invokeMethod(object,"mouseWheelEvent",
							  Q_ARG(QVariant, mouseCombo));

}
void MainWindow::mouseMoveEvent(QMouseEvent *e) {

	mouseDx += abs(mouseOrigPoint.x()-e->pos().x());
	mouseDy += abs(mouseOrigPoint.y()-e->pos().y());

	object->setProperty("localcursorpos",this->mapFromGlobal(QCursor::pos()));

	QQuickView::mouseMoveEvent(e);

}

bool MainWindow::event(QEvent *e) {

	if(e->type() == QEvent::KeyPress) {

		if(variables->verbose)
			LOG << CURDATE << "keyPressEvent()" << NL;
		detectedKeyCombo(shortcuts->handleKeyPress((QKeyEvent*)e));

	} else if(e->type() == QEvent::KeyRelease) {

		if(variables->verbose)
			LOG << CURDATE << "keyReleaseEvent()" << NL;
		QMetaObject::invokeMethod(object, "keysReleased",
								  Q_ARG(QVariant,shortcuts->handleKeyPress((QKeyEvent*)e)));

	} else if (e->type() == QEvent::Close) {

		if(variables->verbose)
			LOG << CURDATE << "closeEvent()" << NL;

		// Hide to system tray (except if a 'quit' was requested)
		if(settingsPermanent->trayicon == 1 && !variables->skipSystemTrayAndQuit) {

			trayAction(QSystemTrayIcon::Trigger);
			if(variables->verbose) LOG << CURDATE << "closeEvent(): Hiding to System Tray." << NL;
			e->ignore();

		// Quit
		} else {

			// Save current geometry
			QFile geo(CFG_MAINWINDOW_GEOMETRY_FILE);
			if(geo.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				QTextStream out(&geo);
				QRect rect = geometry();
				QString txt = "[General]\n";
				txt += QString("mainWindowGeometry=@Rect(%1 %2 %3 %4)\n").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
				out << txt;
				geo.close();
			}

			e->accept();

			if(variables->verbose)
				LOG << CURDATE;
			LOG << "Goodbye!" << NL;

			qApp->quit();

		}

	} else if(e->type() == QEvent::TouchBegin || e->type() == QEvent::TouchUpdate || e->type() == QEvent::TouchEnd)
		if(settingsPermanent->experimentalTouchscreenSupport)
			touch->handle(e);


	return QQuickWindow::event(e);

}

void MainWindow::trayAction(QSystemTrayIcon::ActivationReason reason) {

	if(variables->verbose)
		LOG << CURDATE << "trayAction()" << NL;

	if(reason == QSystemTrayIcon::Trigger) {

		if(!variables->hiddenToTrayIcon) {
			variables->geometryWhenHiding = this->geometry();
			if(variables->verbose)
				LOG << CURDATE << "trayAction(): Hiding to tray" << NL;
			this->hide();
		} else {

			if(variables->verbose)
				LOG << CURDATE << "trayAction(): Updating screenshots" << NL;

			// Get screenshots
			for(int i = 0; i < QGuiApplication::screens().count(); ++i) {
				QScreen *screen = QGuiApplication::screens().at(i);
				QRect r = screen->geometry();
				QPixmap pix = screen->grabWindow(0,r.x(),r.y(),r.width(),r.height());
				if(!pix.save(QDir::tempPath() + QString("/photoqt_screenshot_%1.jpg").arg(i)))
					LOG << CURDATE << "ERROR: Unable to update screenshot for screen #" << i << NL;
			}

			if(variables->verbose)
				LOG << CURDATE << "trayAction(): SHowing window" << NL;

			updateWindowGeometry();

			if(variables->currentDir == "")
				QMetaObject::invokeMethod(object, "openFile");
		}

	}

}

void MainWindow::hideToSystemTray() {
		this->close();
}
void MainWindow::quitPhotoQt() {
	variables->skipSystemTrayAndQuit = true;
	this->close();
}

void MainWindow::showTrayIcon() {

	if(variables->verbose)
		LOG << CURDATE << "showTrayIcon()" << NL;

	if(settingsPermanent->trayicon != 0) {

		if(!variables->trayiconSetup) {

			if(variables->verbose)
				LOG << CURDATE << "showTrayIcon(): Setting up" << NL;

			trayIcon = new QSystemTrayIcon(this);
			trayIcon->setIcon(QIcon(":/img/icon.png"));
			trayIcon->setToolTip("PhotoQt - " + tr("Image Viewer"));

			// A context menu for the tray icon
			QMenu *trayIconMenu = new QMenu;
			trayIconMenu->setStyleSheet("background-color: rgb(67,67,67); color: white; border-radius: 5px;");
			QAction *trayAcToggle = new QAction(QIcon(":/img/logo.png"),tr("Hide/Show PhotoQt"),this);
			trayIconMenu->addAction(trayAcToggle);
			connect(trayAcToggle, SIGNAL(triggered()), this, SLOT(show()));

			// Set the menu to the tray icon
			trayIcon->setContextMenu(trayIconMenu);
			connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayAction(QSystemTrayIcon::ActivationReason)));

			variables->trayiconSetup = true;

		}

		if(variables->verbose)
			LOG << CURDATE << "showTrayIcon(): Setting icon to visible" << NL;

		trayIcon->show();
		variables->trayiconVisible = true;

	}

}

void MainWindow::hideTrayIcon() {

	if(variables->verbose)
		LOG << CURDATE << "hideTrayIcon()" << NL;

	if(settingsPermanent->trayicon == 0 && variables->trayiconSetup) {

		trayIcon->hide();
		variables->trayiconVisible = false;

	}

}

// Remote controlling
void MainWindow::remoteAction(QString cmd) {

	if(variables->verbose)
		LOG << CURDATE << "remoteAction(): " << cmd.toStdString() << NL;

	if(cmd == "open") {

		if(variables->verbose)
			LOG << CURDATE << "remoteAction(): Open file" << NL;
		if(!this->isVisible()) {
			// Get screenshots
			for(int i = 0; i < QGuiApplication::screens().count(); ++i) {
				QScreen *screen = QGuiApplication::screens().at(i);
				QRect r = screen->geometry();
				QPixmap pix = screen->grabWindow(0,r.x(),r.y(),r.width(),r.height());
				pix.save(QDir::tempPath() + QString("/photoqt_screenshot_%1.jpg").arg(i));
			}
			updateWindowGeometry();
			this->raise();
			this->requestActivate();
		}

		QMetaObject::invokeMethod(object, "openFile");

	} else if(cmd == "nothumbs") {

		if(variables->verbose)
			LOG << CURDATE << "remoteAction(): Disable thumbnails" << NL;
		settingsPermanent->thumbnailDisable = true;
		settingsPermanent->thumbnailDisableChanged(settingsPermanent->thumbnailDisable);

	} else if(cmd == "thumbs") {

		if(variables->verbose)
			LOG << CURDATE << "remoteAction(): Enable thumbnails" << NL;
		settingsPermanent->thumbnailDisable = true;
		settingsPermanent->thumbnailDisableChanged(settingsPermanent->thumbnailDisable);

	} else if(cmd == "hide" || (cmd == "toggle" && this->isVisible())) {

		if(variables->verbose)
			LOG << CURDATE << "remoteAction(): Hiding" << NL;
		if(settingsPermanent->trayicon != 1) {
			settingsPermanent->trayicon = 1;
			settingsPermanent->trayiconChanged(settingsPermanent->trayicon);
		}
		QMetaObject::invokeMethod(object, "hideOpenFile");
		this->hide();

	} else if(cmd.startsWith("show") || (cmd == "toggle" && !this->isVisible())) {

		if(variables->verbose)
			LOG << CURDATE << "remoteAction(): Showing" << NL;

		// The same code can be found at the end of main.cpp
		if(!this->isVisible()) {
			// Get screenshots
			for(int i = 0; i < QGuiApplication::screens().count(); ++i) {
				QScreen *screen = QGuiApplication::screens().at(i);
				QRect r = screen->geometry();
				QPixmap pix = screen->grabWindow(0,r.x(),r.y(),r.width(),r.height());
				pix.save(QDir::tempPath() + QString("/photoqt_screenshot_%1.jpg").arg(i));
			}
			updateWindowGeometry();
		}
		this->raise();
		this->requestActivate();

		if(variables->currentDir == "" && cmd != "show_noopen")
			QMetaObject::invokeMethod(object, "openFile");

	} else if(cmd.startsWith("::file::")) {

		if(variables->verbose)
			LOG << CURDATE << "remoteAction(): Opening passed-on file" << NL;
		QMetaObject::invokeMethod(object, "hideOpenFile");
		handleOpenFileEvent(cmd.remove(0,8));

	}


}

void MainWindow::updateWindowGeometry() {

	if(variables->verbose)
		LOG << CURDATE << "updateWindowGeometry()" << NL;

	if(settingsPermanent->windowmode) {
		if(settingsPermanent->keepOnTop) {
			settingsPermanent->windowDecoration
					  ? this->setFlags(Qt::Window | Qt::WindowStaysOnTopHint)
					  : this->setFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		} else {
			settingsPermanent->windowDecoration
					  ? this->setFlags(Qt::Window)
					  : this->setFlags(Qt::Window | Qt::FramelessWindowHint);
		}
#ifndef Q_OS_WIN
		if(settingsPermanent->saveWindowGeometry) {
			QFile geo(CFG_MAINWINDOW_GEOMETRY_FILE);
			if(geo.open(QIODevice::ReadOnly)) {
				QTextStream in(&geo);
				QString all = in.readAll();
				if(all.contains("mainWindowGeometry=@Rect(")) {
					QStringList vars = all.split("mainWindowGeometry=@Rect(").at(1).split(")\n").at(0).split(" ");
					if(vars.length() == 4) {
						this->show();
						this->setGeometry(QRect(vars.at(0).toInt(),vars.at(1).toInt(),vars.at(2).toInt(),vars.at(3).toInt()));
					} else
						this->showMaximized();
				} else
					this->showMaximized();
			} else
				this->showMaximized();
		} else
#endif
			this->showMaximized();
	} else {

		if(settingsPermanent->keepOnTop)
			this->setFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
		else
			this->setFlags(Qt::FramelessWindowHint);


/***************************************/
// Patch provided by John Morris

#ifdef Q_OS_MAC
		// If on a Mac, show fullscreen on monitor containing the mouse pointer.
		int screenNum = qApp->desktop()->screenNumber(QCursor::pos());
		this->setScreen(qApp->screens()[screenNum]);
#endif

/***************************************/

		QString(getenv("DESKTOP")).startsWith("Enlightenment") ? this->showMaximized() : this->showFullScreen();
	}

}

void MainWindow::resetWindowGeometry() {
	if(variables->verbose)
		LOG << CURDATE << "resetWindowGeometry()" << NL;
	QSettings settings("photoqt","photoqt");
	this->setGeometry(settings.value("mainWindowGeometry").toRect());
}

void MainWindow::updateWindowXandY() {

	object->setProperty("windowx",this->x());
	object->setProperty("windowy",this->y());

	QRect rect = this->screen()->geometry();
	int x_cur = this->x()-rect.x();
	int y_cur = this->y()-rect.y();
	object->setProperty("windowx_currentscreen",x_cur < 0 ? this->x() : x_cur);
	object->setProperty("windowy_currentscreen",y_cur < 0 ? this->y() : x_cur);

}

void MainWindow::resizeEvent(QResizeEvent *e) {

	QMetaObject::invokeMethod(object, "windowResized");

	QQuickWindow::resizeEvent(e);

}

void MainWindow::showStartup(QString type) {

	if(variables->verbose)
		LOG << CURDATE << "showStartup(): " << type.toStdString() << NL;

	QMetaObject::invokeMethod(object,"showStartup",
							  Q_ARG(QVariant, type));

}

void MainWindow::qmlVerboseMessage(QString loc, QString msg) {
	if(variables->verbose) {
		LOG << CURDATE << "[QML] " << loc.toStdString();
		if(msg.trimmed() != "") LOG << ": " << msg.toStdString() << NL;
	}
}

MainWindow::~MainWindow() {
	QFile file(CFG_SETTINGS_SESSION_FILE);
	file.remove();
	delete settingsPerSession;
	delete settingsPermanent;
	delete fileformats;
	if(variables->trayiconSetup) delete trayIcon;
	delete variables;
	delete shortcuts;
	delete loadDir;
	delete touch;
}
