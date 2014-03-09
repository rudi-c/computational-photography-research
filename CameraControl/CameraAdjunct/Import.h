/*
 *  Import dialog: where to save images and how to rename them.
 */
#ifndef _IMPORT_H_
#define _IMPORT_H_

#include <QDialog>
#include <QDir>
#include "EDSDKTypes.h"

class QComboBox;
class QCheckBox;
class QLineEdit;
class QLabel;
class QPushButton;
class QSpinBox;
class QRadioButton;
class QGroupBox;
class Camera;

class Import : public QDialog
{
    Q_OBJECT

    public:
	Import( QWidget *parent = 0, Qt::WFlags flags = 0 );
	~Import();
	void rename( char *newName, char *origName );
	int  getSaveTo();
	bool getSaveFile( char *fileName );

    signals:
	void propertyChanged( int property, int newValue );

    protected slots:
	void saveTo( bool checked );
	void setImportDirectory( int state );
	void updatePath();
	void updateSubfolder();
	void updatePrefix();
	void updateFileNaming( int index );
	void updateSequence( int value );

    protected:
	void initialize();
	void setDefaults();
	void setExample( int index );
	bool validFileName( QString *name );
	void readSettings();
	void writeSettings();
	void closeEvent( QCloseEvent *event );
	QPushButton *closeButton;

	/*
	 *  Destination: camera, computer, both.
	 */
	QRadioButton *saveToCamera;
	QRadioButton *saveToComputer;
	QRadioButton *saveToBoth;

	/*
	 *  Destination directory.
	 */
	bool isWritable( QString path );
	QString path;
	QLabel *pathDisplayLabel;
	QString subfolder;
	QCheckBox *subfolderCheck;
	QLineEdit *subfolderLineEdit;
	QGroupBox *folderGroupBox;

	/*
	 *  File naming.
	 */
	int renamingOption;
	int renamingSequence;
	QString renameHelper( QString fileName );
	QString renamingPrefix;
	QLabel *exampleDisplayLabel;
	QComboBox *fileNamingComboBox;
	QLabel *prefixLabel;
	QLineEdit *prefixLineEdit;
	QLabel *sequenceLabel;
	QSpinBox *sequenceSpinBox;
	QGroupBox *namingGroupBox;
};

#endif // _IMPORT_H_
