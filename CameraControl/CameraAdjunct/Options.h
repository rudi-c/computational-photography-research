/*
 *  Options dialog: some less frequently changed exposure, image,
 *  and other settings.
 */
#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <QDialog>
#include <QLabel>
#include <QDateTime>

class QComboBox;
class QTabWidget;
class QDateTimeEdit;
class QLineEdit;
class QLabel;
class Camera;

class Options : public QDialog
{
    Q_OBJECT

    public:
	Options( Camera *camera, QWidget *parent = 0, Qt::WFlags flags = 0 );
	~Options();
	void updateProperty( int property, int parameter );
	void updatePictureStyleList();
	void updateImageQualityList();

    signals:
	void customPropertyChanged( int property, int value, int parameter );
	void propertyChanged( int property, int value );
	void propertyChanged( int property, QString string );
	void propertyChanged( int property, QDateTime dateTime );
	void propertyChanged( int property, int *shift );

    protected slots:
	void setPictureStyle( int index );
	void setImageQuality( int index );
	void setLongExposureNoiseReduction( int index );
	void setISONoiseReduction( int index );
	void setMirrorLockup( int index );
	void setExposureLevelIncrement( int index );
	void setWhiteBalanceShift();
	void scrollWhiteBalanceShift( int quadrant );
	void updateAuthorName();
	void updateCopyright();
	void synchronizeDateTime();
	void updateDateTime( const QDateTime &dateTime );
	void setFontSize( int index );
	void setColorSpace( int index );

    protected:
	Camera *camera;
	QTabWidget *tabWidget;
	void initialize();
	void showEvent( QShowEvent *event );
	void showConnected( bool status );
	void readSettings();
	void writeSettings();
	void closeEvent( QCloseEvent *event );

	/*
	 *  Image page: long exposure noise reduction, high ISO noise
	 *  reduction, highlight tone priority, color space.
	 */
	QWidget *getImagePage();
	QWidget *imagePage;
	QComboBox *pictureStyleComboBox;
	int nPS;  // number of entries
	int *vPS; // list of picture styles
	int nIQ;  // number of entries
	int *vIQ; // list of image qualities
	QComboBox *imageQualityComboBox;
	QComboBox *longExposureNoiseComboBox;
	QComboBox *isoNoiseComboBox;
	QComboBox *highlightToneComboBox;
	QComboBox *colorSpaceComboBox;
	void updatePictureStyleValue();
	void updateImageQualityValue();

	/*
	 *  Exposure page: mirror lockup, increments, ISO expansion.
	 */
	class WBShift : public QLabel
	{
	    public:
		WBShift( QWidget *parent ) { wbX = 0; wbY = 0; };
		~WBShift() {};
		void setValues( int x, int y );
		void getValues( int *x, int *y );
		void scrollValues( int dx, int dy );
	    protected:
		int wbX;
		int wbY;
		void paintEvent( QPaintEvent *event );
	};
	QWidget *getExposurePage();
	QWidget *exposurePage;
	QComboBox *mirrorLockupComboBox;
	QComboBox *incrementComboBox;
	WBShift *wbShift;
	void updateWBShiftValue();

	/*
	 *  EXIF page: camera information, copyright information, data & time.
	 */
	QWidget *getExifPage();
	QWidget *exifPage;
	QLabel *modelNameDisplay;
	QLabel *bodySerialNumberDisplay;
	QLabel *firmwareVersionDisplay;
	QLineEdit *authorNameLineEdit;
	QLineEdit *copyrightLineEdit;
	QDateTimeEdit *cameraDateEdit;
	void updateExifPage();

	/*
	 *  Interface page:
	 */
	QWidget *getInterfacePage();
	QWidget *interfacePage;
	QComboBox *fontSizeComboBox;
};

#endif // _OPTIONS_H_
