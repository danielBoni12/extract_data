#include <iostream>
#include <sstream>
#include <vector>
#include "audio_decoder.h"
#include "qr_decoder.h"

const std::string INPUT_VIDEO = "output.mp4";

void splitAndPrint(const std::string& msg) {
    std::stringstream ss(msg);
    std::vector<std::string> parts;
    std::string part;

    while (std::getline(ss, part, ','))
        parts.push_back(part);

    if (parts.size() < 3)
        std::cout << "[!] Missing data fields, Decoding partial message...\n";

    const std::string& serial = parts[0];
    if (serial.length() != 10)
        std::cout << "Tablet serial number might be cropped or invalid: " << serial << '\n';
    else
        std::cout << "Tablet serial number: " << serial << '\n';

    if (parts.size() < 2) {
        std::cout << "USB serial number is missing!\n";
        return;
    }

    const std::string& usb = parts[1];
    if (usb.length() != 12)
        std::cout << "USB serial number might be cropped or invalid: " << usb << '\n';
    else
        std::cout << "USB serial number: " << usb << '\n';

    if (parts.size() < 3) {
        std::cout << "Timestamp number is missing!\n";
        return;
    }

    const std::string& date = parts[2];
    if (date.length() != 19)
        std::cout << "Timestamp might be cropped or invalid: " << date << '\n';
    else
        std::cout << "Time signature: " << date << '\n';
}



int main() {
    if (!extractAudio(INPUT_VIDEO)) {
        std::cerr << "[!] Audio extraction failed.\n";
        return 1;
    }

    std::string audioMsg = decodeAudio();
    std::cout << "Decoded from audio:\n";
    splitAndPrint(audioMsg);

    std::string qrMsg = extractQRFromVideo(INPUT_VIDEO);
    if (!qrMsg.empty()) {
        std::cout << "Decoded from QR:\n";
        splitAndPrint(qrMsg);
    } else {
        std::cout << "[-] No QR code found.\n";
    }

    return 0;
}
