#include "nx/ipc/es.h"

#include <string.h>

#include "service_guard.h"
#include <switch.h>

static Service g_esSrv;

NX_GENERATE_SERVICE_GUARD(es);

Result _esInitialize() { return smGetService(&g_esSrv, "es"); }

void _esCleanup() { serviceClose(&g_esSrv); }

Result esImportTicket(void const *tikBuf, size_t tikSize, void const *certBuf, size_t certSize) {
  return serviceDispatch(&g_esSrv, 1,
                         .buffer_attrs =
                             {
                                 SfBufferAttr_HipcMapAlias | SfBufferAttr_In,
                                 SfBufferAttr_HipcMapAlias | SfBufferAttr_In,
                             },
                         .buffers = {
                             {tikBuf, tikSize},
                             {certBuf, certSize},
                         }, );
}
