/*
 *  Import dialog: where to save images and how to rename them.
 */

#include <QtGui>
#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"
#include "Camera.h"
#include "Import.h"
#include "Map.h"


/*
 *  Constructor for import dialog.
 */
Import::Import( QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    initialize();

    setWindowTitle( tr("Import Destination[*]") );

    readSettings();
}

/*
 *  Destructor for import dialog.
 */
Import::~Import()
{
}

void
Import::initialize()
{
    /*
     *  Destination: camera, computer, both.
     */
    QLabel *saveLabel = new QLabel( tr("Save to:") );
    saveLabel->setMinimumWidth( 60 );
    saveLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    saveToCamera = new QRadioButton( tr("camera") );
    saveToComputer = new QRadioButton( tr("computer") );
    saveToBoth = new QRadioButton( tr("camera + computer") );
    saveToComputer->setChecked( true );
    QObject::connect(
	saveToCamera, SIGNAL(toggled(bool)),
	this, SLOT(saveTo(bool)) );
    QObject::connect(
	saveToComputer, SIGNAL(toggled(bool)),
	this, SLOT(saveTo(bool)) );
    QObject::connect(
	saveToBoth, SIGNAL(toggled(bool)),
	this, SLOT(saveTo(bool)) );

    QGridLayout *destinationLayout = new QGridLayout();
    destinationLayout->addWidget( saveLabel, 0, 0 );
    destinationLayout->addWidget( saveToCamera, 0, 1 );
    destinationLayout->addWidget( saveToComputer, 1, 1 );
    destinationLayout->addWidget( saveToBoth, 2, 1 );
    destinationLayout->setColumnStretch( /*column*/2, /*stretch*/1 );

    QGroupBox *destinationGroupBox = new QGroupBox( tr("Destination") );
    destinationGroupBox->setLayout( destinationLayout );

    /*
     *  Destination directory.
     */
    QLabel *pathLabel = new QLabel( tr("Move to:") );
    pathLabel->setMinimumWidth( 60 );
    pathLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    pathDisplayLabel = new QLabel();
    QPushButton *changeButton = new QPushButton( tr("Change...") );
    QObject::connect(
	changeButton, SIGNAL(clicked()),
	this, SLOT(updatePath()) );

    subfolderCheck = new QCheckBox( tr("Put in subfolder:") );
    QObject::connect(
	subfolderCheck, SIGNAL(stateChanged(int)),
	this, SLOT(setImportDirectory(int)) );
    subfolderLineEdit = new QLineEdit();
    QObject::connect(
	subfolderLineEdit, SIGNAL(editingFinished()),
	this, SLOT(updateSubfolder()) );

    QGridLayout *destinationFolderLayout = new QGridLayout();
    destinationFolderLayout->addWidget( pathLabel, 0, 0 );
    destinationFolderLayout->addWidget( pathDisplayLabel, 0, 1 );
    destinationFolderLayout->addWidget( changeButton, 0, 2 );
    destinationFolderLayout->addWidget( subfolderCheck, 1, 1 );
    destinationFolderLayout->addWidget( subfolderLineEdit, 2, 1, 1, 2 );
    destinationFolderLayout->setColumnStretch( /*column*/1, /*stretch*/1 );

    folderGroupBox = new QGroupBox( tr("Destination folder") );
    folderGroupBox->setLayout( destinationFolderLayout );

    /*
     *  File naming.
     */
    exampleDisplayLabel = new QLabel();
    QLabel *exampleLabel = new QLabel( tr("Example:") );
    exampleLabel->setMinimumWidth( 60 );
    exampleLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    QLabel *fileNamingLabel = new QLabel( tr("Template:") );
    fileNamingLabel->setMinimumWidth( 60 );
    fileNamingLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    fileNamingComboBox = new QComboBox();
    fileNamingComboBox->insertItem( 0, "Custom prefix - original file number" );
    fileNamingComboBox->insertItem( 1, "Custom prefix - sequence" );
    fileNamingComboBox->insertItem( 2, "Date - filename" );
    fileNamingComboBox->insertItem( 3, "Filename" );
    QObject::connect(
	fileNamingComboBox, SIGNAL(currentIndexChanged(int)),
	this, SLOT(updateFileNaming(int)) );
    prefixLabel = new QLabel( tr("Prefix:") );
    prefixLabel->setMinimumWidth( 60 );
    prefixLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    prefixLabel->hide();
    prefixLineEdit = new QLineEdit();
    prefixLineEdit->hide();
    QObject::connect(
	prefixLineEdit, SIGNAL(editingFinished()),
	this, SLOT(updatePrefix()) );
    sequenceLabel = new QLabel( tr("Sequence:") );
    sequenceLabel->setMinimumWidth( 60 );
    sequenceLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    sequenceLabel->hide();
    sequenceSpinBox = new QSpinBox();
    sequenceSpinBox->setRange( 1, 9999 );
    sequenceSpinBox->hide();
    QObject::connect(
	sequenceSpinBox, SIGNAL(valueChanged(int)),
	this, SLOT(updateSequence(int)) );

    QGridLayout *fileNamingLayout = new QGridLayout();
    fileNamingLayout->addWidget( exampleLabel, 0, 0 );
    fileNamingLayout->addWidget( exampleDisplayLabel, 0, 1 );
    fileNamingLayout->addWidget( fileNamingLabel, 1, 0 );
    fileNamingLayout->addWidget( fileNamingComboBox, 1, 1 );
    fileNamingLayout->addWidget( prefixLabel, 2, 0 );
    fileNamingLayout->addWidget( prefixLineEdit, 2, 1 );
    fileNamingLayout->addWidget( sequenceLabel, 3, 0 );
    fileNamingLayout->addWidget( sequenceSpinBox, 3, 1 );
    fileNamingLayout->setColumnStretch( 1, 1 );

    setDefaults();

    namingGroupBox = new QGroupBox( tr("File naming") );
    namingGroupBox->setLayout( fileNamingLayout );

    /*
     *  Buttons.
     */
    QPushButton *closeButton = new QPushButton( tr("Close") );
    closeButton->setDefault( true );
    QObject::connect(
	closeButton, SIGNAL(clicked()),
	this, SLOT(close()) );
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    buttonsLayout->addWidget( closeButton );

    /*
     *  Final layout.
     */
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( destinationGroupBox );
    layout->addWidget( folderGroupBox );
    layout->addWidget( namingGroupBox );
    layout->addStretch();
    layout->addLayout( buttonsLayout );
    setLayout( layout );
}

