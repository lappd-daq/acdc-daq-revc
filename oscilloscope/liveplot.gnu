set xrange [0:256]
set autoscale y

set pointsize .8
set grid ls 1
set border ls 10
set size 1, 1

set xlabel 'Sample Number' 
set ylabel 'voltage [mV]'

done = 0
bind "ctrl-q" "done = 1"

set multiplot layout 3,2

plot\
"./Results/Data_b0_evno0.txt" using ($1):2 ls 2 smooth csplines title 'CHAN 1',\
"./Results/Data_b0_evno0.txt" using ($1):3 ls 3 smooth csplines title 'CHAN 2',\
"./Results/Data_b0_evno0.txt" using ($1):4 ls 4 smooth csplines title 'CHAN 3',\
"./Results/Data_b0_evno0.txt" using ($1):5 ls 5 smooth csplines title 'CHAN 4',\
"./Results/Data_b0_evno0.txt" using ($1):6 ls 6 smooth csplines title 'CHAN 5',\
"./Results/Data_b0_evno0.txt" using ($1):7 ls 7 smooth csplines title 'CHAN 6'

plot\
"./Results/Data_b0_evno0.txt" using ($1):8 ls 2 smooth csplines title 'CHAN 1',\
"./Results/Data_b0_evno0.txt" using ($1):9 ls 3 smooth csplines title 'CHAN 2',\
"./Results/Data_b0_evno0.txt" using ($1):10 ls 4 smooth csplines title 'CHAN 3',\
"./Results/Data_b0_evno0.txt" using ($1):11 ls 5 smooth csplines title 'CHAN 4',\
"./Results/Data_b0_evno0.txt" using ($1):12 ls 6 smooth csplines title 'CHAN 5',\
"./Results/Data_b0_evno0.txt" using ($1):13 ls 7 smooth csplines title 'CHAN 6'

plot\
"./Results/Data_b0_evno0.txt" using ($1):14 ls 2 smooth csplines title 'CHAN 1',\
"./Results/Data_b0_evno0.txt" using ($1):15 ls 3 smooth csplines title 'CHAN 2',\
"./Results/Data_b0_evno0.txt" using ($1):16 ls 4 smooth csplines title 'CHAN 3',\
"./Results/Data_b0_evno0.txt" using ($1):17 ls 5 smooth csplines title 'CHAN 4',\
"./Results/Data_b0_evno0.txt" using ($1):18 ls 6 smooth csplines title 'CHAN 5',\
"./Results/Data_b0_evno0.txt" using ($1):19 ls 7 smooth csplines title 'CHAN 6'

plot\
"./Results/Data_b0_evno0.txt" using ($1):20 ls 2 smooth csplines title 'CHAN 1',\
"./Results/Data_b0_evno0.txt" using ($1):21 ls 3 smooth csplines title 'CHAN 2',\
"./Results/Data_b0_evno0.txt" using ($1):22 ls 4 smooth csplines title 'CHAN 3',\
"./Results/Data_b0_evno0.txt" using ($1):23 ls 5 smooth csplines title 'CHAN 4',\
"./Results/Data_b0_evno0.txt" using ($1):24 ls 6 smooth csplines title 'CHAN 5',\
"./Results/Data_b0_evno0.txt" using ($1):25 ls 7 smooth csplines title 'CHAN 6'

plot\
"./Results/Data_b0_evno0.txt" using ($1):26 ls 2 smooth csplines title 'CHAN 1',\
"./Results/Data_b0_evno0.txt" using ($1):27 ls 3 smooth csplines title 'CHAN 2',\
"./Results/Data_b0_evno0.txt" using ($1):28 ls 4 smooth csplines title 'CHAN 3',\
"./Results/Data_b0_evno0.txt" using ($1):29 ls 5 smooth csplines title 'CHAN 4',\
"./Results/Data_b0_evno0.txt" using ($1):30 ls 6 smooth csplines title 'CHAN 5',\
"./Results/Data_b0_evno0.txt" using ($1):31 ls 7 smooth csplines title 'CHAN 6' 


pause 1
if (!done) reread

