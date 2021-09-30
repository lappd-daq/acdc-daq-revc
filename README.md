# acdc-daq-revc
New read-me in pregress

## Prerequisites
This code is built using `cmake` and requires `libusb` headers as well as `gnuplot` if you want to use the oscilloscope mode.
For the pedestal calibration `python3` with `numpy`, `scipy` and optional for plots `matplotlib`. 

To install on a debian-based machine

```bash
$ sudo apt install libusb-1.0-0-dev cmake gnuplot
```

gcc/g++ version optimally is >7.1 
cmake version is >3.1

## Installation
To install the software use
```bash
$ cmake . -Bbuild
$ cmake --build build -- -jN 
```
with N the as the number of cores you want to dedicate to cmake. Never use all available cores!

## Info frames
Both the ACC as well as the ACDC offer an info frame to give the user information about the current firmware versions as well as creation dates.
```bash
$ For the ACC use: ./bin/debug 0x00200000 r
$ For the ACDC use: ./bin/debug 0xFFB54000 0xFFD00000 0x0021000N r
```
with N the corresponding port on the ACC that the ACDC is plugged into as a number from 0 to 7. This will print a 32 word response in the console with following structure:
| Word | ACC response | ACDC response | Value |
|------|--------------|---------------|-------|
| 0 | 1234 | 1234 | Startword |
| 1 | AAAA | BBBB | Identification | 
| 2 |  ->  |  ->  | Firmware version number |
| 3 |  ->  |  ->  | Firmware date year |
| 4 |  ->  |  ->  | Firmware date month/year |
| .. | .. | .. | .. |

## IMPORTANT NOTICE
It is best to run all commands from the main folder via `./bin/command` to be sure all paths are used correctly.

## Errorlog is available
As soon as the software throws an error of any kind an errorlog.txt is generated with a timestamp. This way unexpected failures can be taken care of.

## Executable commands

### setPed
This function lets you set a pedestal value on the acdc boards. The value is an adc count value from 0 to 4095 and can be entered for any of the 8 acdc boards as well as any of the 5 psec chips seperatly if wanted.
If the pedestal value should be set for all boards and chips equaly enter:
```bash
$ ./bin/setPed value
```
If the pedestal value should be set seperatly for different boards and chips enter:
```bash
$ ./bin/setPed boardmask chipmask value
- boardmask is a hex value from 0x01 to 0xFF and has to be entered as such. Each bit represents an acdc board
- boardmask is a bin value from 00000 to 11111 and has to be entered as such. Each bit represents a psec chip
```
After the command is started the value will be set on the acdc boards via the firmware and config files will be generated. The generated files will be filled with the set pedestal value as a base calibration. The comamnd `./bin/calibratePed` is necessary to get the actual pedestal values. The set values can then be found again in the metadata 21-26.

### calibratePed
This function is used to generate a config file for the connected ACDC boards. The command takes some time so be patient. To get the pedestal values for each active channel an empty trace is read N times (currently 100). From these 100 traces each sample for each channel is put in a histogram and fit with a gaussian distribution. The mean is then saved as the real pedestal value and the corresponding sigma is saved for quality control. 
It is advides to execute  `./bin/calibratePed` after `./bin/setPed <...>` but not the other way around. This way the set pedestal value is more precisely determined via averaging. 

### debug
The `./bin/debug` command allows the user to send seperate usb commands to the acc/acdc board. There is no limit to commands on can append to `./bin/debug`, they just need to be seperated by ' '. To get a response after a command use `command r`. The list of commands can be found in the docx int the repository.
**THIS IS A COMMAND FOR DEBUGGING ONLY. BE CAREFUL USING IT FOR NORMAL OPERATION**

### connectedBoards
This command allows the user to check on the connected acdc boards. After executing the command a responce for all possible 8 boards is given. Only boards with a 32 word responde are connected. 

### listenForData
To use full extend of the software use `./bin/listenForData` (important is that you use it from the same folder you launched the make from as well, since otherwise several refences will point to nothing). Once launched the software will guide you through the set up of the boards and then automatically detect and set up all detected boards. The setup process looks like this:
 
1. Choose the wanted trigger mode:
```bash
0 - trigger off
1 - software trigger
2 - ACC SMA trigger
3 - ACDC SMA trigger
4 - self trigger
5 - self trigger with ACC SMA validation
6 - self trigger with ACDC SMA validation
7 - SMA trigger (ACC) with SMA validation (ACDC)
8 - SMA trigger (ACDC) with SMA validation (ACC)
```

