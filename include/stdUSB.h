#ifndef _STDUSB_H_INCLUDED
#define _STDUSB_H_INCLUDED

/*****

stdUSBL - A libusb implementation of ezusbsys.

StdUSB libusb implementation used here uses same function interface with native stdUSB Windows WDM driver.

*****/

#include <usb.h>

#define INVALID_HANDLE_VALUE NULL
#define USB_TOUT_MS 50 // in ms
#define EP_WRITE 0x02 //USBFX2 end point address for bulk write
#define EP_READ 0x86 //USBFX2 end point address for bulk read
#define INTFNO 0 //USBFX2 interface number
#define CNFNO 1 //USBFX2 configuration number

class stdUSB {
public:
    stdUSB();
    stdUSB(uint16_t vid, uint16_t pid);

    ~stdUSB();


    bool createHandles(int device_count = 1);

    bool freeHandle();

    //bool sendData(unsigned short data);
    bool sendData(unsigned int data);

    bool readData(unsigned short *pData, int l, int *lread);

    void writeAndReadNothing();

    bool isOpen();

    bool reset();

    void printByte(unsigned int val);

private:
    struct usb_device* init(int device_count);

    /* USBFX2 device descriptions */
    uint16_t USBFX2_VENDOR_ID; //0x090c;
    uint16_t USBFX2_PRODUCT_ID; //0x1000;

    struct usb_dev_handle *stdHandle;
};

#endif
