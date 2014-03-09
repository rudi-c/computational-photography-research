/*
 *  Panel widget.
 */

#include <QtGui>
#include "Panel.h"


/*
 *  Constructor for a panel.
 */
Panel::Panel( QString panelName, QWidget *parent )
    : QWidget( parent )
{
    initialize( panelName );
}

/*
 *  Destructor for a panel.
 */
Panel::~Panel()
{
}

void
Panel::initialize( QString panelName )
{
    QFont font;
    font.setFamily( "Calibri" );
    font.setPointSize( 13 );

    const int panelWidth = 256;

    QLabel *panelLabel = new QLabel( panelName );
    panelLabel->setAlignment( Qt::AlignRight );
    panelLabel->setFont( font );

    QToolButton *panelButton = new QToolButton();
    icon.addPixmap( QPixmap(":/Resources/Misc/triangle-left.png"),
		QIcon::Normal, QIcon::Off );
    icon.addPixmap( QPixmap(":/Resources/Misc/triangle-down.png"),
		QIcon::Normal, QIcon::On );
    panelButton->setIcon( icon );
    panelButton->setIconSize( QSize(16,16) );
    panelButton->setAutoRaise( true );
    panelButton->setCheckable( true );

    QFrame *line = new QFrame(); 
    line->setFixedWidth( panelWidth );
    line->setFrameShape( QFrame::HLine ); 
    line->setFrameShadow( QFrame::Sunken ); 
    line->setLineWidth( 0 ); 
    line->setMidLineWidth( 2 ); 

    QHBoxLayout *barLayout = new QHBoxLayout();
    barLayout->addWidget( panelLabel, 1, Qt::AlignVCenter );
    barLayout->addWidget( panelButton, 0, Qt::AlignVCenter );

    panelExtension = new QWidget();
    panelExtension->hide();

    QObject::connect(
	panelButton, SIGNAL(toggled(bool)),
	panelExtension, SLOT(setVisible(bool)) );

    QVBoxLayout *panelLayout = new QVBoxLayout( this );
    panelLayout->addLayout( barLayout );
    panelLayout->addWidget( panelExtension );
    //panelLayout->addSpacing( -2 );
    panelLayout->addWidget( line );

    setLayout( panelLayout );
}

void
Panel::setPanelLayout( QBoxLayout *layout )
{
    panelExtension->setLayout( layout );
}

