#include "nx/ipc/ns_ext.h"

#include "service_guard.h"
#include <stdio.h>
#include <string.h>
#include <switch.h>

static Service g_nsAppManSrv, g_nsGetterSrv;

static Result _nsextGetSession(Service *srv, Service *srv_out, u32 cmd_id) {
  return serviceDispatch(srv, cmd_id, .out_num_objects = 1, .out_objects = srv_out, );
}

NX_GENERATE_SERVICE_GUARD(nsext);

Result _nsextInitialize(void) {
  Result rc = 0;
  rc = smGetService(&g_nsGetterSrv, "ns:am2");
  if (R_FAILED(rc))
    return rc;
  rc = _nsextGetSession(&g_nsGetterSrv, &g_nsAppManSrv, 7996);
  if (R_FAILED(rc))
    serviceClose(&g_nsGetterSrv);
  return rc;
}

void _nsextCleanup(void) {
  serviceClose(&g_nsAppManSrv);
  serviceClose(&g_nsGetterSrv);
}

Result nsPushApplicationRecord(u64 title_id, u8 last_modified_event, ContentStorageRecord *content_records_buf,
                               size_t buf_size) {

  struct {
    u8 last_modified_event;
    u8 padding[0x7];
    u64 title_id;
  } in = {last_modified_event, {0}, title_id};

  return serviceDispatchIn(&g_nsAppManSrv, 16, in, .buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
                           .buffers = {{content_records_buf, buf_size}});
}

Result nsListApplicationRecordContentMeta(u64 offset, u64 titleID, void *out_buf, size_t out_buf_size,
                                          u32 *entries_read_out) {

  struct {
    u64 offset;
    u64 titleID;
  } in = {offset, titleID};

  struct {
    u32 entries_read;
  } out;

  Result rc =
      serviceDispatchInOut(&g_nsAppManSrv, 17, in, out, .buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
                           .buffers = {{out_buf, out_buf_size}});

  if (R_SUCCEEDED(rc) && entries_read_out)
    *entries_read_out = out.entries_read;

  return rc;
}

Result nsDeleteApplicationRecord(u64 titleID) {
  struct {
    u64 titleID;
  } in = {titleID};

  return serviceDispatchIn(&g_nsAppManSrv, 27, in);
}
