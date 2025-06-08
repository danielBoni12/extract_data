#include "audio_decoder.h"
#include "qr_decoder.h"

extern "C" {

__declspec(dllexport)
const char* decode_audio(const char* filepath) {
    static std::string result;
    bool audio =  extractAudio(filepath);
    result = decodeAudio();
    //result = decode_audio_from_video(filepath);
    return result.c_str();
}

__declspec(dllexport)
const char* decode_qr(const char* filepath) {
    static std::string result;
    result = extractQRFromVideo(filepath);
    return result.c_str();
}

}
