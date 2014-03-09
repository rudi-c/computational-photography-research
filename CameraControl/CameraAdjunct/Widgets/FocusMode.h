/*
 *  Auto focus mode setting and display widget.
 */
#ifndef _FOCUSMODE_H_
#define _FOCUSMODE_H_

#include <QWidget>

class QComboBox;
class Camera;


class FocusMode : public QWidget
{
    Q_OBJECT

    public:
	FocusMode( QWidget *parent = 0 );
	~FocusMode();
	void updateValue( Camera *camera );
	void updateList( Camera *camera );

    signals:
	void propertyChanged( int property, int newValue );

    protected slots:
	void toValue( int index );

    protected:
	QComboBox *comboBox;
	/*
	*  List of choices for auto focus mode.
	*/
	int n;
	int *v;
};

#endif // _FOCUSMODE_H_
