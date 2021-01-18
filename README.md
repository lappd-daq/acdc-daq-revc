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
The entries are the following:

| Line | Metakey | Line | Metakey | Line | Metakey |
|-----------|-----------|-----------|-----------|-----------|-----------|
| 0 | Boardnumber | 34 | feedback_count_4 | 68 | self_trigger_rate_count_psec_ch8 |
| 1 | DLLVDD_setting_0 | 35 | feedback_target_count_0 | 69 | self_trigger_rate_count_psec_ch9 |
| 2 | DLLVDD_setting_1 | 36 | feedback_target_count_1 | 70 | selftrigger_threshold_setting_0 |
| 3 | DLLVDD_setting_2 | 37 | feedback_target_count_2 | 71 | selftrigger_threshold_setting_1 |
| 4 | DLLVDD_setting_3 | 38 | feedback_target_count_3 | 72 | selftrigger_threshold_setting_2 |
| 5 | DLLVDD_setting_4 | 39 | feedback_target_count_4 | 73 | selftrigger_threshold_setting_3 |
| 6 | PROVDD_setting_0 | 40 | self_trigger_rate_count_psec_ch0 | 74 | selftrigger_threshold_setting_4 |
| 7 | PROVDD_setting_1 | 41 | self_trigger_rate_count_psec_ch1 | 75 | timestamp_0 |
| 8 | PROVDD_setting_2 | 42 | self_trigger_rate_count_psec_ch10 | 76 | timestamp_1 |
| 9 | PROVDD_setting_3 | 43 | self_trigger_rate_count_psec_ch11 | 77 | timestamp_2 |
| 10 | PROVDD_setting_4 | 44 | self_trigger_rate_count_psec_ch12 | 78 | timestamp_3 |
| 11 | VCDL_count_hi_0 | 45 | self_trigger_rate_count_psec_ch13 | 79 | trigger_acc_detection_mode |
| 12 | VCDL_count_hi_1 | 46 | self_trigger_rate_count_psec_ch14 | 80 | trigger_acc_invert |
| 13 | VCDL_count_hi_2 | 47 | self_trigger_rate_count_psec_ch15 | 81 | trigger_mode |
| 14 | VCDL_count_hi_3 | 48 | self_trigger_rate_count_psec_ch16 | 82 | trigger_self_coin |
| 15 | VCDL_count_hi_4 | 49 | self_trigger_rate_count_psec_ch17 | 83 | trigger_self_detection_mode |
| 16 | VCDL_count_lo_0 | 50 | self_trigger_rate_count_psec_ch18 | 84 | trigger_self_sign |
| 17 | VCDL_count_lo_1 | 51 | self_trigger_rate_count_psec_ch19 | 85 | trigger_self_threshold_0 |
| 18 | VCDL_count_lo_2 | 52 | self_trigger_rate_count_psec_ch2 | 86 | trigger_self_threshold_1 |
| 19 | VCDL_count_lo_3 | 53 | self_trigger_rate_count_psec_ch20 | 87 | trigger_self_threshold_2 |
| 20 | VCDL_count_lo_4 | 54 | self_trigger_rate_count_psec_ch21 | 88 | trigger_self_threshold_3 |
| 21 | Vbias_setting_0 | 55 | self_trigger_rate_count_psec_ch22 | 89 | trigger_self_threshold_4 |
| 22 | Vbias_setting_1 | 56 | self_trigger_rate_count_psec_ch23 | 90 | trigger_selfmask_0 |
| 23 | Vbias_setting_2 | 57 | self_trigger_rate_count_psec_ch24 | 91 | trigger_selfmask_1 |
| 24 | Vbias_setting_3 | 58 | self_trigger_rate_count_psec_ch25 | 92 | trigger_selfmask_2 |
| 25 | Vbias_setting_4 | 59 | self_trigger_rate_count_psec_ch26 | 93 | trigger_selfmask_3 |
| 26 | clockcycle_bits | 60 | self_trigger_rate_count_psec_ch27 | 94 | trigger_selfmask_4 |
| 27 | combined_trigger_rate_count | 61 | self_trigger_rate_count_psec_ch28 | 95 | trigger_sma_detection_mode |
| 28 | event_count_hi | 62 | self_trigger_rate_count_psec_ch29 | 96 | trigger_sma_invert |
| 29 | event_count_lo | 63 | self_trigger_rate_count_psec_ch3 | 97 | trigger_validation_window_length |
| 30 | feedback_count_0 | 64 | self_trigger_rate_count_psec_ch4 | 98 | trigger_validation_window_start |
| 31 | feedback_count_1 | 65 | self_trigger_rate_count_psec_ch5 | 100-255 | 0 |
| 32 | feedback_count_2 | 66 | self_trigger_rate_count_psec_ch6 | | |    
| 33 | feedback_count_3 | 67 | self_trigger_rate_count_psec_ch7 | | |   
