/* 
 * File:   Voice.cpp
 * Author: Daniel
 * 
 * Created on December 25, 2019, 9:18 PM
 */

#include "Voice.hpp"

//static
Voice* Voice::instance = NULL;

Voice::Voice():
mute(false),
garbage(0), rxtotal(0), txtotal(0), rxSeq(0), txSeq(0), skipped(0), oorange(0),
reconnectTries(0),
stopRequested(false),
encodeThreadAlive(false),
decodeThreadAlive(false),
receiveMonitorAlive(false),
reconnectionAttempted(false),
logger(Logger::getInstance()),
r(R::getInstance())
{
	memset(&lastReceivedTimestamp, 0, sizeof(struct timeval));
}

Voice::~Voice()
{
}

//static
Voice* Voice::getInstance()
{
	if(instance == NULL)
	{
		instance = new Voice();
	}
	return instance;
}

void Voice::start()
{
	try
	{
		receiveMonitorThread = std::thread(&Voice::receiveMonitor, this);
		receiveMonitorAlive = true;
	}
	catch(std::system_error& e)
	{
		const std::string error = "receiveMonitorThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
		stopOnError();
		return;
	}
	
	Opus::init();
	try
	{
		encodeThread = std::thread(&Voice::mediaEncode, this);
		encodeThreadAlive = true;
	}
	catch(std::system_error& e)
	{
		const std::string error = "mediaEncodeThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
		stopOnError();
		return;
	}
	
	try
	{
		decodeThread = std::thread(&Voice::mediaDecode, this);
		decodeThreadAlive = true;
	}
	catch(std::system_error& e)
	{
		const std::string error = "mediaDecodeThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
		stopOnError();
		return;
	}
}

void Voice::stop()
{
	Vars::ustate = Vars::UserState::NONE;
	if(Vars::mediaSocket != -1)
	{
		shutdown(Vars::mediaSocket, 2);
		close(Vars::mediaSocket);
	}
	Vars::mediaSocket = -1;
	
	if(receiveMonitorAlive)
	{
		receiveMonitorThread.join();
		receiveMonitorAlive = false;
	}
	if(encodeThreadAlive)
	{
		encodeThread.join();
		encodeThreadAlive = false;
	}
	if(decodeThreadAlive)
	{
		decodeThread.join();
		decodeThreadAlive = false;
	}
	
	unsigned char* voiceKey = Vars::voiceKey.get();
	if(voiceKey != NULL)
	{
		randombytes_buf(voiceKey, crypto_box_SECRETKEYBYTES);
	}
	
	instance = NULL;
}

void Voice::mediaEncode()
{
	//assuming pulse audio get microphone always works (unlike android audio record)
	logger->insertLog(Log(Log::TAG::VOIP_VOICE, r->getString(R::StringID::VOIP_MEDIA_ENC_START), Log::TYPE::INFO).toString());

	//setup pulse audio as the voice recorder
	pa_simple* wavRecorder = NULL;
	pa_sample_spec ss;
	memset(&ss, 0, sizeof(pa_sample_spec));
	ss.format = PA_SAMPLE_S16LE;
	ss.channels = 2;
	ss.rate = Opus::SAMPLERATE;
	const std::string self = r->getString(R::StringID::SELF);
	const std::string description = r->getString(R::StringID::VOIP_MEDIA_ENC_DESC);
	wavRecorder = pa_simple_new(NULL, self.c_str(), PA_STREAM_RECORD, NULL, description.c_str(), &ss, NULL, NULL, NULL);
	int latencyErr;
	pa_usec_t latency = pa_simple_get_latency(wavRecorder, &latencyErr);
	const std::string info = r->getString(R::StringID::VOIP_MEDIA_ENC_LATENCY) + std::to_string(latency);
	logger->insertLog(Log(Log::TAG::VOIP_VOICE, info, Log::TYPE::INFO).toString());

	const int wavFrames = Opus::WAVFRAMESIZE;
	std::unique_ptr<unsigned char[]> packetBufferArray = std::make_unique<unsigned char[]>(Vars::MAX_UDP);
	unsigned char* packetBuffer = packetBufferArray.get();
	std::unique_ptr<short[]> wavBufferArray = std::make_unique<short[]>(wavFrames);
	short* wavBuffer = wavBufferArray.get();
	std::unique_ptr<unsigned char[]> encodedBufferArray = std::make_unique<unsigned char[]>(wavFrames);
	unsigned char* encodedBuffer = encodedBufferArray.get();

	while(Vars::ustate == Vars::UserState::INCALL)
	{
		memset(wavBuffer, 0, wavFrames*sizeof(short));
		int paReadError = 0;
		const int paread = pa_simple_read(wavRecorder, wavBuffer, wavFrames*sizeof(short), &paReadError);
		if(paread != 0 )
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_ENC_PA_ERR) + std::to_string(paReadError);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
			continue;
		}

		//even if muted, still need to record audio in real time
		if(mute)
		{
			memset(wavBuffer, 0, wavFrames*sizeof(short));
		}

		memset(encodedBuffer, 0, wavFrames);
		const int encodeLength = Opus::encode(wavBuffer, wavFrames, encodedBuffer, wavFrames);
		if(encodeLength < 1)
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_ENC_OPUS_ERR) + std::to_string(encodeLength);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
			continue;
		}

		memset(packetBuffer, 0, Vars::MAX_UDP);
		SodiumUtils::disassembleInt(txSeq, packetBuffer);
		txSeq++;
		memcpy(packetBuffer+sizeof(uint32_t), encodedBuffer, encodeLength);

		std::unique_ptr<unsigned char[]> packetEncrypted;
		int packetEncryptedLength = 0;
		SodiumUtils::sodiumEncrypt(false, packetBuffer, sizeof(uint32_t)+encodeLength, Vars::voiceKey.get(), NULL, packetEncrypted, packetEncryptedLength);
		if(packetEncryptedLength < 1)
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_ENC_SODIUM_ERR);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
			continue;
		}

		const int sent = sendto(Vars::mediaSocket, packetEncrypted.get(), packetEncryptedLength, 0, (struct sockaddr*)&Vars::mediaPortAddrIn, sizeof(struct sockaddr_in));
		if(sent < 0)
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_ENC_NETWORK_ERR);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
			reconnectUDP();
			continue;
		}
		else
		{
			txtotal = txtotal + packetEncryptedLength + HEADERS;
		}
	}
	randombytes_buf(packetBuffer, Vars::MAX_UDP);
	randombytes_buf(wavBuffer, wavFrames*sizeof(short));
	randombytes_buf(encodedBuffer, wavFrames);
	Opus::closeEncoder();
	pa_simple_free(wavRecorder);
	logger->insertLog(Log(Log::TAG::VOIP_VOICE, r->getString(R::StringID::VOIP_MEDIA_ENC_STOP), Log::TYPE::INFO).toString());
}

