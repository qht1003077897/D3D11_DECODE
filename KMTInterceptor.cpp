#include "KMTInterceptor.h"
#include <string>
#include <sstream>
#include <fstream>
#include <detours.h>
#include <cstdarg>
#include <cassert>
#include <intrin.h>

// 静态成员初始化
HMODULE KMTInterceptor::s_hGdi32 = nullptr;
PFN_D3DKMT_OPENADAPTERFROMHDC KMTInterceptor::s_OriginalOpenAdapterFromHdc = nullptr;
PFN_D3DKMT_OPENADAPTERFROMDEVICENAME KMTInterceptor::s_OriginalOpenAdapterFromDeviceName = nullptr;
PFN_D3DKMT_CREATEDEVICE KMTInterceptor::s_OriginalCreateDevice = nullptr;
PFN_D3DKMT_CREATECONTEXT KMTInterceptor::s_OriginalCreateContext = nullptr;
PFN_D3DKMT_DESTROYDEVICE KMTInterceptor::s_OriginalDestroyDevice = nullptr;
PFN_D3DKMT_DESTROYCONTEXT KMTInterceptor::s_OriginalDestroyContext = nullptr;
PFN_D3DKMT_PRESENT KMTInterceptor::s_OriginalPresent = nullptr;

PFN_D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME KMTInterceptor::s_OriginalOpenAdapterFromGdiDisplayName = nullptr;
PFN_D3DKMT_QUERYADAPTERINFO KMTInterceptor::s_OriginalQueryAdapterInfo = nullptr;
PFN_D3DKMT_OPENRESOURCE KMTInterceptor::s_OriginalOpenResource = nullptr;
PFN_D3DKMT_OPENRESOURCEFROMNTHANDLE KMTInterceptor::s_OriginalOpenResourceFromNtHandle = nullptr;
PFN_D3DKMT_QUERYRESOURCEINFO KMTInterceptor::s_OriginalQueryResourceInfo = nullptr;
PFN_D3DKMT_QUERYRESOURCEINFOFROMNTHANDLE KMTInterceptor::s_OriginalQueryResourceInfoFromNtHandle = nullptr;
PFN_D3DKMT_CREATEALLOCATION KMTInterceptor::s_OriginalCreateAllocation = nullptr;
PFN_D3DKMT_DESTROYALLOCATION KMTInterceptor::s_OriginalDestroyAllocation = nullptr;
PFN_D3DKMT_GETDEVICESTATE KMTInterceptor::s_OriginalGetDeviceState = nullptr;
PFN_D3DKMT_SETGAMMARAMP KMTInterceptor::s_OriginalSetGammaRamp = nullptr;
PFN_D3DKMT_WAITFORVERTICALBLANKEVENT KMTInterceptor::s_OriginalWaitForVerticalBlankEvent = nullptr;

// New original pointers
PFN_D3DKMT_SUBMITCOMMAND KMTInterceptor::s_OriginalSubmitCommand = nullptr;
PFN_D3DKMT_CREATEHWQUEUE KMTInterceptor::s_OriginalCreateHwQueue = nullptr;
PFN_D3DKMT_DESTROYHWQUEUE KMTInterceptor::s_OriginalDestroyHwQueue = nullptr;
PFN_D3DKMT_ENUMADAPTERS KMTInterceptor::s_OriginalEnumAdapters = nullptr;
PFN_D3DKMT_LOCK KMTInterceptor::s_OriginalLock = nullptr;
PFN_D3DKMT_UNLOCK KMTInterceptor::s_OriginalUnlock = nullptr;

PFN_D3DKMT_RENDER KMTInterceptor::s_OriginalRender;
PFN_D3DKMT_FLIPOVERLAY KMTInterceptor::s_OriginalFlipOverlay = nullptr;
PFN_D3DKMT_CREATEOVERLAY KMTInterceptor::s_OriginalCreateOverlay = nullptr;
PFN_D3DKMT_UPDATEOVERLAY KMTInterceptor::s_OriginalUpdateOverlay = nullptr;
PFN_D3DKMT_SETVIDPNSOURCEOWNER KMTInterceptor::s_OriginalSetVidPnSourceOwner = nullptr;
PFN_D3DKMT_PRESENT_MULTIPLANE_OVERLAY KMTInterceptor::s_OriginalPresentMultiPlaneOverlay = nullptr;

// Generic originals for newer functions
PFN_D3DKMT_CREATEALLOCATION2 KMTInterceptor::s_OriginalCreateAllocation2 = nullptr;
PFN_D3DKMT_CONNECTDOORBELL KMTInterceptor::s_OriginalConnectDoorbell = nullptr;
PFN_D3DKMT_CREATEDOORBELL KMTInterceptor::s_OriginalCreateDoorbell = nullptr;
PFN_D3DKMT_CREATEDCFROMMEMORY KMTInterceptor::s_OriginalCreateDCFromMemory = nullptr;
PFN_D3DKMT_CREATEHWCONTEXT KMTInterceptor::s_OriginalCreateHwContext = nullptr;
PFN_D3DKMT_CREATENATIVEFENCE KMTInterceptor::s_OriginalCreateNativeFence = nullptr;
PFN_D3DKMT_ENUMADAPTERS2 KMTInterceptor::s_OriginalEnumAdapters2 = nullptr;
PFN_D3DKMT_ENUMADAPTERS3 KMTInterceptor::s_OriginalEnumAdapters3 = nullptr;
PFN_D3DKMT_ESCAPE KMTInterceptor::s_OriginalEscape = nullptr;
PFN_D3DKMT_GETDISPLAYMODELIST KMTInterceptor::s_OriginalGetDisplayModeList = nullptr;
PFN_D3DKMT_GETPRESENTHISTORY KMTInterceptor::s_OriginalGetPresentHistory = nullptr;
PFN_D3DKMT_LOCK2 KMTInterceptor::s_OriginalLock2 = nullptr;
PFN_D3DKMT_MAPGPUVIRTUALADDRESS KMTInterceptor::s_OriginalMapGpuVirtualAddress = nullptr;
PFN_D3DKMT_OFFERALLOCATIONS KMTInterceptor::s_OriginalOfferAllocations = nullptr;
PFN_D3DKMT_OPENRESOURCE2 KMTInterceptor::s_OriginalOpenResource2 = nullptr;
PFN_D3DKMT_PRESENTMULTIPLANEOVERLAY2 KMTInterceptor::s_OriginalPresentMultiPlaneOverlay2 = nullptr;
PFN_D3DKMT_PRESENTMULTIPLANEOVERLAY3 KMTInterceptor::s_OriginalPresentMultiPlaneOverlay3 = nullptr;
PFN_D3DKMT_PRESENT_REDIRECTED KMTInterceptor::s_OriginalPresentRedirected = nullptr;
PFN_D3DKMT_SETDISPLAYMODE KMTInterceptor::s_OriginalSetDisplayMode = nullptr;
PFN_D3DKMT_SUBMITCOMMANDTOHWQUEUE KMTInterceptor::s_OriginalSubmitCommandToHwQueue = nullptr;
PFN_D3DKMT_SUBMITPRESENTBLTTOHWQUEUE KMTInterceptor::s_OriginalSubmitPresentBltToHwQueue = nullptr;
PFN_D3DKMT_SUBMITPRESENTTOHWQUEUE KMTInterceptor::s_OriginalSubmitPresentToHwQueue = nullptr;

