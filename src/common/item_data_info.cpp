/*
 * (C) 2007-2017 Alibaba Group Holding Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See the AUTHORS file for names of contributors.
 *
 */

#include "item_data_info.hpp"
#include "databuffer.hpp"

namespace tair {

void _item_meta::encode(DataBuffer *output, bool is_new) const {
    // 41 bytes
    output->writeInt16(magic);
    output->writeInt16(checksum);
    output->writeInt16(keysize);
    output->writeInt16(version);
    output->writeInt32(prefixsize);
    output->writeInt32(valsize);
    if (is_new)
        output->writeInt8(flag | TAIR_ITEM_FLAG_COMPRESS);
    else
        output->writeInt8(flag);
    output->writeInt64(cdate);
    output->writeInt64(mdate);
    output->writeInt64(edate);
}

bool _item_meta::decode(DataBuffer *input) {
    uint32_t valsize_i = 0;
    if (!input->readInt16(&magic)) return false;
    if (!input->readInt16(&checksum)) return false;
    if (!input->readInt16(&keysize)) return false;
    if (!input->readInt16(&version)) return false;
    if (!input->readInt32(&prefixsize)) return false;
    if (!input->readInt32(&valsize_i)) return false;
    if (!input->readInt8(&flag)) return false;
    if (!input->readInt64(&cdate)) return false;
    if (!input->readInt64(&mdate)) return false;
    if (!input->readInt64(&edate)) return false;

    valsize = valsize_i & 0xFFFFFF;
    return true;
}

} // tair
