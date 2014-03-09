/*
 *  Run through a checklist for a successful multi-shot.
 */
#ifndef _CHECKLIST_H_
#define _CHECKLIST_H_

#include <QDialog>

class QLabel;
class QPushButton;
class Camera;
class ShotSeq;

class Checklist : public QDialog
{
    public:
	Checklist( QWidget *parent = 0, Qt::WFlags flags = 0 );
	~Checklist();
	void setChecklist( Camera *camera, ShotSeq *shotSeq );

    protected:
	void initialize();
	void setPixmap( int index, int value );
	static const int maxConditions = 10;
	QLabel *header;
	QLabel *item[maxConditions][2];
	QPushButton *closeButton;

	enum {
	    ICON_tip = 1,
	    ICON_checkmark = 2,
	    ICON_warning = 3,
	};
};

#endif // _CHECKLIST_H_
