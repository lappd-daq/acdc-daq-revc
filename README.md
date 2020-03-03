# acdc-daq-revc
This code is a re-write of Eric Oberla's thesis Central Card/ACDC control code. It has an emphasis on error catching due to the somewhat inconsistent nature of the ACDC/ACC firmware. It is meant to be used with ACC/ACDC firmware after the uart-comms modification. 

## Prerequisites
This code is built using `cmake` and requires `libusb` headers (which usually come prepackaged with your os-- you may want to double-check).

To install on a debian-based machine

```bash
$ sudo apt install libusb-dev cmake
```

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
1. (optional) filename - the filename of the config file
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

### calEn

### dumpData

### ledEn

### makeLUT

### Reset

### resetDLL

### setPed

### setupLVDS

### toggle_led

### Metadata Descriptions
Please find a description of each metadata key in the Metadata.cpp::parseBuffer function. This will also point to locations in firmware where metadata words are clarified. 