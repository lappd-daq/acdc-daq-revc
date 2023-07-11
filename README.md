# ACC and ACDC software for testing purposes with new ethernet interface
Test version for the ethernet comms to read out an ACC. Please keep in mind that this is a trial version and may not work at all.

## Install
To install the software just execute 
```bash
./INSATLL.sh 
```
if it does not execute because of missing permissions use 
```bash
chmod +x ./INSTALL.sh
```

## Configfile
For the connection via IP and PORT a file called ConnectionSettings is available.
Here you can change the setting to match the ACC.

## Use
Currently the only working commands are `./TestConnection` and `./Command`.
The first tests the connection to the ACC by using the settings in ConnectionSettings.
The second will send a single command as read or write to the ACC. The usage is as follows:

```bash
./Command [command address] [command value] [read/write]
```

Where `command address` and `command value` are what you want to send.
`read/write` determines id a command is only sent, like for example an LED setting, or if 
after a command is sent a readback should be expected. `rs` expects a single value to come back, while `rv` expects
a vector of data to be sent. In both cases it should print the response.