2. Depending on the trigger mode the following settings might need to be set as well:
```bash
- level detect or edge detect 
- high/low level for the level detect or rising/falling edge for the edge detect
- self trigger threshold, coincidence minimum number of channels, sign, detection mode, etc...
- the validation window start and length  
```

3. Choose the acdc boards you want to set the settings for. Bits `31-24` each represent one of the 8 acdc board slots on the acc card. By entering an 8bit hex number from `0x00` to `0xFF` individual boards can be selected. If all boards shlould be set the same use `0xFF`, which is also the default. It is advised to use `0xFF` if special requirements are not needed.

4. Set the calibration mode on/off (The calibration mode clones the signal input via on the SMA on ACC/ACDC to all available PSEC channels). Only use this if there is nothing connected to the Samtec connector.

5. Choose between raw output or calibrated output.
```bash
raw mode on - outputs the psec data as it raw data stream, i.e. a vector of 7795 hex words.
raw mode off - outputs the raw data stream of the psec chips in a parsed form, seperating data and metadata and ordering them for channels. See later for the data format.
```

### onlySetup (DON'T USE)
Executes only the setup portion of the `./bin/listenForData` command.

### onlyListen (DON'T USE)
Executes only the data-readout portion of the `./bin/listenForData` command. This way data can be read without complete setup of the trigger every time.
If a different trigger is desired `./onlySetup` needs to be executed again.

### reorder (Currently in progress)
This command `./bin/reorder <file> boardID value boardID value ...`can be used to reorder recorded data in between recording data and analysis. If the command is executed with additional arguments an additional offset will be included in the reordering. There are two timing based reorder reasons:
1. Because the trigger is always in one of 8 clock cycles the correct clock cycle has to be set as the first one. This will always be done by the command by reading the metadata and extracting the cycle as a number from 0 to 7. Each clock cycle being 32 samples the allows for a reorder of the samples by a multiples of 32.
2. Because of the pcb layout, track delays and also the fpga delays, as well as the number of processing clocks used in the fpga the data will have an additional offset. This will be a fixed number for every board between 0 and 255 representing the samples of a waveform. This value has to be determined experimentally and will not change unless the hardware or firmware is adapted.
The command will read the input of the additional arguments and reorder the data according to the value that is set for the corresponding boards. Boards not included in the command won't be reordered. 

| Board ID| value | Board ID| value | Board ID| value | Board ID| value |
|---------|-------|---------|-------|---------|-------|---------|-------|    
| 0 | 0-255 | 2 | 0-255 | 4 | 0-255 | 6 | 0-255 |
| 1 | 0-255 | 3 | 0-255 | 5 | 0-255 | 7 | 0-255 |

### oscope
Oscope works similar to the `listenForData` command, but requires less input. Other than the previous command this one doesn't reqord data but simply overwrites one existing data file to continuesly plot data. Use `Crtl+C` to exit the oscope mode.

## Settings for the Oscilloscope
All settings and plot commands for the oscilloscope are handled in seperate gnu files. 
```bash
./oscilloscope/settings.gnu 	-> sets the axis limits, labels as well as line and point settings 
./oscilloscope/settings_raw.gnu	-> sets the y axis to arbitrary numbers from 0 to 4095 instead od mV
./oscilloscope/liveplot.gnu		-> contains all the plot handling, especially the seperation into 5 PSEC chips and the repeated updating
./oscilloscope/liveplot_b2.gnu	-> experimental plot mode in which 60 channels (2 acdc boards) are plotted instead of 30. A promt will ask for this
```
## Data format
The new data format saves all available channels for up to 8 ACDC boards including the metadata into one file. The structure is
| | Board 0 | ... | Board 7 |
|-|---------|-----|---------|    
|Enumeration | Channel 0 ... Channel 29 & Metadata | ... | Channel 0 ... Channel 29 & Metadata |

which will look like this in the file:
|  |                      |     |                       |
|--|----------------------|-----|-----------------------| 
|0 | data + meta sample 1 | ... |  data + meta sample 1 |
|...| ... | ... | ... |
|255 | data + meta sample 256 | ... |  data + meta sample 256 |

each consecutive event is appended at the end of the file 

## Metadata format 
Metadata is saved with the data in one file. It is always in the coloumn after the 30 channels of an acdc board (e.g. 31 [Because 0 is the enumeration and 1-30 the channels], 62, 93, ...).
The entries are the following as 16bit hex words:

