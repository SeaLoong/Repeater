#include "autorepeat.h"

AutoRepeat::AutoRepeat(QObject *parent) : QObject(parent) {
	working = false;
	mDelay = 3000;
	m_pAudioCapture = new AudioCapture(this);
	connect(m_pAudioCapture, &AudioCapture::error, this, &AutoRepeat::error);
	connect(m_pAudioCapture, &AudioCapture::volumeChanged, this, &AutoRepeat::volumeChanged);
	m_pAudioRender = new AudioRender(this);
	connect(m_pAudioRender, &AudioRender::error, this, &AutoRepeat::error);
	connect(m_pAudioRender, &AudioRender::volumeChanged, this, &AutoRepeat::volumeChanged);
	connect(m_pAudioRender, &AudioRender::finish, [&]() {
		emit repeatEnd();
		if (working) {
			m_pAudioCapture->start();
		}
	});
	m_pSoundAnalysis = new SoundAnalysis(this);
	connect(m_pSoundAnalysis, &SoundAnalysis::soundStart, this, &AutoRepeat::soundStart);
	connect(m_pSoundAnalysis, &SoundAnalysis::soundEnd, [this](QByteArray bytearray) {
		m_pAudioCapture->stop();
		emit soundEnd();
		m_pAudioRender->write(bytearray);
		QTimer::singleShot(mDelay, [&]() {
			emit repeatStart();
			if (working) m_pAudioRender->start();
		});
	});
	m_pAudioCapture->bindSoundAnalysis(m_pSoundAnalysis);
}

AutoRepeat::~AutoRepeat() {
}

void AutoRepeat::start() {
	if (working) return;
	working = true;
	m_pAudioCapture->start();
}

void AutoRepeat::stop() {
	if (!working) return;
	working = false;
	if (m_pAudioCapture->isCapturing()) {
		m_pAudioCapture->stop();
	}
	if (m_pAudioRender->isRendering()) {
		m_pAudioRender->stop();
	}
}

bool AutoRepeat::isRunning() const {
	return working || m_pAudioCapture->isCapturing() || m_pAudioRender->isRendering();
}

void AutoRepeat::setDelay(int ms) {
	mDelay = ms;
}