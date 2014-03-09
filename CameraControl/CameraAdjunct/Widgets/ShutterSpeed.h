/*
 *  Shutter speed setting and display widget.
 */
#ifndef _SHUTTERSPEED_H_
#define _SHUTTERSPEED_H_

#include <QWidget>

class QStandardItem;
class QTableView;
class QComboBox;
class Camera;


class ShutterSpeed : public QWidget
{
    Q_OBJECT

    public:
	ShutterSpeed( QWidget *parent = 0 );
	~ShutterSpeed();
	void updateValue( Camera *camera );
	void updateList( Camera *camera );

    signals:
	void propertyChanged( int property, int value );

    protected slots:
	void toValue();

    protected:
	void initialize();
	QStandardItem **item;
	QTableView *view;
	QComboBox *comboBox;
	int maxCols;
	int maxRows;
	int n;	// list of choices for shutter speed
	int *v;

	bool eventFilter( QObject *target, QEvent *event );
};

#endif // _SHUTTERSPEED_H_
