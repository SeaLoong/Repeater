#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include "global.h"
#include "soundanalysis.h"

extern IMMDevice* GetDefaultDevice(EDataFlow dataFlow);
extern bool AdjustFormatTo16Bits(WAVEFORMATEX *pwfx);
extern double computeVolume(const char *pData, qint64 len, const WAVEFORMATEX *pwfx);

class AudioCapture;
class AudioCaptureThread;

class AudioCapture : public QObject {
	Q_OBJECT
public:
	explicit AudioCapture(QObject *parent = nullptr);
	~AudioCapture();

	void start();
	void stop();
	bool isCapturing() const;

	QByteArray read();

	void bindSoundAnalysis(SoundAnalysis*);

signals:
	void error(const QString &);
	void volumeChanged(double);
	void __stopCaptureThread();

private:
	AudioCaptureThread *m_pAudioCaptureThread;
	QThread *m_pThread;
	WAVEFORMATEX *m_pwfx;
	QByteArray m_buffer;
	SoundAnalysis *m_pSoundAnalysis;
};



class AudioCaptureThread : public QObject {
	Q_OBJECT
public:
	explicit AudioCaptureThread();
	~AudioCaptureThread();
signals:
	void formatChanged(WAVEFORMATEX *); // 允许在这个信号连接的槽中改变信号的参数
	void dataReceived(const unsigned char *, qint64);
	void error(const QString &);
	void __canQuit();
public slots:
	void start();
	void stop();
private:
	QTimer *m_pTimer;
	QEventLoop *m_pEventLoop;
};

#endif // AUDIOCAPTURE_H
