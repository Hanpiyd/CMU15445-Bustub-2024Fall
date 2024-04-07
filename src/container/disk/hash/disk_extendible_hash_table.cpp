//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// disk_extendible_hash_table.cpp
//
// Identification: src/container/disk/hash/disk_extendible_hash_table.cpp
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "common/config.h"
#include "common/exception.h"
#include "common/logger.h"
#include "common/macros.h"
#include "common/rid.h"
#include "common/util/hash_util.h"
#include "container/disk/hash/disk_extendible_hash_table.h"
#include "storage/index/hash_comparator.h"
#include "storage/page/extendible_htable_bucket_page.h"
#include "storage/page/extendible_htable_directory_page.h"
#include "storage/page/extendible_htable_header_page.h"
#include "storage/page/page_guard.h"

namespace bustub {

template <typename K, typename V, typename KC>
DiskExtendibleHashTable<K, V, KC>::DiskExtendibleHashTable(const std::string &name, BufferPoolManager *bpm,
                                                           const KC &cmp, const HashFunction<K> &hash_fn,
                                                           uint32_t header_max_depth, uint32_t directory_max_depth,
                                                           uint32_t bucket_max_size)
    : bpm_(bpm),
      cmp_(cmp),
      hash_fn_(std::move(hash_fn)),
      header_max_depth_(header_max_depth),
      directory_max_depth_(directory_max_depth),
      bucket_max_size_(bucket_max_size) {
  WritePageGuard header_guard = bpm->NewPageGuarded(&header_page_id_).UpgradeWrite();
  auto header_page = header_guard.AsMut<ExtendibleHTableHeaderPage>();
  header_page->Init(header_max_depth);
  header_guard.Drop();
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::GetValue(const K &key, std::vector<V> *result, Transaction *transaction) const
    -> bool {
  ReadPageGuard header_guard = bpm_->FetchPageRead(header_page_id_);
  auto header_page = header_guard.As<ExtendibleHTableHeaderPage>();
  auto hash = Hash(key);
  auto dir_index = header_page->HashToDirectoryIndex(hash);
  page_id_t dir_page_id = header_page->GetDirectoryPageId(dir_index);
  if(dir_page_id==INVALID_PAGE_ID){
    return false;
  }
  ReadPageGuard dir_guard = bpm_->FetchPageRead(dir_page_id);
  auto dir_page = dir_guard.As<ExtendibleHTableDirectoryPage>();
  auto bucket_index = dir_page->HashToBucketIndex(hash);
  auto bucket_page_id = dir_page->GetBucketPageId(bucket_index);
  if(bucket_page_id==INVALID_PAGE_ID){
    return false;
  }
  ReadPageGuard bucket_guard = bpm_->FetchPageRead(bucket_page_id);
  auto bucket_page = bucket_guard.As<ExtendibleHTableBucketPage<K, V, KC>>();
  V value;
  if(bucket_page->Lookup(key,value,cmp_)){
    result->push_back(value);
    return true;
  }
  else{
    return false;
  }
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/

template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::Insert(const K &key, const V &value, Transaction *transaction) -> bool {
  auto hash = Hash(key);
  WritePageGuard header_guard = bpm_->FetchPageWrite(header_page_id_);
  auto header_page = header_guard.AsMut<ExtendibleHTableHeaderPage>();
  auto dir_index = header_page->HashToDirectoryIndex(hash);
  page_id_t dir_page_id = header_page->GetDirectoryPageId(dir_index);
  if(dir_page_id == INVALID_PAGE_ID){
    return InsertToNewDirectory(header_page,dir_index,hash,key,value);
  }
  WritePageGuard dir_guard = bpm_->FetchPageWrite(dir_page_id);
  auto dir_page = dir_guard.AsMut<ExtendibleHTableDirectoryPage>();
  auto bucket_index = dir_page->HashToBucketIndex(hash);
  page_id_t bucket_page_id = dir_page->GetBucketPageId(bucket_index);
  if(bucket_page_id == INVALID_PAGE_ID){
    return InsertToNewBucket(dir_page,bucket_index,key,value);
  }
  WritePageGuard bucket_guard = bpm_->FetchPageWrite(bucket_page_id);
  auto bucket_page = bucket_guard.AsMut<ExtendibleHTableBucketPage<K, V, KC>>();
  if(bucket_page->Insert(key,value,cmp_)){
    return true;
  }
  if(dir_page->GetLocalDepth(bucket_index)==dir_page->GetGlobalDepth()){
    if (dir_page->GetGlobalDepth() >= dir_page->GetMaxDepth()) {
      return false;
    }
    dir_page->IncrGlobalDepth();
  }
  dir_page->IncrLocalDepth(bucket_index);
  if (!SplitBucket(dir_page, bucket_page, bucket_index)) {
    return false;
  }
  bucket_guard.Drop();
  dir_guard.Drop();
  header_guard.Drop();
  return Insert(key,value,transaction);
}

template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::InsertToNewDirectory(ExtendibleHTableHeaderPage *header, uint32_t directory_idx,
                                                             uint32_t hash, const K &key, const V &value) -> bool {
  page_id_t dir_page_id = INVALID_PAGE_ID;
  WritePageGuard dir_guard = bpm_->NewPageGuarded(&dir_page_id).UpgradeWrite();
  auto dir_page = dir_guard.AsMut<ExtendibleHTableDirectoryPage>();
  dir_page->Init(directory_max_depth_);
  header->SetDirectoryPageId(directory_idx,dir_page_id);
  uint32_t bucket_idx = dir_page->HashToBucketIndex(hash);
  return InsertToNewBucket(dir_page,bucket_idx,key,value);
}

template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::InsertToNewBucket(ExtendibleHTableDirectoryPage *directory, uint32_t bucket_idx,
                                                          const K &key, const V &value) -> bool {
  page_id_t bucket_page_id = INVALID_PAGE_ID;
  WritePageGuard bucket_guard = bpm_->NewPageGuarded(&bucket_page_id).UpgradeWrite();
  auto bucket_page = bucket_guard.AsMut<ExtendibleHTableBucketPage<K, V, KC>>();
  bucket_page->Init(bucket_max_size_);
  directory->SetBucketPageId(bucket_idx,bucket_page_id);
  return bucket_page->Insert(key,value,cmp_);
}

template <typename K, typename V, typename KC>
void DiskExtendibleHashTable<K, V, KC>::UpdateDirectoryMapping(ExtendibleHTableDirectoryPage *directory,
                                                               uint32_t new_bucket_idx, page_id_t new_bucket_page_id,
                                                               uint32_t new_local_depth, uint32_t local_depth_mask) {
  throw NotImplementedException("DiskExtendibleHashTable is not implemented");
}

template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::SplitBucket(ExtendibleHTableDirectoryPage *directory,
                                                    ExtendibleHTableBucketPage<K, V, KC> *bucket, uint32_t bucket_idx)
    -> bool{
  page_id_t split_page_id;
  WritePageGuard split_bucket_guard = bpm_->NewPageGuarded(&split_page_id).UpgradeWrite();
  if (split_page_id == INVALID_PAGE_ID) {
    return false;
  }
  auto split_bucket = split_bucket_guard.AsMut<ExtendibleHTableBucketPage<K, V, KC>>();
  split_bucket->Init(bucket_max_size_);
  uint32_t split_idx = directory->GetSplitImageIndex(bucket_idx);
  uint32_t local_depth = directory->GetLocalDepth(bucket_idx);
  directory->SetBucketPageId(split_idx,split_page_id);
  directory->SetLocalDepth(split_idx,local_depth);
  page_id_t bucket_page_id = directory->GetBucketPageId(bucket_idx);
  if (bucket_page_id == INVALID_PAGE_ID) {
    return false;
  }
  int size = bucket->Size();
  std::list<std::pair<K, V>> entries;
  for (int i = 0; i < size; i++) {
    entries.emplace_back(bucket->EntryAt(i));
  }
  bucket->Clear();
  for (const auto &entry : entries) {
    uint32_t target_idx = directory->HashToBucketIndex(Hash(entry.first));
    page_id_t target_page_id = directory->GetBucketPageId(target_idx);
    assert(target_page_id == bucket_page_id || target_page_id == split_page_id);
    if (target_page_id == bucket_page_id) {
      bucket->Insert(entry.first, entry.second, cmp_);
    } else if (target_page_id == split_page_id) {
      split_bucket->Insert(entry.first, entry.second, cmp_);
    }
  }
  return true;
}


/*****************************************************************************
 * REMOVE
 *****************************************************************************/
template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::Remove(const K &key, Transaction *transaction) -> bool {
  ReadPageGuard header_guard = bpm_->FetchPageRead(header_page_id_);
  auto header_page = header_guard.As<ExtendibleHTableHeaderPage>();
  auto hash = Hash(key);
  auto dir_index = header_page->HashToDirectoryIndex(hash);
  page_id_t dir_page_id = header_page->GetDirectoryPageId(dir_index);
  if(dir_page_id==INVALID_PAGE_ID){
    return false;
  }
  ReadPageGuard dir_guard = bpm_->FetchPageRead(dir_page_id);
  auto dir_page = dir_guard.As<ExtendibleHTableDirectoryPage>();
  auto bucket_idx = dir_page->HashToBucketIndex(hash);
  page_id_t bucket_page_id = dir_page->GetBucketPageId(bucket_idx);
  if(bucket_page_id==INVALID_PAGE_ID){
    return false;
  }
  WritePageGuard bucket_guard = bpm_->FetchPageWrite(bucket_page_id);
  auto bucket_page = bucket_guard.AsMut<ExtendibleHTableBucketPage<K, V, KC>>();
  return bucket_page->Remove(key,cmp_);
}

template class DiskExtendibleHashTable<int, int, IntComparator>;
template class DiskExtendibleHashTable<GenericKey<4>, RID, GenericComparator<4>>;
template class DiskExtendibleHashTable<GenericKey<8>, RID, GenericComparator<8>>;
template class DiskExtendibleHashTable<GenericKey<16>, RID, GenericComparator<16>>;
template class DiskExtendibleHashTable<GenericKey<32>, RID, GenericComparator<32>>;
template class DiskExtendibleHashTable<GenericKey<64>, RID, GenericComparator<64>>;
}  // namespace bustub