void Voice::mediaDecode()
{
	logger->insertLog(Log(Log::TAG::VOIP_VOICE, r->getString(R::StringID::VOIP_MEDIA_DEC_START), Log::TYPE::INFO).toString());

	//setup pulse audio for voice playback
	pa_simple* wavPlayer = NULL;
	pa_sample_spec ss;
	memset(&ss, 0, sizeof(pa_sample_spec));
	ss.format = PA_SAMPLE_S16LE;
	ss.channels = 2;
	ss.rate = Opus::SAMPLERATE;
	const std::string self = r->getString(R::StringID::SELF);
	const std::string description = r->getString(R::StringID::VOIP_MEDIA_DEC_DESC);
	wavPlayer = pa_simple_new(NULL, self.c_str(), PA_STREAM_PLAYBACK, NULL, description.c_str(), &ss, NULL, NULL, NULL);
	int latencyErr;
	pa_usec_t latency = pa_simple_get_latency(wavPlayer, &latencyErr);
	const std::string info = r->getString(R::StringID::VOIP_MEDIA_DEC_LATENCY) + std::to_string(latency);
	logger->insertLog(Log(Log::TAG::VOIP_VOICE, info, Log::TYPE::INFO).toString());

	const int wavFrames = Opus::WAVFRAMESIZE;
	std::unique_ptr<unsigned char[]> encBufferArray = std::make_unique<unsigned char[]>(wavFrames);
	unsigned char* encBuffer = encBufferArray.get();
	std::unique_ptr<short[]> wavBufferArray = std::make_unique<short[]>(wavFrames);
	short* wavBuffer = wavBufferArray.get();
	std::unique_ptr<unsigned char[]> packetBufferArray = std::make_unique<unsigned char[]>(Vars::MAX_UDP);
	unsigned char* packetBuffer = packetBufferArray.get();

	while(Vars::ustate == Vars::UserState::INCALL)
	{
		memset(packetBuffer, 0, Vars::MAX_UDP);

		struct sockaddr_in sender;
		socklen_t senderLength = sizeof(struct sockaddr_in);
		const int receivedLength = recvfrom(Vars::mediaSocket, packetBuffer, Vars::MAX_UDP, 0, (struct sockaddr*)&sender, &senderLength);
		if(receivedLength < 0)
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_DEC_NETWORK_ERR);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
			reconnectUDP();
			continue;
		}
		else
		{
			{
				std::unique_lock<std::mutex> receivedTimestampLock(receivedTimestampMutex);
				gettimeofday(&lastReceivedTimestamp, NULL);
			}
			rxtotal = rxtotal + receivedLength + HEADERS;
		}
		std::unique_ptr<unsigned char[]> packetDecrypted;
		int packetDecryptedLength = 0;
		SodiumUtils::sodiumDecrypt(false, packetBuffer, receivedLength, Vars::voiceKey.get(), NULL, packetDecrypted, packetDecryptedLength);
		if(packetDecryptedLength < sizeof(uint32_t)) //should have received at least the sequence number
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_DEC_SODIUM_ERR);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
			continue;
		}

		const int sequence = SodiumUtils::reassembleInt(packetDecrypted.get());
		if(sequence <= rxSeq)
		{
			skipped++;
			continue;
		}

		if(std::abs(sequence - rxSeq) > OORANGE_LIMIT)
		{
			oorange++;
		}
		else
		{
			rxSeq = sequence;
		}

		const int opusLength = packetDecryptedLength-sizeof(uint32_t);
		memset(encBuffer, 0, wavFrames);
		memcpy(encBuffer, packetDecrypted.get()+sizeof(uint32_t), opusLength);

		memset(wavBuffer, 0, wavFrames*sizeof(short));
		const int frames = Opus::decode(encBuffer, opusLength, wavBuffer, wavFrames);
		if(frames < 1)
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_DEC_OPUS_ERR) + std::to_string(frames);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
			randombytes_buf(packetDecrypted.get(), packetDecryptedLength);
			continue;
		}
		int paWriteError = 0;
		const int paWrite = pa_simple_write(wavPlayer, wavBuffer, frames*sizeof(short), &paWriteError);
		if(paWrite != 0 )
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_DEC_PA_ERR) + std::to_string(paWriteError);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
		}
		randombytes_buf(packetDecrypted.get(), packetDecryptedLength);
	}

	randombytes_buf(packetBuffer, Vars::MAX_UDP);
	randombytes_buf(encBuffer, wavFrames);
	randombytes_buf(wavBuffer, wavFrames*sizeof(short));
	pa_simple_free(wavPlayer);
	Opus::closeDecoder();
	logger->insertLog(Log(Log::TAG::VOIP_VOICE, r->getString(R::StringID::VOIP_MEDIA_DEC_STOP), Log::TYPE::INFO).toString());
}

