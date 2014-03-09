/*
 *  ISO speed setting and display widget.
 */
#ifndef _ISOSPEED_H_
#define _ISOSPEED_H_

#include <QWidget>

class QStandardItem;
class QTableView;
class QComboBox;
class Camera;


class ISOSpeed : public QWidget
{
    Q_OBJECT

    public:
	ISOSpeed( QWidget *parent = 0 );
	~ISOSpeed();
	void updateValue( Camera *camera );
	void updateList( Camera *camera );

    signals:
	void propertyChanged( int property, int newValue );

    protected slots:
	void toValue();

    protected:
	void initialize();
	QIcon *ISO_icon;
	QStandardItem **item;
	QTableView *view;
	QComboBox *comboBox;
	int maxCols;
	int maxRows;
	int n;	// list of choices for ISO speed
	int *v;

	bool eventFilter( QObject *target, QEvent *event );
};

#endif // _ISOSPEED_H_
