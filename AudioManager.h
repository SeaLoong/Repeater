#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "AudioCapture.h"
#include "AudioRender.h"

class AudioManager : public QObject {
	Q_OBJECT
public:
	explicit AudioManager(QObject *parent = nullptr);
	~AudioManager();

	void startCapture();
	void stopCapture();
	bool isCapturing() const;

	void startRender();
	void stopRender();
	bool isRendering() const;
	bool canRender() const;
signals:
	void error(const QString &);
	void renderFinish();
	void volumeChanged(double);

private:
	AudioCapture *m_pAudioCapture;
	AudioRender *m_pAudioRender;

};

#endif // AUDIOMANAGER_H
