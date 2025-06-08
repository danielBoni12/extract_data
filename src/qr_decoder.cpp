#include "qr_decoder.h"
#include <opencv2/opencv.hpp>
#include <ZXing/ReadBarcode.h>
#include <ZXing/BarcodeFormat.h>
#include <ZXing/DecodeHints.h>
#include <ZXing/ImageView.h>
#include <ZXing/Barcode.h>

constexpr int QR_SIZE = 55;

std::vector<cv::Point> getQRPositions(const cv::Size& frameSize, int qrSize) {
    int w = frameSize.width;
    int h = frameSize.height;
    return {
        {10, 10}, {w - qrSize - 10, 10},
        {10, h - qrSize - 10}, {w - qrSize - 10, h - qrSize - 10}
    };
}

std::string decodeQRRegion(const cv::Mat& frame, const cv::Rect& roi) {
    if (roi.x < 0 || roi.y < 0 || roi.x + roi.width > frame.cols || roi.y + roi.height > frame.rows)
        return "";

    cv::Mat gray, cropped = frame(roi);
    cv::cvtColor(cropped, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, gray, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    ZXing::ImageView image(gray.data, gray.cols, gray.rows, ZXing::ImageFormat::Lum);
    ZXing::DecodeHints hints;
    hints.setFormats(ZXing::BarcodeFormat::QRCode);
    auto result = ZXing::ReadBarcode(image, hints);

    return result.isValid() ? result.text() : "";
}

std::string decodeFullFrameQR(const cv::Mat& frame) {
    ZXing::ImageView image(frame.data, frame.cols, frame.rows, ZXing::ImageFormat::BGR);
    ZXing::DecodeHints hints;
    hints.setFormats(ZXing::BarcodeFormat::QRCode);
    auto result = ZXing::ReadBarcode(image, hints);
    return result.isValid() ? result.text() : "";
}

std::string extractQRFromVideo(const std::string& path) {
    cv::VideoCapture cap(path);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open video\n";
        return "";
    }

    const int maxFrames = 200;
    cv::Size frameSize((int)cap.get(cv::CAP_PROP_FRAME_WIDTH), (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    auto positions = getQRPositions(frameSize, QR_SIZE);

    for (int i = 0; i < maxFrames; ++i) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        for (const auto& pos : positions) {
            cv::Rect roi(pos.x, pos.y, QR_SIZE, QR_SIZE);
            std::string decoded = decodeQRRegion(frame, roi);
            if (!decoded.empty()) return decoded;
        }

        std::string fallbackQR = decodeFullFrameQR(frame);
        if (!fallbackQR.empty()) return fallbackQR;
    }

    return "";
}
