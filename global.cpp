
#include "Global.h"

IMMDevice* GetDefaultDevice(EDataFlow dataFlow) {
	IMMDevice* pDevice = nullptr;
	IMMDeviceEnumerator *pMMDeviceEnumerator = nullptr;
	HRESULT hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL,
		IID_IMMDeviceEnumerator,
		reinterpret_cast<void**>(&pMMDeviceEnumerator));
	if (FAILED(hr)) return nullptr;

	hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(dataFlow, eConsole, &pDevice);
	pMMDeviceEnumerator->Release();
	return pDevice;
}

bool AdjustFormatTo16Bits(WAVEFORMATEX *pwfx) {
	bool bRet = false;

	if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
		pwfx->wFormatTag = WAVE_FORMAT_PCM;
		pwfx->wBitsPerSample = 16;
		pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
		pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;

		bRet = true;
	} else if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
		if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) {
			pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			pEx->Samples.wValidBitsPerSample = 16;
			pwfx->wBitsPerSample = 16;
			pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
			pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;

			bRet = true;
		}
	}

	return bRet;
}

double computeVolume(const unsigned char *pData, qint64 len, const WAVEFORMATEX *pwfx) {
	static const qint16 maxAmplitude = 32767;
	qint16 maxValue = 0;
	quint32 channelBytes = pwfx->nBlockAlign / pwfx->nChannels;
	quint32 nNumFrames = len / pwfx->nBlockAlign;
	const unsigned char *ptr = reinterpret_cast<const unsigned char *>(pData);
	for (quint32 i = 0; i < nNumFrames; ++i) {
		for (quint16 j = 0; j < pwfx->nChannels; ++j, ptr += channelBytes) {
			qint16 value = qAbs(*reinterpret_cast<const qint16*>(ptr));
			maxValue = qMax(value, maxValue);
		}
	}
	maxValue = qMin(maxValue, maxAmplitude);
	return double(maxValue) / maxAmplitude;
}
