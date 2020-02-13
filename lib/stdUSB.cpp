/////////////////////////////////////////////////////
//  file: stdUSBl.cxx
//
//  A libusb implementation of ezusbsys--
//  StdUSB libusb implementation used here uses same 
//  function interface with native stdUSB Windows WDM 
//  driver.
// 
//////////////////////////////////////////////////////

#include <string>
#include <stdio.h>
#include "stdUSB.h"
#include <iostream>
#include <sstream>
#include <bitset>

using namespace std;

stdUSB::stdUSB() 
{
    stdHandle = INVALID_HANDLE_VALUE; 
    USBFX2_VENDOR_ID = 0x6672;
    USBFX2_PRODUCT_ID = 0x2920;
    bool retval;
    retval = createHandles();
    if(!retval)
    {
        cout << "Usb was unable to connect to USB line, exiting" << endl;
        exit(EXIT_FAILURE);
    }
}

stdUSB::stdUSB(uint16_t vid, uint16_t pid) 
{
    stdHandle = INVALID_HANDLE_VALUE; 
    USBFX2_VENDOR_ID = vid;
    USBFX2_PRODUCT_ID = pid;
    bool retval;
    retval = createHandles();
    if(!retval)
    {
        cout << "Usb was unable to connect to USB line, exiting" << endl;
        exit(EXIT_FAILURE);
    }
}

stdUSB::~stdUSB() { 
  if (stdHandle != INVALID_HANDLE_VALUE){
    freeHandle();
  }
}


/**
 * Finds USBFX2 device, opens it, sets configuration, and claims interface.
 *  Using of goto is mad bad. Check out linux kernel sources.
 * @param  
 * @return bool 
 */
//device_count represents the index of the 
//device with VID/PID that matches. If there 
//are two such devices, and you want to connect to
//the second, device_count = 2
bool stdUSB::createHandles(int device_count) {
    if (stdHandle != INVALID_HANDLE_VALUE) {
        //do nothing, it is open
        return true;
    }

    int retval;
    struct usb_device *dev = init(device_count);

    if(dev == nullptr)
    {
        return false;
    }


    stdHandle = usb_open(dev);

    if (stdHandle == INVALID_HANDLE_VALUE) {
        cout << "Failed to open device. handle=" << stdHandle << endl;
        return false;
    }
    
    retval = usb_set_configuration(stdHandle, CNFNO);

    if (retval != 0) {
        cout << "Failed to set USB Configuration " << CNFNO << ". Return value: " << retval << endl;
        return false;
    }        
    
    retval = usb_claim_interface(stdHandle, INTFNO);

    if (retval != 0) {
        cout << "Failed to claim USB interface " << INTFNO << ". Return value: " << retval << endl;
        return false;
    }

    retval = usb_set_altinterface(stdHandle, INTFNO);

    if (retval != 0) {
        cout << "Failed to set alt USB interface " << INTFNO << ". Return value: " << retval << endl;
        return false;
    }


    //cout << "Handle created successfully" << endl;

    //when the handle is created, the ACC
    //needs to be read once to "initialize". 
    //this is strange behavior that I think is
    //due to poor writing of the ACC usb firmware. 
    int samples; //number of bytes actually read
    unsigned short* buffer;
    buffer = (unsigned short*)calloc(1, sizeof(unsigned short));

    readData(buffer, 1, &samples);    
    free(buffer);

    return true;
}

/**
 * Internal function.
 *  Initialises libusb and finds the correct usb device.
 * @param  
 * @return usb_device* -- A pointer to USBFX2 libusb dev entry, 
 * or INVALID_HANDLE_VALUE on failure.
 */
//device_count represents the index of the 
//device with VID/PID that matches. If there 
//are two such devices, and you want to connect to
//the second, device_count = 2
struct usb_device* stdUSB::init(int device_count) { //init is a function that returns a structure called usb_device. 
    struct usb_bus *usb_bus;
    struct usb_device *dev;

    

    /* init libusb*/
    usb_init();
    usb_find_busses();
    usb_find_devices();

    /* usb_busses is a linked list which after above init 
       function calls contains every single usb device in the computer.
        We need to browse that linked list and find EZ USB-FX2 by 
	VENDOR and PRODUCT ID 
    */
    int count = 0;
    for (usb_bus = usb_busses; usb_bus; usb_bus = usb_bus->next) 
    {
        for (dev = usb_bus->devices; dev; dev = dev->next) 
        {
            if ((dev->descriptor.idVendor == USBFX2_VENDOR_ID) && (dev->descriptor.idProduct == USBFX2_PRODUCT_ID)) 
            {
                count++;
                if (count == device_count)
                {
                    return dev;
                } 
            }
        }
    }

