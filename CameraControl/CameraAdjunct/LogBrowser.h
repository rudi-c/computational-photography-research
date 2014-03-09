/*
 *  Browse the log of shots and settings, clear the log, edit the log,
 *  save the log. The log is plain text.
 */
#ifndef _LOGBROWSER_H_
#define _LOGBROWSER_H_

#include <QDialog>

class QPlainTextEdit;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;

class LogBrowser : public QDialog
{
    Q_OBJECT

    public:
	LogBrowser( QWidget *parent = 0, Qt::WFlags flags = 0 );
	~LogBrowser();
	bool isModified();
	void appendLog( const QString text );

    public slots:
	bool saveLog();
	void clearLog();

    protected:
	void initialize();
	void timeStamp();
	QPlainTextEdit *shootingLog;
	QPushButton *saveButton;
	QPushButton *clearButton;
	QPushButton *closeButton;
	QString saveFileName;

	/*
	 *  Position and size settings.
	 */
	void readSettings();
	void writeSettings();

	/*
	 *  Reimplementation of closeEvent to save settings.
	 */
	void closeEvent( QCloseEvent *event );
};

#endif // _LOGBROWSER_H_
