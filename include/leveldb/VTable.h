#ifndef VTABLE_H
#define VTABLE_H

#include "slice.h"
#include "util/coding.h"
#include "options.h"
#include "status.h"

namespace leveldb{

struct KVOffset
{
    uint64_t segment_start;
    uint64_t segment_offset;
    size_t val_size;
};

class VTable
{
private:
    std::string* tmp_value_; //value offset
    uint64_t segment_size_;

public:
    VTable(const VTable&) = delete;
    VTable& operator=(const VTable&) = delete;

    VTable(const Options &option,std::string* tmp_value);

    KVOffset* Getoffset();
    Status Getvalue(KVOffset *kvoffset,std::string* value);
};

}

#endif