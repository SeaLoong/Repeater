#include "widget.h"
#include "ui_widget.h"

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
		QMessageBox::warning(this, "Error:", msg);
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
}

Widget::~Widget() {
	delete ui;
}

void Widget::on_pushButton_capture_clicked() {
	ui->pushButton_capture->setVisible(false);
	capturing = !capturing;
	ui->checkBox_auto->setEnabled(!capturing);
	ui->pushButton_play->setEnabled(!capturing);
	if (capturing) m_pAudioManager->startCapture();
	else m_pAudioManager->stopCapture();
	if (capturing) {
		ui->pushButton_capture->setText("停止");
		ui->label_status->setText("Capturing...");
	} else {
		ui->pushButton_capture->setText("捕获");
		ui->label_status->setText("Capture End");
		if (ui->checkBox_autoPlay->isChecked()) {
			ui->label_status->setText("Wait to Play...");
			QTimer::singleShot(ui->spinBox_delay->value(), [&]() {
				if (!playing) {
					on_pushButton_play_clicked();
				}
			});
		}
	}
	ui->pushButton_capture->setVisible(true);
}

void Widget::on_pushButton_play_clicked() {
	ui->pushButton_play->setVisible(false);
	playing = !playing;
	ui->checkBox_auto->setEnabled(!playing);
	ui->pushButton_capture->setEnabled(!playing);
	if (playing) m_pAudioManager->startRender();
	else m_pAudioManager->stopRender();
	if (playing) {
		ui->pushButton_play->setText("停止");
		ui->label_status->setText("Playing");
	} else {
		ui->pushButton_play->setText("播放");
		ui->label_status->setText("Stop Play");
	}
	ui->pushButton_play->setVisible(true);
}

void Widget::on_checkBox_auto_clicked(bool checked) {
	ui->checkBox_auto->setVisible(false);
	ui->checkBox_autoPlay->setEnabled(!checked);
	if (checked) {
		ui->label->setEnabled(false);
		ui->label_2->setEnabled(false);
		ui->spinBox_delay->setEnabled(false);
		ui->checkBox_autoPlay->setChecked(true);
	} else {
		ui->label->setEnabled(ap_checked);
		ui->label_2->setEnabled(ap_checked);
		ui->spinBox_delay->setEnabled(ap_checked);
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
	ui->checkBox_autoPlay->setVisible(false);
	ui->label->setEnabled(checked);
	ui->label_2->setEnabled(checked);
	ui->spinBox_delay->setEnabled(checked);
	ap_checked = checked;
	ui->checkBox_autoPlay->setVisible(true);
}