// Helper: convert narrow (ANSI) C string to wide string using system ANSI code page
static std::wstring AnsiToWString(const char* s) {
 if (!s) return std::wstring();
 int len = MultiByteToWideChar(CP_ACP,0, s, -1, nullptr,0);
 if (len <=0) return std::wstring();
 std::wstring out;
 out.resize(len -1);
 MultiByteToWideChar(CP_ACP,0, s, -1, &out[0], len);
 return out;
}

// 简单的日志函数
void WriteUtf8ToLogFile(const std::string& utf8) {
 const char bom[] = "\xEF\xBB\xBF";
 // Check if file exists and is empty
 std::ifstream inFile("../../KMTInterceptor.log", std::ios::binary);
 bool empty = true;
 if (inFile) {
 inFile.seekg(0, std::ios::end);
 empty = (inFile.tellg() <=0);
 }
 inFile.close();

 std::ofstream outFile("../../KMTInterceptor.log", std::ios::binary | std::ios::app);
 if (!outFile) return;
 if (empty) outFile.write(bom,3);
 outFile.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
 outFile.put('\n');
}

void LogMessage(const std::wstring& message) {
 // Build timestamped wide string
 SYSTEMTIME st;
 GetLocalTime(&st);
 wchar_t timebuf[64];
 swprintf_s(timebuf, L"[%02u:%02u:%02u] ", st.wHour, st.wMinute, st.wSecond);
 std::wstring full = timebuf;
 full += message;

 // Convert to UTF-8
 int utf8len = WideCharToMultiByte(CP_UTF8,0, full.c_str(), -1, nullptr,0, nullptr, nullptr);
 if (utf8len <=0) return;
 std::string utf8;
 utf8.resize(utf8len -1);
 WideCharToMultiByte(CP_UTF8,0, full.c_str(), -1, &utf8[0], utf8len, nullptr, nullptr);

 WriteUtf8ToLogFile(utf8);
}

// Variadic wide-format logger
void LogMessage(const wchar_t* format, ...) {
 va_list args;
 va_start(args, format);
 // compute required size
 va_list argsCopy;
 va_copy(argsCopy, args);
 int len = _vscwprintf(format, argsCopy) +1;
 va_end(argsCopy);

 std::wstring buf;
 buf.resize(len);
 vswprintf_s(&buf[0], len, format, args);
 va_end(args);
 if (!buf.empty() && buf.back() == L'\0') buf.resize(buf.size() -1);
 LogMessage(buf);
}

