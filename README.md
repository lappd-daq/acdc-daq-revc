# acdc-daq-revc
This code is a re-write of Eric Oberla's thesis Central Card/ACDC control code. It has an emphasis on error catching due to the somewhat inconsistent nature of the ACDC/ACC firmware. It is meant to be used with ACC/ACDC firmware after the uart-comms modification. 

## Prerequisites
This code is built using `cmake` and requires `libusb` headers (which usually come prepackaged with your os-- you may want to double-check).

To install on a debian-based machine

```bash
$ sudo apt install libusb-dev cmake
```

gcc/g++ version optimally is >7.1 
cmake version is >3.1

## Usage

To make and use the functions, issue the following commands
```bash
$ cmake . -Bbuild
$ cmake --build build -- -j4
```

Where `-j4` specifies the number of cores available to multi-thread the process. This will create a directory `build/` 
with the proper build files and then use those to compile and link the libraries and executables.

After this any command can be issued, for example

```bash
$ ./bin/readACDC
```

If you ever need to remake without having changed to CMakeLists file, issue
```bash
$ cmake --build build -- -j4
```
again.

## Most common usage for taking waveform data

Boot boards in the following order for most successful alignment:
ACC first, wait 10 seconds or so, then ACDCs. If you have a USB line
connected to the ACC upon boot, you will see the last LED (green) go
off when the board is done initializing. 

It is good practice to check that ACDC boards are connected
and that the ACC usb line is connected. This command works well for that:
```bash
$ ./bin/readACDC
```

If you have an issue, often a reboot of the boards is a good idea. 

If you have not yet used the ACDC boards in the configuration as connected
to the ACC, a linearity calibration should be performed as well. This measures
the conversion from ADC counts to mV by setting pedestal DACs over the full range
of the PSEC4 ADCs, and measuring the ADC counts at each DAC setting. A file is saved
indexed by the ACC board index, and is referenced during data logging. This calibration
takes a long time, so you may want to back-up calibration files an associate them with
ACDC numbers instead of ACC indices (in case you change the order of the RJ45 cables). 

```bash
$ ./bin/calibrateLinearity [board mask = 11111111]
```

For any given dataset, the ACDC boards need to be configured with
their trigger, threshold, pedestal (dc offset), and other settings. 
Edit a yaml type configuration file (sometimes found in config/ folder)
and then run the command

```bash
$ ./bin/setConfig <path to config file>.yaml [-v]
```

This sends data to the ACDCs to tell them to set their DACs and other
firmware flags to prep it for the desired data run. 

The configuration files always tell the pedestal DACs to set new values. 
Because the DACs and PSEC4 chips have manufacturing variations, they
need to be calibrated to acheive noise levels around 1mV rms. For this, run
the calibration script:

```bash
$ ./bin/calibratePed
```

This routine toggles a switch on the board that connects to a calibration
input (open circuit to the signal input samtec), takes 100 software triggers,
averages the measured voltages on each sample of each channel of each board, 
and records those averages to a calibration file. That calibration file
is referenced in each data logging run and is used to subtract pedestal values
during the data logging. 

Now you are ready for data logging. See the logData description, and descriptions
of other commands below. 

## Commands

There are executables available after building the project. Below is a list of them

### readACDC
This will read out the settings of any connected ADC boards. This is useful for checking the status of each ADC board
```bash
$ ./bin/readACDC
```

### setConfig
This configures the PSEC electronics given a yaml based configuration file. 
See the `config/` folder examples for more info.

```bash
$ ./bin/setConfig [<filename>] [-v] 
```

**Parameters**

If no parameters set, will set the default (trig off) settings 

1. (optional) filename - the filename of the config file with its yaml tag and relative path. 
2. (optional) verbose flag - if provided, will show verbose output

### logData
This will log data from the PSEC electronics. For more information, see the data heading
```bash
$ ./bin/logData <file> <num_events> <trigger_mode>
```
**Parameters**
1. file - The prefix of files. Two files will be created `<file>.acdc` and `<file>.meta`
2. num_events - The number of events to capture
3. trigger_mode - 0 for software trigger, 1 for hardware trigger

If in hardware trigger mode, the boards will look to the configuration that was set by
setConfig to decide whether to use self-trigger, self-trig with system-coincidence, SMA
trigger, etc. 

### calEnable
The ACDC boards have traces routed and a splitter and switch such that, 
if a switch is toggled, the inputs of the signal lines are connected to 
a common trace routed to an on-board SMA jack. There are a number of switches, 
routed such that a single switch toggles 2 channels to be on this common calibration line. 

```bash
$ ./bin/calEnable <on/off> [<board mask>] [<channel mask>]
```
**Parameters**
1. on/off - 1 or 0 interpreted as ints. 1 means that the lines are connected to the calibration line (disconnected from samtec)
2. board mask - binary format, example 00001011 toggles boards 0, 1, and 3 to the specified calibraiton mode. 
3. channel mask - hex format, example 0x0001 connects channels 1 and 2 to cal line, 0xFFFF connects all channels to cal line. 

### ledEnable
to be written

### calibrateLinearity
to be written

### updateLinkStatus
Tells the ACC to check which ACDCs are currently plugged in. It updates its own info buffer to indicate which ACDCs are connected and then sends back a few data packets. The ACC::updateLinkStatus then clears the buffer of those USB packets.

### Metadata Descriptions
Please find a description of each metadata key in the Metadata.cpp::parseBuffer function. This will also point to locations in firmware where metadata words are clarified. 