    //if it makes it out of loop, didnt find 
    cout << "Could not find " << device_count << " usb devices with VID:PID " << USBFX2_VENDOR_ID << ":" << USBFX2_PRODUCT_ID << endl;
    cout << "Possibly adjust the number " << device_count << endl;
    return nullptr;

}
/**
 * Frees handles and resources allcated by createHandle().
 * @param  
 * @return bool -- SUCCEED or FAILED
 */
bool stdUSB::freeHandle(void) //throw(...)
{
    usb_release_interface(stdHandle, INTFNO);

    /* close usb handle */
    //retval = usb_reset(stdHandle); 
    usb_close(stdHandle);

    return true;
}

void stdUSB::printByte(unsigned int val)
{
    cout << val << ", "; //decimal
    stringstream ss;
    ss << std::hex << val;
    string hexstr(ss.str());
    cout << hexstr << ", "; //hex
    unsigned n;
    ss >> n;
    bitset<32> b(n);
    cout << b.to_string(); //binary
}

/**
 * Frees handles and resources allcated by createHandle().
 * @param data -- command, control code, value, or such 
 * to be sent to the device
 * @return bool -- SUCCEED or FAILED
 */
bool stdUSB::sendData(unsigned int data)// throw(...)
{
    if (stdHandle == INVALID_HANDLE_VALUE) return false;

    /* Shifted right because send value needs to be in 
       HEX base. char[4] ^= int (char -> 1byte, int -> 4 bytes)
    */
    char buff[4];
    buff[0] = data;
    buff[1] = data >> 8;
    buff[2] = data >> 16;
    buff[3] = data >> 24;
   
    
    cout << "Data is: ";
    printByte(data);
    cout << endl;
    

    int retval = usb_bulk_write(stdHandle, 0x02, buff, sizeof(buff), USB_TOUT_MS);

    //Evan using sleep statements to give the USB some time. 
    //it is likely that usb_bulk_write includes this time factor
    //but I am having problems so I am slowing things down. 
    //4bytes*8bits per byte/48Mbits per sec x 2 for good luck
    usleep(2*4*8.0/(48.0));

    //printf("SendData: sent %d bytes\n", retval);

    if (retval == 4){ //return value must be exact as the bytes transferred
      //printf("sending data to board...\n");  
      return true;
    }
    else{
      cout << "Bytes sent were not equal to the bytes transferred on usb write, equal to " << retval << endl;
      return false;
    }
}

/**
 * Performs bulk transfer to IN end point. Reads data off the board.
 * @param pData -- pointer to input buffer data array
 * @param l -- size of the data buffer (think: pData[l] )
 * @param lread -- out param: number of samples read
 * @return bool -- SUCCEED or FAILED
 */
bool stdUSB::readData(unsigned short * pData, int l, int* lread)// throw(...)
{
    // no handle = don't read out any data
    if (stdHandle == INVALID_HANDLE_VALUE) {
      *lread = 0;
      cout << "Could not read usb, invalid handle" << endl;
      return false;
    }

    int buff_sz = l*sizeof(unsigned short);

    //Evan using sleep statements to give the USB some time. 
    //it is likely that usb_bulk_read includes this time factor
    //but I am having problems so I am slowing things down. 
    //l-packets*4bytes per packet*8bits per byte/48Mbits per sec = ~6ms x2 for good luck
    usleep(2*l*4.0*8.0/(48.0)); 

    //cout << "Read buffer maximum size is " << buff_sz << endl;
    int retval = usb_bulk_read(stdHandle, 0x86, (char*)pData, buff_sz, USB_TOUT_MS);
    //cout << "Got " << retval << " bytes" << endl;




    if (retval > 0) {
        *lread = (int)(retval / (unsigned int)sizeof(unsigned short));
        return true;
    } 
    else{
        *lread = retval;
        //cout << "usb read failed with retval " << retval << endl;
        return false;
    }

    return false;
}

//this is a function Evan wrote to properly allocate
//and delete memory for the readData function that was
//written in C back in 2000 something. 
vector<unsigned short> stdUSB::safeReadData(int maxSamples)
{
    int samples; //number of bytes actually read
    unsigned short* buffer; //this type required by usb driver
    buffer = (unsigned short*)calloc(maxSamples + 2, sizeof(unsigned short)); //safe allocation
    readData(buffer, maxSamples + 2, &samples); //read up to maxSamples

    //fill buffer into a vector
    vector<unsigned short> v_buffer;
    cout << "Got " << samples << " samples from usbread" << endl;
    //loop over each element in buffer
    for(int i = 0; i < samples; i++)
    {
        v_buffer.push_back(buffer[i]);
    }

    free(buffer); //free the calloc'ed memory
    return v_buffer;
}

bool stdUSB::isOpen() {
	return (stdHandle != INVALID_HANDLE_VALUE);
}

bool stdUSB::reset(){
  int retval = usb_reset(stdHandle);

  if(retval == 0)
    return true;

  else
    return false;
}
