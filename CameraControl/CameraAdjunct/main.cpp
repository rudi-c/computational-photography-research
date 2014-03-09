
#include <QtGui/QApplication>
#include <QMetaType>
#include "View.h"
#include "Camera.h"
#include "Event.h"
#include "Import.h"
#include "PrepareImage.h"
#include "Controller.h"
#include "LiveImageData.h"


void connections( View *view, Controller *controller, PrepareImage *prepareImage );

int main( int argc, char *argv[] )
{
    QApplication app(argc, argv);
    app.setOrganizationName("PvB");
    app.setOrganizationDomain("pvb.ca");
    app.setApplicationName("Camera Adjunct");
    app.setWindowIcon( QIcon(":/Resources/Misc/camera.png") );

    Camera camera;
    Import import( 0, Qt::Window );
    Controller controller( &camera, &import );
    PrepareImage prepareImage;
    View view( &camera, &import );

    connections( &view, &controller, &prepareImage );
    if( controller.openSessionCommand() ) {
	controller.messagePump();
    }

    view.show();
    app.exec();

    controller.closeSessionCommand();
}

void
connections( View *view, Controller *controller, PrepareImage *prepareImage )
{
    /*
     *  View --> Controller
     */
    QObject::connect(	// property changes
	view, SIGNAL(propertyChanged(int,int)),
	controller, SLOT(setPropertyCommand(int,int)));
    QObject::connect(	// property changes
	view, SIGNAL(propertyChanged(int,QString)),
	controller, SLOT(setPropertyCommand(int,QString)));
    QObject::connect(	// property changes
	view, SIGNAL(propertyChanged(int,QDateTime)),
	controller, SLOT(setPropertyCommand(int,QDateTime)));
    QObject::connect(	// property changes
	view, SIGNAL(propertyChanged(int,int *)),
	controller, SLOT(setPropertyCommand(int,int *)));
    QObject::connect(	// custom property changes
	view, SIGNAL(customPropertyChanged(int,int,int)),
	controller, SLOT(setPropertyCommand(int,int,int)));
    QObject::connect(	// focus property changes
	view, SIGNAL(focusPropertyChanged(int,int)),
	controller, SLOT(setFocusPropertyCommand(int,int)));
    QObject::connect(	// auto metering and shutter
	view, SIGNAL(shutterChanged(int)),
	controller, SLOT(shutterCommand(int)));
    QObject::connect(	// live view
	view, SIGNAL(startLiveView()),
	controller, SLOT(startLiveView()));
    QObject::connect(	// live view
	view, SIGNAL(stopLiveView()),
	controller, SLOT(stopLiveView()));
    QObject::connect(	// live view focus adjustment
	view, SIGNAL(focusAdjustment(int)),
	controller, SLOT(focusAdjustment(int)));
    QObject::connect(	// live view auto focus
	view, SIGNAL(autoFocusChanged(int)),
	controller, SLOT(autoFocusChanged(int)));
    QObject::connect(	// live view move zoom rectangle
	view, SIGNAL(zoomRectChanged(int,int)),
	controller, SLOT(moveZoomRectCommand(int,int)));
    QObject::connect(	// video
	view, SIGNAL(startVideo()),
	controller, SLOT(startVideo()));
    QObject::connect(	// video
	view, SIGNAL(stopVideo()),
	controller, SLOT(stopVideo()));
    QObject::connect(	// multi-shot
	view, SIGNAL(startSequence(ShotSeq *)),
	controller, SLOT(startSequence(ShotSeq *)));
    QObject::connect(	// multi-shot
	view, SIGNAL(cancelSequence()),
	controller, SLOT(cancelSequence()));
    QObject::connect(	// multi-shot
	view, SIGNAL(highISOtest(int)),
	controller, SLOT(highISOtest(int)) );

    /*
     *  Controller --> View
     */
    QObject::connect(	// property changes
	controller, SIGNAL(propertyChanged(int,int)),
	view, SLOT(updateProperty(int,int)));
    QObject::connect(	// property changes
	controller, SIGNAL(propertyListChanged(int)),
	view, SLOT(updatePropertyList(int)));
    qRegisterMetaType<QImage>("QImage");
    QObject::connect(	// image report
	controller, SIGNAL(updatePanorama(QImage,int)),
	view, SLOT(updatePanorama(QImage,int)));
    qRegisterMetaType<Event>("Event");
    QObject::connect(	// event reports
	controller, SIGNAL(eventReport(Event *)),
	view, SLOT(eventReport(Event *)));

    /*
     *  Controller --> PrepareImage --> View
     */
/*
    qRegisterMetaType<EvfImageData>("EvfImageData");
    QObject::connect(	// live view
	controller, SIGNAL(liveViewChanged(EvfImageData *)),
	view, SLOT(updateLiveView(EvfImageData *)));
*/
    qRegisterMetaType<EvfImageData>("EvfImageData");
    QObject::connect(	// live view
	controller, SIGNAL(liveViewChanged(EvfImageData *)),
	prepareImage, SLOT(updateLiveView(EvfImageData *)));
    QObject::connect(	// live view
	prepareImage, SIGNAL(liveViewChanged(EvfImageData *)),
	view, SLOT(updateLiveView(EvfImageData *)));
}

