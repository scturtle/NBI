#pragma once

#include <switch/services/ncm.h>
#include <switch/services/ns.h>

typedef struct {
  NcmContentMetaKey metaRecord;
  u64 storageId;
} NX_PACKED ContentStorageRecord;

Result nsextInitialize(void);
void nsextExit(void);

Result nsPushApplicationRecord(u64 title_id, u8 last_modified_event, ContentStorageRecord *content_records_buf,
                               size_t buf_size);
Result nsListApplicationRecordContentMeta(u64 offset, u64 titleID, void *out_buf, size_t out_buf_size,
                                          u32 *entries_read_out);
Result nsDeleteApplicationRecord(u64 titleID);
