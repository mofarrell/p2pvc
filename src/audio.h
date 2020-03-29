#pragma once

#include "frame.h"

#include <cassert>

#include <condition_variable>
#include <mutex>

#include <portaudiocpp/PortAudioCpp.hxx>

struct AudioFrame : Frame<uint8_t, 2> {
  AudioFrame()
  : Frame({0, 0}) 
  {}
  AudioFrame(uint32_t samples, uint32_t channels)
  : Frame({samples, channels}) 
  {}

  uint32_t samples() const {
    return dim(0);
  }

  uint32_t channels() const {
    return dim(1);
  }
};

struct Audio {
  static constexpr uint8_t kSampleSilence = 128;
  Audio() {
    auto& system = portaudio::System::instance();
    m_framesPerBuffer = 128; //paFramesPerBufferUnspecified;
    for (auto it = system.devicesBegin();
         it != system.devicesEnd();
         ++it) {
      std::cout << it->name() << std::endl;
      std::cout << "Max Input Channels: "
                << it->maxInputChannels() << std::endl;
      std::cout << "Max Output Channels: "
                << it->maxOutputChannels() << std::endl;
      std::cout << "Input Only: "
                << it->isInputOnlyDevice() << std::endl;
      std::cout << "Output Only: "
                << it->isOutputOnlyDevice() << std::endl;
      std::cout << "Full Duplex: "
                << it->isFullDuplexDevice() << std::endl;

      if (it->isSystemDefaultInputDevice()) {
        m_inputChannels = it->maxInputChannels();
        m_inputFrame = createInputFrame();

        portaudio::DirectionSpecificStreamParameters dirSpecConfig(
          *it, it->maxInputChannels(), 
          portaudio::SampleDataFormat::UINT8,
          true,
          it->defaultLowInputLatency(),
          nullptr
        );
        portaudio::StreamParameters streamConfig(
          dirSpecConfig,
          portaudio::DirectionSpecificStreamParameters::null(),
          it->defaultSampleRate(),
          m_framesPerBuffer,
          paNoFlag
        );
        
        m_inputStreamInternal.open(
          streamConfig,
          *this,
          &Audio::processInput
        );

        m_inputStreamInternal.start();
      }
      if (it->isSystemDefaultOutputDevice()) {
        m_outputChannels = it->maxOutputChannels();
        m_outputFrame = createOutputFrame();
        m_silentFrame = createOutputFrame();
        for (uint32_t i = 0; i < m_silentFrame.size(); i++) {
          m_silentFrame.data()[i] = kSampleSilence;
        }

        portaudio::DirectionSpecificStreamParameters dirSpecConfig(
          *it, it->maxOutputChannels(), 
          portaudio::SampleDataFormat::UINT8,
          true,
          it->defaultLowOutputLatency(),
          nullptr
        );
        portaudio::StreamParameters streamConfig(
          portaudio::DirectionSpecificStreamParameters::null(),
          dirSpecConfig,
          it->defaultSampleRate(),
          m_framesPerBuffer,
          paNoFlag
        );
        
        m_outputStreamInternal.open(
          streamConfig,
          *this,
          &Audio::processOutput
        );
        m_outputStreamInternal.start();
      }
    }
  }

  void waitForPending() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (!m_pendingOutput && !m_pendingInput) {
      m_cond.wait(lock);
    }
  }

  bool input(AudioFrame& frame) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_pendingInput) {
      std::swap(frame, m_inputFrame);
      m_pendingInput = false;
      return true;
    }
    return false;
  }

  bool output(AudioFrame& frame) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_pendingOutput) {
      std::swap(frame, m_outputFrame);
      m_pendingOutput = false;
      return true;
    }
    return false;
  }

  AudioFrame createInputFrame() {
    return AudioFrame(m_framesPerBuffer, m_inputChannels);
  }

  AudioFrame createOutputFrame() {
    return AudioFrame(m_framesPerBuffer, m_outputChannels);
  }

  ~Audio() {}

  // Internal processing.
  int processInput(const void* inputBuffer, void* outputBuffer,
                   unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags flags) {
    std::unique_lock<std::mutex> lock(m_mutex);
    const uint8_t* in = (uint8_t*)inputBuffer;
    if (m_pendingInput) return 0;
    assert(framesPerBuffer == m_inputFrame.samples());
    uint8_t* out = m_inputFrame.data();
    for (uint32_t i = 0; i < framesPerBuffer; i++) {
      if (m_inputChannels == 2) {
        *out++ = *in++;  // left
        *out++ = *in++;  // right
      } else {
        *out++ = *in++;
      }
    }
    m_pendingInput = true;
    m_cond.notify_one();
    return 0;
  }

  int processOutput(const void* inputBuffer, void* outputBuffer,
                    unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo,
                    PaStreamCallbackFlags flags) {
    std::unique_lock<std::mutex> lock(m_mutex);
    uint8_t* out = (uint8_t*)outputBuffer;
    uint8_t* in = [&] {
      if (!m_pendingOutput) {
        assert(framesPerBuffer == m_outputFrame.samples());
        return m_outputFrame.data();
      }
      assert(framesPerBuffer == m_silentFrame.samples());
      return m_silentFrame.data();
    }();
    for (uint32_t i = 0; i < framesPerBuffer; i++) {
      if (m_outputChannels == 2) {
        *out++ = *in++;  // left
        *out++ = *in++;  // right
      } else {
        *out++ = *in++;
      }
    }
    m_pendingOutput = true;
    m_cond.notify_one();
    return 0;
  }
private:
  std::mutex m_mutex;
  std::condition_variable m_cond;
  bool m_pendingInput{false};
  bool m_pendingOutput{true};

  AudioFrame m_inputFrame;
  AudioFrame m_outputFrame;
  AudioFrame m_silentFrame;

  uint32_t m_framesPerBuffer;
  uint32_t m_inputChannels;
  uint32_t m_outputChannels;
  portaudio::MemFunCallbackStream<Audio> m_inputStreamInternal;
  portaudio::MemFunCallbackStream<Audio> m_outputStreamInternal;
  portaudio::AutoSystem m_systemInitializer;
};
