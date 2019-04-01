#include "Widget.h"
#include "ui_widget.h"

#include "MyGlobalShortCut/MyGlobalShortCut.h"

Widget::Widget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Widget) {
	ui->setupUi(this);
	area = new RenderArea(this);
	ui->subWidget->layout()->addWidget(area);
	setWindowFlags(Qt::WindowStaysOnTopHint);

	capturing = playing = false;
	ap_checked = false;

	m_pAudioManager = new AudioManager(this);
	connect(m_pAudioManager, &AudioManager::error, [this](const QString &msg) {
		QMessageBox::warning(this, "Error", msg);
	});
	connect(m_pAudioManager, &AudioManager::volumeChanged, [this](double level) {
		area->setLevel(level);
	});
	connect(m_pAudioManager, &AudioManager::renderFinish, [this]() {
		on_pushButton_play_clicked();
	});
	m_pAutoRepeat = new AutoRepeat(this);
	connect(m_pAutoRepeat, &AutoRepeat::volumeChanged, [this](double level) {
		area->setLevel(level);
	});
	connect(m_pAutoRepeat, &AutoRepeat::soundStart, [&]() {
		ui->label_status->setText("Capture Start");
	});
	connect(m_pAutoRepeat, &AutoRepeat::soundEnd, [&]() {
		ui->label_status->setText("Capture End");
	});
	connect(m_pAutoRepeat, &AutoRepeat::repeatStart, [&]() {
		ui->label_status->setText("Repeat Start");
	});
	connect(m_pAutoRepeat, &AutoRepeat::repeatEnd, [&]() {
		ui->label_status->setText("Repeat End");
	});
	connect(ui->pushButton_about, &QPushButton::clicked, [&]() {
		QMessageBox::about(this, "About Author", "Github:  SeaLoong\nLicence:  LGPLv3");
	});
	connect(ui->pushButton_aboutQt, &QPushButton::clicked, [&]() {
		QMessageBox::aboutQt(this, "About Qt - Repeater");
	});

	MyGlobalShortCut *shortcutF9 = new MyGlobalShortCut("F9", this);
	connect(shortcutF9, &MyGlobalShortCut::activated, [&]() {
		on_pushButton_capture_clicked();
	});

	MyGlobalShortCut *shortcutF10 = new MyGlobalShortCut("F10", this);
	connect(shortcutF10, &MyGlobalShortCut::activated, [&]() {
		on_pushButton_play_clicked();
	});
}

Widget::~Widget() {
	delete ui;
}

void Widget::on_pushButton_capture_clicked() {
	if (!ui->pushButton_capture->isEnabled()) return;
	ui->pushButton_capture->setVisible(false);
	capturing = !capturing;
	ui->pushButton_play->setEnabled(!capturing);
	ui->checkBox_auto->setEnabled(!capturing);
	ui->label->setEnabled(!capturing);
	ui->label_2->setEnabled(!capturing);
	ui->spinBox_delay->setEnabled(!capturing);
	if (capturing) m_pAudioManager->startCapture();
	else m_pAudioManager->stopCapture();
	if (capturing) {
		ui->pushButton_capture->setText("停止(F9)");
		ui->label_status->setText("Capturing...");
	} else {
		ui->pushButton_capture->setText("捕获(F9)");
		ui->label_status->setText("Capture End");
		if (ui->checkBox_autoPlay->isChecked()) {
			ui->label_status->setText("Wait to Play...");
			if (!playing) {
				on_pushButton_play_clicked();
			}
		}
	}
	ui->pushButton_capture->setVisible(true);
}

void Widget::on_pushButton_play_clicked() {
	if (!ui->pushButton_play->isEnabled()) return;
	ui->pushButton_play->setVisible(false);
	playing = !playing;
	ui->pushButton_capture->setEnabled(!playing);
	ui->checkBox_auto->setEnabled(!playing);
	ui->label->setEnabled(!playing);
	ui->label_2->setEnabled(!playing);
	ui->spinBox_delay->setEnabled(!playing);
	if (playing) m_pAudioManager->startRender();
	else m_pAudioManager->stopRender();
	if (playing) {
		ui->pushButton_play->setText("停止(F10)");
		ui->label_status->setText("Playing");
	} else {
		ui->pushButton_play->setText("播放(F10)");
		ui->label_status->setText("Stop Play");
	}
	ui->pushButton_play->setVisible(true);
}

void Widget::on_checkBox_auto_clicked(bool checked) {
	ui->checkBox_auto->setVisible(false);
	ui->checkBox_autoPlay->setEnabled(!checked);
	ui->label->setEnabled(!checked);
	ui->label_2->setEnabled(!checked);
	ui->spinBox_delay->setEnabled(!checked);
	if (checked) {
		ui->checkBox_autoPlay->setChecked(true);
	} else {
		ui->checkBox_autoPlay->setChecked(ap_checked);
	}
	ui->pushButton_capture->setEnabled(!checked);
	ui->pushButton_play->setEnabled(false);
	m_pAutoRepeat->setDelay(ui->spinBox_delay->value());
	if (checked) {
		ui->label_status->setText("Auto Repeat Start!");
		m_pAutoRepeat->start();
	} else {
		ui->label_status->setText("Auto Repeat Stop.");
		m_pAutoRepeat->stop();
	}
	ui->checkBox_auto->setVisible(true);
}

void Widget::on_checkBox_autoPlay_clicked(bool checked) {
	ap_checked = checked;
}
