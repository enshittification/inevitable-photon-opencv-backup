#include <opencv2/opencv.hpp>
#include <gif_lib.h>

#include "gif-palette.h"
#include "frame.h"
#include "encoder.h"
#include "opencv-encoder.h"

OpenCV_Encoder::OpenCV_Encoder(const std::string &format,
    int quality,
    const std::map<std::string, std::string> *options,
    std::vector<uint8_t> *output) {
  _options = options;
  _quality = quality;
  _format = format;
  _output = output;

  _output->clear();
}

bool OpenCV_Encoder::add_frame(const Frame &frame) {
  if (_output->size()) {
    _last_error = "Already encoded";
    return false;
  }

  std::vector<int> img_parameters;
  if ("jpeg" == _format) {
    img_parameters.push_back(cv::IMWRITE_JPEG_QUALITY);
    img_parameters.push_back(_quality);
  }
  else if ("png" == _format) {
    /* GMagick uses a single scalar for storing two values:
      _compressioN_quality = compression_level*10 + filter_type */
    img_parameters.push_back(cv::IMWRITE_PNG_COMPRESSION);
    img_parameters.push_back(_quality/10);
    /* OpenCV does not support setting the filters like GMagick,
       instead we get to pick the strategy, so we ignore it
       https://docs.opencv.org/4.1.0/d4/da8/group__imgcodecs.html */
  }
  else if ("webp" == _format) {
    auto lossless_option = _options->find("webp:lossless");

    img_parameters.push_back(cv::IMWRITE_WEBP_QUALITY);
    if (lossless_option != _options->end()
        && "true" == lossless_option->second) {
      img_parameters.push_back(101);
    }
    else {
      img_parameters.push_back(_quality);
    }
  }

  bool encoded = false;
  try {
    encoded = cv::imencode("." + _format,
        frame.img,
        *_output,
        img_parameters);
  }
  catch (cv::Exception &e) {
    _last_error = e.what();
    return false;
  }

  return encoded;
}

bool OpenCV_Encoder::finalize() {
  return _output->size();
}
