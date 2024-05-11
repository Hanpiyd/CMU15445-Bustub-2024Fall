//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager.h"

#include "common/exception.h"
#include "common/macros.h"
#include "storage/page/page_guard.h"

namespace bustub {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, size_t replacer_k,
                                     LogManager *log_manager)
    : pool_size_(pool_size), disk_scheduler_(std::make_unique<DiskScheduler>(disk_manager)), log_manager_(log_manager) {
  // TODO(students): remove this line after you have implemented the buffer pool manager
  /* throw NotImplementedException(
      "BufferPoolManager is not implemented yet. If you have finished implementing BPM, please remove the throw "
      "exception line in `buffer_pool_manager.cpp`."); */

  // we allocate a consecutive memory space for the buffer pool
  pages_ = new Page[pool_size_];
  replacer_ = std::make_unique<LRUKReplacer>(pool_size, replacer_k);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() { delete[] pages_; }

auto BufferPoolManager::NewPage(page_id_t *page_id) -> Page * {
  Page *page;
  frame_id_t id = -1;
  std::scoped_lock<std::mutex> lock(latch_);
  if (!free_list_.empty()) {
    id = free_list_.front();
    free_list_.pop_front();
    page = pages_ + id;
  } else {
    if (!replacer_->Evict(&id)) {
      return nullptr;
    }
    page = pages_ + id;
  }
  if (!page) {
    return nullptr;
  }
  if (page->IsDirty()) {
    auto promise = disk_scheduler_->CreatePromise();
    auto future = promise.get_future();
    disk_scheduler_->Schedule({true, page->GetData(), page->GetPageId(), std::move(promise)});
    future.get();
    page->is_dirty_ = false;
  }
  *page_id = AllocatePage();
  page_table_.erase(page->GetPageId());
  page_table_.emplace(*page_id, id);
  page->page_id_ = *page_id;
  page->pin_count_ = 1;
  page->ResetMemory();
  replacer_->RecordAccess(id);
  replacer_->SetEvictable(id, false);
  return page;
}

auto BufferPoolManager::FetchPage(page_id_t page_id, [[maybe_unused]] AccessType access_type) -> Page * {
  if (page_id == INVALID_PAGE_ID) {
    return nullptr;
  }
  std::scoped_lock<std::mutex> lock(latch_);
  if (page_table_.find(page_id) != page_table_.end()) {
    auto id = page_table_[page_id];
    auto page = pages_ + id;
    replacer_->RecordAccess(id);
    replacer_->SetEvictable(id, false);
    page->pin_count_ += 1;
    return page;
  } else {
    Page *page;
    frame_id_t id = -1;
    if (!free_list_.empty()) {
      id = free_list_.back();
      free_list_.pop_back();
      page = pages_ + id;
    } else {
      if (!replacer_->Evict(&id)) {
        return nullptr;
      }
      page = pages_ + id;
    }
    if (!page) {
      return nullptr;
    }
    if (page->IsDirty()) {
      auto promise = disk_scheduler_->CreatePromise();
      auto future = promise.get_future();
      disk_scheduler_->Schedule({true, page->GetData(), page->GetPageId(), std::move(promise)});
      future.get();
      page->is_dirty_ = false;
    }
    page_table_.erase(page->GetPageId());
    page_table_.emplace(page_id, id);
    page->page_id_ = page_id;
    page->pin_count_ = 1;
    page->ResetMemory();
    replacer_->RecordAccess(id);
    replacer_->SetEvictable(id, false);
    auto promise = disk_scheduler_->CreatePromise();
    auto future = promise.get_future();
    disk_scheduler_->Schedule({false, page->GetData(), page->GetPageId(), std::move(promise)});
    future.get();
    return page;
  }
}

auto BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty, [[maybe_unused]] AccessType access_type) -> bool {
  if (page_id == INVALID_PAGE_ID) {
    return false;
  } else if (page_table_.find(page_id) != page_table_.end()) {
    std::scoped_lock<std::mutex> lock(latch_);
    auto id = page_table_[page_id];
    auto page = pages_ + id;
    if (is_dirty) {
      page->is_dirty_ = is_dirty;
    }
    if (page->GetPinCount() == 0) {
      return false;
    } else {
      page->pin_count_ -= 1;
    }
    if (page->GetPinCount() == 0) {
      replacer_->SetEvictable(id, true);
    }
    return true;
  } else {
    return false;
  }
}

auto BufferPoolManager::FlushPage(page_id_t page_id) -> bool {
  if (page_id == INVALID_PAGE_ID) {
    return false;
  }
  std::scoped_lock lock(latch_);
  if (page_table_.find(page_id) == page_table_.end()) {
    return false;
  } else {
    auto page = pages_ + page_table_[page_id];
    auto promise = disk_scheduler_->CreatePromise();
    auto future = promise.get_future();
    disk_scheduler_->Schedule({true, page->GetData(), page->GetPageId(), std::move(promise)});
    future.get();
    page->is_dirty_ = false;
    return true;
  }
}

void BufferPoolManager::FlushAllPages() {
  for (size_t i = 0; i < pool_size_; i++) {
    auto page = pages_ + i;
    if (page->GetPageId() == INVALID_PAGE_ID) {
      continue;
    } else {
      FlushPage(page->GetPageId());
    }
  }
}

auto BufferPoolManager::DeletePage(page_id_t page_id) -> bool {
  if (page_id == INVALID_PAGE_ID) {
    return true;
  }
  if (page_table_.find(page_id) == page_table_.end()) {
    DeallocatePage(page_id);
    return true;
  }
  std::scoped_lock<std::mutex> lock(latch_);
  auto id = page_table_[page_id];
  auto page = pages_ + id;
  if (page->GetPinCount() > 0) {
    return false;
  } else {
    page_table_.erase(page_id);
    free_list_.push_back(id);
    replacer_->Remove(id);
    page->is_dirty_ = false;
    page->pin_count_ = 0;
    page->page_id_ = INVALID_PAGE_ID;
    page->ResetMemory();
    DeallocatePage(page_id);
    return true;
  }
}

auto BufferPoolManager::AllocatePage() -> page_id_t { return next_page_id_++; }

auto BufferPoolManager::FetchPageBasic(page_id_t page_id) -> BasicPageGuard {
  return BasicPageGuard(this, FetchPage(page_id));
}

auto BufferPoolManager::FetchPageRead(page_id_t page_id) -> ReadPageGuard {
  ReadPageGuard res = FetchPageBasic(page_id).UpgradeRead();
  return res;
}

auto BufferPoolManager::FetchPageWrite(page_id_t page_id) -> WritePageGuard {
  WritePageGuard res = FetchPageBasic(page_id).UpgradeWrite();
  return res;
}

auto BufferPoolManager::NewPageGuarded(page_id_t *page_id) -> BasicPageGuard {
  return BasicPageGuard(this, NewPage(page_id));
}

}  // namespace bustub 
