#ifndef PM_ALLOCATOR_H
#define PM_ALLOCATOR_H

#include <stdint.h>
#include <queue>
#include <fcntl.h>
#include "options.h"
#include <string>
#include <libpmem.h>
#include "status.h"
#include "leveldb/env.h"

namespace leveldb{

class Options;

struct segmentMapping{
      uint64_t mapped_size;
      uint64_t start_addr;
      uint64_t segId;
};

class PM_allocator
{
  public:
    
    PM_allocator(const PM_allocator&) = delete;
    PM_allocator& operator=(const PM_allocator&) = delete;
    
    PM_allocator(const Options &option,const std::string& path);
    ~PM_allocator();
    
    Status Create_pool();
    segmentMapping* Allocate_segment();
    void Collect_segment(segmentMapping* segment);
    uint64_t Numfreelist () const { return freelist.size(); }
    void  Destroy_pool();
    char* Getraw(){
      return raw_;
    }
    uint64_t Get_usenum()
    {
      return use_num_;
    }
    uint64_t Getleftnum()
    {
      return freelist.size();
    }
    uint64_t GetByteswritten()
    {
      return bytes_written;
    }

  private:
    Env* const env_;
    char* raw_;//mapped_base
    uint64_t pool_size_;
    // bool is_new_pool_;
    uint64_t use_num_;
    uint64_t segment_size_;
    std::string path_;
    uint64_t bytes_written;
    std::queue<segmentMapping *> freelist;//freelist应该持久化
    
};






}
#endif