void Voice::receiveMonitor()
{//this function is run on the UI thread: started form the constructor (which was called by the ui thread).
	const int A_SECOND = 1;
	while(Vars::ustate == Vars::UserState::INCALL)
	{
		std::unique_lock<std::mutex> receivedTimestampLock(receivedTimestampMutex);
		const int ASECOND_AS_US = 1000000;
		struct timeval now;
		memset(&now, 0, sizeof(struct timeval));
		gettimeofday(&now, NULL);
		const int btw = now.tv_usec - lastReceivedTimestamp.tv_usec;
		if((lastReceivedTimestamp.tv_usec > 0) && (btw > ASECOND_AS_US && Vars::mediaSocket != -1))
		{
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, r->getString(R::StringID::VOIP_LAST_UDP_FOREVER), Log::TYPE::ERROR).toString());
			shutdown(Vars::mediaSocket, 2);
			close(Vars::mediaSocket);
			Vars::mediaSocket = -1;
		}
		receivedTimestampLock.~unique_lock();
		sleep(A_SECOND);
	}
}

bool Voice::reconnectUDP()
{
	std::unique_lock<std::mutex> deadUDPLock(deadUDPMutex);
	if(Vars::ustate == Vars::UserState::NONE)
	{
		return false;
	}
	
	const int MAX_UDP_RECONNECTS = 10;
	if(reconnectTries > MAX_UDP_RECONNECTS)
	{
		return false;
	}

	if(reconnectionAttempted)
	{
		reconnectionAttempted = false; //already attempted, reset it for the next connection failure
		return true;
	}
	else
	{
		reconnectTries++;
		const bool reconnected = CmdListener::registerUDP();
		reconnectionAttempted = true;
		return reconnected;
	}
}

void Voice::stopOnError()
{
	std::unique_lock<std::mutex> stopLock(stopMutex);
	if(!stopRequested)
	{
		stopRequested = true;
		AsyncCentral::getInstance()->broadcast(Vars::Broadcast::CALL_END);
	}
	stop();
}

void Voice::toggleMic()
{
	mute = !mute;
	if(mute)
	{
		AsyncCentral::getInstance()->broadcast(Vars::Broadcast::MIC_MUTE);
	}
	else
	{
		AsyncCentral::getInstance()->broadcast(Vars::Broadcast::MIC_UNMUTE);
	}
}

int Voice::getGarbage() const
{
	return garbage;
}

int Voice::getRxtotal() const
{
	return rxtotal;
}

int Voice::getTxtotal() const
{
	return txtotal;
}

int Voice::getRxSeq() const
{
	return rxSeq;
}

int Voice::getTxSeq() const
{
	return txSeq;
}

int Voice::getSkipped() const
{
	return skipped;
}

int Voice::getOorange() const
{
	return oorange;
}