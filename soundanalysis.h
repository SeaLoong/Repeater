#ifndef SOUNDANALYSIS_H
#define SOUNDANALYSIS_H

#include "global.h"

class SoundAnalysis : public QObject {
	Q_OBJECT
public:
	explicit SoundAnalysis(QObject *parent = nullptr);
	~SoundAnalysis();

	static const quint16 maxAmplitude = 32767;

signals:
	void soundStart();
	void soundEnd(QByteArray);

public slots:
	void onDataReceived(const unsigned char *, qint64);
	void onFormatChanged(WAVEFORMATEX *);
	void stop();

private:
	void analyse();
	double getAve(const QByteArray &);

	void __start();
	void __end();

	int sec;

	quint32 channelBytes;

	WAVEFORMATEX *m_pwfx;

	QByteArray m_buffer;
	QTimer *m_pTimer;

	int state;
};

#endif // SOUNDANALYSIS_H