/*
 * Opus.hpp
 *
 *  Created on: Jan 1, 2019
 *      Author: Daniel
 */

#ifndef SRC_CODEC_OPUS_HPP_
#define SRC_CODEC_OPUS_HPP_

#include <memory>
#include <sodium.h>
#include <string.h>
#include <opus/opus.h>
#include <alloca.h>
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../R.hpp"

namespace Opus
{
	const int SAMPLERATE = 24000;
	const int WAVFRAMESIZE = 2880;

	void init();
	int encode(short wav[], int wavSize, std::unique_ptr<unsigned char[]>& opus, int opusSize);
	void closeEncoder();
	int decode(unsigned char opus[], int opusSize, std::unique_ptr<short[]>& wav, int wavSize);
	void closeDecoder();
};

#endif /* SRC_CODEC_OPUS_HPP_ */
