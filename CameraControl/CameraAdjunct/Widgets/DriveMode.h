/*
 *  Drive mode setting and display widget.
 */
#ifndef _DRIVEMODE_H_
#define _DRIVEMODE_H_

#include <QWidget>

class QStandardItemModel;
class QTableView;
class QComboBox;
class Camera;


class DriveMode : public QWidget
{
    Q_OBJECT

    public:
	DriveMode( QWidget *parent = 0 );
	~DriveMode();
	void updateValue( Camera *camera );
	void updateList( Camera *camera );

    signals:
	void propertyChanged( int property, int newValue );

    protected slots:
	void toValue( int index );

    protected:
	void initialize();
	void setPage( int index, int value );
	QStandardItemModel *model;
	QTableView *view;
	QComboBox *comboBox;
	int n;	// list of choices for drive mode
	int *v;
};

#endif // _DRIVEMODE_H_
