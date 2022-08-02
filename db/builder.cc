// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/builder.h"
#include "leveldb/VTable_build.h"
#include "db/dbformat.h"
#include "db/filename.h"
#include "db/table_cache.h"
#include "db/version_edit.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/iterator.h"
#include <unordered_map>
namespace leveldb {
class PM_allocator;

Status BuildTable(const std::string& dbname, Env* env, const Options& options,
                  TableCache* table_cache, Iterator* iter, FileMetaData* meta,std::shared_ptr<PM_allocator>pm_alloc ,std::unordered_map<uint64_t,std::vector<segmentMapping*>> &VTmeta, uint64_t filenum) {
  Status s;
  meta->file_size = 0;
  iter->SeekToFirst();

  std::string fname = TableFileName(dbname, meta->number);
  if (iter->Valid()) {
    WritableFile* file;
    s = env->NewWritableFile(fname, &file);
    if (!s.ok()) {
      return s;
    }

    TableBuilder* builder = new TableBuilder(options, file);
    VTableBuilder* vbuilder=new VTableBuilderNVM(options,pm_alloc);
    meta->smallest.DecodeFrom(iter->key());
    // Slice key;
    Slice key,value;


    //implement KV separation.
    for (; iter->Valid(); iter->Next()) {
      key = iter->key();
      value=iter->value(); //返回实际的value
      // printf("value: %s\n",value.data());
      //先add到VTable，然后再将KVoffset包装为slice，添加到builder中
      vbuilder->Add(value,pm_alloc);
      // meta->file_size+=value.size();
      std::string test(vbuilder->GetKVOffset(value));
      Slice value_offset(test.data(), test.size());
      // uint64_t start = DecodeFixed64(value_offset.data());
      builder->Add(key, value_offset);
      

    }
    if (!key.empty()) {
      meta->largest.DecodeFrom(key);
    }
    

    // Finish and check for builder errors
    VTmeta[filenum]=vbuilder->Getsegs();
    // VTmeta.emplace(filenum,vbuilder->Getsegs()); 
    Log(options.info_log, "Vtable: %llu",
      (unsigned long long)filenum);
    
    s = builder->Finish();
    if (s.ok()) {
      meta->file_size = builder->FileSize();
      meta->keys=builder->NumEntries();
      assert(meta->file_size > 0);
    }
    delete builder;

    // Finish and check for vbuilder errors
    s=vbuilder->Finish();
    

    if(s.ok())
    {
        // to doing
    }

    // Finish and check for file errors
    if (s.ok()) {
      s = file->Sync();
    }
    if (s.ok()) {
      s = file->Close();
    }
    delete file;
    file = nullptr;

    if (s.ok()) {
      // Verify that the table is usable
      Iterator* it = table_cache->NewIterator(ReadOptions(), meta->number,
                                              meta->file_size);
      s = it->status();
      delete it;
    }
  }

  // Check for input iterator errors
  if (!iter->status().ok()) {
    s = iter->status();
  }

  if (s.ok() && meta->file_size > 0) {
    // Keep it
  } else {
    env->RemoveFile(fname);
  }
  return s;
}

}  // namespace leveldb
