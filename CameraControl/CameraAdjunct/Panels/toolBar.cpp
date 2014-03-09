/*
 *  Tool bar: Import, Log, Settings, Tools, Live view
 */
#include <QtGui>
#include "View.h"
#include "Import.h"
#include "LogBrowser.h"
#include "Tools.h"
#include "Options.h"
#include "MultiShot.h"
#include "Widgets/LiveImage.h"


/*
 *  Setup toolbar.
 */
QLayout *
View::toolbarLayout()
{
    //const QSize iconSize( 24, 24 );	// default size is 24
    const QSize iconSize( 32, 32 );	// default size is 24
    const bool autoRaise = true;

    QToolButton *importButton = new QToolButton();
    importButton->setText( tr("Import") );
    importButton->setAutoRaise( autoRaise );
    importButton->setStatusTip(
	tr("Import destination folder and file naming ...") );
    importButton->setIcon(
	QIcon(":/Resources/Toolbar/import.png") );
    importButton->setIconSize( iconSize );
    importButton->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    QObject::connect(
	importButton, SIGNAL(clicked()),
	import, SLOT(show()));
    QObject::connect(
	importButton, SIGNAL(clicked()),
	import, SLOT(showNormal()));
    QObject::connect(
	importButton, SIGNAL(clicked()),
	import, SLOT(raise()));

    QToolButton *logButton = new QToolButton();
    logButton->setText( tr("Log") );
    logButton->setAutoRaise( autoRaise );
    logButton->setStatusTip( tr("Shooting and settings log...") );
    logButton->setIcon( QIcon(":/Resources/Toolbar/log.png") );
    logButton->setIconSize( iconSize );
    logButton->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    QObject::connect(
	logButton, SIGNAL(clicked()),
	logBrowser, SLOT(show()));
    QObject::connect(
	logButton, SIGNAL(clicked()),
	logBrowser, SLOT(showNormal()));
    QObject::connect(
	logButton, SIGNAL(clicked()),
	logBrowser, SLOT(raise()));

    QToolButton *settingsButton = new QToolButton();
    settingsButton->setText( tr("Settings") );
    settingsButton->setAutoRaise( autoRaise );
    settingsButton->setStatusTip(
	tr("Additional camera settings for image, exposure, and EXIF...") );
    settingsButton->setIcon( QIcon(":/Resources/Toolbar/settings.png") );
    settingsButton->setIconSize( iconSize );
    settingsButton->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    QObject::connect(
	settingsButton, SIGNAL(clicked()),
	options, SLOT(show()));
    QObject::connect(
	settingsButton, SIGNAL(clicked()),
	options, SLOT(showNormal()));
    QObject::connect(
	settingsButton, SIGNAL(clicked()),
	options, SLOT(raise()));

    QToolButton *toolsButton = new QToolButton();
    toolsButton->setText( tr("Tools") );
    toolsButton->setAutoRaise( autoRaise );
    toolsButton->setStatusTip(
	tr("Depth of field and field of view calculators...") );
    toolsButton->setIcon( QIcon(":/Resources/Toolbar/tools.png") );
    toolsButton->setIconSize( iconSize );
    toolsButton->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    QObject::connect(
	toolsButton, SIGNAL(clicked()),
	this, SLOT(showTools()));

    QToolButton *liveviewButton = new QToolButton();
    liveviewButton->setText( tr("Live view") );
    liveviewButton->setAutoRaise( autoRaise );
    liveviewButton->setStatusTip( tr("Start and stop live view") );
    liveviewButton->setIcon( QIcon(":/Resources/Toolbar/liveview.png") );
    liveviewButton->setIconSize( iconSize );
    liveviewButton->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    liveviewButton->setCheckable( true );
    QObject::connect(
	liveviewButton, SIGNAL(toggled(bool)),
	this, SLOT(liveviewButtonChanged(bool)) );

    QToolButton *videoButton = new QToolButton();
    videoButton->setText( tr("Video") );
    videoButton->setAutoRaise( autoRaise );
    videoButton->setStatusTip( tr("Start and stop video") );
    videoButton->setIcon( QIcon(":/Resources/Toolbar/video.png") );
    videoButton->setIconSize( iconSize );
    videoButton->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    videoButton->setCheckable( true );
    QObject::connect(
	videoButton, SIGNAL(toggled(bool)),
	this, SLOT(videoButtonChanged(bool)) );

    QToolButton *multiShotButton = new QToolButton();
    multiShotButton->setText( tr("Multi-shot") );
    multiShotButton->setAutoRaise( autoRaise );
    multiShotButton->setStatusTip(
	tr("Multi-shot for exposure bracketing, panoramas, ...") );
    multiShotButton->setIcon( QIcon(":/Resources/Toolbar/multi-shot.png") );
    multiShotButton->setIconSize( iconSize );
    multiShotButton->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    QObject::connect(
	multiShotButton, SIGNAL(clicked()),
	multiShot, SLOT(show()));
    QObject::connect(
	multiShotButton, SIGNAL(clicked()),
	multiShot, SLOT(showNormal()));
    QObject::connect(
	multiShotButton, SIGNAL(clicked()),
	multiShot, SLOT(raise()));

    QToolButton *shutterButton = new QToolButton();
    shutterButton->setText( tr("Shutter") );
    shutterButton->setAutoRaise( autoRaise );
    shutterButton->setStatusTip( tr("Shutter and auto exposure button") );
    shutterButton->setIcon( QIcon(":/Resources/Toolbar/shutter.png") );
    shutterButton->setIconSize( iconSize );
    shutterButton->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    QObject::connect(
	shutterButton, SIGNAL(pressed()),
	this, SLOT(shutterInitiated()) );
    QObject::connect(
	shutterButton, SIGNAL(released()),
	this, SLOT(shutterDiscontinued()) );

    const int panelWidth = 256;
    QFrame *line = new QFrame(); 
    line->setFixedWidth( panelWidth );
    line->setFrameShape( QFrame::HLine ); 
    line->setFrameShadow( QFrame::Sunken ); 
    line->setLineWidth( 0 ); 
    line->setMidLineWidth( 2 ); 

    QGridLayout *layout = new QGridLayout();
    layout->addWidget( importButton,    0, 0, Qt::AlignCenter );
    layout->addWidget( logButton,       0, 1, Qt::AlignCenter );
    layout->addWidget( settingsButton,  0, 2, Qt::AlignCenter );
    layout->addWidget( toolsButton,     0, 3, Qt::AlignCenter );
    layout->addWidget( liveviewButton,  1, 0, Qt::AlignCenter );
    layout->addWidget( videoButton,     1, 1, Qt::AlignCenter );
    layout->addWidget( multiShotButton, 1, 2, Qt::AlignCenter );
    layout->addWidget( shutterButton,   1, 3, Qt::AlignCenter );
    layout->addWidget( line,	        2, 0, 1, 4, Qt::AlignCenter );

    return( layout );
}

/*
 *  Signal start of shutter press.
 */
void
View::shutterInitiated()
{
    emit shutterChanged( kEdsCameraCommand_ShutterButton_Completely_NonAF );
}

/*
 *  Signal end of shutter press.
 */
void
View::shutterDiscontinued()
{
    emit shutterChanged( kEdsCameraCommand_ShutterButton_OFF );
}

void
View::liveviewButtonChanged( bool checked )
{
    if( checked ) {
	inLiveViewMode = true;
	emit startLiveView();
    }
    else {
	inLiveViewMode = false;
	emit stopLiveView();
    }
}

void
View::videoButtonChanged( bool checked )
{
    if( checked ) {
	emit startVideo();
    }
    else {
	emit stopVideo();
    }
}

void
View::showTools()
{
    if( tools == 0 ) {
	tools = new Tools( camera, 0, Qt::Window );
    }
    tools->show();
    tools->showNormal();
    tools->raise();
    tools->activateWindow();
}

