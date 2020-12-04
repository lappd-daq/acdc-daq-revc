# acdc-daq-revc
New read-me in pregress

## Prerequisites
This code is built using `cmake` and requires `libusb` headers as well as `gnuplot` if you want to use the oscilloscope mode.

To install on a debian-based machine

```bash
$ sudo apt install libusb-1.0-0-dev cmake gnuplot
```

gcc/g++ version optimally is >7.1 
cmake version is >3.1

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
- boardmask is a hex value from 0x00 to 0xFF and has to be entered as such. Each bit represents an acdc board
- boardmask is a bin value from 00000 to 11111 and has to be entered as such. Each bit represents a psec chip
```
After the command is started the value will be set on the acdc boards via the firmware and a config file will be generated. If raw mode is turned off (explained in later sections) the config file will be read and the values will be substracted.

### calibratePed
This function is used if no pedestal value is manually set. The command takes some time so be patient. To get the pedestal values for each active channel an empty trace is read N times (currently 50). From these 50 traces for each channel an average is calulated and written into a config file. If raw mode is turned off (explained in later sections) the config file will be read and the values will be substracted.
It is possible to execute  `./bin/calibratePed` after `./bin/setPed <...>` but not the other way around. This way the set pedestal value is also determined via averaging.

### debug
The `./bin/debug` command allows the user to send seperate usb commands to the acc/acdc board. There is no limit to commands on can append to `./bin/debug`, they just need to be seperated by ' '. To get a response after a command use `command r`.
**IMPORTANT IT SEEMS DEBUG IS NOT WORKING PERFECTLY SO BE CAREFUL USING IT. IT SHOULD BE FIXED**

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
3. Choose the acdc boards you want to set the settings for. Bits `31-24` each represent one of the 8 acdc board slots on the acc card. By entering an 8bit hex number from `0x00` to `0xFF` individual boards can be selected. If all boards shlould be set the same use `0xFF`, which is also the default. To set individual settings it is best to use `./bin/onlySetup` since its can be run repedetly without reading data.

4. Set the calibration mode on/off (The calibration mode clones the signal input via on the SMA on ACC/ACDC to all available PSEC channels). Only use this if there is nothing connected to the Samtec connector.

5. Choose between raw output or calibrated output.
```bash
raw on - outputs the channel data as a number between 0 and 4095 and an offset of around 1500 to 2000 (~600 mV) is visible for the baseline if not set before.
raw off- outputs the channel data and the baseline is actively corrected to 0.
```
6. Choose between Oscope mode or save mode:
```bash
oscope mode - only one file is saved and overwritten constantly. This file is then plotted by gnuplot into five windows, each being one psec chip.
save mode   - a specified number of waveforms will be saved on the computer as txt files. In addition Metadata files will be saved as well.
```

### onlySetup
Executes only the setup portion of the `./bin/listenForData` command.

### onlyListen (experimental)
Executes only the data-readout portion of the `./bin/listenForData` command. This way data can be read without complete setup of the trigger every time.
If a different trigger is desired `./onlySetup` needs to be executed again.

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

| Line | Meta key |
|------|-------------- |  
| 0 | Boardnumber |
| 1 | DLLVDD_setting_0 |    
| 2 | DLLVDD_setting_1 |    
| 3 | DLLVDD_setting_2 |    
| 4 | DLLVDD_setting_3 |    
| 5 | DLLVDD_setting_4 |    
| 6 | PROVDD_setting_0 |    
| 7 | PROVDD_setting_1 |    
| 8 | PROVDD_setting_2 |    
| 9 | PROVDD_setting_3 |    
| 10 | PROVDD_setting_4 |    
| 11 | VCDL_count_hi_0 |    
| 12 | VCDL_count_hi_1 |    
| 13 | VCDL_count_hi_2 |    
| 14 | VCDL_count_hi_3 |    
| 15 | VCDL_count_hi_4 |    
| 16 | VCDL_count_lo_0 |    
| 17 | VCDL_count_lo_1 |    
| 18 | VCDL_count_lo_2 |    
| 19 | VCDL_count_lo_3 |    
| 20 | VCDL_count_lo_4 |    
| 21 | Vbias_setting_0 |    
| 22 | Vbias_setting_1 |    
| 23 | Vbias_setting_2 |    
| 24 | Vbias_setting_3 |    
| 25 | Vbias_setting_4 |    
| 26 | clockcycle_bits |    
| 27 | combined_trigger_rate_count |    
| 28 | event_count_hi |    
| 29 | event_count_lo |    
| 30 | feedback_count_0 |    
| 31 | feedback_count_1 |    
| 32 | feedback_count_2 |    
| 33 | feedback_count_3 |    
| 34 | feedback_count_4 |    
| 35 | feedback_target_count_0 |    
| 36 | feedback_target_count_1 |    
| 37 | feedback_target_count_2 |    
| 38 | feedback_target_count_3 |    
| 39 | feedback_target_count_4 |    
| 40 | self_trigger_rate_count_psec_ch0 |    
| 41 | self_trigger_rate_count_psec_ch1 |    
| 42 | self_trigger_rate_count_psec_ch10 |    
| 43 | self_trigger_rate_count_psec_ch11 |    
| 44 | self_trigger_rate_count_psec_ch12 |    
| 45 | self_trigger_rate_count_psec_ch13 |    
| 46 | self_trigger_rate_count_psec_ch14 |    
| 47 | self_trigger_rate_count_psec_ch15 |    
| 48 | self_trigger_rate_count_psec_ch16 |    
| 49 | self_trigger_rate_count_psec_ch17 |    
| 50 | self_trigger_rate_count_psec_ch18 |    
| 51 | self_trigger_rate_count_psec_ch19 |    
| 52 | self_trigger_rate_count_psec_ch2 |    
| 53 | self_trigger_rate_count_psec_ch20 |    
| 54 | self_trigger_rate_count_psec_ch21 |    
| 55 | self_trigger_rate_count_psec_ch22 |    
| 56 | self_trigger_rate_count_psec_ch23 |    
| 57 | self_trigger_rate_count_psec_ch24 |    
| 58 | self_trigger_rate_count_psec_ch25 |    
| 59 | self_trigger_rate_count_psec_ch26 |    
| 60 | self_trigger_rate_count_psec_ch27 |    
| 61 | self_trigger_rate_count_psec_ch28 |    
| 62 | self_trigger_rate_count_psec_ch29 |    
| 63 | self_trigger_rate_count_psec_ch3 |    
| 64 | self_trigger_rate_count_psec_ch4 |    
| 65 | self_trigger_rate_count_psec_ch5 |    
| 66 | self_trigger_rate_count_psec_ch6 |    
| 67 | self_trigger_rate_count_psec_ch7 |    
| 68 | self_trigger_rate_count_psec_ch8 |    
| 69 | self_trigger_rate_count_psec_ch9 |    
| 70 | selftrigger_threshold_setting_0 |    
| 71 | selftrigger_threshold_setting_1 |    
| 72 | selftrigger_threshold_setting_2 |    
| 73 | selftrigger_threshold_setting_3 |    
| 74 | selftrigger_threshold_setting_4 |    
| 75 | timestamp_0 |    
| 76 | timestamp_1 |    
| 77 | timestamp_2 |    
| 78 | timestamp_3 |    
| 79 | trigger_acc_detection_mode |    
| 80 | trigger_acc_invert |    
| 81 | trigger_mode |    
| 82 | trigger_self_coin |    
| 83 | trigger_self_detection_mode |    
| 84 | trigger_self_sign |    
| 85 | trigger_self_threshold_0 |    
| 86 | trigger_self_threshold_1 |    
| 87 | trigger_self_threshold_2 |    
| 88 | trigger_self_threshold_3 |    
| 89 | trigger_self_threshold_4 |    
| 90 | trigger_selfmask_0 |    
| 91 | trigger_selfmask_1 |    
| 92 | trigger_selfmask_2 |    
| 93 | trigger_selfmask_3 |    
| 94 | trigger_selfmask_4 |    
| 95 | trigger_sma_detection_mode |    
| 96 | trigger_sma_invert |    
| 97 | trigger_validation_window_length |    
| 98 | trigger_validation_window_start |    
|99 - 255 | 0 |
