/*
 *  Browse the log of shots and settings, clear the log,
 *  edit the log, and save the log. The log is plain text.
 *  Can undo changes using CTRL+Z, cut & paste using usual
 *  control keys. Can also implement redo, as there is a
 *  slot for this action.
 */

#include <QtGui>
#include "LogBrowser.h"

/*
 *  Constructor for log browser.
 */
LogBrowser::LogBrowser( QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    setWindowTitle( tr("Shooting and Settings Log[*]") );

    initialize();

    timeStamp();

    readSettings();
}

/*
 *  Destructor for log browser.
 */
LogBrowser::~LogBrowser()
{
}

void
LogBrowser::initialize()
{
    shootingLog = new QPlainTextEdit( this );
    shootingLog->setTabChangesFocus( true );
    shootingLog->setTextInteractionFlags( Qt::TextEditorInteraction );

    clearButton = new QPushButton();
    clearButton->setText( tr("Clear") );
    QObject::connect(
	clearButton, SIGNAL(clicked()),
	this, SLOT(clearLog()) );

    saveButton = new QPushButton();
    saveButton->setText( tr("Save as...") );
    QObject::connect(
	saveButton, SIGNAL(clicked()),
	this, SLOT(saveLog()) );

    closeButton = new QPushButton();
    closeButton->setText( tr("Close") );
    closeButton->setDefault( true );
    QObject::connect(
	closeButton, SIGNAL(clicked()),
	this, SLOT(close()) );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget( clearButton );
    buttonLayout->addStretch();
    buttonLayout->addWidget( saveButton );
    buttonLayout->addWidget( closeButton );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( shootingLog );
    layout->addLayout( buttonLayout );
    setLayout( layout );
}

void
LogBrowser::timeStamp()
{
    QString text;
    QDate date = QDate::currentDate();
    text = date.toString( "dddd, MMMM dd, yyyy" );
    shootingLog->appendPlainText( text );
    QTime time = QTime::currentTime();
    text = time.toString( "hh:mm:ss" ) + "  log created";
    shootingLog->appendPlainText( text );
    shootingLog->document()->setModified( false );
    setWindowModified( false );
}

/*
 *  Has the log been modified since last save or clear?
 */
bool
LogBrowser::isModified()
{
    return( shootingLog->document()->isModified() );
}

void
LogBrowser::appendLog( const QString text )
{
    QString s = QTime::currentTime().toString( "hh:mm:ss " );
    shootingLog->appendPlainText( s + text );
    setWindowModified( true );
}

bool
LogBrowser::saveLog()
{
    saveFileName = QFileDialog::getSaveFileName(this,
			tr("Save shooting and settings log"),
			saveFileName,
			tr("Text files (*.txt)"));
    if( saveFileName.isEmpty() ) {
	return( false );
    }

    QFile file(saveFileName);
    if( !file.open( QFile::WriteOnly | QFile::Text ) ) {
	QMessageBox::warning(this, "Camera Adjunct",
			tr("Cannot write file %1:\n%2.")
			.arg(saveFileName)
			.arg(file.errorString()));
	return( false );
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << shootingLog->toPlainText();
    QApplication::restoreOverrideCursor();
    file.close();

    QString text;
    QTime time = QTime::currentTime();
    text = time.toString( "hh:mm:ss" ) + "  log saved to " + saveFileName;
    shootingLog->appendPlainText( text );
    shootingLog->document()->setModified( false );
    setWindowModified( false );

    return( true );
}

void
LogBrowser::clearLog()
{
    shootingLog->setPlainText( "" );
    timeStamp();
}

void
LogBrowser::readSettings()
{
    QSettings settings;

    settings.beginGroup( "Log" );
    resize( settings.value("size", QSize(200, 200)).toSize() );
    move( settings.value("pos", QPoint(100, 100)).toPoint() );
    settings.endGroup();
}

void
LogBrowser::writeSettings()
{
    QSettings settings;

    settings.beginGroup( "Log" );
    settings.setValue( "size", size() );
    settings.setValue( "pos", pos() );
    settings.endGroup();
}

/*
 *  Reimplementation of closeEvent to save settings.
 */
void
LogBrowser::closeEvent( QCloseEvent *event )
{
    writeSettings();
    event->accept();
}

