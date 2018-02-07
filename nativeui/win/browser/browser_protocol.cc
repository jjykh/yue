// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser/browser_protocol.h"

#include <shlwapi.h>
#include <wrl.h>

#include <string>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

namespace nu {

BrowserProtocol::BrowserProtocol(const Browser::ProtocolHandler& handler)
    : ref_(0),
      handler_(handler) {
}

BrowserProtocol::~BrowserProtocol() {
}

STDMETHODIMP BrowserProtocol::QueryInterface(REFIID riid,
                                             void **ppvObject) {
  const QITAB QITable[] = {
    QITABENT(BrowserProtocol, IInternetProtocol),
    QITABENT(BrowserProtocol, IInternetProtocolInfo),
    { 0 },
  };
  return QISearch(this, QITable, riid, ppvObject);
}

STDMETHODIMP_(ULONG) BrowserProtocol::AddRef() {
  return InterlockedIncrement(&ref_);
}

STDMETHODIMP_(ULONG) BrowserProtocol::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0) {
    delete this;
  }
  return cref;
}

IFACEMETHODIMP BrowserProtocol::Start(LPCWSTR szUrl,
                                      IInternetProtocolSink *pIProtSink,
                                      IInternetBindInfo *pIBindInfo,
                                      DWORD grfSTI,
                                      HANDLE_PTR dwReserved) {
  // Request the handler for protocol job.
  if (!szUrl || !pIProtSink)
    return E_INVALIDARG;
  if (!handler_)
    return E_FAIL;
  protocol_job_ = handler_(base::UTF16ToUTF8(szUrl));
  if (!protocol_job_)
    return E_FAIL;

  // Start the job.
  Microsoft::WRL::ComPtr<IInternetProtocolSink> sink(pIProtSink);
  protocol_job_->Plug([sink](size_t size) {
    sink->ReportData(BSCF_FIRSTDATANOTIFICATION |
                     BSCF_LASTDATANOTIFICATION |
                     BSCF_DATAFULLYAVAILABLE,
                     0, static_cast<int>(size));
  });
  std::string mime_type;
  if (protocol_job_->GetMimeType(&mime_type)) {
    sink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE,
                         base::UTF8ToUTF16(mime_type).c_str());
  }
  return protocol_job_->Start() ? S_OK : E_FAIL;
}

IFACEMETHODIMP BrowserProtocol::Continue(PROTOCOLDATA *pStateInfo) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::Abort(HRESULT hrReason, DWORD dwOptions) {
  protocol_job_->Kill();
  protocol_job_ = nullptr;
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::Terminate(DWORD dwOptions) {
  protocol_job_ = nullptr;
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::Suspend() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::Resume() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::Read(void *pv, ULONG cb, ULONG *pcbRead) {
  size_t nread = protocol_job_->Read(pv, cb);
  if (nread == 0)
    return S_OK;
  *pcbRead = static_cast<ULONG>(nread);
  return S_FALSE;
}

IFACEMETHODIMP BrowserProtocol::Seek(LARGE_INTEGER dlibMove,
                                     DWORD dwOrigin,
                                     ULARGE_INTEGER *plibNewPosition) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::LockRequest(DWORD dwOptions) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::UnlockRequest() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::ParseUrl(LPCWSTR pwzUrl,
                                         PARSEACTION ParseAction,
                                         DWORD dwParseFlags,
                                         LPWSTR pwzResult,
                                         DWORD cchResult,
                                         DWORD *pcchResult,
                                         DWORD dwReserved) {
  return INET_E_DEFAULT_ACTION;
}

IFACEMETHODIMP BrowserProtocol::CombineUrl(LPCWSTR pwzBaseUrl,
                                           LPCWSTR pwzRelativeUrl,
                                           DWORD dwCombineFlags,
                                           LPWSTR pwzResult,
                                           DWORD cchResult,
                                           DWORD *pcchResult,
                                           DWORD dwReserved) {
  return INET_E_DEFAULT_ACTION;
}

IFACEMETHODIMP BrowserProtocol::CompareUrl(LPCWSTR pwzUrl1,
                                           LPCWSTR pwzUrl2,
                                           DWORD dwCompareFlags) {
  return INET_E_DEFAULT_ACTION;
}

IFACEMETHODIMP BrowserProtocol::QueryInfo(LPCWSTR pwzUrl,
                                          QUERYOPTION QueryOption,
                                          DWORD dwQueryFlags,
                                          LPVOID pBuffer,
                                          DWORD cbBuffer,
                                          DWORD *pcbBuf,
                                          DWORD dwReserved) {
  return INET_E_DEFAULT_ACTION;
}

}  // namespace nu
