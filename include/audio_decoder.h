#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <string>

bool extractAudio(const std::string& videoPath);
std::string decodeAudio();

#endif // AUDIO_DECODER_H