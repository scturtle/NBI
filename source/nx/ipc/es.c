/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "nx/ipc/es.h"

#include <string.h>

#include "service_guard.h"
#include <switch.h>

static Service g_esSrv;

NX_GENERATE_SERVICE_GUARD(es);

Result _esInitialize() { return smGetService(&g_esSrv, "es"); }

void _esCleanup() { serviceClose(&g_esSrv); }

Service *esGetServiceSession() { return &g_esSrv; }

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

Result esDeleteTicket(const RightsId *rightsIdBuf, size_t bufSize) {
  return serviceDispatch(&g_esSrv, 3, .buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
                         .buffers = {
                             {rightsIdBuf, bufSize},
                         }, );
}

Result esGetTitleKey(const RightsId *rightsId, u8 *outBuf, size_t bufSize) {
  struct {
    RightsId rights_Id;
    u32 key_generation;
  } in;
  memcpy(&in.rights_Id, rightsId, sizeof(RightsId));
  in.key_generation = 0;

  return serviceDispatchIn(&g_esSrv, 8, in, .buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
                           .buffers = {{outBuf, bufSize}}, );
}

Result esCountCommonTicket(u32 *numTickets) {
  struct {
    u32 num_tickets;
  } out;

  Result rc = serviceDispatchOut(&g_esSrv, 9, out);
  if (R_SUCCEEDED(rc) && numTickets)
    *numTickets = out.num_tickets;

  return rc;
}

Result esCountPersonalizedTicket(u32 *numTickets) {
  struct {
    u32 num_tickets;
  } out;

  Result rc = serviceDispatchOut(&g_esSrv, 10, out);
  if (R_SUCCEEDED(rc) && numTickets)
    *numTickets = out.num_tickets;

  return rc;
}

Result esListCommonTicket(u32 *numRightsIdsWritten, RightsId *outBuf, size_t bufSize) {
  struct {
    u32 num_rights_ids_written;
  } out;

  Result rc = serviceDispatchInOut(&g_esSrv, 11, *numRightsIdsWritten, out,
                                   .buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
                                   .buffers = {{outBuf, bufSize}}, );
  if (R_SUCCEEDED(rc) && numRightsIdsWritten)
    *numRightsIdsWritten = out.num_rights_ids_written;

  return rc;
}

Result esListPersonalizedTicket(u32 *numRightsIdsWritten, RightsId *outBuf, size_t bufSize) {
  struct {
    u32 num_rights_ids_written;
  } out;

  Result rc = serviceDispatchInOut(&g_esSrv, 12, *numRightsIdsWritten, out,
                                   .buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
                                   .buffers = {{outBuf, bufSize}}, );
  if (R_SUCCEEDED(rc) && numRightsIdsWritten)
    *numRightsIdsWritten = out.num_rights_ids_written;

  return rc;
}

Result esGetCommonTicketData(u64 *unkOut, void *outBuf1, size_t bufSize1, const RightsId *rightsId) {
  struct {
    RightsId rights_id;
  } in;
  memcpy(&in.rights_id, rightsId, sizeof(RightsId));

  struct {
    u64 unk;
  } out;

  Result rc =
      serviceDispatchInOut(&g_esSrv, 16, in, out, .buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
                           .buffers = {{outBuf1, bufSize1}}, );
  return rc;
}