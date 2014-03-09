/*
 *  Panel for histogram.
 */

#include <QtGui>
#include "View.h"
#include "LiveImageData.h"
#include "Widgets/Histogram.h"


QBoxLayout *
View::histogramPanelLayout()
{
    histogram = new Histogram( this );

    QLabel *modeLabel = new QLabel( tr("Histogram mode:") );
    histogramModeComboBox = new QComboBox();
    histogramModeComboBox->addItem( "Luminosity",
	EvfImageData::HISTOGRAM_Luminosity );
    histogramModeComboBox->addItem( "RGB",
	EvfImageData::HISTOGRAM_RGB );
    QSettings settings;
    histogramMode = settings.value( "histogramMode",
	EvfImageData::HISTOGRAM_RGB ).toInt();
    if( histogramMode == EvfImageData::HISTOGRAM_Luminosity ) {
	histogramModeComboBox->setCurrentIndex( 0 );
    }
    else {
	histogramModeComboBox->setCurrentIndex( 1 );
    }
    QObject::connect(
	histogramModeComboBox, SIGNAL(currentIndexChanged(int)),
	this, SLOT(setHistogramMode(int)) );
    QVBoxLayout *modeLayout = new QVBoxLayout();
    modeLayout->addWidget( modeLabel );
    modeLayout->addWidget( histogramModeComboBox );

    QLabel *clippingLabel = new QLabel( tr("Clipping preview:") );
    clippingComboBox = new QComboBox();
    for( int i = EvfImageData::ShowClippingNone;
	     i <= EvfImageData::ShowClippingLast; i++ ) {
	clippingComboBox->addItem( menuText[i], i );
    }
    clippingComboBox->setCurrentIndex( 0 );

    QVBoxLayout *clippingLayout = new QVBoxLayout();
    clippingLayout->addWidget( clippingLabel );
    clippingLayout->addWidget( clippingComboBox );

    QHBoxLayout *selectionLayout = new QHBoxLayout();
    selectionLayout->addStretch( 2 );
    selectionLayout->addLayout( modeLayout );
    selectionLayout->addStretch( 1 );
    selectionLayout->addLayout( clippingLayout );
    selectionLayout->addStretch( 2 );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget( histogram );
    layout->setAlignment( histogram, Qt::AlignHCenter );
    layout->addLayout( selectionLayout );
    layout->setContentsMargins( 0, 5, 0, 5 );

    return( layout );
}

/*
 *  Determine current setting for how to display clipping
 *  on live view image.
 */
int
View::getClippingSetting()
{
    int index = clippingComboBox->currentIndex();
    int flag = clippingComboBox->itemData( index ).toInt();

    return( flag );
}

void
View::setHistogramMode( int index )
{
    histogramMode = histogramModeComboBox->itemData( index ).toInt();

    QSettings settings;
    settings.setValue( "histogramMode", histogramMode );
}

