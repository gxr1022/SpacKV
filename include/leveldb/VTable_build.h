#ifndef VTABLE_BUILD_H
#define VTABLE_BUILD_H

#include <stdint.h>
#include <queue>
#include <fcntl.h>
#include<string>
#include <unistd.h>
#include <vector>
#include <libpmem.h>


#include "options.h"
#include "pm_allocator.h"
#include "slice.h"
#include "status.h"
#include "env.h"
namespace leveldb {

class PM_allocator;
class PosixPmemWritableFile;

class VTableBuilder{
public:
  virtual ~VTableBuilder() {}
  // TableBuilderNVM(const Options &option, char *nvm_mem_);
  // TableBuilderNVM(const TableBuilder&) = delete;
  // TableBuilder& operator=(const TableBuilder&) = delete;

  // Add key,value to the table being constructed.
  // REQUIRES: key is after any previously added key according to comparator.
  // REQUIRES: Finish(), Abandon() have not been called
  //virtual void Add(const Slice& key, const Slice& value) = 0;
  virtual void Add(const Slice& value,std::shared_ptr<PM_allocator>pm_alloc)=0;
  virtual Status status() const = 0;

  virtual Status Finish() = 0;

  virtual std::string GetKVOffset(const Slice& value)=0;
  virtual std::vector<segmentMapping*> Getsegs()=0;

};
// TableBuilderNVM::~TableBuilderNVM() {}

class VTableBuilderNVM : public VTableBuilder{
public:
  VTableBuilderNVM(const VTableBuilderNVM&) = delete;
  VTableBuilderNVM& operator=(const VTableBuilderNVM&) = delete;

  VTableBuilderNVM(const Options &option,std::shared_ptr<PM_allocator>pm_alloc);
  ~VTableBuilderNVM(){};//析构函数声明了没有实现

  // Add key,value to the table being constructed.
  // REQUIRES: key is after any previously added key according to comparator.
  // REQUIRES: Finish(), Abandon() have not been called
  void Add(const Slice& value,std::shared_ptr<PM_allocator>pm_alloc) override;

  Status Finish() override;

  std::string GetKVOffset(const Slice& value)override;

  // uint64_t FileSize() const override;// {return offset_;} 

  // uint64_t RawDataSize() const override; //{ return raw_data_size_;}

  // uint64_t MetaDataSize() const override; //{ return meta_data_size_;}

  // uint64_t NumEntries() const override;

  

  Status status() const override;
  
  std::vector<segmentMapping*> Getsegs(){return this->segs_;}

  // Slice Data() const override;

  // Slice KeyData() const override;

private:
   
  
  char* base_addr;
//   struct Segmeta
//   {
//       uint64_t seg_offset;//当前segment已经使用的偏移位置
//       uint64_t seg_start; 
//       uint64_t seg_size;
//       uint64_t segId;
//   };

  std::string segment_buffer_; //存放value
  uint64_t segment_offset_;
  uint64_t segment_start_;
  uint64_t segId_;
  uint64_t segment_size_;

  uint64_t last_segment_start;
  uint64_t last_segment_offset;


  std::string meta_buffer_;//存放各个segment的元数据，
  uint64_t meta_data_size_;//用于判断需要分配几个segment
  
  std::string fname_; 
  uint64_t kv_num_;

  Options option_;
  std::vector<segmentMapping*> segs_;



//   struct KeyMeta {
//     // InternalKey key;
//     std::string key;
//     uint64_t key_offset_meta;
//     uint64_t data_offset;
//   };
//   // char *raw_;
//   std::string buffer_;
//   std::string meta_buffer_;
//   uint64_t data_offset_;
//   uint64_t meta_offset_;
//   // uint64_t raw_data_size_;
//   uint64_t meta_data_size_;
//   // int offset;

//   std::string fname_;
//   std::vector<KeyMeta*> meta_data_;
//   bool closed_;
};

}

#endif

