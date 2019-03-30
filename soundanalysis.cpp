#include "soundanalysis.h"

SoundAnalysis::SoundAnalysis(QObject *parent) : QObject(parent){
	m_pTimer = new QTimer(this);
	m_pTimer->callOnTimeout([&]() {
		analyse();
	});
	lastAve = 0;
	state = 0;
	competition = 0;
}

SoundAnalysis::~SoundAnalysis() {
	if (m_pTimer != nullptr && m_pTimer->isActive()) m_pTimer->stop();
}

void SoundAnalysis::onDataReceived(const unsigned char * pData, qint64 len) {
	QMutexLocker locker(&mutex);
	m_buffer.append(reinterpret_cast<const char *>(pData), len);
}

void SoundAnalysis::onFormatChanged(WAVEFORMATEX *pwfx) {
	QMutexLocker locker(&mutex);
	m_pwfx = pwfx;
	channelBytes = pwfx->nBlockAlign / pwfx->nChannels;
	m_buffer.clear();
	m_pTimer->start(1000);
}

void SoundAnalysis::stop() {
	m_pTimer->stop();
}

void SoundAnalysis::analyse() {
	QMutexLocker locker(&mutex);
	if (m_buffer.size() < int(m_pwfx->nAvgBytesPerSec * 3)) return;
	double ave = getAve(m_buffer.right(m_pwfx->nAvgBytesPerSec * 3));
	if (ave <= 2) {
		switch (state) {
			case 0:
				m_buffer.clear();
				return;
			case 1:
				__end();
				return;
		}
	}
	if (ave == lastAve) return;
	int sign = ave > lastAve ? 1 : -1;
	int diff = qAbs(ave - lastAve);
    competition += sign * log10(1 + diff * diff);
	if (competition > 5) __start();
	else if (competition < -5) __end();
	lastAve = ave;
}

void SoundAnalysis::__start() {
	if (state != 0) return;
	emit soundStart();
	state = 1;
	competition = 0;
	lastAve = 0;
}

void SoundAnalysis::__end() {
	if (state != 1) return;
	emit soundEnd(m_buffer);
	state = 0;
	competition = 0;
	lastAve = 0;
	m_pTimer->stop();
	m_buffer.clear();
}

double SoundAnalysis::getAve(const QByteArray &bytearray) {
	quint32 nNumFrames = bytearray.size() / m_pwfx->nBlockAlign;
	const unsigned char *ptr = reinterpret_cast<const unsigned char *>(bytearray.data());
	double ave = 0;
	for (quint32 i = 0; i < nNumFrames; ++i) {
		double nChannelsAve = 0;
		for (quint16 j = 0; j < m_pwfx->nChannels; ++j, ptr += channelBytes) {
			qint16 value = qAbs(*reinterpret_cast<const qint16*>(ptr));
			nChannelsAve += value;
		}
		nChannelsAve /= m_pwfx->nChannels; // 一个块的所有通道的平均声音
		ave += nChannelsAve;
	}
	ave /= nNumFrames;  // 这段时间内的平均声音
	return ave;
}