/*
 *  Set the default directory, subfolder, prefix,
 *  and beginning sequence number.
 */
void
Import::setDefaults()
{
    /*
     *  Set the default destination directory.
     */
    QSettings settings;
    path = settings.value( "Import/path",
		QDir::homePath() + "/Pictures" ).toString();
    bool importDirectorySet = QDir::setCurrent( path );
    if( !importDirectorySet ) {
	path = QDir::homePath();
	importDirectorySet = QDir::setCurrent( path );
	if( !importDirectorySet ) {
	    path = QDir::tempPath();
	    importDirectorySet = QDir::setCurrent( path );
	}
    }
    pathDisplayLabel->setText( QDir::toNativeSeparators( path ) );

    /*
     *  Set the default destination.
     */
    if( importDirectorySet ) {
	saveToComputer->setChecked( true );
    }
    else {
	saveToCamera->setChecked( true );
    }

    /*
     *  Set the default subfolder.
     */
    subfolderCheck->setChecked( false );
    QDate date = QDate::currentDate();
    subfolder = date.toString( Qt::ISODate );
    subfolderLineEdit->setText( subfolder );

    /*
     *  Set the default prefix and beginning sequence number.
     */
    renamingOption = 3;
    renamingSequence = 1;
    renamingPrefix = "prefix";

    setExample( renamingOption );
    fileNamingComboBox->setCurrentIndex( renamingOption );
    prefixLineEdit->setText( renamingPrefix );
    sequenceSpinBox->setValue( renamingSequence );
}

/*
 *  Set where to save images.
 */
void
Import::saveTo( bool checked )
{
    if( checked ) {
	if( saveToCamera->isChecked() ) {
	    emit propertyChanged( kEdsPropID_SaveTo, kEdsSaveTo_Camera );
	    folderGroupBox->setEnabled( false );
	    namingGroupBox->setEnabled( false );
	}
	else if( saveToComputer->isChecked() ) {
	    emit propertyChanged( kEdsPropID_SaveTo, kEdsSaveTo_Host );
	    folderGroupBox->setEnabled( true );
	    namingGroupBox->setEnabled( true );
	}
	else {
	    emit propertyChanged( kEdsPropID_SaveTo, kEdsSaveTo_Both );
	    folderGroupBox->setEnabled( true );
	    namingGroupBox->setEnabled( true );
	}
    }
}

int
Import::getSaveTo()
{
    if( saveToCamera->isChecked() ) {
	return( kEdsSaveTo_Camera );
    }
    else if( saveToComputer->isChecked() ) {
	return( kEdsSaveTo_Host );
    }
    else {
	return( kEdsSaveTo_Both );
    }
}

