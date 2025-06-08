#include "audio_decoder.h"
#include "kiss_fft.h"
#include <fstream>
#include <vector>
#include <cmath>
#include <iostream>
#include <bitset>
#include <sstream>
#include <algorithm>

const std::string TEMP_AUDIO = "extracted_audio.wav";

constexpr int SAMPLE_RATE = 44100;
constexpr double DURATION_PER_BIT = 0.05;
constexpr int FREQ_0 = 16000;
constexpr int FREQ_1 = 17000;
constexpr int SIGNAL_HZ = 15000;

bool extractAudio(const std::string& videoPath) {
    std::string cmd = "ffmpeg -y -i \"" + videoPath + "\" -vn -acodec pcm_s16le " + TEMP_AUDIO;
    return system(cmd.c_str()) == 0;
}

int findPeakFrequencyKissFFT(const std::vector<short>& chunk) {
    int N = static_cast<int>(chunk.size());
    kiss_fft_cfg cfg = kiss_fft_alloc(N, 0, nullptr, nullptr);
    if (!cfg) {
        std::cerr << "Failed to allocate KissFFT config.\n";
        return -1;
    }

    std::vector<kiss_fft_cpx> in(N), out(N);
    for (int i = 0; i < N; ++i) {
        in[i].r = static_cast<float>(chunk[i]);
        in[i].i = 0;
    }

    kiss_fft(cfg, in.data(), out.data());

    std::vector<double> magnitudes(N / 2);
    for (int i = 0; i < N / 2; ++i)
        magnitudes[i] = sqrt(out[i].r * out[i].r + out[i].i * out[i].i);

    int peakIndex = static_cast<int>(std::distance(magnitudes.begin(), std::max_element(magnitudes.begin(), magnitudes.end())));
    double freq = static_cast<double>(peakIndex) * SAMPLE_RATE / N;

    free(cfg);
    return static_cast<int>(freq);
}

std::string decodeAudio() {
    std::ifstream audio(TEMP_AUDIO, std::ios::binary);
    if (!audio.is_open()) {
        std::cerr << "[!] Failed to open audio file.\n";
        return "";
    }

    audio.seekg(44);
    std::vector<char> buffer((std::istreambuf_iterator<char>(audio)), {});
    std::vector<short> samples;
    for (size_t i = 0; i + 1 < buffer.size(); i += 2)
        samples.push_back(*reinterpret_cast<short*>(&buffer[i]));

    int chunkSize = static_cast<int>(SAMPLE_RATE * DURATION_PER_BIT);
    std::string bits;
    bool foundStart = false, prevHz = false, messageComplete = false;

    for (size_t i = 0; i + chunkSize < samples.size(); i += chunkSize) {
        std::vector<short> chunk(samples.begin() + i, samples.begin() + i + chunkSize);
        int freq = findPeakFrequencyKissFFT(chunk);

        if (!foundStart) {
            if (std::abs(freq - SIGNAL_HZ) < 100) {
                foundStart = true;
                prevHz = true;
                continue;
            }
        } else {
            if (std::abs(freq - SIGNAL_HZ) < 100) {
                if (prevHz) continue;
                messageComplete = true;
                break;
            }
            bits += (std::abs(freq - FREQ_1) < std::abs(freq - FREQ_0)) ? '1' : '0';
        }
        prevHz = false;
    }

    if (!foundStart) {
        std::cerr << "[!] Start signal not found.\n";
        return "";
    }

    std::stringstream result;
    for (size_t i = 0; i + 8 <= bits.size(); i += 8) {
        std::bitset<8> b(bits.substr(i, 8));
        result << static_cast<char>(b.to_ulong());
    }

    return result.str();
}
