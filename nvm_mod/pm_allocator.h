#ifndef PM_ALLOCATOR_H
#define PM_ALLOCATOR_H

#include <stdint.h>
#include <queue>
#include <fcntl.h>
#include "options.h"
#include <string>
#include <libpmem.h>
// #include "VTable_build.h"
#include "status.h"
namespace leveldb{


// class  VTableBuilderNVM;
struct segmentMapping{
      uint64_t mapped_size;
      uint64_t start_addr;
      uint64_t segId;
};

class PM_allocator
{
  public:
    // friend class  VTableBuilderNVM;
    
    PM_allocator(const PM_allocator&) = delete;
    PM_allocator& operator=(const PM_allocator&) = delete;
    
    PM_allocator(const Options &option,const std::string& path);
    ~PM_allocator();
    
    Status Create_pool();
    segmentMapping* Allocate_segment();
    void Collect_segment(segmentMapping* segment);
    uint64_t Numfreelist () const { return freelist.size(); }
    char* Getraw()const {return raw_;}


  private:
    char* raw_;//mapped_base
    uint64_t pool_size_;
    // bool is_new_pool_;
    uint64_t use_num_;
    uint64_t bytes_written;
    uint64_t five;
    uint64_t segment_size_;
    std::string path_;
    std::queue<segmentMapping *> freelist;//freelist应该持久化
    
};






}
#endif