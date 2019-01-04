/*
 * Opus.cpp
 *
 *  Created on: Jan 1, 2019
 *      Author: Daniel
 */

#include "Opus.hpp"

namespace
{
	const int STEREO2CH = 2;
	const int ENCODE_BITRATE = 32000;

	R* r = R::getInstance();
	Logger* logger = Logger::getInstance("");

	OpusEncoder* enc = NULL;
	OpusDecoder* dec = NULL;
}

void Opus::init()
{
	if(enc != NULL)
	{
		opus_encoder_destroy(enc);
	}
	int encerror;
	enc = opus_encoder_create(SAMPLERATE, STEREO2CH, OPUS_APPLICATION_VOIP, &encerror);
	if(encerror != OPUS_OK)
	{
		const std::string error = r->getString(R::StringID::OPUS_INIT_ENCERR) + std::to_string(encerror);
		logger->insertLog(Log(Log::TAG::OPUS_CODEC, error, Log::TYPE::ERROR).toString());
	}
	opus_encoder_ctl(enc, OPUS_SET_BITRATE(ENCODE_BITRATE));

	if(dec != NULL)
	{
		opus_decoder_destroy(dec);
	}
	int decerror;
	dec = opus_decoder_create(SAMPLERATE, STEREO2CH, &decerror);
	if(decerror != OPUS_OK)
	{
		const std::string error = r->getString(R::StringID::OPUS_INIT_DECERR) + std::to_string(decerror);
		logger->insertLog(Log(Log::TAG::OPUS_CODEC, error, Log::TYPE::ERROR).toString());
	}

}

int Opus::encode(short* wav, int wavSize, unsigned char* opus, int opusSize)
{
	const int wavSamplesPerChannel = wavSize/STEREO2CH;
	const int length = opus_encode(enc, wav, wavSamplesPerChannel, opus, opusSize);
	return length;
}

void Opus::closeEncoder()
{
	opus_encoder_destroy(enc);
	enc = NULL;
}

int Opus::decode(unsigned char* opus, int opusSize, short* wav, int wavSize)
{
	const int decodedSamples = opus_decode(dec, opus, opusSize, wav, wavSize/STEREO2CH, false)*STEREO2CH;
	return decodedSamples;
}

void Opus::closeDecoder()
{
	opus_decoder_destroy(dec);
	dec = NULL;
}
