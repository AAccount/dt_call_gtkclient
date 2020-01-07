/* 
 * File:   SoundEffects.cpp
 * Author: Daniel
 * 
 * Created on December 25, 2019, 9:19 PM
 */

#include "SoundEffects.hpp"

//static
SoundEffects* SoundEffects::instance = NULL;

//static
SoundEffects* SoundEffects::getInstance()
{
	if(instance == NULL)
	{
		instance = new SoundEffects();
	}
	return instance;
}

SoundEffects::SoundEffects() :
logger(Logger::getInstance()),
r(R::getInstance()),
ringtone(std::make_unique<short[]>(RINGTONE_SAMPLERATE/RINGTONE_DIVISION)),
ringThreadAlive(false)
{
	//generate 1/10th of a second of the ~440hz tone used as the ringtone
	const double TONE_FREQ = 440;
	const double TOTAL_SAMPLES = RINGTONE_SAMPLERATE/RINGTONE_DIVISION;

	const double AMP = SHRT_MAX-1;
	const double FACTOR = (2.0*M_PI*TONE_FREQ) / RINGTONE_SAMPLERATE;
	short* ringtoneArray = ringtone.get();
	for(double i=0.0; i<TOTAL_SAMPLES; i=i+1.0)
	{
		ringtoneArray[(int)i] = AMP*sin(FACTOR*i);
	}
}

SoundEffects::~SoundEffects()
{
}

void SoundEffects::ring()
{
	try
	{
		ringThread = std::thread([this] {
			pa_sample_spec ss;
			memset(&ss, 0, sizeof (pa_sample_spec));
			ss.format = PA_SAMPLE_S16LE;
			ss.channels = 1;
			ss.rate = RINGTONE_SAMPLERATE;
			const std::string self = r->getString(R::StringID::SELF);
			const std::string description = r->getString(R::StringID::CALL_SCREEN_MEDIA_RINGTONE_DESC);
			ringtonePlayer = pa_simple_new(NULL, self.c_str(), PA_STREAM_PLAYBACK, NULL, description.c_str(), &ss, NULL, NULL, NULL);

			//write the ringtone 1/10th of a second at a time for 1.5s then 1s of silence.
			//using this weird 1/10th of a second at a time scheme because pa_simple functions all block
			const double TOTAL_SAMPLES = RINGTONE_SAMPLERATE / RINGTONE_DIVISION;
			std::unique_ptr<short[] > silenceArray = std::make_unique<short[]>((int) TOTAL_SAMPLES);
			const int DIVISIONS_RINGTONE = TONE_TIME*RINGTONE_DIVISION;
			const int DIVISIONS_SILENCE = SILENCE_TIME*RINGTONE_DIVISION;
			bool playRingtone = true;
			int divisionsPlayed = 0;
			struct timespec divisionTime;
			memset(&divisionTime, 0, sizeof (struct timespec));
			divisionTime.tv_sec = 0;
			divisionTime.tv_nsec = (int) (1000000000.0 / RINGTONE_DIVISION);
			ringtoneDone = false;
			while(!ringtoneDone)
			{
				short* item = silenceArray.get();;
				if (playRingtone)
				{
					item = ringtone.get();
				}

				int paWriteError = 0;
				pa_simple_write(ringtonePlayer, item, TOTAL_SAMPLES * sizeof(short), &paWriteError);
				nanosleep(&divisionTime, NULL);
				divisionsPlayed++;

				if (playRingtone && (divisionsPlayed == DIVISIONS_RINGTONE))
				{
					divisionsPlayed = 0;
					playRingtone = false;
				}
				else if (!playRingtone && (divisionsPlayed == DIVISIONS_SILENCE))
				{
					divisionsPlayed = 0;
					playRingtone = true;
				}
			}
			pa_simple_free(ringtonePlayer);
		});
		ringThreadAlive = true;
	}
	catch(std::system_error& e)
	{
		const std::string error = "ringThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
	}
}

void SoundEffects::stopRing()
{
	std::unique_lock<std::mutex> stopLock(stopRequestMutex);
	ringtoneDone = true;
	if(ringThreadAlive) //only call join once
	{
		ringThread.join();
		ringThreadAlive = false;
	}
}