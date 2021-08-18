import numpy as np
import sys
import matplotlib.mlab as mlab
from scipy.stats import norm
import matplotlib.pyplot as plt



#take the given data file and make a list of lists that contains each word 
#in the file separated by line; data [1][0] gives the first word of the 
#second line of the data file
def makeDataArray(file):
    data = []
    f = open(file, "r")
    listOfLines=f.readlines()
    f.close()
    listOfLines[-1]+="/n" #this is added so that the last metadata is not erased below
    for line in listOfLines:
        lineNew = line[:-2] #remove the "/n"'s at the end of each line in ListOfLines
        lineContentList = lineNew.split() #split words
        data.append(lineContentList)
    if len(data[0]) == 32:
        for j in range(0,len(data)):
            for i in range(0,31):
                data[j][i]=int(data[j][i])
    else:
        for j in range(0,len(data)):
            for i in range(0,len(data[0])):
                data[j][i]=float(data[j][i])
    return data
    
    
#Given the data set, finds how many events there are
def getNumEvents(data):
    #There should be an even multiple of 256 lines for each event
    if len(data)%256!=0:
        print("Wrong number of lines, could be faulty data!")
    return len(data)//256
#Given the dataset, looks at the last element in every line to extract the metadata


def getMetaDataList(data):
    metadata=[]
    #Appends the last element of every line to list
    for line in data:
        metadata.append(line[31])
    print(len(metadata))
    return metadata
    
#timestep is taken from Evan Angelico's assumption of a constant sampling time
def getTimes(acdc_clock=25*10**6):
    #from Evan's code: timestep = 1.0e9/(self.nsamples*acdc_clock*10e6) in nanoseconds
    timestep=10**9/(256*acdc_clock)
    times = [i*timestep for i in range(256)]
    return times
#Separates out one event (256 samples, 30 channels + 1 channel metadata) from 'data' and applies calibrations
def getEvent(d, calibration_data, eventNumber):
    #flags an error if you ask for an event that did not happen
    data=makeDataArray(d)
    if eventNumber*256 > len(data):
        raise ValueError("Data given does not have "+str(eventNumber)+" events")
    calib=makeDataArray(calibration_data)
    event = []
    #loops over each sample and subtracts the measured value from the calibration
    for i in range((eventNumber-1)*256, eventNumber*256):
        event.append(data[i])
    for i in range(0,256):
        for j in range(0,30):
            event[i][j+1]=event[i][j+1]-calib[i][j]
    return event
    
#Separates out one channel (column) of data from the overall structure
def getChannelData(event,channelNumber):
    channelData = []
    if channelNumber > 30:
        raise ValueError("C'mon, ACDCs have 30 channels")
    for i in range(len(event)):
        channelData.append(event[i][channelNumber])
    return channelData
#Plots an event of one channel 
def makeChart(data, calibration_data, eventNumber, channelNumber):
    #Takes the data you want to look at
    event = getEvent(data, calibration_data, eventNumber)
    d = getChannelData(event,channelNumber)
    #Uses the time of the first sample as t=0
    times = getTimes()
    #convert ADC counts to mv (1200/4096 taken from Evan Angelico):
    ADC_to_mv = 1200/4096
    d=[i*ADC_to_mv for i in d]
    plt.xlabel("Time (ns)")
    plt.ylabel("Voltage (mv)")
    plt.plot(times, d, "bo")
    plt.title("Event "+str(eventNumber) + " channel " + str(channelNumber))
    plt.savefig("Event"+str(eventNumber) + "_Channel" + str(channelNumber)+".png", dpi=300, )
    plt.show()
    
#takes in data, event number, psec number
def plotOnePSEC(data, calibration_data, pn, en):
    e = getEvent(data, calibration_data, en)
    channelList = []
    t = getTimes()
    for i in range(1,7):
        channelList.append((pn-1)*6+i)
    fig, axs = plt.subplots(2,3)
    ADC_to_mv = 1200/4096
    for i in range(3):
        d1=getChannelData(e, channelList[i])
        d2=getChannelData(e, channelList[i+3])
        d1 = [i*ADC_to_mv for i in d1]
        d2 = [i*ADC_to_mv for i in d2]
        axs[0, i].plot(t, d1, "bo", markersize=2)
        axs[1, i].plot(t, d2, "bo", markersize=2)
        axs[0, i].set_title("Event " +str(en) + ", PSEC "+ str(pn) +", Channel " +str(i+1))
        axs[1, i].set_title("Event " +str(en) + ", PSEC "+ str(pn) +", Channel " +str(i+4))
    for ax in axs.flat:
        ax.set(xlabel="Time (ns)", ylabel="Voltage (mv)")
    plt.tight_layout()
    plt.savefig("PSEC" +str(pn) +"_Event"+str(en)+".png", dpi=300)
    plt.show()
    print("my goodness gracious, this is working so swimmingly")
    
def plotChannel(data, calibration_data, en, cn):
    e=getEvent(data,calibration_data, en)
    t=getTimes()
    d=[]
    for i in range(5):
        d.append(getChannelData(e,6*i+cn))
    fig, axs = plt.subplots(2,3)
    ADC_to_mv = 1200/4096
    for i in range(3):
        d[i]=[j*ADC_to_mv for j in d[i]]
        axs[0, i].plot(t, d[i], "bo", markersize=2)
        axs[0, i].set_title("PSEC "+ str(i+1))
    for i in range(2):
        d[i+3]=[j*ADC_to_mv for j in d[i+3]]
        axs[1, i].plot(t, d[i+3], "bo", markersize=2)
        axs[1, i].set_title( "PSEC "+ str(i+4))
    for ax in axs.flat:
        ax.set(xlabel="Time (ns)", ylabel="Voltage (mv)")
    plt.tight_layout()
    plt.savefig("Channel" +str(cn) +"_Event"+str(en)+".png", dpi=300)
    plt.show()
    
#main functionality:::::::
if True:
    d = "path_to_data_file.txt"
    c = "path_to_pedestal_file.txt"
    #makeChart(d,c,3,6)
    #plotChannel(d,c,8,6)
    #plotOnePSEC(d,c,4,7)



    
