/*
 *  White balance setting and display widget.
 */
#ifndef _WHITEBALANCE_H_
#define _WHITEBALANCE_H_

#include <QWidget>

class QStandardItemModel;
class QTableView;
class QComboBox;
class Camera;


class WhiteBalance : public QWidget
{
    Q_OBJECT

    public:
	WhiteBalance( QWidget *parent = 0 );
	~WhiteBalance();
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
	int n;	// list of choices for white balance
	int *v;
};

#endif // _WHITEBALANCE_H_