bool KMTInterceptor::Initialize() {
 LogMessage(L"Initializing KMT Interceptor...");

 s_hGdi32 = GetModuleHandleW(L"C:\\Windows\\System32\\gdi32.dll");
 if (!s_hGdi32) {
 LogMessage(L"Failed to get gdi32.dll handle");
 return false;
 }

 // 获取原始函数地址
 s_OriginalOpenAdapterFromHdc = reinterpret_cast<PFN_D3DKMT_OPENADAPTERFROMHDC>(
 GetProcAddress(s_hGdi32, "D3DKMTOpenAdapterFromHdc"));
 s_OriginalOpenAdapterFromDeviceName = reinterpret_cast<PFN_D3DKMT_OPENADAPTERFROMDEVICENAME>(
 GetProcAddress(s_hGdi32, "D3DKMTOpenAdapterFromDeviceName"));
 s_OriginalCreateDevice = reinterpret_cast<PFN_D3DKMT_CREATEDEVICE>(
 GetProcAddress(s_hGdi32, "D3DKMTCreateDevice"));
 s_OriginalCreateContext = reinterpret_cast<PFN_D3DKMT_CREATECONTEXT>(
 GetProcAddress(s_hGdi32, "D3DKMTCreateContext"));
 s_OriginalDestroyDevice = reinterpret_cast<PFN_D3DKMT_DESTROYDEVICE>(
 GetProcAddress(s_hGdi32, "D3DKMTDestroyDevice"));
 s_OriginalDestroyContext = reinterpret_cast<PFN_D3DKMT_DESTROYCONTEXT>(
 GetProcAddress(s_hGdi32, "D3DKMTDestroyContext"));
 s_OriginalPresent = reinterpret_cast<PFN_D3DKMT_PRESENT>(
 GetProcAddress(s_hGdi32, "D3DKMTPresent"));

 s_OriginalOpenAdapterFromGdiDisplayName = reinterpret_cast<PFN_D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME>(
 GetProcAddress(s_hGdi32, "D3DKMTOpenAdapterFromGdiDisplayName"));
 s_OriginalQueryAdapterInfo = reinterpret_cast<PFN_D3DKMT_QUERYADAPTERINFO>(
 GetProcAddress(s_hGdi32, "D3DKMTQueryAdapterInfo"));
 s_OriginalOpenResource = reinterpret_cast<PFN_D3DKMT_OPENRESOURCE>(
 GetProcAddress(s_hGdi32, "D3DKMTOpenResource"));
 s_OriginalOpenResourceFromNtHandle = reinterpret_cast<PFN_D3DKMT_OPENRESOURCEFROMNTHANDLE>(
 GetProcAddress(s_hGdi32, "D3DKMTOpenResourceFromNtHandle"));
 s_OriginalQueryResourceInfo = reinterpret_cast<PFN_D3DKMT_QUERYRESOURCEINFO>(
 GetProcAddress(s_hGdi32, "D3DKMTQueryResourceInfo"));
 s_OriginalQueryResourceInfoFromNtHandle = reinterpret_cast<PFN_D3DKMT_QUERYRESOURCEINFOFROMNTHANDLE>(
 GetProcAddress(s_hGdi32, "D3DKMTQueryResourceInfoFromNtHandle"));
 s_OriginalCreateAllocation = reinterpret_cast<PFN_D3DKMT_CREATEALLOCATION>(
 GetProcAddress(s_hGdi32, "D3DKMTCreateAllocation"));
 s_OriginalDestroyAllocation = reinterpret_cast<PFN_D3DKMT_DESTROYALLOCATION>(
 GetProcAddress(s_hGdi32, "D3DKMTDestroyAllocation"));
 s_OriginalGetDeviceState = reinterpret_cast<PFN_D3DKMT_GETDEVICESTATE>(
 GetProcAddress(s_hGdi32, "D3DKMTGetDeviceState"));
 s_OriginalSetGammaRamp = reinterpret_cast<PFN_D3DKMT_SETGAMMARAMP>(
 GetProcAddress(s_hGdi32, "D3DKMTSetGammaRamp"));
 s_OriginalWaitForVerticalBlankEvent = reinterpret_cast<PFN_D3DKMT_WAITFORVERTICALBLANKEVENT>(
 GetProcAddress(s_hGdi32, "D3DKMTWaitForVerticalBlankEvent"));

 s_OriginalSubmitCommand = reinterpret_cast<PFN_D3DKMT_SUBMITCOMMAND>(
 GetProcAddress(s_hGdi32, "D3DKMTSubmitCommand"));
 s_OriginalCreateHwQueue = reinterpret_cast<PFN_D3DKMT_CREATEHWQUEUE>(
 GetProcAddress(s_hGdi32, "D3DKMTCreateHwQueue"));
 s_OriginalDestroyHwQueue = reinterpret_cast<PFN_D3DKMT_DESTROYHWQUEUE>(
 GetProcAddress(s_hGdi32, "D3DKMTDestroyHwQueue"));
 s_OriginalEnumAdapters = reinterpret_cast<PFN_D3DKMT_ENUMADAPTERS>(
 GetProcAddress(s_hGdi32, "D3DKMTEnumAdapters"));
 s_OriginalLock = reinterpret_cast<PFN_D3DKMT_LOCK>(
 GetProcAddress(s_hGdi32, "D3DKMTLock"));
 s_OriginalUnlock = reinterpret_cast<PFN_D3DKMT_UNLOCK>(
 GetProcAddress(s_hGdi32, "D3DKMTUnlock"));

 s_OriginalRender = reinterpret_cast<PFN_D3DKMT_RENDER>(
     GetProcAddress(s_hGdi32, "D3DKMTRender"));
 s_OriginalFlipOverlay = reinterpret_cast<PFN_D3DKMT_FLIPOVERLAY>(
     GetProcAddress(s_hGdi32, "D3DKMTFlipOverlay"));
 s_OriginalCreateOverlay = reinterpret_cast<PFN_D3DKMT_CREATEOVERLAY>(
     GetProcAddress(s_hGdi32, "D3DKMTCreateOverlay"));
 s_OriginalUpdateOverlay = reinterpret_cast<PFN_D3DKMT_UPDATEOVERLAY>(
     GetProcAddress(s_hGdi32, "D3DKMTUpdateOverlay"));
 s_OriginalSetVidPnSourceOwner = reinterpret_cast<PFN_D3DKMT_SETVIDPNSOURCEOWNER>(
     GetProcAddress(s_hGdi32, "D3DKMTSetVidPnSourceOwner"));
 s_OriginalPresentMultiPlaneOverlay = reinterpret_cast<PFN_D3DKMT_PRESENT_MULTIPLANE_OVERLAY>(
     GetProcAddress(s_hGdi32, "D3DKMTPresentMultiPlaneOverlay"));

 // Generic original pointers (new functions)
 s_OriginalCreateAllocation2 = reinterpret_cast<PFN_D3DKMT_CREATEALLOCATION2>(
 GetProcAddress(s_hGdi32, "D3DKMTCreateAllocation2"));
 s_OriginalConnectDoorbell = reinterpret_cast<PFN_D3DKMT_CONNECTDOORBELL>(
 GetProcAddress(s_hGdi32, "D3DKMTConnectDoorbell"));
 s_OriginalCreateDoorbell = reinterpret_cast<PFN_D3DKMT_CREATEDOORBELL>(
 GetProcAddress(s_hGdi32, "D3DKMTCreateDoorbell"));
 s_OriginalCreateDCFromMemory = reinterpret_cast<PFN_D3DKMT_CREATEDCFROMMEMORY>(
 GetProcAddress(s_hGdi32, "D3DKMTCreateDCFromMemory"));
 s_OriginalCreateHwContext = reinterpret_cast<PFN_D3DKMT_CREATEHWCONTEXT>(
 GetProcAddress(s_hGdi32, "D3DKMTCreateHwContext"));
 s_OriginalCreateNativeFence = reinterpret_cast<PFN_D3DKMT_CREATENATIVEFENCE>(
 GetProcAddress(s_hGdi32, "D3DKMTCreateNativeFence"));
 s_OriginalEnumAdapters2 = reinterpret_cast<PFN_D3DKMT_ENUMADAPTERS2>(
 GetProcAddress(s_hGdi32, "D3DKMTEnumAdapters2"));
 s_OriginalEnumAdapters3 = reinterpret_cast<PFN_D3DKMT_ENUMADAPTERS3>(
 GetProcAddress(s_hGdi32, "D3DKMTEnumAdapters3"));
 s_OriginalEscape = reinterpret_cast<PFN_D3DKMT_ESCAPE>(
 GetProcAddress(s_hGdi32, "D3DKMTEscape"));
 s_OriginalGetDisplayModeList = reinterpret_cast<PFN_D3DKMT_GETDISPLAYMODELIST>(
 GetProcAddress(s_hGdi32, "D3DKMTGetDisplayModeList"));
 s_OriginalGetPresentHistory = reinterpret_cast<PFN_D3DKMT_GETPRESENTHISTORY>(
 GetProcAddress(s_hGdi32, "D3DKMTGetPresentHistory"));
 s_OriginalLock2 = reinterpret_cast<PFN_D3DKMT_LOCK2>(
 GetProcAddress(s_hGdi32, "D3DKMTLock2"));
 s_OriginalMapGpuVirtualAddress = reinterpret_cast<PFN_D3DKMT_MAPGPUVIRTUALADDRESS>(
 GetProcAddress(s_hGdi32, "D3DKMTMapGpuVirtualAddress"));
 s_OriginalOfferAllocations = reinterpret_cast<PFN_D3DKMT_OFFERALLOCATIONS>(
 GetProcAddress(s_hGdi32, "D3DKMTOfferAllocations"));
 s_OriginalOpenResource2 = reinterpret_cast<PFN_D3DKMT_OPENRESOURCE2>(
 GetProcAddress(s_hGdi32, "D3DKMTOpenResource2"));
 s_OriginalPresentMultiPlaneOverlay2 = reinterpret_cast<PFN_D3DKMT_PRESENTMULTIPLANEOVERLAY2>(
 GetProcAddress(s_hGdi32, "D3DKMTPresentMultiPlaneOverlay2"));
 s_OriginalPresentMultiPlaneOverlay3 = reinterpret_cast<PFN_D3DKMT_PRESENTMULTIPLANEOVERLAY3>(
 GetProcAddress(s_hGdi32, "D3DKMTPresentMultiPlaneOverlay3"));
 s_OriginalPresentRedirected = reinterpret_cast<PFN_D3DKMT_PRESENT_REDIRECTED>(
 GetProcAddress(s_hGdi32, "D3DKMTPresentRedirected"));
 s_OriginalSetDisplayMode = reinterpret_cast<PFN_D3DKMT_SETDISPLAYMODE>(
 GetProcAddress(s_hGdi32, "D3DKMTSetDisplayMode"));
 s_OriginalSubmitCommandToHwQueue = reinterpret_cast<PFN_D3DKMT_SUBMITCOMMANDTOHWQUEUE>(
 GetProcAddress(s_hGdi32, "D3DKMTSubmitCommandToHwQueue"));
 s_OriginalSubmitPresentBltToHwQueue = reinterpret_cast<PFN_D3DKMT_SUBMITPRESENTBLTTOHWQUEUE>(
 GetProcAddress(s_hGdi32, "D3DKMTSubmitPresentBltToHwQueue"));
 s_OriginalSubmitPresentToHwQueue = reinterpret_cast<PFN_D3DKMT_SUBMITPRESENTTOHWQUEUE>(
 GetProcAddress(s_hGdi32, "D3DKMTSubmitPresentToHwQueue"));

 // 安装钩子
 InstallHooks();

 LogMessage(L"KMT Interceptor initialized successfully");
 return true;
}

void KMTInterceptor::Shutdown() {
 LogMessage(L"Shutting down KMT Interceptor...");
 RemoveHooks();
}

