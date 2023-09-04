/******************************************************************************
*	device/hid/keyboard.c
*	 by Alex Chadwick
*
*	A light weight implementation of the USB protocol stack fit for a simple
*	driver.
*
*	device/hid/keyboard.c contains code relating to USB hid keyboards. The 
*	driver maintains a list of the keyboards on the system, and allows the 
*	operating system to retrieve the status of each one separately. It is coded
*	a little awkwardly on purpose to make OS development more fun!
******************************************************************************/
#include <device/hid/hid.h>
#include <device/hid/touch.h>
#include <device/hid/report.h>
#include <platform/platform.h>
#include <types.h>
#include <string.h>
#include <usbd/device.h>
#include <usbd/usbd.h>

struct UsbDevice* touchDev = NULL;

void TouchLoad() 
{
	LOG_DEBUG("CSUD: Touch driver version 0.1\n"); 
	HidUsageAttach[4] = TouchAttach;
}

void TouchDetached(struct UsbDevice *device) {
	struct TouchDevice *data;
	touchDev = NULL;
}

void TouchDeallocate(struct UsbDevice *device) {
	struct TouchDevice *data;
	data = (struct KeyboardDevice*)((struct HidDevice*)device->DriverData)->DriverData;
	if (data != NULL) {
		MemoryDeallocate(data);
		((struct HidDevice*)device->DriverData)->DriverData = NULL;
	}
	((struct HidDevice*)device->DriverData)->HidDeallocate = NULL;
	((struct HidDevice*)device->DriverData)->HidDetached = NULL;
}

Result TouchAttach(struct UsbDevice *device, u32 interface) {
	u32 keyboardNumber;
	struct HidDevice *hidData;
	struct HidParserResult *parse;

	hidData = (struct HidDevice*)device->DriverData;
	if (hidData->Header.DeviceDriver != DeviceDriverHid) {
		LOGF("TOUCH: %s isn't a HID device. The touch driver is built upon the HID driver.\n", UsbGetDescription(device));
		return ErrorIncompatible;
	}

	parse = hidData->ParserResult;
	if ((parse->Application.Page != Digitlizer && parse->Application.Page != Undefined) ||
		parse->Application.Digitlizer != 4) {
		LOGF("TOUCH: %s doesn't seem to be a touch (%x != %x || %x != %x)...\n", UsbGetDescription(device), parse->Application.Page, Digitlizer, parse->Application.Digitlizer, 4);
		return ErrorIncompatible;
	}
	if (parse->ReportCount < 1) {
		LOGF("TOUCH: %s doesn't have enough outputs to be a touch.\n", UsbGetDescription(device));
		return ErrorIncompatible;
	}
	hidData->HidDetached = TouchDetached;
	hidData->HidDeallocate = TouchDeallocate;
	if ((hidData->DriverData = MemoryAllocate(sizeof(struct TouchDevice))) == NULL) {
		LOGF("TOUCH: Not enough memory to allocate touch %s.\n", UsbGetDescription(device));
		return ErrorMemory;
	}
	touchDev = device;
	LOG_DEBUGF("TOUCH: New Touch assigned %d!\n", device->Number);

	return OK;
}


Result TouchPoll(u32 keyboardAddress) {

	return OK;
}

static struct TouchEvent _event = {0};
Result TouchGetEvent(struct TouchEvent* event){
	u8 buffer[128] = {0};
	Result ret;

	if(touchDev == NULL)
		return ErrorDevice;

	if(touchDev->Descriptor.ProductId == 0xe2e4){
		ret = HidReadDeviceRaw(touchDev, 2, 0, buffer);
		if(ret == 0){
			_event.event = !!(buffer[1]&0x01);
			int a = buffer[3];
			int b = buffer[4];
			int c = b*256 + a;
			_event.x = c;
			a = buffer[5];
			b = buffer[6];
			c = b*256 + a;
			_event.y = c;
			memcpy(event, &_event, sizeof(struct TouchEvent));
			return OK;
		}else if(_event.event){
			_event.event = 0;
			memcpy(event, &_event, sizeof(struct TouchEvent));
			return OK;
		}
	}else if (touchDev->Descriptor.ProductId == 0x9){
		ret = HidReadDeviceRaw(touchDev, 1, 4, buffer);
		if(ret == 0){
			_event.event = !!(buffer[1]&0x40);
			int a = buffer[2];
			int b = buffer[3];
			int c = b*256 + a;
			_event.x = c;
			a = buffer[4];
			b = buffer[5];
			c = b*256 + a;
			_event.y = c;
			memcpy(event, &_event, sizeof(struct TouchEvent));
			return OK;
		}else if(_event.event){
			_event.event = 0;
			memcpy(event, &_event, sizeof(struct TouchEvent));
			return OK;
		}
	}else if (touchDev->Descriptor.ProductId == 0xa){
		ret = HidReadDeviceRaw(touchDev, 2, 1, buffer);
		if(ret == 0){
			_event.event = !!(buffer[1]&0x40);
			int a = buffer[2];
			int b = buffer[3];
			int c = b*256 + a;
			_event.x = c;
			a = buffer[4];
			b = buffer[5];
			c = b*256 + a;
			_event.y = c;
			memcpy(event, &_event, sizeof(struct TouchEvent));
			return OK;
		}else if(_event.event){
			_event.event = 0;
			memcpy(event, &_event, sizeof(struct TouchEvent));
			return OK;
		}
	}else{
		//Unknow device
		return ErrorDevice;
	}
	return ret;
}


bool TouchPersent(){
	return !!touchDev;
}
