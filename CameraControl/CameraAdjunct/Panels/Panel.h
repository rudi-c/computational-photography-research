/*
 *  Panel widget.
 */
#ifndef _PANEL_H_
#define _PANEL_H_

#include <QWidget>


class Panel : public QWidget
{
    public:
	Panel( QString panelName, QWidget *parent = 0 );
	~Panel();
	void setPanelLayout( QBoxLayout *layout );

    protected:
	void initialize( QString panelName );
	QIcon icon;
	QWidget *panelExtension;
};

#endif // _PANEL_H_
