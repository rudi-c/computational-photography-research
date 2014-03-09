/*
 *  Tools: depth of field and field of view calculators.
 */
#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <QDialog>

class QTabWidget;
class QStackedWidget;
class QSlider;
class QComboBox;
class QLineEdit;
class QLabel;
class QSpinBox;
class DepthOfField;
class Camera;

class Tools : public QDialog
{
    Q_OBJECT

    public:
	Tools( Camera *camera, QWidget *parent = 0, Qt::WFlags flags = 0 );
	~Tools();

    protected slots:
	/*
	 *  Depth of field page.
	 */
	void setAperture( int index );
	void setSubjectDistance( const QString &text );
	void setUnits( int index );

	/*
	 *  Depth of field and field of view pages.
	 */
	void setCamera( int index );
	void setFocalLength( const QString &text );

	/*
	 *  Field of view page.
	 */
	void setOrientation( bool checked );
	void setRows( int value );
	void setCols( int value );
	void setCoverage( int value );

    protected:
	Camera *camera;
	QTabWidget *tabWidget;
	void initialize();
	void setDefaults();
	void readSettings();
	void writeSettings();
	void closeEvent( QCloseEvent *event );

	/*
	 *  Depth of field page.
	 */
	QWidget *getDepthOfFieldPage();
	QComboBox *dofCameraComboBox;
	QLineEdit *dofFocalLengthLineEdit;
	QStackedWidget *apertureStackedWidget;
	QSlider *apertureSlider;
	QLineEdit *subjectDistanceLineEdit;
	QComboBox *unitsComboBox;
	DepthOfField *subjectDisplay;
	DepthOfField *hyperfocalDisplay;
	double circleOfConfusion;
	double aperture;
	double subjectDistance;
	int    units;

	/*
	 *  Field of view page.
	 */
	QWidget *getFieldOfViewPage();
	QString angleToText( double angle );
	void setFOVData( double hAngle, double vAngle );
	QComboBox *fovCameraComboBox;
	QLineEdit *fovFocalLengthLineEdit;
	QLabel *hAngle1;
	QLabel *vAngle1;
	QLabel *hAngleP;
	QLabel *vAngleP;
	QLabel *rowText;
	QLabel *colText;
	bool landscapeOrientation;
	int rows; 
	int cols; 
	double coverage;

	/*
	 *  Depth of field and field of view pages.
	 */
	void updateCalculations();
	double sensorWidth;
	double sensorHeight;
	double focalLength;

	static const float Full;
	static const float APS_H;
	static const float APS_C;

	typedef struct {
	    char *description;
	    double circleOfConfusion;
	    double sensorWidth;
	    double sensorHeight;
	} SensorDimensionType;
	static const SensorDimensionType SensorMap[];

	typedef struct {
	    char *description;
	    double exponent;
	} PairType;
	static const PairType ApertureMap[];
};

#endif // _TOOLS_H_
