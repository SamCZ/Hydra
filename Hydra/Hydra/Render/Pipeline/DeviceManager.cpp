#include "DeviceManager.h"

#ifdef OPERATING_SYSTEM_WINDOWS
#include "Hydra/Render/Pipeline/Windows/DX11/DeviceManager11.h"
#include "Hydra/Render/Pipeline/Windows/DX11/GFSDK_NVRHI_D3D11.h"

void signalError(const char* file, int line, const char* errorDesc)
{
	char buffer[4096];
	int length = (int)strlen(errorDesc);
	length = std::min<int>(length, 4000); // avoid a "buffer too small" exception for really long error messages
	sprintf_s(buffer, "%s:%i\n%.*s", file, line, length, errorDesc);

	OutputDebugStringA(buffer);
	OutputDebugStringA("\n");
	MessageBoxA(NULL, buffer, "ERROR", MB_ICONERROR | MB_OK);
}
#define CHECK_ERROR(expr, msg) if (!(expr)) signalError(__FILE__, __LINE__, msg)

class RendererErrorCallback : public NVRHI::IErrorCallback
{
	void signalError(const char* file, int line, const char* errorDesc)
	{
		char buffer[4096];
		int length = (int)strlen(errorDesc);
		length = std::min<int>(length, 4000); // avoid a "buffer too small" exception for really long error messages
		sprintf_s(buffer, "%s:%i\n%.*s", file, line, length, errorDesc);

		OutputDebugStringA(buffer);
		OutputDebugStringA("\n");
		MessageBoxA(NULL, buffer, "ERROR", MB_ICONERROR | MB_OK);
	}
};
RendererErrorCallback g_ErrorCallback;

#endif

void DeviceManager::InitContext()
{

}

DeviceManager * DeviceManager::CreateDeviceManagerForPlatform()
{
#ifdef OPERATING_SYSTEM_WINDOWS
	return new DeviceManagerDX11();
#endif
	return nullptr;
}

NVRHI::IRendererInterface* DeviceManager::CreateRenderInterfaceForPlatform(DeviceManager* deviceManager)
{
#ifdef OPERATING_SYSTEM_WINDOWS
	DeviceManagerDX11* dvdx11 = static_cast<DeviceManagerDX11*>(deviceManager);

	return new NVRHI::RendererInterfaceD3D11(&g_ErrorCallback, dvdx11->GetImmediateContext());
#endif
	return nullptr;
}