| Word | What it is | What bits are relevant |
|-----------|-----------|-----------|
| 0 | Board ID, refers to the connected port on the ACC | 16 |
| 1 | PSEC ID for PSEC chip 0 (Always 0xDCBN with N as PSEC ID) | 16 |
| 2 | Wilkinson feedback count (current) | 16 |
| 3 | Wilkinson feedback target count setting | 16 |
| 4 | Vbias (pedestal) value setting | 16 |
| 5 | Self trigger threshold value setting | 16 |
| 6 | PROVDD parameter setting | 16 |
| 7 | Trigger info 0, Beamgate timestamp[63:48] | 16 together with Words 27,47 and 67 | 
| 8 | Trigger info 1, Selftrigger mask PSEC 0 | 16 |
| 9 | Trigger info 2, Selftrigger threshold PSEC 0 | 16 |
| 10 | PSEC0 timestamp [15:0] | 16 together with Words 30,50 and 70 |
| . | Clockcycle bit | The last 3 bit (dec 0-7) will give the clockcycle the trigger happened | 
| 11 | PSEC0 event count [15:0] | 16 together with Word 31 |
| 12 | VCDL count [15:0] | 16 together with Word 13 |
| 13 | VCDL count [31:16] | 16 together with Word 12 |
| 14 | DLLVDD parameter setting | 16 |
| 15 | PSEC0-ch0 Self trig rate counts | 16 | 
| 16 | PSEC0-ch1 Self trig rate counts | 16 |
| 17 | PSEC0-ch2 Self trig rate counts | 16 |
| 18 | PSEC0-ch3 Self trig rate counts | 16 |
| 19 | PSEC0-ch4 Self trig rate counts | 16 |
| 20 | PSEC0-ch5 Self trig rate counts | 16 |
| 21 | PSEC ID for PSEC chip 1 (Always 0xDCBN with N as PSEC ID) | 16 |
| 22 | Wilkinson feedback count (current) | 16 |
| 23 | Wilkinson feedback target count setting | 16 |
| 24 | Vbias (pedestal) value setting | 16 |
| 25 | Self trigger threshold value setting | 16 |
| 26 | PROVDD parameter setting | 16 |
| 27 | Trigger info 0, Beamgate timestamp[47:32] | 16 together with Words 7,47 and 67 | 
| 28 | Trigger info 1, Selftrigger mask PSEC 1 | 16 |
| 29 | Trigger info 2, Selftrigger threshold PSEC 1 | 16 |
| 30 | PSEC1 timestamp [31:16] | 16 together with Words 10,50 and 70 |
| 31 | PSEC1 event count [31:16] | 16 together with Word 11 |
| 32 | VCDL count [15:0] | 16 together with Word 33 |
| 33 | VCDL count [31:16] | 16 together with Word 32 |
| 34 | DLLVDD parameter setting | 16 |
| 35 | PSEC1-ch0 Self trig rate counts | 16 | 
| 36 | PSEC1-ch1 Self trig rate counts | 16 |
| 37 | PSEC1-ch2 Self trig rate counts | 16 |
| 38 | PSEC1-ch3 Self trig rate counts | 16 |
| 39 | PSEC1-ch4 Self trig rate counts | 16 |
| 40 | PSEC1-ch5 Self trig rate counts | 16 |
| 41 | PSEC ID for PSEC chip 2 (Always 0xDCBN with N as PSEC ID) | 16 |
| 42 | Wilkinson feedback count (current) | 16 |
| 43 | Wilkinson feedback target count setting | 16 |
| 44 | Vbias (pedestal) value setting | 16 |
| 45 | Self trigger threshold value setting | 16 |
| 46 | PROVDD parameter setting | 16 |
| 47 | Trigger info 0, Beamgate timestamp[31:16] | 16 together with Words 7,27 and 67 | 
| 48 | Trigger info 1, Selftrigger mask PSEC 2 | 16 |
| 49 | Trigger info 2, Selftrigger threshold PSEC 2 | 16 |
| 50 | PSEC2 timestamp [47:32] | 16 together with Words 10,30 and 70 |
| 51 | 0 | 0 |
| 52 | VCDL count [15:0] | 16 together with Word 53 |
| 53 | VCDL count [31:16] | 16 together with Word 52 |
| 54 | DLLVDD parameter setting | 16 |
| 55 | PSEC2-ch0 Self trig rate counts | 16 | 
| 56 | PSEC2-ch1 Self trig rate counts | 16 |
| 57 | PSEC2-ch2 Self trig rate counts | 16 |
| 58 | PSEC2-ch3 Self trig rate counts | 16 |
| 59 | PSEC2-ch4 Self trig rate counts | 16 |
| 60 | PSEC2-ch5 Self trig rate counts | 16 |
| 61 | PSEC ID for PSEC chip 3 (Always 0xDCBN with N as PSEC ID) | 16 |
| 62 | Wilkinson feedback count (current) | 16 |
| 63 | Wilkinson feedback target count setting | 16 |
| 64 | Vbias (pedestal) value setting | 16 |
| 65 | Self trigger threshold value setting | 16 |
| 66 | PROVDD parameter setting | 16 |
| 67 | Trigger info 0, Beamgate timestamp[15:0] | 16 together with Words 7,27 and 47 | 
| 68 | Trigger info 1, Selftrigger mask PSEC 3 | 16 |
| 69 | Trigger info 2, Selftrigger threshold PSEC 3 | 16 |
| 70 | PSEC3 timestamp [63:48] | 16 together with Words 10,30 and 50 |
| 71 | 0 | 0 |
| 72 | VCDL count [15:0] | 16 together with Word 73 |
| 73 | VCDL count [31:16] | 16 together with Word 72 |
| 74 | DLLVDD parameter setting | 16 |
| 75 | PSEC3-ch0 Self trig rate counts | 16 | 
| 76 | PSEC3-ch1 Self trig rate counts | 16 |
| 77 | PSEC3-ch2 Self trig rate counts | 16 |
| 78 | PSEC3-ch3 Self trig rate counts | 16 |
| 79 | PSEC3-ch4 Self trig rate counts | 16 |
| 80 | PSEC3-ch5 Self trig rate counts | 16 |
| 81 | PSEC ID for PSEC chip 4 (Always 0xDCBN with N as PSEC ID) | 16 |
| 82 | Wilkinson feedback count (current) | 16 |
| 83 | Wilkinson feedback target count setting | 16 |
| 84 | Vbias (pedestal) value setting | 16 |
| 85 | Self trigger threshold value setting | 16 |
| 86 | PROVDD parameter setting | 16 |
| 87 | Trigger info 0 | seperated see below |
| . | Trigger setup mode | [15:12] | 
| . | SMA invert setting | [11] |
| . | Selftrigger sign | [10] |
| . | Selftrigger coincidence minimum | [9:0] |
| 88 | Trigger info 1, Selftrigger mask PSEC 4 | 16 |
| 89 | Trigger info 2, Selftrigger threshold PSEC 4 | 16 |
| 90 | 0 | 0 |
| 91 | 0 | 0 |
| 92 | VCDL count [15:0] | 16 together with Word 93 |
| 93 | VCDL count [31:16] | 16 together with Word 92 |
| 94 | DLLVDD parameter setting | 16 |
| 95 | PSEC4-ch0 Self trig rate counts | 16 | 
| 96 | PSEC4-ch1 Self trig rate counts | 16 |
| 97 | PSEC4-ch2 Self trig rate counts | 16 |
| 98 | PSEC4-ch3 Self trig rate counts | 16 |
| 99 | PSEC4-ch4 Self trig rate counts | 16 |
| 100 | PSEC4-ch5 Self trig rate counts | 16 |
| 101 | Combined trigger rate count | 16 |
| 102 | Endword 0xeeee | 0 |


PPS frame:
| Word | What it is | What bits are relevant |
|-----------|-----------|-----------|
| 0 | Startword 0x1234 | 16 |
| 1 | 0xEEEE | 16 |
| 2 | PPS timestamp[63:48] | 16 |
| 3 | PPS timestamp[47:32] | 16 |
| 4 | PPS timestamp[31:16] | 16 |
| 5 | PPS timestamp[15:0] | 16 |
| 6 | Serialnumber[31:16] | 16 |
| 7 | Serialnumber[15:0] | 16 | 
| 8 | PPS count[31:16] | 16 |
| 9 | PPS count[15:0] | 16 |
| 10 | 0x0000 |
| 11 | 0x0000 |
| 12 | 0x0000 |
| 13 | 0x0000 |
| 14 | 0xEEEE | 16 |
| 15 | Endword 0x4321 | 16 | 

