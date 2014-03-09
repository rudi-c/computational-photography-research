/*
 *  View of camera in model-view-controller design.
 */
#ifndef _VIEW_H_
#define _VIEW_H_

#include <QMainWindow>
#include <QDateTime>

class QAction;
class QBoxLayout;
class QButtonGroup;
class QComboBox;
class QLabel;
class QMenu;
class QProgressBar;
class QPushButton;
class QScrollArea;
class QScrollBar;
class QSlider;
class QWidget;

class Camera;
class Event;
class Aperture;
class DriveMode;
class EvfImageData;
class ExposureComp;
class FocusMode;
class Histogram;
class ISOSpeed;
class Import;
class LiveImage;
class LogBrowser;
class MeteringMode;
class Navigation;
class Options;
class Panel;
class ShootingMode;
class ShotSeq;
class ShutterSpeed;
class MultiShot;
class Tools;
class WhiteBalance;

class View : public QMainWindow
{
    Q_OBJECT

    public:
	View( Camera *camera, Import *import,
	      QWidget *parent = 0, Qt::WFlags flags = 0 );
	~View();

    public slots:
	void updateProperty( int property, int parameter = 0 );
	void updatePropertyList( int property );
	void updateLiveView( EvfImageData *data );
	void updatePanorama( QImage image, int shot );
	void eventReport( Event *event );
	void appendLog( const QString text );

    signals:
	void propertyChanged( int property, int value );
	void propertyChanged( int property, QString string );
	void propertyChanged( int property, QDateTime dateTime );
	void propertyChanged( int property, int *shift );
	void customPropertyChanged( int property, int value, int parameter );
	void focusPropertyChanged( int selectionMode, int fp );
	void startLiveView();
	void stopLiveView();
	void startVideo();
	void stopVideo();
	void startSequence( ShotSeq *shotSeq );
	void cancelSequence();
	void highISOtest( int shutterSpeed );
	void focusAdjustment( int value );
	void autoFocusChanged( int status );
	void shutterChanged( int value );
	void zoomRectChanged( int x, int y );

    protected slots:
	void showTools();
	void showContextMenu();
	void zoomLevelChanged( int zoom );
	void setDOFPreview( int index );
	void toEvfAFValue( int index );
	void autoFocusInitiated();
	void autoFocusDiscontinued();
	void autoMeteringInitiated();
	void autoMeteringDiscontinued();
	void shutterInitiated();
	void shutterDiscontinued();
	void liveviewButtonChanged( bool checked );
	void videoButtonChanged( bool checked );
	void setHistogramMode( int index );
	void updateHorizontalSlider( int min, int max );
	void updateVerticalSlider( int min, int max );
	void horizontalSliderMoved( int value );
	void verticalSliderMoved( int value );
	void setRotation( int index );
	void setFocusMap( int index );
	void zoomIn();
	void zoomOut();
	void setZoomFactor( int value );

    protected:
	Camera *camera;

	/*
	 *  Main window.
	 */
	void createMainWindow();
	QWidget *centralWidget;
	void closeEvent( QCloseEvent *event );
	void readSettings();
	void writeSettings();

	/*
	 *  Live view display.
	 */
	QScrollArea *liveScrollArea;
	LiveImage *liveImage;
	bool eventFilter( QObject *target, QEvent *event );
	void processZoomEvent( int delta );
	bool scrollBarUsesWheel( QScrollBar *bar );
	bool inLiveViewMode;
	int scalingFactor;
	int hMax;
	int vMax;
	bool hMoved;
	bool vMoved;
	static const int MAX_SCALING_FACTOR;
	static const double scaling[];

	/*
	 *  Panels.
	 */
	QScrollArea *panelArea;
	QWidget *shootingPanelWidget();

	/*
	 *  Histogram panel.
	 */
	Panel *histogramPanel;
	QBoxLayout *histogramPanelLayout();
	Histogram *histogram;
	QComboBox *histogramModeComboBox;
	QComboBox *clippingComboBox;
	int histogramMode;
	int getClippingSetting();

	/*
	 *  Tools panel.
	 */
	QLayout *toolbarLayout();
	Import *import;
	LogBrowser *logBrowser;
	Options *options;
	MultiShot *multiShot;
	void cleanupMultiShot();
	Tools *tools;

	/*
	 *  Exposure panel.
	 */
	Panel *exposurePanel;
	QBoxLayout *exposurePanelLayout();
	Aperture *aperture;
	ShutterSpeed *shutterSpeed;
	ISOSpeed *isoSpeed;
	ShootingMode *shootingMode;
	MeteringMode *meteringMode;
	WhiteBalance *whiteBalance;
	ExposureComp *exposureComp;
	DriveMode *driveMode;

	/*
	 *  Focus panel.
	 */
	Panel *focusPanel;
	QBoxLayout *focusPanelLayout();
	FocusMode *focusMode;
	QComboBox *focusModeLive;
	QPushButton *dofPreviewButton;
	void updateEvfAFValue( Camera *camera );
	void setEvfAFList( Camera *camera );
	int nAF;  // number of choices for Evf AF mode
	int *vAF; // list of choices for Evf AF mode

	/*
	 *  View/Navigation panel.
	 */
	Panel *navigationPanel;
	QBoxLayout *navigationPanelLayout();
	int getOverlaySetting();
	QSlider *zoomSlider;
	QComboBox *overlayComboBox;
	QComboBox *rotationComboBox;
	QButtonGroup *scaleChosen;
	Navigation *navigation;

	/*
	 *  Context menu.
	 */
	void createContextMenu();
	static const int MAX_Menu = 27;
	static const char *menuText[];
	QMenu *menu;
	QAction *action[MAX_Menu];

	/*
	 *  Status bar.
	 */
	void createStatusBar();
	void updateProgressBar( int percent );
	QStatusBar *statusBar;
	QProgressBar *progressBar;
	void updateBatteryLevel();
	QLabel *battery;
	QLabel *alert;
	void updateConnectionStatus();
	QLabel *connectionStatus;
};

#endif // _VIEW_H_
