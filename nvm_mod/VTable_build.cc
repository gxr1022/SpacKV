#include "leveldb/VTable_build.h"
#include "leveldb/pm_allocator.h"
#include "util/coding.h"
#include <libpmem.h>


namespace leveldb{

    VTableBuilderNVM::VTableBuilderNVM(const Options &option,std::shared_ptr<PM_allocator>pm_alloc)
    :option_(option),segment_offset_(0),segment_size_(option.segment_size),meta_data_size_(0),kv_num_(0){
        //allocate the fist segment for VTable;
        segmentMapping* segmap=pm_alloc->Allocate_segment();
        segment_start_=segmap->start_addr;
        segId_=segmap->segId; 
        base_addr=pm_alloc->Getraw();
        
    }

    /***layout*****
    value_data|value_data|value_data|value_data……|next_segment_addrseg|smeta
    */
    //segment_size=16384
    void VTableBuilderNVM::Add(const Slice& value,std::shared_ptr<PM_allocator>pm_alloc){
        //current segment can put into value.  next_segment_addr=8B
        if(segment_offset_ + (uint64_t)value.size() +8 <= segment_size_)
        {
            // PutFixed64(&segment_buffer_, (uint64_t)value.size());
            segment_buffer_.append(value.data(), value.size());
            last_segment_offset=segment_offset_;
            last_segment_start=segment_start_;
            segment_offset_ += (uint64_t)value.size();
            // printf("segment_offset:%ul\n",segment_offset_);
            // PutFixed64(&segment_buffer_, segment_offset_);                 
 
        }
        else{
            
            uint64_t left_size=(uint64_t)value.size();//left part' size of the value
            uint64_t remain_size=segment_size_-segment_offset_-8;// remaining part of current segment
            size_t value_offset=0;
            while(left_size>remain_size)
            {
                //make full of current segment
                segment_buffer_.append(value.ToString(),value_offset,(size_t)remain_size);
                last_segment_offset=segment_offset_;
                last_segment_start=segment_start_;
                // if(segment_offset_ +remain_size+8==segment_size_)
                // {
                    
                    //put current segment meta into VTable
                    segmentMapping* current_segment=new segmentMapping();
                    current_segment->mapped_size=segment_size_;
                    current_segment->start_addr=segment_start_;
                    current_segment->segId=segId_;
                    segs_.push_back(current_segment);

                    //generate and upadate new current segment' meta
                    segmentMapping* segmap=pm_alloc->Allocate_segment();
                    segment_start_=segmap->start_addr;
                    segId_=segmap->segId;
                    segment_offset_=0;

                    //put next segment's address into segment_buffer
                    PutFixed64(&segment_buffer_, segment_start_);

                    // persist last segment

                    // WritableFile* segmentfile;
                    // int is_pmem=0;
                    // if(pmem_is_pmem((void*)current_segment->start_addr, segment_size_)) {
                    //     is_pmem=1;
                    // }
                    // segmentfile=new PosixPmemWritableFile((void*)current_segment->start_addr, segment_size_, is_pmem);
                    // Status s;
                    // s = segmentfile->Append(segment_buffer_);
                    // if(s.ok()) {
                    //     s = segmentfile->Sync();
                    // }
                    if(pmem_is_pmem((void*)current_segment->start_addr, segment_size_)) {
                        pmem_memcpy((void*)current_segment->start_addr, segment_buffer_.c_str(), segment_size_, 0);
                        pmem_persist((void*)current_segment->start_addr, segment_size_);
                    }
                    
                    // Slice result;
                    // result=Slice(base_addr,64);
                    

                    // clear segment_buffer
                    segment_buffer_.clear();
                    left_size-=remain_size;
                    value_offset+=remain_size;
                    remain_size=segment_size_-8;    
                // }
                // else{
                //     segment_offset_=remain_size;
                // }
                //deal with the left part of this value;
                
                // if(left_size<=segment_size_-8)
                // {
                
                // }
                // else{
                //     remain_size=segment_size_;
                // }
            }
            
            segment_buffer_.append(value.ToString(),value_offset,(size_t)left_size);
            //put current segment meta into VTable
            segmentMapping* current_segment=new segmentMapping();
            current_segment->mapped_size=segment_size_;
            current_segment->start_addr=segment_start_;
            current_segment->segId=segId_;
            segs_.push_back(current_segment);
            segment_offset_+=left_size;
       }
       kv_num_++;
    }

    Status VTableBuilderNVM::Finish(){

        // persist current segment

        // WritableFile* segmentfile;
        // int is_pmem=0;
        // if(pmem_is_pmem((void*)segment_start_, segment_size_)) {
        //     is_pmem=1;
        // }
        // segmentfile=new PosixPmemWritableFile((void*)segment_start_, segment_size_, is_pmem);
        // Status s;

        // Slice segment_buffers(segment_buffer_);

        // s = segmentfile->Append(segment_buffers);
        // if(s.ok()) {
        //     s = segmentfile->Sync();
        // }
        if(segment_buffer_.size()<segment_size_)
        {
            pmem_memcpy((char*)segment_start_, segment_buffer_.c_str(), segment_buffer_.size(), 0);
        }
        pmem_persist((char*)segment_start_, segment_buffer_.size());




        // clear segment_buffer
        segment_buffer_.clear();

        //persist segmappings, generate a few new segments to store segmeta
        // assert(segs_.size() > 0);
        // uint64_t meta_start_offset = 0;
        // meta_data_size_=16*segs_.size();
        // int seg_num=meta_data_size_/segment_size_;
        
        // for(int i=0; i < segs_.size(); i++) {
        //     PutFixed64(&meta_buffer_, segs_[i]->segId);
        //     PutFixed64(&meta_buffer_, segs_[i]->start_addr);
        // } //every segmentmapping occpy 16 Bytes


        // Status s;
        // // ……………… to do: persist segmeta!

        // if(!s.ok()) {
        //     Log(option_.info_log, "VTable build failed");
        //     Log(option_.info_log, s.ToString().data());
        //     // delete file;
        //     return s;
        // }
        return Status::OK();
    }

    std::string VTableBuilderNVM::GetKVOffset(const Slice& value){
        char* buf = new char[24];
        
        // printf("segment_offset_:" "0x%016lx\n", segment_start_); 
        EncodeFixed64(buf, last_segment_start);
        // uint64_t start=DecodeFixed64(buf);
        // printf("start: %ul\n",start);


        EncodeFixed64(buf + 8, last_segment_offset);
        EncodeFixed64(buf + 16, (uint64_t)value.size());

        std::string kvffset(buf, 24);
        // uint64_t start=DecodeFixed64(kvffset.data());

        delete buf;
        return kvffset;
    }

    Status VTableBuilderNVM::status() const {
        return Status::OK();
    }



}//namespace leveldb