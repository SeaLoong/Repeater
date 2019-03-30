#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QThread>
#include "renderarea.h"

#include "audiomanager.h"
#include "autorepeat.h"

namespace Ui {
	class Widget;
}

class Widget : public QWidget {
	Q_OBJECT
public:
	explicit Widget(QWidget *parent = nullptr);
	~Widget();

private slots:
	void on_pushButton_capture_clicked();

	void on_pushButton_play_clicked();

	void on_checkBox_auto_clicked(bool checked);

	void on_checkBox_autoPlay_clicked(bool checked);

private:
	Ui::Widget *ui;
	RenderArea *area;

	AudioManager *m_pAudioManager;

	AutoRepeat *m_pAutoRepeat;

	bool capturing, playing;
	bool ap_checked;
};

#endif // WIDGET_H
