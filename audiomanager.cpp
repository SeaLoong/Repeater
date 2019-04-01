#include "AudioManager.h"

AudioManager::AudioManager(QObject *parent) : QObject(parent) {
	m_pAudioCapture = new AudioCapture(this);
	connect(m_pAudioCapture, &AudioCapture::error, this, &AudioManager::error);
	connect(m_pAudioCapture, &AudioCapture::volumeChanged, this, &AudioManager::volumeChanged);
	m_pAudioRender = new AudioRender(this);
	connect(m_pAudioRender, &AudioRender::error, this, &AudioManager::error);
	connect(m_pAudioRender, &AudioRender::finish, this, &AudioManager::renderFinish);
	connect(m_pAudioRender, &AudioRender::volumeChanged, this, &AudioManager::volumeChanged);
}

AudioManager::~AudioManager() {
	if (m_pAudioCapture->isCapturing()) {
		m_pAudioCapture->stop();
		m_pAudioCapture->deleteLater();
	}
	if (m_pAudioRender->isRendering()) {
		m_pAudioRender->stop();
		m_pAudioRender->deleteLater();
	}
}

void AudioManager::startCapture() {
	m_pAudioCapture->start();
}

void AudioManager::stopCapture() {
	m_pAudioCapture->stop();
	m_pAudioRender->clear();
	m_pAudioRender->write(m_pAudioCapture->read());
}

bool AudioManager::isCapturing() const {
	return m_pAudioCapture->isCapturing();
}


void AudioManager::startRender() {
	m_pAudioRender->start();
}

void AudioManager::stopRender() {
	m_pAudioRender->stop();
}

bool AudioManager::isRendering() const {
	return m_pAudioRender->isRendering();
}

bool AudioManager::canRender() const {
	return m_pAudioRender->canRender();
}
