import sys
import numpy as np
import matplotlib.mlab as mlab
from scipy.stats import norm
import matplotlib.pyplot as plt
plt.rcParams.update({'font.size': 22})

N_SAMPLE = 256 # Number of samples per waveform
N_CHANNEL = 30 # Number of channels per acdc board
N_BOARDS = 8 # Maximum number of acdc boards
SIGN = -1 # Setting for the expected sign of the pulse
THRESHOLD = 500 # Setting for the threshold to count a pulse as a pulse
WIDTH_NS = 5 # Setting for the expected width of a puse

# Function to load a set of events from one acdc board at a time
def load_board_data(filename, offset):
    # Loads 30 channels with an offset that is determined by the board number 
	raw_data = np.loadtxt(filename, delimiter=" ", usecols = range(0+offset,30+offset))
    # Return a matrix of data with the dimensions [N*256,30], 
    # where n is the number of available events in the file
	return raw_data

# Function to get the amount of read-out acdc boards from the amount 
# of available channels
def get_board_number(filename):
    # Read the first line of the file and split it on every ' '
    f = open(filename, "r").readline()
    # Then count the amount of entries
    connectedBoards = len(f.split())
    # Return the number of acd boards
    return connectedBoards

# Function to determine the pedestal of the current PSEC chip
def getPedestal(channel, meta_event):
    # Depending on the current channels corresponding PSEC chip
    # read the metadata coloumn to get the set pedestal value
    if ch>=0 and ch<=5:
        pedestal = int(meta_event[21],16)
    elif ch>=6 and ch<=11:
        pedestal = int(meta_event[22],16)
    elif ch>=12 and ch<=17:
        pedestal = int(meta_event[24],16)
    elif ch>=18 and ch<=23:
        pedestal = int(meta_event[25],16)
    elif ch>=24 and ch<=29:
        pedestal = int(meta_event[26],16)
    else:
        print("Pedestal error")
    # Return the pedestal value
    return pedestal

# Function to restructure the data of an acdc board given by the clockcycle 
# the trigger happend in 
def restructure(data, cycleBit):
    # The cycleBit is given by the metadata and determines which of the 8
    # 320 MHz clockcycles the trigger happend in 32 samples * this bit is 
    # then the actual first sample 
    cycleSample = 32 * cycleBit
    # Create an empty new array for the transformation
    new_vector = np.empty(256)
    # Loop over all available samples
    for i in range(0,N_SAMPLE):
        # Copy every sample from the actual first sample to the last sample in first
        # then switch around and start at 0 
        if i<(N_SAMPLE-cycleSample):
            new_vector[i] = data[i+cycleSample]
        else: 
            new_vector[i] = data[i-(N_SAMPLE-cycleSample)]
    # Retrun this new restructured array
    return new_vector
  

# Main execution:::::::::
if __name__ == "__main__":
    # Set the filename from an input argument
    filename = sys.argv[1] #input
    savefolder = sys.argv[2] #output

    # Get the number of acdc boards that were read out
    num_boards = (get_board_number(filename)-1)/31

    # Loop ober all the read out acdc boards
    for bi in range(0,int(num_boards)):
        # Grab the data of one acdc board and get the number of recorded events
        data = load_board_data(filename,bi*31+1)
        meta = np.loadtxt(filename, delimiter=" ", usecols = 31*(bi+1))
        boardnumber[bi] = meta[0]
        number_of_events = len(data[:,1])/N_SAMPLE
        # Helper arrays
        array = np.empty(int(number_of_events))
        matrix = np.empty([int(number_of_events),N_SAMPLE])
        mu_arr = np.empty(N_SAMPLE)
        sigma_arr = np.empty(N_SAMPLE)
        print_arr = np.empty([N_SAMPLE,N_CHANNEL])
        # Loop over all channels
        for ch in range(0,N_CHANNEL):
            # Loop over all events
            for ev in range(0,int(number_of_events)):
                # Generate an event offset
                event = ev*N_SAMPLE
                # Grab only the respective data
                y = data[0+event:256+event,ch]
                matrix[ev,:] = np.transpose(y)

            for smp in range(0,N_SAMPLE):
                
                # best fit of data
                (mu, sigma) = norm.fit(matrix[:,smp])
                mu_arr[smp] = mu
                sigma_arr[smp] = sigma
                
                '''
                if ch==3:
                    plt.figure(num=smp, figsize=[25,25], facecolor='white')
                    plt.hist(matrix[:,smp], density=True,bins= np.arange(min(matrix[:,smp]),max(matrix[:,smp])+1,1))

                    xmin, xmax = plt.xlim()
                    x_pdf = np.linspace(xmin, xmax, 100)
                    y_pdf = norm.pdf(x_pdf, mu, sigma)

                    plt.plot(x_pdf, y_pdf, 'r--', linewidth=2)

                    plt.xlabel('pulse height in adc')
                    plt.ylabel('count')
                    title = "Fit results: mu = %.2f,  std = %.2f" % (mu, sigma)
                    plt.title(title)
                    printname = savefolder + "Hist_ch" + str(ch) + "_smp" + str(smp) + ".png"
                    plt.savefig(printname)
                    plt.close(smp)
                '''

            print_arr[:,ch] = mu_arr   
            '''
            plt.figure(num=ch, figsize=[25,25], facecolor='white')
            plt.plot(mu_arr)
            printname = savefolder + "Mu_ch" + str(ch) + ".png"
            plt.savefig(printname)
            # Every channel
            plt.close(ch)
            plt.figure(num=ch, figsize=[25,25], facecolor='white')
            plt.plot(sigma_arr)
            printname = savefolder + "Sigma_ch" + str(ch) + ".png"
            plt.savefig(printname)
            # Every channel
            plt.close(ch)
            '''
        calibsave = savefolder + "PEDS_ACDC_board" + str(boardnumber[bi]) + ".txt"
        f = open(calibsave, "w")
        for i in range(0,N_SAMPLE):
            for j in range(0,N_CHANNEL):
                f.write(str(print_arr[i,j]))
                f.write(" ")
            f.write("\n")
        f.close()