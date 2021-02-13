#ifndef PREZ_STREAMS_TESTING_WIDGET_HPP
#define PREZ_STREAMS_TESTING_WIDGET_HPP

#include <ostream>

class Widget {
public:
  Widget(int n) : num_(n) {}
  ~Widget() = default;
  Widget(const Widget&) = delete;
  Widget(Widget&&) = default;
  Widget& operator=(const Widget&) = delete;
  Widget& operator=(Widget&&) = default;

  int num_ = 0;
};

bool operator==(const Widget& w1, const Widget& w2) noexcept { return w1.num_ == w2.num_; }
bool operator<(const Widget& w1, const Widget& w2) noexcept { return w1.num_ < w2.num_; }

std::ostream& operator<<(std::ostream& out, const Widget& w) { return out << w.num_; }

#endif  // WIDGET_HPP