/*
 *  Unable to find a more elegant way to determine whether
 *  a directory is writeable.
 */
bool
Import::isWritable( QString path )
{
    QFile f("cameraadjunct.log");
    if( !f.open( QIODevice::WriteOnly ) ) {
	f.close();
	return( false );
    }
    else {
	f.close();
	f.remove();
	return( true );
    }
}

/*
 *  Browse for an existing directory in which to import images.
 */
void
Import::updatePath()
{
    QString newPath =
	QFileDialog::getExistingDirectory( this, tr("Destination Folder"),
		path, QFileDialog::ShowDirsOnly );

    if( newPath.isEmpty() ) {
	return;	// cancel
    }
    bool ok;
    ok = QDir::setCurrent( newPath );
    ok = ok && isWritable( newPath );
    if( !ok ) {
	QMessageBox::warning( this, tr("Destination Folder"),
	    newPath + QString("\n") +
	    tr("You don't have permission to save files in this location.") );
	return;
    }

    path = newPath;
    pathDisplayLabel->setText( QDir::toNativeSeparators( path ) );
    if( subfolderCheck->isChecked() ) {
	setImportDirectory( Qt::Checked );
    }
}

void
Import::updateSubfolder()
{
    subfolder = subfolderLineEdit->text();
    if( !subfolder.isEmpty() ) {
	if( !validFileName( &subfolder ) ) {
	    subfolderLineEdit->setText( subfolder );
	    QMessageBox::information( this, tr("Destination Subfolder"),
		subfolder + QString("\n") +
		tr("The subfolder has been modified to be a valid\n"
		   "directory name. Some characters have been replaced.") );
	}
    }
    if( subfolderCheck->isChecked() ) {
	setImportDirectory( Qt::Checked );
    }
}

/*
 *  The working directory is set to "path/subfolder".
 *  Subfolder is created if it isn't empty and it doesn't already exist.
 */
void
Import::setImportDirectory( int state )
{
    bool ok;
    QString workingDirectory;

    if( subfolder.isEmpty() || (state == Qt::Unchecked) ) {
	ok = QDir::setCurrent( path );
	workingDirectory = path;
    }
    else {
	QDir directory;
	workingDirectory = path + "/" + subfolder;
	directory.setPath( workingDirectory );
	if( directory.exists() ) {
	    ok = QDir::setCurrent( workingDirectory );
	}
	else {
	    ok = directory.mkpath( workingDirectory );
	    ok = ok && QDir::setCurrent( workingDirectory );
	}
    }

    if( subfolderCheck->isChecked() && !ok ) {
	QMessageBox::warning( this, tr("Destination Subfolder"),
	    workingDirectory + QString("\n") +
	    tr("The subfolder cannot be created or set as working directory.") );
	subfolderCheck->setChecked( false );
    }
}

/*
 *  Give an example for file naming.
 */
void
Import::setExample( int index )
{
    QDate date = QDate::currentDate();
    switch( index ) {
	case 0:
	    exampleDisplayLabel->setText( "prefix-0001.CR2" );
	    break;
	case 1:
	    exampleDisplayLabel->setText( "prefix-1.CR2" );
	    break;
	case 2:
	    exampleDisplayLabel->setText(
		date.toString( Qt::ISODate ) +
		"-IMG_0001.CR2" );
	    break;
	case 3:
	    exampleDisplayLabel->setText( "IMG_0001.CR2" );
	    break;
    }
}

/*
 *  Show widgets for gathering file naming information.
 */
void
Import::updateFileNaming( int index )
{
    switch( index ) {
	/*
	 *  Get custom prefix.
	 */
	case 0:
	    prefixLabel->show();
	    prefixLineEdit->show();
	    sequenceLabel->hide();
	    sequenceSpinBox->hide();
	    break;
	/*
	 *  Get custom prefix, start number, leading zeros.
	 */
	case 1:
	    prefixLabel->show();
	    prefixLineEdit->show();
	    sequenceLabel->show();
	    sequenceSpinBox->show();
	    break;
	/*
	 *  No further information needed.
	 */
	case 2:
	case 3:
	    prefixLabel->hide();
	    prefixLineEdit->hide();
	    sequenceLabel->hide();
	    sequenceSpinBox->hide();
	    break;
    }
    renamingOption = index;
    setExample( index );
}

void
Import::updateSequence( int value )
{
    renamingSequence = value;
}

