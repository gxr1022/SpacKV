#include "leveldb/VTable.h"
#include <string>
#include "util/coding.h"
namespace leveldb{
    
    VTable::VTable(const Options &option,std::string* tmp_value):segment_size_(option.segment_size),tmp_value_(tmp_value)
    {

    }

    KVOffset* VTable::Getoffset()
    {
        KVOffset *kvoffset=new KVOffset();
        kvoffset->segment_start= DecodeFixed64(tmp_value_->data());
        kvoffset->segment_offset=DecodeFixed64(tmp_value_->data()+8);
        kvoffset->val_size=DecodeFixed64(tmp_value_->data()+16);
        return kvoffset;
    }

    Status VTable::Getvalue(KVOffset *offset,std::string* Value)
    {
        // KVOffset* offset= Getoffset();
        Slice result;
        Value->clear();
        if(segment_size_-8-offset->segment_offset >= offset->val_size) {
            // printf("offset: %lu, n:%lu\n ", offset->segment_offset,offset->val_size);
            result=Slice((char *)(offset->segment_start+offset->segment_offset),offset->val_size);
            Value->assign(result.data(),result.size());
            return Status::OK();
        }
        else{
            size_t remain_size=segment_size_-8-offset->segment_offset;
            uint64_t left_size=offset->val_size;
            uint64_t tmp_offset=offset->segment_start+offset->segment_offset;
            // std::string* Value;
            uint64_t next_segment_start, cur_segment_start;
            cur_segment_start=offset->segment_start;

            while(left_size>remain_size)
            {
                // char *a="fjsk";
                // Value->append(a,sizeof(a));
                if(remain_size!=0)
                {
                    Value->append((char*)tmp_offset,remain_size);
                }
                left_size-=remain_size;
                
                // printf("cur_segment_start:%lu\n",cur_segment_start);

                next_segment_start=DecodeFixed64((char*)cur_segment_start+segment_size_-8);
                // printf("next_segment_start:%lu\n",next_segment_start);

                tmp_offset=next_segment_start;
                if(left_size>segment_size_-8)
                {
                    remain_size=segment_size_-8;
                }
                else
                {
                    remain_size=left_size;
                } 
                
            }
            Value->append((char*)tmp_offset,remain_size);
            return Status::OK();
        }
    }

}
