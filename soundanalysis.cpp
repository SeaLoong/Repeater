#include "SoundAnalysis.h"

SoundAnalysis::SoundAnalysis(QObject *parent) : QObject(parent){
	m_pTimer = new QTimer(this);
	m_pTimer->callOnTimeout([&]() {
		analyse();
	});
	state = 0;
	sec = 3;
}

SoundAnalysis::~SoundAnalysis() {
	if (m_pTimer != nullptr && m_pTimer->isActive()) m_pTimer->stop();
}

void SoundAnalysis::onDataReceived(const unsigned char * pData, qint64 len) {
	m_buffer.append(reinterpret_cast<const char *>(pData), len);
}

void SoundAnalysis::onFormatChanged(WAVEFORMATEX *pwfx) {
	m_pwfx = pwfx;
	channelBytes = pwfx->nBlockAlign / pwfx->nChannels;
	m_buffer.clear();
	m_pTimer->start(500);
}

void SoundAnalysis::stop() {
	if (m_pTimer != nullptr && m_pTimer->isActive()) m_pTimer->stop();
}

void SoundAnalysis::analyse() {
	if (m_buffer.size() < int(m_pwfx->nAvgBytesPerSec * sec)) return;
	double ave = getAve(m_buffer.right(m_pwfx->nAvgBytesPerSec * sec));
	if (ave > 80) {
		__start();
	} else {
		__end();
	}
}

void SoundAnalysis::__start() {
	if (state == 0) {
		m_buffer = m_buffer.right(m_pwfx->nAvgBytesPerSec * sec);
		emit soundStart();
		state = 1;
	}
}

void SoundAnalysis::__end() {
	if (state == 1) {
		m_pTimer->stop();
		emit soundEnd(m_buffer);
		m_buffer.clear();
		state = 0;
	}else if (state == 0) {
		m_buffer = m_buffer.right(m_pwfx->nAvgBytesPerSec * sec);
	}
}

double SoundAnalysis::getAve(const QByteArray &bytearray) {
	quint32 nNumFrames = bytearray.size() / m_pwfx->nBlockAlign;
	const unsigned char *ptr = reinterpret_cast<const unsigned char *>(bytearray.data());
	double ave = 0;
	for (quint32 i = 0; i < nNumFrames; ++i) {
		double nChannelsAve = 0;
		for (quint16 j = 0; j < m_pwfx->nChannels; ++j, ptr += channelBytes) {
			qint16 value = qAbs(*reinterpret_cast<const qint16*>(ptr));
			nChannelsAve += qMin(value, maxAmplitude);
		}
		nChannelsAve /= m_pwfx->nChannels; // 一个块的所有通道的平均声音
		ave += nChannelsAve;
	}
	ave /= nNumFrames;  // 这段时间内的平均声音
	return ave;
}
