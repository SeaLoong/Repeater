#ifndef AUDIORENDER_H
#define AUDIORENDER_H

#include "global.h"

extern IMMDevice* GetDefaultDevice(EDataFlow dataFlow);
extern bool AdjustFormatTo16Bits(WAVEFORMATEX *pwfx);
extern double computeVolume(const unsigned char *pData, qint64 len, const WAVEFORMATEX *pwfx);

class AudioRender;
class AudioRenderThread;

class AudioRender : public QObject {
	Q_OBJECT
public:
	explicit AudioRender(QObject *parent = nullptr);
	~AudioRender();

	void start();
	void stop();
	bool isRendering() const;
	bool canRender() const;

	void write(QByteArray bytearray);
	void clear();

signals:
	void error(const QString &);
	void finish();
	void volumeChanged(double);
	void __stopRenderThread();

private:
	AudioRenderThread *m_pAudioRenderThread;
	QThread *m_pThread;
	WAVEFORMATEX *m_pwfx;
	QByteArray m_buffer;
	qint64 m_pos;

};

class AudioRenderThread : public QObject {
	Q_OBJECT
public:
	explicit AudioRenderThread();
	~AudioRenderThread();
signals:
	void formatChanged(WAVEFORMATEX *); // 允许在这个信号连接的槽中改变信号的参数
	void dataRequest(unsigned char *, qint64, DWORD*); // 需要在槽中对这个信号中的指针操作，为了保证数据安全，这个信号只能被连接到一个槽
	void finish();
	void error(const QString &);
	void __canQuit();
public slots:
	void start();
	void stop();
private:
	QTimer *m_pTimer;
	QEventLoop *m_pEventLoop;
};

#endif // AUDIORENDER_H
