/*
 *  Multi-shot dialog: multiple shots for exposure bracketing,
 *  focus bracketing, super-resolution, noise reduction, interval
 *  shooting, and panoramas.
 */
#ifndef _MULTISHOT_H_
#define _MULTISHOT_H_

#include <QDialog>
#include <QLabel>

class QComboBox;
class QScrollArea;
class QSlider;
class QSpinBox;
class QTabWidget;
class QTimeEdit;

class Camera;
class Event;
class ShotSeq;
class Overlay;
class Checklist;
class ExposureComp;
class Frames;
class Increment;
class ImageTable;

class MultiShot : public QDialog
{
    Q_OBJECT

    public:
	MultiShot( Camera *camera, QWidget *parent = 0, Qt::WFlags flags = 0 );
	~MultiShot();
	void updateProperty( int property, int parameter = 0 );
	void updatePropertyList( int property );
	bool sequenceInProgress();
	void finishSeq();
	enum {
	    ORDER_LtoR,
	    ORDER_LtoR_alt,
	    ORDER_RtoL,
	    ORDER_RtoL_alt
	};
	void setImage( QImage image, int rotationFlag, int shot );
	Overlay *getOverlay( int shot );

    signals:
	void propertyChanged( int property, int value );
	void startSequence( ShotSeq *shotSeq );
	void cancelSequence();
	void highISOtest( int shutterSpeed );
	void appendLog( const QString text );

    protected slots:
	void checklistButtonClicked();
	void runButtonClicked();
	void stopButtonClicked();
	void updateIntervalTiming();
	void updateChecklist();
	void highISOtestButtonClicked();

    protected:
	Camera *camera;
	ShotSeq *shotSeq;
	Checklist *checklist;
	QTabWidget *tabWidget;
	void initialize();
	void setShotSeq();
	void setShutterSpeeds();
	void setCompensations();
	bool inProgress;
	void startSeq();
	void updateLog( bool start );
	QPushButton *runButton;
	QPushButton *stopButton;
	void readSettings();
	void writeSettings();
	void closeEvent( QCloseEvent *event );

	/*
	 *  Exposure bracketing page.
	 */
	QWidget *getExposureBracketingPage();
	QWidget *exposureBracketingPage;
	ExposureComp *eCompensation;
	QSlider *eFrames;
	int step;
	QLabel *eIncrementPage[10];
	void setIncrementPages();
	QSlider *eIncrement;
	static const char *textHalves[];
	static const char *textThirds[];

	/*
	 *  Focus bracketing page.
	 */
	QWidget *getFocusBracketingPage();
	QWidget *focusBracketingPage;
	QSpinBox *fFrames;
	QComboBox *fStep;

	/*
	 *  Intervals page.
	 */
	QWidget *getIntervalsPage();
	QWidget *intervalsPage;
	class TimingDisplay : public QLabel
	{
	    public:
		TimingDisplay( QWidget *parent );
		~TimingDisplay() {};
		void updateTiming(
			int speed,
			QTime &bulb,
			QTime &delay,
			QTime &interval,
			int frames );
	    protected:
		void paintEvent( QPaintEvent *event );
		QString delay;
		QString interval;
		QString shutter;
		QString estTotalTime;
	};
	TimingDisplay *timingDisplay;
	int msec( const QTime &time );
	QTimeEdit *iDelay;
	QTimeEdit *iInterval;
	QTimeEdit *iBulb;
	QPushButton *iTest;
	QSpinBox *iFrames;

	/*
	 *  Stitching images page.
	 */
	QWidget *getStitchPage();
	QWidget *stitchPage;
	QScrollArea *scrollArea;
	ImageTable *imageTable;
	QSpinBox *columns;
	QSpinBox *rows;
	QSpinBox *overlap;
	QSpinBox *opacity;
};

#endif // _MULTISHOT_H_
