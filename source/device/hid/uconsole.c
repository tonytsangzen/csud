//uconsole multi function device
#include <device/hid/hid.h>
#include <device/hid/report.h>
#include <platform/platform.h>
#include <types.h>
#include <usbd/device.h>
#include <usbd/usbd.h>

static struct UsbDevice* uconsoleDev = NULL;

Result uConsoleAttach(struct UsbDevice *device, u32 interface) {
    if(device->Descriptor.ProductId == 0x24 && device->Descriptor.VendorId == 0x1EAF){
   	    LOG_DEBUG("uConsole Attach\n");  
        uconsoleDev = device;
        return OK;
    }
    return ErrorDevice;
}

void uConsoleLoad() 
{
	LOG_DEBUG("CSUD: uConsole MFD driver version 0.1\n"); 
	HidUsageAttach[DesktopMouse] = uConsoleAttach;
}

bool uConsolePersent(){
	return !!uconsoleDev;
}



Result uConsolePoll(u32 keyboardAddress) {

	return OK;
}


Result uConsoleGetEvent(u8* event){
	Result ret;

	if(uconsoleDev == NULL)
		return ErrorDevice;

	ret = HidReadDeviceRaw(uconsoleDev, 1, 1, event);

	return ret;
}