void
Import::updatePrefix()
{
    renamingPrefix = prefixLineEdit->text();
    if( renamingPrefix.isEmpty() ) {
	QMessageBox::warning( this, tr("File Naming"),
	    tr("File name prefix cannot be empty.") );
	prefixLineEdit->setText( "prefix" );
	renamingPrefix = "prefix";
    }
    else
    if( !validFileName( &renamingPrefix ) ) {
	prefixLineEdit->setText( renamingPrefix );
	QMessageBox::information( this, tr("File Naming"),
	    renamingPrefix + QString("\n") +
	    tr("Prefix has been modified so that it will form a valid\n"
	       "file name. Some characters have been replaced.") );
    }
}

/*
 *  Check file name for invalid characters.
 */
bool
Import::validFileName( QString *name )
{
    bool modified = false;
    for( int i = 0; i < name->size(); i++ ) {
	QChar c = (*name)[i];
	if( c.isLetter() ||
	    c.isNumber() ||
	    ((c == QChar('-')) && (i != 0)) ||
	    (c == QChar('_')) ) {
	    continue;
	}
	else {
	    (*name)[i] = QChar('_');
	    modified = true;
	}
    }

    return( !modified );
}

/*
 *  File renaming.
 *
 *  DCF files names are 8 characters plus a 3 character file extension.
 *  The only allowed characters are 0..9, A..Z, _ (underscore).
 *  File extensions include: JPG, TIF, THM, WAV, TXT, CR2, MOV.
 */
QString
Import::renameHelper( QString fileName )
{
    QString newName;
    QDate date = QDate::currentDate();
    int p = fileName.indexOf( QRegExp("[0-9.]"), 0 );
    int q = fileName.indexOf( ".", 0 );

    switch( renamingOption ) {
	/*
	 *  Template: Custom Prefix - Original File Number.
	 */
	case 0:
	    newName = renamingPrefix +
		      "-" +
		      fileName.remove( 0, p );
	    break;
	/*
	 *  Template: Custom Prefix - Sequence.
	 */
	case 1:
	    newName = renamingPrefix +
		      "-" +
		      QString::number( renamingSequence ) +
		      fileName.remove( 0, q );
	    renamingSequence++;
	    sequenceSpinBox->setValue( renamingSequence );
	    break;
	/*
	 *  Template: Date - Filename.
	 */
	case 2:
	    if( fileName.startsWith("_") ) {
		newName = date.toString( Qt::ISODate ) +
			  fileName;
	    }
	    else {
		newName = date.toString( Qt::ISODate ) +
			  "-" +
			  fileName;
	    }
	    break;
	/*
	 *  Template: Filename.
	 */
	case 3:
	    newName = fileName;
	    break;
    }

    return( newName );
}

void
Import::rename( char *newName, char *origName )
{
    QString s(renameHelper( origName ));

    for( int i = 0; i < s.size(); i++ ) {
	newName[i] = s.at(i).toAscii();
    }
    newName[s.size()] = '\0';
}

bool
Import::getSaveFile( char *fileName )
{
    QMessageBox::StandardButton answer;
    QString msg = QString(fileName) +
		  tr(" already exists.\n") +
		  tr("Do you want to replace it?");
    answer = QMessageBox::warning(this, tr("Confirm Save As"), msg,
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No );

    if( answer == QMessageBox::Yes ) {
	return( true );
    }
    else {
	/*
	 *  Get a new file name.
	 */
	QString suffix(fileName);
	int q = suffix.indexOf( ".", 0 );
	suffix.remove( 0, q );
	suffix = "*" + suffix;

	QString s =
	    QFileDialog::getSaveFileName( this,
		tr("Save File:"),
		QDir::currentPath(),
		suffix
	    );

	if( s.isEmpty() ) {
	    return( false );
	}
	else {
	    for( int i = 0; i < s.size(); i++ ) {
		fileName[i] = s.at(i).toAscii();
	    }
	    fileName[s.size()] = '\0';
	    return( true );
	}
    }
}

void
Import::readSettings()
{
    QSettings settings;

    settings.beginGroup( "Import" );
    resize( settings.value("size", QSize(300, 300)).toSize() );
    move( settings.value("pos", QPoint(100, 100)).toPoint() );
    settings.endGroup();
}

void
Import::writeSettings()
{
    QSettings settings;

    settings.beginGroup( "Import" );
    settings.setValue( "size", size() );
    settings.setValue( "pos", pos() );
    settings.setValue( "path", path );
    settings.endGroup();
}

/*
 *  Reimplementation of closeEvent to save settings.
 */
void
Import::closeEvent( QCloseEvent *event )
{
    writeSettings();
    event->accept();
}

