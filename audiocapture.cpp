#include "audiocapture.h"

AudioCapture::AudioCapture(QObject *parent) : QObject(parent) {
	m_pAudioCaptureThread = nullptr;
	m_pThread = nullptr;
	m_pwfx = nullptr;
	m_pSoundAnalysis = nullptr;
}

AudioCapture::~AudioCapture() {
	stop();
}

void AudioCapture::start() {
	if (m_pThread != nullptr) return;
	m_buffer.clear();
	m_pAudioCaptureThread = new AudioCaptureThread();
	m_pThread = new QThread();
	m_pAudioCaptureThread->moveToThread(m_pThread);
	connect(m_pThread, &QThread::started, m_pAudioCaptureThread, &AudioCaptureThread::start);
	connect(this, &AudioCapture::__stopCaptureThread, m_pAudioCaptureThread, &AudioCaptureThread::stop);
	connect(m_pAudioCaptureThread, &AudioCaptureThread::formatChanged, [this](WAVEFORMATEX *pwfx) {
		m_pwfx = pwfx;
	});
	connect(m_pAudioCaptureThread, &AudioCaptureThread::dataReceived, [this](const unsigned char *pData, quint64 len) {
		m_buffer.append(reinterpret_cast<const char*>(pData), len);
		emit volumeChanged(computeVolume(pData, len, m_pwfx));
	});
	connect(m_pAudioCaptureThread, &AudioCaptureThread::error, this, &AudioCapture::error);
	connect(m_pAudioCaptureThread, &AudioCaptureThread::__canQuit, [&]() {
		m_pThread->quit();
	});
	if (m_pSoundAnalysis != nullptr) {
		connect(m_pAudioCaptureThread, &AudioCaptureThread::formatChanged, m_pSoundAnalysis, &SoundAnalysis::onFormatChanged);
		connect(m_pAudioCaptureThread, &AudioCaptureThread::dataReceived, m_pSoundAnalysis, &SoundAnalysis::onDataReceived);
	}
	m_pThread->start();
}

void AudioCapture::stop() {
	if (isCapturing()) {
		emit __stopCaptureThread();
		m_pThread->wait();
		delete m_pThread;
		m_pThread = nullptr;
		delete m_pAudioCaptureThread;
		m_pAudioCaptureThread = nullptr;
	}
}

bool AudioCapture::isCapturing() const {
	return m_pThread != nullptr && m_pThread->isRunning();
}

QByteArray AudioCapture::read() {
	return m_buffer;
}

void AudioCapture::bindSoundAnalysis(SoundAnalysis *pSoundAnalysis) {
	m_pSoundAnalysis = pSoundAnalysis;
}




AudioCaptureThread::AudioCaptureThread() : QObject(nullptr) {
	m_pEventLoop = nullptr;
	m_pTimer = nullptr;
}

AudioCaptureThread::~AudioCaptureThread() {
}

void AudioCaptureThread::start() {
	CoInitialize(nullptr);

	IMMDevice *pDevice = GetDefaultDevice(eRender);
	HRESULT hr;
	IAudioClient *pAudioClient = nullptr;
	WAVEFORMATEX *pwfx = nullptr;
	REFERENCE_TIME hnsDefaultDevicePeriod;
	IAudioCaptureClient *pAudioCaptureClient = nullptr;
	bool bStarted = false;

	do {
		hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&pAudioClient));
		if (FAILED(hr)) {
			emit error("Activate");
			break;
		}

		hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, nullptr); // hnsDefaultDevicePeriod以100ns为单位
		if (FAILED(hr)) {
			emit error("GetDevicePeriod");
			break;
		}

		hr = pAudioClient->GetMixFormat(&pwfx);
		if (FAILED(hr)) {
			emit error("GetMixFormat");
			break;
		}

		if (!AdjustFormatTo16Bits(pwfx)) {
			emit error("AdjustFormatTo16Bits");
			break;
		}

		emit formatChanged(pwfx);

		hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, pwfx, 0);
		if (FAILED(hr)) {
			emit error("Initialize");
			break;
		}

		hr = pAudioClient->GetService(IID_IAudioCaptureClient, reinterpret_cast<void**>(&pAudioCaptureClient));
		if (FAILED(hr)) {
			emit error("GetService");
			break;
		}

		UINT32 nNextPacketSize = 0;
		BYTE *pData = nullptr;
		UINT32 nNumFramesRead;
		DWORD dwFlags;

		m_pEventLoop = new QEventLoop();
		m_pTimer = new QTimer();
		m_pTimer->callOnTimeout([&]() {

			hr = pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize);
			if (FAILED(hr)) {
				emit error("GetNextPacketSize");
				m_pEventLoop->quit();
				return;
			}

			if (nNextPacketSize == 0) return;

			hr = pAudioCaptureClient->GetBuffer(&pData, &nNumFramesRead, &dwFlags, nullptr, nullptr);
			if (FAILED(hr)) {
				emit error("GetBuffer");
				m_pEventLoop->quit();
				return;
			}

			if (0 != nNumFramesRead) { // 读到数据
				emit dataReceived(pData, nNumFramesRead * pwfx->nBlockAlign); // nNumFramesToRead是读到的样本数，pwfx->nBlockAlign是每个样本的字节大小
			}

			pAudioCaptureClient->ReleaseBuffer(nNumFramesRead);

		});

		m_pTimer->start(hnsDefaultDevicePeriod / 2 / REFTIMES_PER_MILLISEC); // 转成以ms单位，并设置timer的周期是设备的一半

		hr = pAudioClient->Start();
		if (FAILED(hr)) {
			emit error("Start");
			break;
		}

		bStarted = true;

		m_pEventLoop->exec(); // Qt的事件循环

	} while (false);

	if (pAudioClient != nullptr) {
		if (bStarted) {
			pAudioClient->Stop();
		}
		pAudioClient->Release();
		pAudioClient = nullptr;
	}

	if (m_pTimer != nullptr) {
		m_pTimer->stop();
		delete m_pTimer;
		m_pTimer = nullptr;
	}

	if (m_pEventLoop != nullptr) {
		m_pEventLoop->quit();
		delete m_pEventLoop;
		m_pEventLoop = nullptr;
	}

	if (pAudioCaptureClient != nullptr) {
		pAudioCaptureClient->Release();
		pAudioCaptureClient = nullptr;
	}

	if (pwfx != nullptr) {
		CoTaskMemFree(pwfx);
		pwfx = nullptr;
	}

	CoUninitialize();
	emit __canQuit();
}

void AudioCaptureThread::stop() {
	if (m_pTimer != nullptr) {
		m_pTimer->stop();
	}
	if (m_pEventLoop != nullptr) {
		m_pEventLoop->quit();
	}
}