// 如果没有 Detours，使用简单的 IAT Hook
void KMTInterceptor::InstallHooks() {
 LogMessage(L"Installing hooks...");
 DetourRestoreAfterWith();
 HRESULT ret = DetourTransactionBegin();
 assert(ret == NO_ERROR);
 DetourUpdateThread(GetCurrentThread());

 DetourAttach(&(PVOID&)s_OriginalOpenAdapterFromHdc, Hooked_D3DKMTOpenAdapterFromHdc);
 DetourAttach(&(PVOID&)s_OriginalOpenAdapterFromDeviceName, Hooked_D3DKMTOpenAdapterFromDeviceName);
 DetourAttach(&(PVOID&)s_OriginalCreateDevice, Hooked_D3DKMTCreateDevice);
 DetourAttach(&(PVOID&)s_OriginalCreateContext, Hooked_D3DKMTCreateContext);
 DetourAttach(&(PVOID&)s_OriginalDestroyDevice, Hooked_D3DKMTDestroyDevice);
 DetourAttach(&(PVOID&)s_OriginalDestroyContext, Hooked_D3DKMTDestroyContext);
 DetourAttach(&(PVOID&)s_OriginalPresent, Hooked_D3DKMTPresent);

 DetourAttach(&(PVOID&)s_OriginalOpenAdapterFromGdiDisplayName, Hooked_D3DKMTOpenAdapterFromGdiDisplayName);
 DetourAttach(&(PVOID&)s_OriginalQueryAdapterInfo, Hooked_D3DKMTQueryAdapterInfo);
 DetourAttach(&(PVOID&)s_OriginalOpenResource, Hooked_D3DKMTOpenResource);
 DetourAttach(&(PVOID&)s_OriginalOpenResourceFromNtHandle, Hooked_D3DKMTOpenResourceFromNtHandle);
 DetourAttach(&(PVOID&)s_OriginalQueryResourceInfo, Hooked_D3DKMTQueryResourceInfo);
 DetourAttach(&(PVOID&)s_OriginalQueryResourceInfoFromNtHandle, Hooked_D3DKMTOpenResourceFromNtHandle);
 DetourAttach(&(PVOID&)s_OriginalCreateAllocation, Hooked_D3DKMTCreateAllocation);
 DetourAttach(&(PVOID&)s_OriginalDestroyAllocation, Hooked_D3DKMTDestroyAllocation);
 DetourAttach(&(PVOID&)s_OriginalGetDeviceState, Hooked_D3DKMTGetDeviceState);
 DetourAttach(&(PVOID&)s_OriginalSetGammaRamp, Hooked_D3DKMTSetGammaRamp);
 DetourAttach(&(PVOID&)s_OriginalWaitForVerticalBlankEvent, Hooked_D3DKMTWaitForVerticalBlankEvent);

 // New attachments
 DetourAttach(&(PVOID&)s_OriginalSubmitCommand, Hooked_D3DKMTSubmitCommand);
 DetourAttach(&(PVOID&)s_OriginalCreateHwQueue, Hooked_D3DKMTCreateHwQueue);
 DetourAttach(&(PVOID&)s_OriginalDestroyHwQueue, Hooked_D3DKMTDestroyHwQueue);
 DetourAttach(&(PVOID&)s_OriginalEnumAdapters, Hooked_D3DKMTEnumAdapters);
 DetourAttach(&(PVOID&)s_OriginalLock, Hooked_D3DKMTLock);
 DetourAttach(&(PVOID&)s_OriginalUnlock, Hooked_D3DKMTUnlock);

 DetourAttach(&(PVOID&)s_OriginalRender, Hooked_D3DKMTRender);
 DetourAttach(&(PVOID&)s_OriginalFlipOverlay, Hooked_D3DKMTFlipOverlay);
 DetourAttach(&(PVOID&)s_OriginalCreateOverlay, Hooked_D3DKMTCreateOverlay);
 DetourAttach(&(PVOID&)s_OriginalUpdateOverlay, Hooked_D3DKMTUpdateOverlay);
 DetourAttach(&(PVOID&)s_OriginalSetVidPnSourceOwner, Hooked_D3DKMTSetVidPnSourceOwner);
 DetourAttach(&(PVOID&)s_OriginalPresentMultiPlaneOverlay, Hooked_D3DKMTPresentMultiPlaneOverlay);

 // Generic hooks attachments
 DetourAttach(&(PVOID&)s_OriginalCreateAllocation2, Hooked_D3DKMTCreateAllocation2);
 DetourAttach(&(PVOID&)s_OriginalConnectDoorbell, Hooked_D3DKMTConnectDoorbell);
 DetourAttach(&(PVOID&)s_OriginalCreateDoorbell, Hooked_D3DKMTCreateDoorbell);
 DetourAttach(&(PVOID&)s_OriginalCreateDCFromMemory, Hooked_D3DKMTCreateDCFromMemory);
 DetourAttach(&(PVOID&)s_OriginalCreateHwContext, Hooked_D3DKMTCreateHwContext);
 DetourAttach(&(PVOID&)s_OriginalCreateNativeFence, Hooked_D3DKMTCreateNativeFence);
 DetourAttach(&(PVOID&)s_OriginalEnumAdapters2, Hooked_D3DKMTEnumAdapters2);
 DetourAttach(&(PVOID&)s_OriginalEnumAdapters3, Hooked_D3DKMTEnumAdapters3);
 DetourAttach(&(PVOID&)s_OriginalEscape, Hooked_D3DKMTEscape);
 DetourAttach(&(PVOID&)s_OriginalGetDisplayModeList, Hooked_D3DKMTGetDisplayModeList);
 DetourAttach(&(PVOID&)s_OriginalGetPresentHistory, Hooked_D3DKMTGetPresentHistory);
 DetourAttach(&(PVOID&)s_OriginalLock2, Hooked_D3DKMTLock2);
 DetourAttach(&(PVOID&)s_OriginalMapGpuVirtualAddress, Hooked_D3DKMTMapGpuVirtualAddress);
 DetourAttach(&(PVOID&)s_OriginalOfferAllocations, Hooked_D3DKMTOfferAllocations);
 DetourAttach(&(PVOID&)s_OriginalOpenResource2, Hooked_D3DKMTOpenResource2);
 DetourAttach(&(PVOID&)s_OriginalPresentMultiPlaneOverlay2, Hooked_D3DKMTPresentMultiPlaneOverlay2);
 DetourAttach(&(PVOID&)s_OriginalPresentMultiPlaneOverlay3, Hooked_D3DKMTPresentMultiPlaneOverlay3);
 DetourAttach(&(PVOID&)s_OriginalPresentRedirected, Hooked_D3DKMTPresentRedirected);
 DetourAttach(&(PVOID&)s_OriginalSetDisplayMode, Hooked_D3DKMTSetDisplayMode);
 DetourAttach(&(PVOID&)s_OriginalSubmitCommandToHwQueue, Hooked_D3DKMTSubmitCommandToHwQueue);
 DetourAttach(&(PVOID&)s_OriginalSubmitPresentBltToHwQueue, Hooked_D3DKMTSubmitPresentBltToHwQueue);
 DetourAttach(&(PVOID&)s_OriginalSubmitPresentToHwQueue, Hooked_D3DKMTSubmitPresentToHwQueue);

 ret = DetourTransactionCommit();
 assert(ret == NO_ERROR);
 //这里使用简单的内存补丁方法
 // 注意：这是一个简化的示例，生产环境应该使用更健壮的hook方法
 LogMessage(L"Hooks would be installed here");
}

