#include "primer/orset.h"
#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include "common/exception.h"
#include "fmt/format.h"

namespace bustub {
template <typename T>
auto ORSet<T>::Contains(const T &elem) const -> bool {
  // TODO(student): Implement this
  for(auto i:v){
    if(i.first==elem){
      return true;
    }
  }
  return false;
  //throw NotImplementedException("ORSet<T>::Contains is not implemented");
}

template <typename T>
void ORSet<T>::Add(const T &elem, uid_t uid) {
  // TODO(student): Implement this
  if(std::find(v.begin(),v.end(),std::make_pair(elem,uid))==v.end()){
    v.push_back(std::make_pair(elem,uid));
  }
  for(auto i:d){
    if(std::find(v.begin(),v.end(),i)!=v.end()){
      v.erase(std::find(v.begin(),v.end(),i));
    }
  }
  return;
  //throw NotImplementedException("ORSet<T>::Add is not implemented");
}

template <typename T>
void ORSet<T>::Remove(const T &elem) {
  // TODO(student): Implement this
  std::vector<std::pair<T, uid_t>> temp;
  for (auto i : v) {
    if (i.first == elem) {
      temp.push_back(i);
    }
  }
  for (auto i : temp) {
    if(std::find(d.begin(),d.end(),i)==d.end()){
      d.push_back(i);
    }
    auto it = std::find(v.begin(), v.end(), i);
    if (it != v.end()) {
      v.erase(it);
    }
  }
  return;
  //throw NotImplementedException("ORSet<T>::Remove is not implemented");
}

template <typename T>
void ORSet<T>::Merge(const ORSet<T> &other) {
  // TODO(student): Implement this
  std::vector<std::pair<T, uid_t>> temp;
  for(auto i:other.v){
    if(std::find(d.begin(),d.end(),i)==d.end()){
      temp.push_back(i);
    }
  }
  for(auto i:other.d){
    if(std::find(v.begin(),v.end(),i)!=v.end()){
      v.erase(std::find(v.begin(),v.end(),i));
    }
  }
  for(auto i:temp){
    if(std::find(v.begin(),v.end(),i)==v.end()){
      v.push_back(i);
    }
  }
  for(auto i:other.d){
    if(std::find(d.begin(),d.end(),i)==d.end()){
          d.push_back(i);
    }
  }
  return;
  //throw NotImplementedException("ORSet<T>::Merge is not implemented");
}

template <typename T>
auto ORSet<T>::Elements() const -> std::vector<T> {
  // TODO(student): Implement this
  std::vector<T>res;
  for(auto i:v){
    res.push_back(i.first);
  }
  return res;
  //throw NotImplementedException("ORSet<T>::Elements is not implemented");
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
