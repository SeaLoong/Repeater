#include "AudioRender.h"

AudioRender::AudioRender(QObject *parent) : QObject(parent) {
	m_pos = 0;
	m_pAudioRenderThread = nullptr;
	m_pThread = nullptr;
	m_pwfx = nullptr;
}

AudioRender::~AudioRender() {
	stop();
}

void AudioRender::start() {
	if (!canRender()) return;
	m_pos = 0;
	m_pAudioRenderThread = new AudioRenderThread();
	m_pThread = new QThread();
	m_pAudioRenderThread->moveToThread(m_pThread);
	connect(m_pThread, &QThread::started, m_pAudioRenderThread, &AudioRenderThread::start);
	connect(this, &AudioRender::__stopRenderThread, m_pAudioRenderThread, &AudioRenderThread::stop);
	connect(m_pAudioRenderThread, &AudioRenderThread::formatChanged, [this](WAVEFORMATEX *pwfx) {
		m_pwfx = pwfx;
	});
	connect(m_pAudioRenderThread, &AudioRenderThread::dataRequest, [this](unsigned char *pData, qint64 len, DWORD *flags) {
		if (m_buffer.isEmpty()) {
			*flags = AUDCLNT_BUFFERFLAGS_SILENT;
		} else {
			qint64 total = 0;
			while (len - total > 0 && m_pos < m_buffer.size()) {
				const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
				memcpy(pData + total, m_buffer.constData() + m_pos, chunk);
				m_pos += chunk;
				total += chunk;
			}
			if (m_pos == m_buffer.size()) *flags = AUDCLNT_BUFFERFLAGS_SILENT;
			else *flags = 0;
			if (*flags == 0) {
				emit volumeChanged(computeVolume(pData, total, m_pwfx));
			}
		}
	});
	connect(m_pAudioRenderThread, &AudioRenderThread::finish, this, &AudioRender::finish);
	connect(m_pAudioRenderThread, &AudioRenderThread::error, [&](const QString &msg) {
		emit error(msg);
		emit finish();
	});
	connect(m_pAudioRenderThread, &AudioRenderThread::__canQuit, [&]() {
		m_pThread->quit();
	});
	m_pThread->start();
}

void AudioRender::stop() {
	if (isRendering()) {
		emit __stopRenderThread();
		m_pThread->wait();
		delete m_pThread;
		m_pThread = nullptr;
		delete m_pAudioRenderThread;
		m_pAudioRenderThread = nullptr;
	}
}

bool AudioRender::isRendering() const {
	return m_pThread != nullptr && m_pThread->isRunning();
}

bool AudioRender::canRender() const {
	return !m_buffer.isEmpty() && !isRendering();
}

void AudioRender::write(QByteArray bytearray) {
	m_buffer = bytearray;
}

void AudioRender::clear() {
	m_buffer.clear();
	m_pos = 0;
}


AudioRenderThread::AudioRenderThread() : QObject(nullptr) {
	m_pEventLoop = nullptr;
	m_pTimer = nullptr;
}

AudioRenderThread::~AudioRenderThread() {
}

void AudioRenderThread::start() {
	CoInitialize(nullptr);

	IMMDevice *pDevice = GetDefaultDevice(eRender);
	HRESULT hr;
	IAudioClient *pAudioClient = nullptr;
	WAVEFORMATEX *pwfx = nullptr;
	IAudioRenderClient *pAudioRenderClient = nullptr;
	UINT32 nFrameBufferSize;
	bool bStarted = false;

	do {
		hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&pAudioClient));
		if (FAILED(hr)) {
			emit error("Activate");
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

		hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, pwfx, 0);
		if (FAILED(hr)) {
			emit error("Initialize");
			break;
		}

		hr = pAudioClient->GetBufferSize(&nFrameBufferSize);

		if (FAILED(hr)) {
			emit error("GetBufferSize");
			break;
		}

		hr = pAudioClient->GetService(IID_IAudioRenderClient, reinterpret_cast<void**>(&pAudioRenderClient));
		if (FAILED(hr)) {
			emit error("GetService");
			break;
		}

		UINT32 nFramesPadding;
		UINT32 nFramesAvailable;
		BYTE *pData = nullptr;
		DWORD flags = 0;

		m_pEventLoop = new QEventLoop();
		m_pTimer = new QTimer();
		m_pTimer->callOnTimeout([&]() {

			hr = pAudioClient->GetCurrentPadding(&nFramesPadding);
			if (FAILED(hr)) {
				emit error("GetCurrentPadding");
				m_pEventLoop->quit();
				return;
			}

			nFramesAvailable = nFrameBufferSize - nFramesPadding;
			if (nFramesAvailable == 0) return;

			hr = pAudioRenderClient->GetBuffer(nFramesAvailable, &pData);
			if (FAILED(hr)) {
				emit error("GetBuffer");
				m_pEventLoop->quit();
				return;
			}

			emit dataRequest(pData, nFramesAvailable * pwfx->nBlockAlign, &flags);

			pAudioRenderClient->ReleaseBuffer(nFramesAvailable, flags);

			if (flags == AUDCLNT_BUFFERFLAGS_SILENT) {
				emit finish();
				m_pEventLoop->quit();
				return;
			}
		});

		m_pTimer->start(int(double(nFrameBufferSize) / pwfx->nAvgBytesPerSec * 1000));

		hr = pAudioClient->Start();
		if (FAILED(hr)) {
			emit error("Start");
			break;
		}

		bStarted = true;

		m_pEventLoop->exec();

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

	if (pAudioRenderClient != nullptr) {
		pAudioRenderClient->Release();
		pAudioRenderClient = nullptr;
	}

	if (pwfx != nullptr) {
		CoTaskMemFree(pwfx);
		pwfx = nullptr;
	}

	CoUninitialize();
	emit __canQuit();
}

void AudioRenderThread::stop() {
	if (m_pTimer != nullptr) {
		m_pTimer->stop();
	}
	if (m_pEventLoop != nullptr) {
		m_pEventLoop->quit();
	}
}
