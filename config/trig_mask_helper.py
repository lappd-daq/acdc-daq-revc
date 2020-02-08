

def convert_ch_list(chs):
	ch_bits = 30
	binstr = ''
	for i in range(30):
		if(i+1 in chs):
			binstr += '1'
		else:
			binstr += '0'

	binmask = binstr[::-1]
	return hex(int(binmask, 2))



if __name__ == "__main__":
	l2_chs = range(1,31)
        lrms = [6,7,8,14,17,2]
        for rmch in lrms:
            l2_chs.remove(rmch)

	print "l2 mask = " + convert_ch_list(l2_chs)
	