void KMTInterceptor::RemoveHooks() {
 LogMessage(L"Removing hooks...");
 HRESULT ret = DetourTransactionBegin();
 assert(ret == NO_ERROR);
 DetourUpdateThread(GetCurrentThread());

 DetourDetach(&(PVOID&)s_OriginalOpenAdapterFromHdc, Hooked_D3DKMTOpenAdapterFromHdc);
 DetourDetach(&(PVOID&)s_OriginalOpenAdapterFromDeviceName, Hooked_D3DKMTOpenAdapterFromDeviceName);
 DetourDetach(&(PVOID&)s_OriginalCreateDevice, Hooked_D3DKMTCreateDevice);
 DetourDetach(&(PVOID&)s_OriginalCreateContext, Hooked_D3DKMTCreateContext);
 DetourDetach(&(PVOID&)s_OriginalDestroyDevice, Hooked_D3DKMTDestroyDevice);
 DetourDetach(&(PVOID&)s_OriginalDestroyContext, Hooked_D3DKMTDestroyContext);
 DetourDetach(&(PVOID&)s_OriginalPresent, Hooked_D3DKMTPresent);

 DetourDetach(&(PVOID&)s_OriginalOpenAdapterFromGdiDisplayName, Hooked_D3DKMTOpenAdapterFromGdiDisplayName);
 DetourDetach(&(PVOID&)s_OriginalQueryAdapterInfo, Hooked_D3DKMTQueryAdapterInfo);
 DetourDetach(&(PVOID&)s_OriginalOpenResource, Hooked_D3DKMTOpenResource);
 DetourDetach(&(PVOID&)s_OriginalOpenResourceFromNtHandle, Hooked_D3DKMTOpenResourceFromNtHandle);
 DetourDetach(&(PVOID&)s_OriginalQueryResourceInfo, Hooked_D3DKMTQueryResourceInfo);
 DetourDetach(&(PVOID&)s_OriginalQueryResourceInfoFromNtHandle, Hooked_D3DKMTOpenResourceFromNtHandle);
 DetourDetach(&(PVOID&)s_OriginalCreateAllocation, Hooked_D3DKMTCreateAllocation);
 DetourDetach(&(PVOID&)s_OriginalDestroyAllocation, Hooked_D3DKMTDestroyAllocation);
 DetourDetach(&(PVOID&)s_OriginalGetDeviceState, Hooked_D3DKMTGetDeviceState);
 DetourDetach(&(PVOID&)s_OriginalSetGammaRamp, Hooked_D3DKMTSetGammaRamp);
 DetourDetach(&(PVOID&)s_OriginalWaitForVerticalBlankEvent, Hooked_D3DKMTWaitForVerticalBlankEvent);

 DetourDetach(&(PVOID&)s_OriginalSubmitCommand, Hooked_D3DKMTSubmitCommand);
 DetourDetach(&(PVOID&)s_OriginalCreateHwQueue, Hooked_D3DKMTCreateHwQueue);
 DetourDetach(&(PVOID&)s_OriginalDestroyHwQueue, Hooked_D3DKMTDestroyHwQueue);
 DetourDetach(&(PVOID&)s_OriginalEnumAdapters, Hooked_D3DKMTEnumAdapters);
 DetourDetach(&(PVOID&)s_OriginalLock, Hooked_D3DKMTLock);
 DetourDetach(&(PVOID&)s_OriginalUnlock, Hooked_D3DKMTUnlock);

 DetourDetach(&(PVOID&)s_OriginalRender, Hooked_D3DKMTRender);
 DetourDetach(&(PVOID&)s_OriginalFlipOverlay, Hooked_D3DKMTFlipOverlay);
 DetourDetach(&(PVOID&)s_OriginalCreateOverlay, Hooked_D3DKMTCreateOverlay);
 DetourDetach(&(PVOID&)s_OriginalUpdateOverlay, Hooked_D3DKMTUpdateOverlay);
 DetourDetach(&(PVOID&)s_OriginalSetVidPnSourceOwner, Hooked_D3DKMTSetVidPnSourceOwner);
 DetourDetach(&(PVOID&)s_OriginalPresentMultiPlaneOverlay, Hooked_D3DKMTPresentMultiPlaneOverlay);

 // Generic hooks detach
 DetourDetach(&(PVOID&)s_OriginalCreateAllocation2, Hooked_D3DKMTCreateAllocation2);
 DetourDetach(&(PVOID&)s_OriginalConnectDoorbell, Hooked_D3DKMTConnectDoorbell);
 DetourDetach(&(PVOID&)s_OriginalCreateDoorbell, Hooked_D3DKMTCreateDoorbell);
 DetourDetach(&(PVOID&)s_OriginalCreateDCFromMemory, Hooked_D3DKMTCreateDCFromMemory);
 DetourDetach(&(PVOID&)s_OriginalCreateHwContext, Hooked_D3DKMTCreateHwContext);
 DetourDetach(&(PVOID&)s_OriginalCreateNativeFence, Hooked_D3DKMTCreateNativeFence);
 DetourDetach(&(PVOID&)s_OriginalEnumAdapters2, Hooked_D3DKMTEnumAdapters2);
 DetourDetach(&(PVOID&)s_OriginalEnumAdapters3, Hooked_D3DKMTEnumAdapters3);
 DetourDetach(&(PVOID&)s_OriginalEscape, Hooked_D3DKMTEscape);
 DetourDetach(&(PVOID&)s_OriginalGetDisplayModeList, Hooked_D3DKMTGetDisplayModeList);
 DetourDetach(&(PVOID&)s_OriginalGetPresentHistory, Hooked_D3DKMTGetPresentHistory);
 DetourDetach(&(PVOID&)s_OriginalLock2, Hooked_D3DKMTLock2);
 DetourDetach(&(PVOID&)s_OriginalMapGpuVirtualAddress, Hooked_D3DKMTMapGpuVirtualAddress);
 DetourDetach(&(PVOID&)s_OriginalOfferAllocations, Hooked_D3DKMTOfferAllocations);
 DetourDetach(&(PVOID&)s_OriginalOpenResource2, Hooked_D3DKMTOpenResource2);
 DetourDetach(&(PVOID&)s_OriginalPresentMultiPlaneOverlay2, Hooked_D3DKMTPresentMultiPlaneOverlay2);
 DetourDetach(&(PVOID&)s_OriginalPresentMultiPlaneOverlay3, Hooked_D3DKMTPresentMultiPlaneOverlay3);
 DetourDetach(&(PVOID&)s_OriginalPresentRedirected, Hooked_D3DKMTPresentRedirected);
 DetourDetach(&(PVOID&)s_OriginalSetDisplayMode, Hooked_D3DKMTSetDisplayMode);
 DetourDetach(&(PVOID&)s_OriginalSubmitCommandToHwQueue, Hooked_D3DKMTSubmitCommandToHwQueue);
 DetourDetach(&(PVOID&)s_OriginalSubmitPresentBltToHwQueue, Hooked_D3DKMTSubmitPresentBltToHwQueue);
 DetourDetach(&(PVOID&)s_OriginalSubmitPresentToHwQueue, Hooked_D3DKMTSubmitPresentToHwQueue);

 ret = DetourTransactionCommit();
 assert(ret == NO_ERROR);
 LogMessage(L"Hooks would be remove here");
}

