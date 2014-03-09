/*
 *  Aperture setting and displaying widget.
 */
#ifndef _APERTURE_H_
#define _APERTURE_H_

#include <QWidget>

class QStandardItem;
class QTableView;
class QComboBox;
class Camera;


class Aperture : public QWidget
{
    Q_OBJECT

    public:
	Aperture( QWidget *parent = 0 );
	~Aperture();
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
	int n;	// list of choices for aperture
	int *v;

bool eventFilter( QObject *target, QEvent *event );
};

#endif // _APERTURE_H_
