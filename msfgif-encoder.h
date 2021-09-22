class Msfgif_Encoder : public Encoder {
protected:
  const std::map<std::string, std::string> *_options;
  std::string _format;
  int _quality;
  std::vector<uint8_t> *_output;
  MsfGifState _gif_state;
  int _delay_error;
  bool _initialized;

  bool _init_state(const cv::Mat &frame) {
    if (!msf_gif_begin(&_gif_state, frame.cols, frame.rows)) {
      return false;
    }

    _initialized = true;
    return true;
  }

public:
  Msfgif_Encoder(const std::string &format,
      int quality,
      const std::map<std::string, std::string> *options,
      std::vector<uint8_t> *output) {
    _options = options;
    _quality = quality;
    _format = format;
    _output = output;

    _output->clear();
    _delay_error = 0;
    _initialized = false;
  }


  bool add_frame(const cv::Mat &frame, int delay) {
    if ("gif" != _format) {
      return false;
    }

    if (!_initialized && !_init_state(frame)) {
      printf("FAHLOU EM INI\n");
      return false;
    }

    cv::Mat img;

    switch (frame.channels()) {
      case 1:
        cv::cvtColor(frame, img, cv::COLOR_GRAY2BGRA);
        break;

      case 2:
        {
          std::vector<cv::Mat> ga_channels, bgra_channels;
          cv::split(frame, ga_channels);
          cv::cvtColor(ga_channels[0], img, cv::COLOR_GRAY2BGRA);
          cv::split(img, bgra_channels);
          bgra_channels[3] = ga_channels[1];
          cv::merge(bgra_channels, img);
        }
        break;

      case 3:
        cv::cvtColor(frame, img, cv::COLOR_BGR2BGRA);
        break;

      default:
        img = frame;
        break;
    }

    // Convert milliseconds to centiseconds
    _delay_error += delay % 10;
    delay /= 10;
    if (_delay_error >= 10) {
      delay += 1;
      _delay_error -= 10;
    }
    if (!msf_gif_frame(&_gif_state,
          img.data,
          delay,
          16,
          img.step)) {
      printf("FAHLOU EM ADD\n");
      return false;
    }

    return true;
  }

  bool finalize() {
    if (!_initialized) {
      return false;
    }

    MsfGifResult result = msf_gif_end(&_gif_state);
    bool success = result.data != nullptr;
    if (success) {
      _output->resize(result.dataSize);
      memcpy(_output->data(), result.data, result.dataSize);
    }

    msf_gif_free(result);
    return success;

    return true;
  }
};