// Hooked functions implementations

// ========== KMT 钩子实现 ==========

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTQueryAdapterInfo(const D3DKMT_QUERYADAPTERINFO* pData) {
 std::wstring header = L"D3DKMTQueryAdapterInfo 被调用 ";
 LogMessage(header.c_str());
 LogMessage(L" hAdapter:0x%llX, Type: %u", (unsigned long long)pData->hAdapter, (unsigned)pData->Type);
 LogMessage(L" pPrivateDriverData 大小: %u bytes", pData->PrivateDriverDataSize);

 NTSTATUS result = s_OriginalQueryAdapterInfo ? s_OriginalQueryAdapterInfo(const_cast<D3DKMT_QUERYADAPTERINFO*>(pData)) :0;

 LogMessage(L"D3DKMTQueryAdapterInfo 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTOpenResource(D3DKMT_OPENRESOURCE* pData) {
 std::wstring header = L"D3DKMTOpenResource 被调用 ";
 LogMessage(header.c_str());
 LogMessage(L" hDevice:0x%llX, hGlobalShare:0x%llX", (unsigned long long)pData->hDevice, (unsigned long long)pData->hGlobalShare);

 NTSTATUS result = s_OriginalOpenResource ? s_OriginalOpenResource(pData) :0;

 LogMessage(L"D3DKMTOpenResource 返回:0x%X, hResource:0x%llX", result, (unsigned long long)pData->hResource);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTOpenResourceFromNtHandle(D3DKMT_OPENRESOURCEFROMNTHANDLE* pData) {
 std::wstring header = L"D3DKMTOpenResourceFromNtHandle 被调用 ";
 LogMessage(header.c_str());
 LogMessage(L" hDevice:0x%llX, hNtHandle:0x%p", (unsigned long long)pData->hDevice, pData->hNtHandle);

 NTSTATUS result = s_OriginalOpenResourceFromNtHandle ? s_OriginalOpenResourceFromNtHandle(pData) :0;

 LogMessage(L"D3DKMTOpenResourceFromNtHandle 返回:0x%X, hResource:0x%llX", result, (unsigned long long)pData->hResource);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTQueryResourceInfo(D3DKMT_QUERYRESOURCEINFO* pData) {
 std::wstring header = L"D3DKMTQueryResourceInfo 被调用 ";
 LogMessage(header.c_str());
 LogMessage(L" hDevice:0x%llX, hGlobalShare:0x%llX", (unsigned long long)pData->hDevice, (unsigned long long)pData->hGlobalShare);

 NTSTATUS result = s_OriginalQueryResourceInfo ? s_OriginalQueryResourceInfo(pData) :0;

 LogMessage(L"D3DKMTQueryResourceInfo 返回:0x%X", result);
 if (SUCCEEDED(result)) {
 LogMessage(L" ResourceInfo: NumAllocations: %u", pData->NumAllocations);
 }
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTQueryResourceInfoFromNtHandle(D3DKMT_QUERYRESOURCEINFOFROMNTHANDLE* pData) {
 std::wstring header = L"D3DKMTQueryResourceInfoFromNtHandle 被调用 ";
 LogMessage(header.c_str());
 LogMessage(L" hDevice:0x%llX, hNtHandle:0x%p", (unsigned long long)pData->hDevice, pData->hNtHandle);

 NTSTATUS result = s_OriginalQueryResourceInfoFromNtHandle ? s_OriginalQueryResourceInfoFromNtHandle(pData) :0;

 LogMessage(L"D3DKMTQueryResourceInfoFromNtHandle 返回:0x%X", result);
 if (SUCCEEDED(result)) {
 LogMessage(L" ResourceInfo: NumAllocations: %u", pData->NumAllocations);
 }
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTCreateAllocation(D3DKMT_CREATEALLOCATION* pData) {
 std::wstring header = L"D3DKMTCreateAllocation 被调用 ";
 LogMessage(header.c_str());
 LogMessage(L" hDevice:0x%llX, NumAllocations: %u", (unsigned long long)pData->hDevice, pData->NumAllocations);

 NTSTATUS result = s_OriginalCreateAllocation ? s_OriginalCreateAllocation(pData) :0;

 LogMessage(L"D3DKMTCreateAllocation 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTDestroyAllocation(const D3DKMT_DESTROYALLOCATION* pData) {
 std::wstring header = L"D3DKMTDestroyAllocation 被调用 ";
 LogMessage(header.c_str());
 // Use AllocationCount member
 LogMessage(L" hDevice:0x%llX, AllocationCount: %u", (unsigned long long)pData->hDevice, pData->AllocationCount);

 NTSTATUS result = s_OriginalDestroyAllocation ? s_OriginalDestroyAllocation(const_cast<D3DKMT_DESTROYALLOCATION*>(pData)) :0;

 LogMessage(L"D3DKMTDestroyAllocation 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTGetDeviceState(D3DKMT_GETDEVICESTATE* pData) {
 NTSTATUS result = s_OriginalGetDeviceState ? s_OriginalGetDeviceState(pData) :0;
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTSetGammaRamp(const D3DKMT_SETGAMMARAMP* pData) {
 std::wstring header = L"D3DKMTSetGammaRamp 被调用 ";
 LogMessage(header.c_str());
 // SETGAMMARAMP may have different member names; attempt to log common fields if present
 LogMessage(L"D3DKMTSetGammaRamp called");

 NTSTATUS result = s_OriginalSetGammaRamp ? s_OriginalSetGammaRamp(const_cast<D3DKMT_SETGAMMARAMP*>(pData)) :0;

 LogMessage(L"D3DKMTSetGammaRamp 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTWaitForVerticalBlankEvent(const D3DKMT_WAITFORVERTICALBLANKEVENT* pData) {
 std::wstring header = L"D3DKMTWaitForVerticalBlankEvent 被调用 ";
 LogMessage(header.c_str());
 LogMessage(L"D3DKMTWaitForVerticalBlankEvent called");

 NTSTATUS result = s_OriginalWaitForVerticalBlankEvent ? s_OriginalWaitForVerticalBlankEvent(const_cast<D3DKMT_WAITFORVERTICALBLANKEVENT*>(pData)) :0;

 LogMessage(L"D3DKMTWaitForVerticalBlankEvent 返回:0x%X", result);
 return result;
}

// ========== 原有的KMT函数钩子 ==========

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTOpenAdapterFromHdc(D3DKMT_OPENADAPTERFROMHDC* pData) {
 LogMessage(L"D3DKMTOpenAdapterFromHdc 被调用 - hDc:0x%p", pData->hDc);
 NTSTATUS result = s_OriginalOpenAdapterFromHdc ? s_OriginalOpenAdapterFromHdc(pData) :0;
 LogMessage(L"D3DKMTOpenAdapterFromHdc 返回:0x%X, hAdapter:0x%llX, AdapterLuid:0x%llX",
 result, (unsigned long long)pData->hAdapter, *(ULONGLONG*)&pData->AdapterLuid);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTOpenAdapterFromDeviceName(D3DKMT_OPENADAPTERFROMDEVICENAME* pData) {
 std::wstring header = L"D3DKMTOpenAdapterFromDeviceName 被调用 " + AnsiToWString(",pDeviceName:") + std::wstring((pData->pDeviceName));
 LogMessage(header.c_str());
 NTSTATUS result = s_OriginalOpenAdapterFromDeviceName ? s_OriginalOpenAdapterFromDeviceName(pData) :0;
 LogMessage(L"D3DKMTOpenAdapterFromDeviceName 返回:0x%X, hAdapter:0x%llX", result, (unsigned long long)pData->hAdapter);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTOpenAdapterFromGdiDisplayName(D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME* pData) {
 LogMessage(L"D3DKMTOpenAdapterFromGdiDisplayName 被调用 - DeviceName: %s", pData->DeviceName);
 NTSTATUS result = s_OriginalOpenAdapterFromGdiDisplayName ? s_OriginalOpenAdapterFromGdiDisplayName(pData) :0;
 LogMessage(L"D3DKMTOpenAdapterFromGdiDisplayName 返回:0x%X, hAdapter:0x%llX", result, (unsigned long long)pData->hAdapter);
 return result;
}

// -- Missing hook implementations added below --
//
NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTCreateDevice(D3DKMT_CREATEDEVICE* pData) {
 std::wstring header = L"D3DKMTCreateDevice 被调用 ";
 LogMessage(header.c_str());
 NTSTATUS result = s_OriginalCreateDevice ? s_OriginalCreateDevice(pData) :0;
 LogMessage(L"D3DKMTCreateDevice 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTCreateContext(D3DKMT_CREATECONTEXT* pData) {
 std::wstring header = L"D3DKMTCreateContext 被调用 ";
 LogMessage(header.c_str());
 NTSTATUS result = s_OriginalCreateContext ? s_OriginalCreateContext(pData) :0;
 LogMessage(L"D3DKMTCreateContext 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTDestroyDevice(const D3DKMT_DESTROYDEVICE* pData) {
 std::wstring header = L"D3DKMTDestroyDevice 被调用 ";
 LogMessage(header.c_str());
 NTSTATUS result = s_OriginalDestroyDevice ? s_OriginalDestroyDevice(const_cast<D3DKMT_DESTROYDEVICE*>(pData)) :0;
 LogMessage(L"D3DKMTDestroyDevice 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTDestroyContext(const D3DKMT_DESTROYCONTEXT* pData) {
 std::wstring header = L"D3DKMTDestroyContext 被调用 ";
 LogMessage(header.c_str());
 NTSTATUS result = s_OriginalDestroyContext ? s_OriginalDestroyContext(const_cast<D3DKMT_DESTROYCONTEXT*>(pData)) :0;
 LogMessage(L"D3DKMTDestroyContext 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTPresent(D3DKMT_PRESENT* pData) {
 std::wstring header = L"D3DKMTPresent 被调用 ";
 LogMessage(header.c_str());
 NTSTATUS result = s_OriginalPresent ? s_OriginalPresent(pData) :0;
 LogMessage(L"D3DKMTPresent 返回:0x%X", result);
 return result;
}

// New hook implementations
NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTSubmitCommand(D3DKMT_SUBMITCOMMAND* pData) {
 LogMessage(L"D3DKMTSubmitCommand 被调用 - CommandLength: %u", pData ? pData->CommandLength :0);
 NTSTATUS result = s_OriginalSubmitCommand ? s_OriginalSubmitCommand(pData) :0;
 LogMessage(L"D3DKMTSubmitCommand 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTCreateHwQueue(D3DKMT_CREATEHWQUEUE* pData) {
 LogMessage(L"D3DKMTCreateHwQueue 被调用");
 NTSTATUS result = s_OriginalCreateHwQueue ? s_OriginalCreateHwQueue(pData) :0;
 LogMessage(L"D3DKMTCreateHwQueue 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTDestroyHwQueue(D3DKMT_DESTROYHWQUEUE* pData) {
 LogMessage(L"D3DKMTDestroyHwQueue 被调用");
 NTSTATUS result = s_OriginalDestroyHwQueue ? s_OriginalDestroyHwQueue(pData) :0;
 LogMessage(L"D3DKMTDestroyHwQueue 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTEnumAdapters(D3DKMT_ENUMADAPTERS* pData) {
 LogMessage(L"D3DKMTEnumAdapters 被调用");
 NTSTATUS result = s_OriginalEnumAdapters ? s_OriginalEnumAdapters(pData) :0;
 LogMessage(L"D3DKMTEnumAdapters 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTLock(D3DKMT_LOCK* pData) {
 LogMessage(L"D3DKMTLock 被调用");
 NTSTATUS result = s_OriginalLock ? s_OriginalLock(pData) :0;
 LogMessage(L"D3DKMTLock 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTUnlock(D3DKMT_UNLOCK* pData) {
 LogMessage(L"D3DKMTUnlock 被调用");
 NTSTATUS result = s_OriginalUnlock ? s_OriginalUnlock(pData) :0;
 LogMessage(L"D3DKMTUnlock 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTRender(D3DKMT_RENDER* pData) {
    LogMessage(L"Hooked_D3DKMTRender 被调用 (void*). pData=%p", pData);
    NTSTATUS res = s_OriginalRender ? s_OriginalRender(pData) : 0;
    LogMessage(L"Hooked_D3DKMTRender 返回:0x%X", res);
    return res;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTFlipOverlay(D3DKMT_FLIPOVERLAY* pData) {
    LogMessage(L"D3DKMTFlipOverlay 被调用 (void*). pData=%p", pData);
    NTSTATUS res = s_OriginalFlipOverlay ? s_OriginalFlipOverlay(pData) : 0;
    LogMessage(L"D3DKMTFlipOverlay 返回:0x%X", res);
    return res;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTCreateOverlay(D3DKMT_CREATEOVERLAY* pData) {
    LogMessage(L"D3DKMTCreateOverlay 被调用 (void*). pData=%p", pData);
    NTSTATUS res = s_OriginalCreateOverlay ? s_OriginalCreateOverlay(pData) : 0;
    LogMessage(L"D3DKMTCreateOverlay 返回:0x%X", res);
    return res;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTUpdateOverlay(D3DKMT_UPDATEOVERLAY* pData) {
    LogMessage(L"D3DKMTUpdateOverlay 被调用 (void*). pData=%p", pData);
    NTSTATUS res = s_OriginalUpdateOverlay ? s_OriginalUpdateOverlay(pData) : 0;
    LogMessage(L"D3DKMTUpdateOverlay 返回:0x%X", res);
    return res;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTSetVidPnSourceOwner(D3DKMT_SETVIDPNSOURCEOWNER* pData) {
    LogMessage(L"D3DKMTSetVidPnSourceOwner 被调用 (void*). pData=%p", pData);
    NTSTATUS res = s_OriginalSetVidPnSourceOwner ? s_OriginalSetVidPnSourceOwner(pData) : 0;
    LogMessage(L"D3DKMTSetVidPnSourceOwner 返回:0x%X", res);
    return res;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTPresentMultiPlaneOverlay(D3DKMT_PRESENT_MULTIPLANE_OVERLAY* pData) {
    LogMessage(L"D3DKMTPresentMultiPlaneOverlay 被调用 (void*). pData=%p", pData);
    NTSTATUS res = s_OriginalPresentMultiPlaneOverlay ? s_OriginalPresentMultiPlaneOverlay(pData) : 0;
    LogMessage(L"D3DKMTPresentMultiPlaneOverlay 返回:0x%X", res);
    return res;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTCreateAllocation2(D3DKMT_CREATEALLOCATION* pData) {

 LogMessage(L"D3DKMTCreateAllocation2 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalCreateAllocation2 ? s_OriginalCreateAllocation2(pData) :0;
 LogMessage(L"D3DKMTCreateAllocation2 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTConnectDoorbell(D3DKMT_CONNECT_DOORBELL* pData) {

 LogMessage(L"D3DKMTConnectDoorbell 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalConnectDoorbell ? s_OriginalConnectDoorbell(pData) :0;
 LogMessage(L"D3DKMTConnectDoorbell 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTCreateDoorbell(D3DKMT_CREATE_DOORBELL* pData) {

 LogMessage(L"D3DKMTCreateDoorbell 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalCreateDoorbell ? s_OriginalCreateDoorbell(pData) :0;
 LogMessage(L"D3DKMTCreateDoorbell 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTCreateDCFromMemory(D3DKMT_CREATEDCFROMMEMORY* pData) {

 LogMessage(L"D3DKMTCreateDCFromMemory 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalCreateDCFromMemory ? s_OriginalCreateDCFromMemory(pData) :0;
 LogMessage(L"D3DKMTCreateDCFromMemory 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTCreateHwContext(D3DKMT_CREATEHWCONTEXT* pData) {

 LogMessage(L"D3DKMTCreateHwContext 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalCreateHwContext ? s_OriginalCreateHwContext(pData) :0;
 LogMessage(L"D3DKMTCreateHwContext 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTCreateNativeFence(D3DKMT_CREATENATIVEFENCE* pData) {

 LogMessage(L"D3DKMTCreateNativeFence 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalCreateNativeFence ? s_OriginalCreateNativeFence(pData) :0;
 LogMessage(L"D3DKMTCreateNativeFence 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTEnumAdapters2(D3DKMT_ENUMADAPTERS2* pData) {

 LogMessage(L"D3DKMTEnumAdapters2 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalEnumAdapters2 ? s_OriginalEnumAdapters2(pData) :0;
 LogMessage(L"D3DKMTEnumAdapters2 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTEnumAdapters3(D3DKMT_ENUMADAPTERS3* pData) {

 LogMessage(L"D3DKMTEnumAdapters3 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalEnumAdapters3 ? s_OriginalEnumAdapters3(pData) :0;
 LogMessage(L"D3DKMTEnumAdapters3 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTEscape(D3DKMT_ESCAPE* pData) {

 LogMessage(L"D3DKMTEscape 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalEscape ? s_OriginalEscape(pData) :0;
 LogMessage(L"D3DKMTEscape 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTGetDisplayModeList(D3DKMT_GETDISPLAYMODELIST* pData) {

 LogMessage(L"D3DKMTGetDisplayModeList 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalGetDisplayModeList ? s_OriginalGetDisplayModeList(pData) :0;
 LogMessage(L"D3DKMTGetDisplayModeList 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTGetPresentHistory(D3DKMT_GETPRESENTHISTORY* pData) {

 LogMessage(L"D3DKMTGetPresentHistory 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalGetPresentHistory ? s_OriginalGetPresentHistory(pData) :0;
 LogMessage(L"D3DKMTGetPresentHistory 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTLock2(D3DKMT_LOCK2* pData) {

 LogMessage(L"D3DKMTLock2 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalLock2 ? s_OriginalLock2(pData) :0;
 LogMessage(L"D3DKMTLock2 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTMapGpuVirtualAddress(D3DDDI_MAPGPUVIRTUALADDRESS* pData) {

 LogMessage(L"D3DKMTMapGpuVirtualAddress 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalMapGpuVirtualAddress ? s_OriginalMapGpuVirtualAddress(pData) :0;
 LogMessage(L"D3DKMTMapGpuVirtualAddress 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTOfferAllocations(D3DKMT_OFFERALLOCATIONS* pData) {

 LogMessage(L"D3DKMTOfferAllocations 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalOfferAllocations ? s_OriginalOfferAllocations(pData) :0;
 LogMessage(L"D3DKMTOfferAllocations 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTOpenResource2(D3DKMT_OPENRESOURCE* pData) {

 LogMessage(L"D3DKMTOpenResource2 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalOpenResource2 ? s_OriginalOpenResource2(pData) :0;
 LogMessage(L"D3DKMTOpenResource2 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTPresentMultiPlaneOverlay2(D3DKMT_PRESENT_MULTIPLANE_OVERLAY2* pData) {

 LogMessage(L"D3DKMTPresentMultiPlaneOverlay2 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalPresentMultiPlaneOverlay2 ? s_OriginalPresentMultiPlaneOverlay2(pData) :0;
 LogMessage(L"D3DKMTPresentMultiPlaneOverlay2 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTPresentMultiPlaneOverlay3(D3DKMT_PRESENT_MULTIPLANE_OVERLAY3* pData) {

 LogMessage(L"D3DKMTPresentMultiPlaneOverlay3 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalPresentMultiPlaneOverlay3 ? s_OriginalPresentMultiPlaneOverlay3(pData) :0;
 LogMessage(L"D3DKMTPresentMultiPlaneOverlay3 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTPresentRedirected(D3DKMT_PRESENT_REDIRECTED* pData) {

 LogMessage(L"D3DKMTPresentRedirected 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalPresentRedirected ? s_OriginalPresentRedirected(pData) :0;
 LogMessage(L"D3DKMTPresentRedirected 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTSetDisplayMode(D3DKMT_SETDISPLAYMODE* pData) {

 LogMessage(L"D3DKMTSetDisplayMode 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalSetDisplayMode ? s_OriginalSetDisplayMode(pData) :0;
 LogMessage(L"D3DKMTSetDisplayMode 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTSubmitCommandToHwQueue(D3DKMT_SUBMITCOMMANDTOHWQUEUE* pData) {

 LogMessage(L"D3DKMTSubmitCommandToHwQueue 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalSubmitCommandToHwQueue ? s_OriginalSubmitCommandToHwQueue(pData) :0;
 LogMessage(L"D3DKMTSubmitCommandToHwQueue 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTSubmitPresentBltToHwQueue(D3DKMT_SUBMITPRESENTBLTTOHWQUEUE* pData) {

 LogMessage(L"D3DKMTSubmitPresentBltToHwQueue 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalSubmitPresentBltToHwQueue ? s_OriginalSubmitPresentBltToHwQueue(pData) :0;
 LogMessage(L"D3DKMTSubmitPresentBltToHwQueue 返回:0x%X", result);
 return result;
}

NTSTATUS WINAPI KMTInterceptor::Hooked_D3DKMTSubmitPresentToHwQueue(D3DKMT_SUBMITPRESENTTOHWQUEUE* pData) {

 LogMessage(L"D3DKMTSubmitPresentToHwQueue 被调用 - pData=%p", pData);
 NTSTATUS result = s_OriginalSubmitPresentToHwQueue ? s_OriginalSubmitPresentToHwQueue(pData) :0;
 LogMessage(L"D3DKMTSubmitPresentToHwQueue 返回:0x%X", result);
 return result;
}
