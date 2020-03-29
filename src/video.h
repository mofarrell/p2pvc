#pragma once

#include <opencv2/opencv.hpp>

#include <ncurses.h>

#include <cassert>
#include <iostream>

#define CHANNELS 3

struct Pixel {
  Pixel(uint8_t red, uint8_t green, uint8_t blue)
    : m_red(red)
    , m_green(green)
    , m_blue(blue)
  {}

  uint8_t toTerm8BitColor() const {
    return 16 + m_red / 48 * 36 + m_green / 48 * 6 + m_blue / 48;
  }

  uint8_t intensity8Bit() const {
    return (m_red + m_green + m_blue) / 3;
  }

private:
  uint8_t m_red;
  uint8_t m_green;
  uint8_t m_blue;
};

struct Frame {
  Frame(uint32_t height, uint32_t width)
  : m_height(height)
  , m_width(width)
  , m_channels(CHANNELS)
  , m_data(m_width * m_height * m_channels)
  {}

  uint32_t height() const {
    return m_height;
  }

  uint32_t width() const {
    return m_width;
  }

  Pixel pixel(uint32_t y, uint32_t x) const {
    assert(m_channels == CHANNELS);
    assert((y * m_width + x) * m_channels + 2 < m_data.size());
    auto const ptr = &m_data[(y * m_width + x) * m_channels];
    return Pixel(ptr[2], ptr[1], ptr[0]);
  }

  uint8_t* data() {
    return m_data.data();
  }
   
private:
  uint32_t m_height;
  uint32_t m_width;
  uint32_t m_channels;
  std::vector<uint8_t> m_data;
};

struct Curses {
  Curses() {
    WINDOW* main_screen = initscr();
    keypad(main_screen, 1);
    nodelay(main_screen, 1);
    nonl();
    noecho();
    curs_set(0);
    attron(A_BOLD);
    refresh();

    if(has_colors()) {
      // Initialize terminal colors
      m_hasColors = true;
      start_color();
      for (int i = 0; i < (1 << 8); i++) {
        init_pair(i, i, COLOR_BLACK);
      }
    }
  }

  ~Curses() {
    endwin();
  }

  void draw(
    const Frame& frame,
    uint32_t startRow,
    uint32_t startCol,
    uint32_t height,
    uint32_t width
  ) {

    // The terminals operate slowly.

    for (uint32_t row = 0; row < height; row++) {
      for (uint32_t col = 0; col < width; col++) {
        const char* characters = " ..::--==+++***###%%%%%%%%@@@@@@@";
        auto const y = row * frame.height() / height;
        auto const x = col * frame.width() / width;
        auto const p = frame.pixel(y, x);
        auto const color = p.toTerm8BitColor();
        auto const c = characters[p.intensity8Bit() * sizeof(characters) / 256];
        drawChar(c | COLOR_PAIR(color), row + startRow, col + startCol);
      }    
    }
    if (m_debug) {
      attroff(A_BOLD);
      move(startRow, startCol);
      printw("Debug Info:");
      move(startRow + 1, startCol);
      printw("H: %d W: %d", frame.height(), frame.width());
      move(startRow + 2, startCol);
      printw("Has Colors: %s", m_hasColors ? "True" : "False");
      move(startRow + 3, startCol);
      printw("Baudrate: %d", baudrate());
      attron(A_BOLD);
    }
    refresh();
  }

  int checkInput() {
    int c = getch();
    switch (c) {
      case 'd':
        m_debug = !m_debug;
        break;
      default:
        break;
    }
    return c;
  }

  // Char `c` may include color codes in the upper bits.
  void drawChar(int32_t c, uint32_t row, uint32_t col) {
    if (col > COLS || col < 0 || row > LINES || row < 0) return;
    mvaddch(row, col, c);
  }

private:
  bool m_debug = false;
  bool m_hasColors = false;
};

struct Video {
  Video() : m_vc(0) {
    if (!m_vc.isOpened()) {
        throw std::runtime_error("Failed to setup camera for video capture.");
    }
    m_height = m_vc.get(cv::CAP_PROP_FRAME_HEIGHT);
    m_width = m_vc.get(cv::CAP_PROP_FRAME_WIDTH);
  }

  ~Video() {
  }
  
  Frame frame() {
    return Frame(m_height, m_width);
  }

  void captureFrame(Frame* frame) {
    cv::Mat frameMat(m_height, m_width, CV_8UC(CHANNELS), frame->data());
    if (!m_vc.read(frameMat)) {
      throw std::runtime_error("No frame grabbed yet.");
    }
  }

private:
  uint32_t m_height;
  uint32_t m_width;
  cv::VideoCapture m_vc;
};

