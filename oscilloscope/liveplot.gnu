
set terminal wxt 1 title "PSEC1" size 500,500 background 'white' noraise 
set key on outside 
plot\
"./Results/Data_b0_evno0.dat" using ($1):2 lw 2 smooth csplines title 'CHAN 1',\
"./Results/Data_b0_evno0.dat" using ($1):3 lw 2 smooth csplines title 'CHAN 2',\
"./Results/Data_b0_evno0.dat" using ($1):4 lw 2 smooth csplines title 'CHAN 3',\
"./Results/Data_b0_evno0.dat" using ($1):5 lw 2 smooth csplines title 'CHAN 4',\
"./Results/Data_b0_evno0.dat" using ($1):6 lw 2 smooth csplines title 'CHAN 5',\
"./Results/Data_b0_evno0.dat" using ($1):7 lw 2 smooth csplines title 'CHAN 6',\

set terminal wxt 2 title 'PSEC2' size 500,500 background 'white' noraise 
set key on outside  
plot\
"./Results/Data_b0_evno0.dat" using ($1):8 lw 2 smooth csplines title 'CHAN 1',\
"./Results/Data_b0_evno0.dat" using ($1):9 lw 2 smooth csplines title 'CHAN 2',\
"./Results/Data_b0_evno0.dat" using ($1):10 lw 2 smooth csplines title 'CHAN 3',\
"./Results/Data_b0_evno0.dat" using ($1):11 lw 2 smooth csplines title 'CHAN 4',\
"./Results/Data_b0_evno0.dat" using ($1):12 lw 2 smooth csplines title 'CHAN 5',\
"./Results/Data_b0_evno0.dat" using ($1):13 lw 2 smooth csplines title 'CHAN 6',\

set terminal wxt 3 title 'PSEC3' size 500,500 background 'white' noraise 
set key on outside  
plot\
"./Results/Data_b0_evno0.dat" using ($1):14 lw 2 smooth csplines title 'CHAN 1',\
"./Results/Data_b0_evno0.dat" using ($1):15 lw 2 smooth csplines title 'CHAN 2',\
"./Results/Data_b0_evno0.dat" using ($1):16 lw 2 smooth csplines title 'CHAN 3',\
"./Results/Data_b0_evno0.dat" using ($1):17 lw 2 smooth csplines title 'CHAN 4',\
"./Results/Data_b0_evno0.dat" using ($1):18 lw 2 smooth csplines title 'CHAN 5',\
"./Results/Data_b0_evno0.dat" using ($1):19 lw 2 smooth csplines title 'CHAN 6',\

set terminal wxt 4 title 'PSEC4' size 500,500 background 'white' noraise 
set key on outside  
plot\
"./Results/Data_b0_evno0.dat" using ($1):20 lw 2 smooth csplines title 'CHAN 1',\
"./Results/Data_b0_evno0.dat" using ($1):21 lw 2 smooth csplines title 'CHAN 2',\
"./Results/Data_b0_evno0.dat" using ($1):22 lw 2 smooth csplines title 'CHAN 3',\
"./Results/Data_b0_evno0.dat" using ($1):23 lw 2 smooth csplines title 'CHAN 4',\
"./Results/Data_b0_evno0.dat" using ($1):24 lw 2 smooth csplines title 'CHAN 5',\
"./Results/Data_b0_evno0.dat" using ($1):25 lw 2 smooth csplines title 'CHAN 6',\

set terminal wxt 5 title 'PSEC5' size 500,500 background 'white' noraise 
set key on outside  
plot\
"./Results/Data_b0_evno0.dat" using ($1):26 lw 2 smooth csplines title 'CHAN 1',\
"./Results/Data_b0_evno0.dat" using ($1):27 lw 2 smooth csplines title 'CHAN 2',\
"./Results/Data_b0_evno0.dat" using ($1):28 lw 2 smooth csplines title 'CHAN 3',\
"./Results/Data_b0_evno0.dat" using ($1):29 lw 2 smooth csplines title 'CHAN 4',\
"./Results/Data_b0_evno0.dat" using ($1):30 lw 2 smooth csplines title 'CHAN 5',\
"./Results/Data_b0_evno0.dat" using ($1):31 lw 2 smooth csplines title 'CHAN 6',\

pause 0.1
reread

