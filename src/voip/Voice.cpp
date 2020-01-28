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
logger(Logger::getInstance()),
r(R::getInstance()),
mute(false),
udp(Vars::serverAddress, Vars::mediaPort),
stopRequested(false),
encodeThreadAlive(false),
encodedb(-100L),
encodedbLabel(r->getInstance()->getString(R::StringID::VOIP_MEDIA_ENC_DB)),	
decodeThreadAlive(false),
decodedb(-100L),
decodedbLabel(r->getInstance()->getString(R::StringID::VOIP_MEDIA_DEC_DB))
{
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

bool Voice::connect()
{
	return udp.connect();
}

void Voice::setVoiceKey(std::unique_ptr<unsigned char[]>& key)
{
	udp.setVoiceSymmetricKey(key);
}

void Voice::start()
{
	udp.start();

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
	udp.closeSocket();

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
	std::unique_ptr<short[]> wavBuffer = std::make_unique<short[]>(wavFrames);
	std::unique_ptr<unsigned char[]> encBuffer = std::make_unique<unsigned char[]>(wavFrames);

	while(Vars::ustate == Vars::UserState::INCALL)
	{
		memset(wavBuffer.get(), 0, wavFrames*sizeof(short));
		int paReadError = 0;
		const int paread = pa_simple_read(wavRecorder, wavBuffer.get(), wavFrames*sizeof(short), &paReadError);
		if(paread != 0 )
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_ENC_PA_ERR) + std::to_string(paReadError);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
			continue;
		}

		encodedb = db(wavBuffer, wavFrames);
		
		//even if muted, still need to record audio in real time
		if(mute)
		{
			memset(wavBuffer.get(), 0, wavFrames*sizeof(short));
		}

		memset(encBuffer.get(), 0, wavFrames);
		const int encodeLength = Opus::encode(wavBuffer.get(), wavFrames, encBuffer.get(), wavFrames);
		if(encodeLength < 1)
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_ENC_OPUS_ERR) + std::to_string(encodeLength);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
			continue;
		}
		udp.write(encBuffer, encodeLength);
	}
	randombytes_buf(wavBuffer.get(), wavFrames*sizeof(short));
	randombytes_buf(encBuffer.get(), wavFrames);
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
	std::unique_ptr<unsigned char[]> encBuffer= std::make_unique<unsigned char[]>(wavFrames);
	std::unique_ptr<short[]> wavBuffer = std::make_unique<short[]>(wavFrames);

	while(Vars::ustate == Vars::UserState::INCALL)
	{
		memset(encBuffer.get(), 0, wavFrames);
		const int opusLength = udp.read(encBuffer, wavFrames);
		if(opusLength < 1)
		{
			continue;
		}
		
		memset(wavBuffer.get(), 0, wavFrames*sizeof(short));
		const int frames = Opus::decode(encBuffer.get(), opusLength, wavBuffer.get(), wavFrames);
		if(frames < 1)
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_DEC_OPUS_ERR) + std::to_string(frames);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
			continue;
		}
		decodedb = db(wavBuffer, wavFrames);
		
		int paWriteError = 0;
		const int paWrite = pa_simple_write(wavPlayer, wavBuffer.get(), frames*sizeof(short), &paWriteError);
		if(paWrite != 0 )
		{
			const std::string error = r->getString(R::StringID::VOIP_MEDIA_DEC_PA_ERR) + std::to_string(paWriteError);
			logger->insertLog(Log(Log::TAG::VOIP_VOICE, error, Log::TYPE::ERROR).toString());
		}
	}
	randombytes_buf(encBuffer.get(), wavFrames);
	randombytes_buf(wavBuffer.get(), wavFrames*sizeof(short));
	pa_simple_free(wavPlayer);
	Opus::closeDecoder();
	logger->insertLog(Log(Log::TAG::VOIP_VOICE, r->getString(R::StringID::VOIP_MEDIA_DEC_STOP), Log::TYPE::INFO).toString());
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

std::string Voice::stats()
{
	statBuilder.str(std::string());
	statBuilder.precision(3);
	statBuilder << udp.stats() << "\n" 
			<< encodedbLabel << std::to_string(encodedb) << " "
			<< decodedbLabel << std::to_string(decodedb);
	return statBuilder.str();
}

double Voice::db(std::unique_ptr<short[]>& sound, int size)
{
	double sum = 0L;
	const short* soundArray = sound.get();
	for(int i=0; i<size; i++)
	{
		const short sample = soundArray[i];
		const double percent = sample / (double)SHRT_MAX;
		sum = sum + (percent * percent);
	}
	double rms = sqrt(sum);
	double db = 20L * log10(rms);
	return db;
}