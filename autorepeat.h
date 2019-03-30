#ifndef AUTOREPEAT_H
#define AUTOREPEAT_H

#include <QObject>
#include "audiocapture.h"
#include "audiorender.h"
#include "soundanalysis.h"

class AutoRepeat : public QObject {
	Q_OBJECT
public:
	explicit AutoRepeat(QObject *parent);
	~AutoRepeat();

	void start();
	void stop();
	bool isRunning() const;

	void setDelay(int ms);

signals:
	void error(const QString &);
	void volumeChanged(double);
	void soundStart();
	void soundEnd();
	void repeatStart();
	void repeatEnd();

private:
	bool working;
	int mDelay;

	AudioCapture *m_pAudioCapture;
	AudioRender *m_pAudioRender;
	SoundAnalysis *m_pSoundAnalysis;

	QByteArray *m_pBuffer;

};

#endif // AUTOREPEAT_H