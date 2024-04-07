#include "storage/page/page_guard.h"
#include "buffer/buffer_pool_manager.h"

namespace bustub {

BasicPageGuard::BasicPageGuard(BasicPageGuard &&that) noexcept
    : bpm_(std::move(that.bpm_)), page_(std::move(that.page_)), is_dirty_(std::move(that.is_dirty_)) {}

void BasicPageGuard::Drop() {
  if (page_ == nullptr) {
    return;
  }
  if (page_->GetPageId() != INVALID_PAGE_ID) {
    bpm_->UnpinPage(page_->GetPageId(), is_dirty_);
  }
  bpm_ = nullptr;
  page_ = nullptr;
}

auto BasicPageGuard::operator=(BasicPageGuard &&that) noexcept -> BasicPageGuard & {
  if (this == &that) {
    return *this;
  }
  Drop();
  bpm_ = std::move(that.bpm_);
  page_ = std::move(that.page_);
  is_dirty_ = std::move(that.is_dirty_);
  that.page_ = nullptr; 
  return *this;
}

BasicPageGuard::~BasicPageGuard() { Drop(); };  // NOLINT

auto BasicPageGuard::UpgradeRead() -> ReadPageGuard {
  if (page_ != nullptr) {
    page_->RLatch();
  }
  ReadPageGuard res(this->bpm_, this->page_);
  bpm_ = nullptr;
  page_ = nullptr;
  return res;
}

auto BasicPageGuard::UpgradeWrite() -> WritePageGuard {
  if (page_ != nullptr) {
    page_->WLatch();
  }
  WritePageGuard res(this->bpm_, this->page_);
  bpm_ = nullptr;
  page_ = nullptr;
  return res;
}

ReadPageGuard::ReadPageGuard(BufferPoolManager *bpm, Page *page) : guard_(bpm, page) {}

ReadPageGuard::ReadPageGuard(ReadPageGuard &&that) noexcept {
   guard_ = std::move(that.guard_); 
   that.guard_.page_ = nullptr; 
}

auto ReadPageGuard::operator=(ReadPageGuard &&that) noexcept -> ReadPageGuard & {
  if (this == &that) {
    return *this;
  }
  Drop();
  guard_ = std::move(that.guard_);
  that.guard_.page_ = nullptr;
  return *this;
}

void ReadPageGuard::Drop() {
  if (guard_.page_ == nullptr) {
    return;
  }
  guard_.page_->RUnlatch();
  guard_.Drop();
  return;
}

ReadPageGuard::~ReadPageGuard() { Drop(); }  // NOLINT

WritePageGuard::WritePageGuard(BufferPoolManager *bpm, Page *page) : guard_(bpm, page) {}

WritePageGuard::WritePageGuard(WritePageGuard &&that) noexcept { guard_ = std::move(that.guard_); }

auto WritePageGuard::operator=(WritePageGuard &&that) noexcept -> WritePageGuard & {
  if (this == &that) {
    return *this;
  }
  Drop();
  guard_ = std::move(that.guard_);
  return *this;
}

void WritePageGuard::Drop() {
  if (guard_.page_ == nullptr) {
    return;
  }
  guard_.page_->WUnlatch();
  guard_.Drop();
  return;
}

WritePageGuard::~WritePageGuard() { Drop(); }  // NOLINT

}  // namespace bustub
