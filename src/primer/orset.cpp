#include "primer/orset.h"
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include "common/exception.h"
#include "fmt/format.h"

namespace bustub {
template <typename T>
auto ORSet<T>::Contains(const T &elem) const -> bool {
  // TODO(student): Implement this
  return std::any_of(v_.begin(), v_.end(), [elem](const auto &i) { return i.first == elem; });
  return false;
  // throw NotImplementedException("ORSet<T>::Contains is not implemented");
}

template <typename T>
void ORSet<T>::Add(const T &elem, uid_t uid) {
  // TODO(student): Implement this
  if (std::find(v_.begin(), v_.end(), std::make_pair(elem, uid)) == v_.end()) {
    v_.push_back(std::make_pair(elem, uid));
  }
  for (const auto &i : d_) {
    if (std::find(v_.begin(), v_.end(), i) != v_.end()) {
      v_.erase(std::find(v_.begin(), v_.end(), i));
    }
  }
  // throw NotImplementedException("ORSet<T>::Add is not implemented");
}

template <typename T>
void ORSet<T>::Remove(const T &elem) {
  // TODO(student): Implement this
  std::vector<std::pair<T, uid_t>> temp;
  for (const auto &i : v_) {
    if (i.first == elem) {
      temp.push_back(i);
    }
  }
  for (const auto &i : temp) {
    if (std::find(d_.begin(), d_.end(), i) == d_.end()) {
      d_.push_back(i);
    }
    auto it = std::find(v_.begin(), v_.end(), i);
    if (it != v_.end()) {
      v_.erase(it);
    }
  }
  // throw NotImplementedException("ORSet<T>::Remove is not implemented");
}

template <typename T>
void ORSet<T>::Merge(const ORSet<T> &other) {
  // TODO(student): Implement this
  std::vector<std::pair<T, uid_t>> temp;
  for (const auto &i : other.v_) {
    if (std::find(d_.begin(), d_.end(), i) == d_.end()) {
      temp.push_back(i);
    }
  }
  for (const auto &i : other.d_) {
    if (std::find(v_.begin(), v_.end(), i) != v_.end()) {
      v_.erase(std::find(v_.begin(), v_.end(), i));
    }
  }
  for (const auto &i : temp) {
    if (std::find(v_.begin(), v_.end(), i) == v_.end()) {
      v_.push_back(i);
    }
  }
  for (const auto &i : other.d_) {
    if (std::find(d_.begin(), d_.end(), i) == d_.end()) {
      d_.push_back(i);
    }
  }
  // throw NotImplementedException("ORSet<T>::Merge is not implemented");
}

template <typename T>
auto ORSet<T>::Elements() const -> std::vector<T> {
  // TODO(student): Implement this
  std::vector<T> res;
  for (const auto &i : v_) {
    res.push_back(i.first);
  }
  return res;
  // throw NotImplementedException("ORSet<T>::Elements is not implemented");
}

template <typename T>
auto ORSet<T>::ToString() const -> std::string {
  auto elements = Elements();
  std::sort(elements.begin(), elements.end());
  return fmt::format("{{{}}}", fmt::join(elements, ", "));
}

template class ORSet<int>;
template class ORSet<std::string>;

}  // namespace bustub
