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
#include <thread>
#include <chrono>
#include <sstream>
#include <bitset>

using namespace std;

stdUSB::stdUSB() {
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

    //clear the line just in case
    //clear the USB line. 
    vector<unsigned short> buf;
    while(buf.size() > 0)
    {
        safeReadData(20000);
    }
}

stdUSB::stdUSB(uint16_t vid, uint16_t pid) {
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


    libusb_device *dev = init(device_count);
    libusb_context *usb_context = NULL;

    if(dev == nullptr)
    {
        return false;
    }

    stdHandle = libusb_open_device_with_vid_pid(usb_context,USBFX2_VENDOR_ID,USBFX2_PRODUCT_ID);

    if (stdHandle == INVALID_HANDLE_VALUE) {
        cout << "Failed to open device. handle=" << stdHandle << endl;
        return false;
    } 

    retval = libusb_set_configuration(stdHandle, CNFNO);

    if (retval != 0) {
        cout << "Failed to set USB Configuration " << CNFNO << ". Return value: " << retval << endl;
        return false;
    }       
    
    retval = libusb_claim_interface(stdHandle, INTFNO);

    if (retval != 0) {
        cout << "Failed to claim USB interface " << INTFNO << ". Return value: " << retval << endl;
        return false;
    }

    libusb_clear_halt(stdHandle,EP_WRITE);
    libusb_clear_halt(stdHandle,EP_READ);


    return true;
}

//New trial version of stdUSB::init()
struct libusb_device* stdUSB::init(int device_count){
    libusb_context *usb_context = NULL;
    libusb_device **list = NULL;
    ssize_t count = 0;

    libusb_init(&usb_context);

    count = libusb_get_device_list(usb_context, &list);

    for (size_t idx = 0; idx < (size_t)count; ++idx){
        libusb_device *dev = list[idx];
        libusb_device_descriptor descriptor;

        libusb_get_device_descriptor(dev, &descriptor);

        if ((descriptor.idVendor == USBFX2_VENDOR_ID) && (descriptor.idProduct == USBFX2_PRODUCT_ID)) {
            return dev;
        }
    }
    cout << "Could not find " << device_count << " usb devices with VID:PID " << USBFX2_VENDOR_ID << ":" << USBFX2_PRODUCT_ID << endl;
    cout << "Possibly adjust the number " << device_count << endl;
    return nullptr;
}


/**
 * Frees handles and resources allcated by createHandle().
 * @param  
 * @return bool -- SUCCEED or FAILED
 */
bool stdUSB::freeHandle(void) { //throw(...)
    libusb_release_interface(stdHandle, INTFNO);

    /* close usb handle */
    //retval = usb_reset(stdHandle); 
    libusb_close(stdHandle);

    return true;
}

