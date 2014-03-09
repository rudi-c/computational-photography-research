/*
 *  Metering mode setting and display widget.
 */
#ifndef _METERINGMODE_H_
#define _METERINGMODE_H_

#include <QWidget>

class QStandardItemModel;
class QTableView;
class QComboBox;
class Camera;


class MeteringMode : public QWidget
{
    Q_OBJECT

    public:
	MeteringMode( QWidget *parent = 0 );
	~MeteringMode();
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
	int n;	// list of choices for metering mode
	int *v;
};

#endif // _METERINGMODE_H_
