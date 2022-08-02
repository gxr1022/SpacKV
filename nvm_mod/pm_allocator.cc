
#include"leveldb/pm_allocator.h"
#include <string>
#include<iostream>
#include<libpmem.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>

namespace leveldb{

PM_allocator::PM_allocator(const Options &option,const std::string& path) 
          :use_num_(0), pool_size_(option.pool_size),segment_size_(option.segment_size),path_(path),env_(option.env),bytes_written(0)
{
    // five=0;
}
Status PM_allocator::Create_pool()
{
    char* pmemaddr=nullptr;
    size_t mapped_len;
    int is_pmem;

    // snprintf(buf, sizeof(buf), "%s/cf_%u_%s_sstable.pool",
    //        nvmcfoption_->pmem_path.c_str(), cf_id, cf_name.c_str());
    // std::string pol_path(buf, strlen(buf));

    std::string pool_path=path_+"VTable.pool";
    if(env_->FileExists(pool_path))// NVM file exsists!
    {
        env_->RemoveFile(pool_path);
        return Status::OK();
    }

    pmemaddr=(char*)(pmem_map_file(pool_path.data(),pool_size_,PMEM_FILE_CREATE, 0666,
                                                &mapped_len, &is_pmem));                     
    assert(pmemaddr != nullptr);
    if(pmemaddr != NULL || pmemaddr != MAP_FAILED) {
        printf("pmem_pool path:%s map_len:%f GB base_addr:%p \n",pool_path.data(),mapped_len/1048576.0/1024,pmemaddr);
        // printf("%s\n",*pmemaddr);
        if(!is_pmem) {
            printf("memory pool: not pmem.\n");
        }
    }
    //赋值
    raw_=pmemaddr;

    uint64_t segment_num=pool_size_/segment_size_;
    
    printf("segments size: %lu\n",segment_size_);

    
    //freelist在内存中存在
    for (size_t segmentid = 0; segmentid < segment_num; segmentid+=1) {
        uint64_t alloc_segment_start = (uint64_t)raw_ + (segmentid * segment_size_);        
        segmentMapping* mapping=new segmentMapping();
        mapping->mapped_size=segment_size_;
        mapping->segId=segmentid;
        mapping->start_addr=alloc_segment_start;
        // printf("segmentid: %d, addr: 0x%016lx\n",segmentid,alloc_segment_start);
        freelist.push(mapping);     
        // printf("segmentID: %lu  address:%lu\n",segmentid,alloc_segment_start);       
    }
    printf("segments num: %lu\n",freelist.size());
    return Status::OK();
}

PM_allocator::~PM_allocator()
{
  Destroy_pool();
}

void PM_allocator::Destroy_pool()
{
    if(pmem_is_pmem((void*)raw_, pool_size_))
    {
        pmem_unmap(raw_, pool_size_);
    }
    
}

segmentMapping* PM_allocator::Allocate_segment(){
    segmentMapping* new_segment=new segmentMapping();
    // printf("used segment: %d\n",use_num_);
    // if(freelist.empty())
    // {
    //     std::cout<<use_num_;
    // }
    assert(freelist.size()>0);
    if(!freelist.empty())
    {  
       new_segment=freelist.front();//返回freelist中第一个元素的引用
       freelist.pop();
       use_num_++;
       bytes_written+=segment_size_; 
    }
    return new_segment;
}

void PM_allocator::Collect_segment(segmentMapping* segment)
{
    freelist.push(segment);

    use_num_--;
}



}