void stdUSB::printByte(unsigned int val) {
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
bool stdUSB::sendData(unsigned int data) { // throw(...)

    if (stdHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    /* Shifted right because send value needs to be in 
       HEX base. char[4] ^= int (char -> 1byte, int -> 4 bytes)
    */
    char buff[4];
    buff[0] = data;
    buff[1] = data >> 8;
    buff[2] = data >> 16;
    buff[3] = data >> 24;
   
    
    cout << "Data sending is: ";
    printByte(data);
    cout << endl;
    
    
    int transferred;

    int retval = libusb_bulk_transfer(stdHandle, 0x02, (unsigned char*)buff, sizeof(buff), &transferred, USB_TOUT_MS);


    if (retval == 0 && transferred == 4){ //return value must be exact as the bytes transferred
      //printf("sending data to board...\n");  
      return true;
    }
    else if(retval != 0 && transferred == 4){
      cout << "Got a strange error where 4 bytes were writtenb to usb, but non-zero error code: " << retval << endl;
      return false;
    }
    else{
        cout << "USB write retval is " << retval << " and bytes transferred is " << transferred << endl;
        return false;
    }
}

/**
 * Performs bulk transfer to IN end point. Reads data off the board.
 * @param pData -- pointer to input buffer data array
 * @param lread -- out param: number of samples read
 * @return bool -- SUCCEED or FAILED
 */
int stdUSB::readData(unsigned char * pData, int* lread) { // throw(...)
    // no handle = don't read out any data
    if (stdHandle == INVALID_HANDLE_VALUE) {
      *lread = 0;
      cout << "Could not read usb, invalid handle" << endl;
      return false;
    }

    //maximum for libusb_bulk_transfer. this is the reason for the safeReadData
    //to triage read-calls in the case of expecting, say 8000 bytes. 
    int buff_sz = 512; //number of unsigned chars, which are sizeof = 1


    //Evan using std::this_thread::sleep_for statements to give the USB some time. 
    //I have measured time of usb_bulk_read and it does not last
    //long enough assuming 48Mbit/s data transfer. Maybe it thinks
    //this is a super fast line? But the usb firmware is clocked at
    //48Mbit per sec. 
    //l-packets*4bytes per packet*8bits per byte/48Mbits per sec = ~0.6ms - 6ms depending on which. 
    //int waitTime = buff_sz*3*4*8/48; //microseconds
    //std::this_thread::sleep_for(std::chrono::microseconds(waitTime)); 
    int retval = libusb_bulk_transfer(stdHandle, 0x86, pData, buff_sz, lread, USB_TOUT_MS);
    //std::this_thread::sleep_for(std::chrono::microseconds(waitTime));

    if (retval == 0) {
        return retval;
    } 
    else{
        //if it isnt a timeout flag, which happens often if you ask for more bytes. 
        if(retval != -7)
        {
            cout << "USB read error with retval : " << retval << " and transferred bytes " << *lread << endl;    
        }
        return retval;
    }

    return 1;
}

//this function handles the reading of the USB rx line in 
//the case of many expected words. If there are more than 512
//bytes expected, then this function reads the line in
//512 byte chunks until no more bytes are found. 
//see https://community.cypress.com/thread/44094?start=0&tstart=0
// and https://www.cs.unm.edu/~hjelmn/libusb_hotplug_api/group__syncio.html

//maxSamples it the number of 2 byte words, i.e. 0x0000. ACC
//sends 2 byte words as ACDC data and other info data. But the
//usb->readData or libusb_bulk_transfer reads 1 byte characters. 
//this is the reason for a few factors of 2 here and there. libusb_bulk_transfer
//reads max of 512 1 byte characters. 
vector<unsigned short> stdUSB::safeReadData(int maxSamples) {
    int samples; //number of bytes actually read
    
    vector<unsigned char> v_buffer;
    int retval;

    int numChars = (int)(2*maxSamples); //number of 1 byte unsigned characters. 
    unsigned char* buffer; //this type required by usb driver
    buffer = (unsigned char*)calloc(512, sizeof(unsigned char)); //safe allocation
    if(numChars > 512)
    {
        //fill buffer into a vector, stringing
        //chunks together if greater than 512 bytes requested.
        int charsRead = 0;
        while(charsRead < numChars)
        {
            retval = readData(buffer, &samples); 
            //loop over each element in buffer
            for(int i = 0; i < samples; i++)
            {
                v_buffer.push_back(buffer[i]);
            }
            //if the read times out, then end...
            if(retval == -7)
            {
                break;
            }
            charsRead += samples;
            //cout << "Got " << samples << " samples from usbread" << endl;
            //cout << "Total chars read is " << charsRead << " of " << numChars << " requested " << endl;
        }
    }
    else
    {
        retval = readData(buffer, &samples); //read up to maxSamples
        //loop over each element in buffer
        for(int i = 0; i < samples; i++)
        {
            v_buffer.push_back(buffer[i]);
        }
        //cout << "Got " << samples << " samples from usbread which is the total" << endl;
    }

    free(buffer); //free the calloc'ed memory

    //if we received no bytes, return
    if(v_buffer.size() == 0)
    {
        //empty vector
        vector<unsigned short> empty;
        return empty;
    }

    vector<unsigned short> v_buffer_short;
    unsigned char lastByte = v_buffer.at(0);
    unsigned char thisByte;
    for(int i = 0; i < (int)v_buffer.size(); i++)
    {
        thisByte = v_buffer.at(i);
        if((i + 1) % 2 == 0)
        {
            v_buffer_short.push_back((unsigned short)lastByte + ((unsigned short)thisByte << 8));
        }
        lastByte = thisByte;
    }


    return v_buffer_short;
}

bool stdUSB::isOpen() {
    return (stdHandle != INVALID_HANDLE_VALUE);
}

bool stdUSB::reset(){
  int retval = libusb_reset_device(stdHandle);

  if(retval == 0)
    return true;

  else
    return false;
}
