#ifndef GLOBAL_H
#define GLOBAL_H

#include <Windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>

// #include <QDebug>
#include <QIODevice>
#include <QThread>
#include <QTimer>
#include <QEventLoop>

#define REFTIMES_PER_SEC					10000000
#define REFTIMES_PER_MILLISEC				10000
#define CLSID_MMDeviceEnumerator			__uuidof(MMDeviceEnumerator)
#define IID_IMMDeviceEnumerator				__uuidof(IMMDeviceEnumerator)
#define IID_IAudioClient					__uuidof(IAudioClient)
#define IID_IAudioCaptureClient				__uuidof(IAudioCaptureClient)
#define IID_IAudioRenderClient				__uuidof(IAudioRenderClient)

IMMDevice* GetDefaultDevice(EDataFlow dataFlow);
bool AdjustFormatTo16Bits(WAVEFORMATEX *pwfx);
double computeVolume(const unsigned char *pData, qint64 len, const WAVEFORMATEX *pwfx);

#endif // GLOBAL_H
