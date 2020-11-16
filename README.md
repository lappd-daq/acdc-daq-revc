# acdc-daq-revc
Here will be a new readme soon!

## Prerequisites
This code is built using `cmake` and requires `libusb` headers as well as `gnuplot` if you want to use the oscilloscope mode.

To install on a debian-based machine

```bash
$ sudo apt install libusb-1.0-0-dev cmake gnuplot
```

gcc/g++ version optimally is >7.1 
cmake version is >3.1

## Executable data monitoring
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
```

2. Depending on the trigger mode the following settings might need to be set as well:
```bash
- level detect or edge detect 
- high/low level for the level detect or rising/falling edge for the edge detect
- self trigger threshold, coincidence minimum number of channels, sign, detection mode, etc...
- the valodation window start and length  
```

3. Set the calibration mode on/off (The calibration mode clones the signal input via on the SMA on ACC/ACDC to all available PSEC channels). Only use this if there is nothing connected to the Samtec connector.

4. Choose between raw output or calibrated output.
```bash
raw on - outputs the channel data as a number between 0 and 4095 and anb offset of around 1500 to 2000 (~600 mV) is visible for the baseline.
raw off- outputs the channel data converted to mV and the baseline is actively corrected to 0.
```
5. Choose between Oscope mode or save mode:
```bash
oscope mode - only one file is saved and overwritten constantly. This file is then plotted by gnuplot into five windows, each being one psec chip.
save mode   - a specified number of waveforms will be saved on the computer as txt files. In addition Metadata files will be saved as well.
```

## Settings for the Oscilloscope
All settings and plot commands for the oscilloscope are handled in seperate gnu files. 
```bash
./oscilloscope/settings.gnu 		-> sets the axis limits, labels as well as line and point settings 
./oscilloscope/settings_raw.gnu	-> sets the y axis to arbitrary numbers from 0 to 4095 instead od mV
./oscilloscope/liveplot.gnu		-> contains all the plot handling, especially the seperation into 5 PSEC chips and the repeated updating
```



