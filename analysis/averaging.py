import sys
import numpy as np
import matplotlib.mlab as mlab
from scipy.stats import norm
import matplotlib.pyplot as plt
plt.rcParams.update({'font.size': 22})

N_SAMPLE = 256 # Number of samples per waveform
N_CHANNEL = 30 # Number of channels per acdc board
N_BOARDS = 8 # Maximum number of acdc boards

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

# Main execution:::::::::
if __name__ == "__main__":
    # Set the filename from an input argument
    filename = sys.argv[1] #input
    savefolder = sys.argv[2] #output

    # Get the number of acdc boards that were read out
    num_boards = (get_board_number(filename)-1)/31
    boardnumber = np.empty(int(num_boards))
    # Loop ober all the read out acdc boards
    for bi in range(0,int(num_boards)):
        # Grab the data of one acdc board and get the number of recorded events
        data = load_board_data(filename,bi*31+1)
        meta = np.loadtxt(filename, dtype=str, delimiter=" ", usecols = 31*(bi+1))
        boardnumber[bi] = int(meta[0])
        number_of_events = len(data[:,1])/N_SAMPLE
        # Helper arrays
        array = np.empty(int(number_of_events))
        matrix = np.empty([int(number_of_events),N_SAMPLE])
        mu_arr = np.empty(N_SAMPLE)
        sigma_arr = np.empty(N_SAMPLE)
        print_mat = np.empty([N_SAMPLE,N_CHANNEL])
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
                ###Generates 256 Histograms for every channel and board
                if ch==0 and smp<100:
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
            print_mat[:,ch] = mu_arr   
            ###Generates overview plots for mu and sigma -> mu/sigma over sample for every channel
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
        calibsave = savefolder + "PEDS_ACDC_board" + str(int(boardnumber[bi])) + ".txt"
        f = open(calibsave, "w")
        for i in range(0,N_SAMPLE):
            for j in range(0,N_CHANNEL):
                f.write(str(print_mat[i,j]))
                f.write(" ")
            f.write("\n")
        f.close()
