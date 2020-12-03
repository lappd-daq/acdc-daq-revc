set xrange [0:256]
set autoscale y
set datafile sep ' '
set decimalsign '.'

set terminal wxt 1 title "PSEC0&1-B0" size 500,500 background rgb 'grey75' noraise 
set key on outside 
plot\
"./Results/Data_Oscope_b0.txt" using 1:2 lw 2 with lines title 'PSEC0 CHAN 1',\
"./Results/Data_Oscope_b0.txt" using 1:3 lw 2 with lines title 'PSEC0 CHAN 2',\
"./Results/Data_Oscope_b0.txt" using 1:4 lw 2 with lines title 'PSEC0 CHAN 3',\
"./Results/Data_Oscope_b0.txt" using 1:5 lw 2 with lines title 'PSEC0 CHAN 4',\
"./Results/Data_Oscope_b0.txt" using 1:6 lw 2 with lines title 'PSEC0 CHAN 5',\
"./Results/Data_Oscope_b0.txt" using 1:7 lw 2 with lines title 'PSEC0 CHAN 6',\
"./Results/Data_Oscope_b0.txt" using 1:8 lw 2 with lines title 'PSEC1 CHAN 1',\
"./Results/Data_Oscope_b0.txt" using 1:9 lw 2 with lines title 'PSEC1 CHAN 2',\
"./Results/Data_Oscope_b0.txt" using 1:10 lw 2 with lines title 'PSEC1 CHAN 3',\
"./Results/Data_Oscope_b0.txt" using 1:11 lw 2 with lines title 'PSEC1 CHAN 4',\
"./Results/Data_Oscope_b0.txt" using 1:12 lw 2 with lines title 'PSEC1 CHAN 5',\
"./Results/Data_Oscope_b0.txt" using 1:13 lw 2 with lines title 'PSEC1 CHAN 6',\

set terminal wxt 2 title 'PSEC2&3-B0' size 500,500 background rgb 'grey75' noraise 
set key on outside  
plot\
"./Results/Data_Oscope_b0.txt" using 1:14 lw 2 with lines title 'PSEC2 CHAN 1',\
"./Results/Data_Oscope_b0.txt" using 1:15 lw 2 with lines title 'PSEC2 CHAN 2',\
"./Results/Data_Oscope_b0.txt" using 1:16 lw 2 with lines title 'PSEC2 CHAN 3',\
"./Results/Data_Oscope_b0.txt" using 1:17 lw 2 with lines title 'PSEC2 CHAN 4',\
"./Results/Data_Oscope_b0.txt" using 1:18 lw 2 with lines title 'PSEC2 CHAN 5',\
"./Results/Data_Oscope_b0.txt" using 1:19 lw 2 with lines title 'PSEC2 CHAN 6',\
"./Results/Data_Oscope_b0.txt" using 1:20 lw 2 with lines title 'PSEC3 CHAN 1',\
"./Results/Data_Oscope_b0.txt" using 1:21 lw 2 with lines title 'PSEC3 CHAN 2',\
"./Results/Data_Oscope_b0.txt" using 1:22 lw 2 with lines title 'PSEC3 CHAN 3',\
"./Results/Data_Oscope_b0.txt" using 1:23 lw 2 with lines title 'PSEC3 CHAN 4',\
"./Results/Data_Oscope_b0.txt" using 1:24 lw 2 with lines title 'PSEC3 CHAN 5',\
"./Results/Data_Oscope_b0.txt" using 1:25 lw 2 with lines title 'PSEC3 CHAN 6',\

set terminal wxt 3 title 'PSEC4-B0 & PSEC0-b1' size 500,500 background rgb 'grey75' noraise 
set key on outside  
plot\
"./Results/Data_Oscope_b0.txt" using 1:26 lw 2 with lines title 'PSEC4 CHAN 1',\
"./Results/Data_Oscope_b0.txt" using 1:27 lw 2 with lines title 'PSEC4 CHAN 2',\
"./Results/Data_Oscope_b0.txt" using 1:28 lw 2 with lines title 'PSEC4 CHAN 3',\
"./Results/Data_Oscope_b0.txt" using 1:29 lw 2 with lines title 'PSEC4 CHAN 4',\
"./Results/Data_Oscope_b0.txt" using 1:30 lw 2 with lines title 'PSEC4 CHAN 5',\
"./Results/Data_Oscope_b0.txt" using 1:31 lw 2 with lines title 'PSEC4 CHAN 6',\
"./Results/Data_Oscope_b1.txt" using 1:2 lw 2 with lines title 'b1 PSEC0 CHAN 1',\
"./Results/Data_Oscope_b1.txt" using 1:3 lw 2 with lines title 'b1 PSEC0 CHAN 2',\
"./Results/Data_Oscope_b1.txt" using 1:4 lw 2 with lines title 'b1 PSEC0 CHAN 3',\
"./Results/Data_Oscope_b1.txt" using 1:5 lw 2 with lines title 'b1 PSEC0 CHAN 4',\
"./Results/Data_Oscope_b1.txt" using 1:6 lw 2 with lines title 'b1 PSEC0 CHAN 5',\
"./Results/Data_Oscope_b1.txt" using 1:7 lw 2 with lines title 'b1 PSEC0 CHAN 6',\

set terminal wxt 4 title 'PSEC1&2-b1' size 500,500 background rgb 'grey75' noraise 
set key on outside  
plot\
"./Results/Data_Oscope_b1.txt" using 1:8 lw 2 with lines title 'b1 PSEC1 CHAN 1',\
"./Results/Data_Oscope_b1.txt" using 1:9 lw 2 with lines title 'b1 PSEC1 CHAN 2',\
"./Results/Data_Oscope_b1.txt" using 1:10 lw 2 with lines title 'b1 PSEC1 CHAN 3',\
"./Results/Data_Oscope_b1.txt" using 1:11 lw 2 with lines title 'b1 PSEC1 CHAN 4',\
"./Results/Data_Oscope_b1.txt" using 1:12 lw 2 with lines title 'b1 PSEC1 CHAN 5',\
"./Results/Data_Oscope_b1.txt" using 1:13 lw 2 with lines title 'b1 PSEC1 CHAN 6',\
"./Results/Data_Oscope_b1.txt" using 1:14 lw 2 with lines title 'b1 PSEC2 CHAN 1',\
"./Results/Data_Oscope_b1.txt" using 1:15 lw 2 with lines title 'b1 PSEC2 CHAN 2',\
"./Results/Data_Oscope_b1.txt" using 1:16 lw 2 with lines title 'b1 PSEC2 CHAN 3',\
"./Results/Data_Oscope_b1.txt" using 1:17 lw 2 with lines title 'b1 PSEC2 CHAN 4',\
"./Results/Data_Oscope_b1.txt" using 1:18 lw 2 with lines title 'b1 PSEC2 CHAN 5',\
"./Results/Data_Oscope_b1.txt" using 1:19 lw 2 with lines title 'b1 PSEC2 CHAN 6',\

set terminal wxt 5 title 'PSEC3&4-b1' size 500,500 background rgb 'grey75' noraise 
set key on outside  
plot\
"./Results/Data_Oscope_b1.txt" using 1:20 lw 2 with lines title 'b1 PSEC3 CHAN 1',\
"./Results/Data_Oscope_b1.txt" using 1:21 lw 2 with lines title 'b1 PSEC3 CHAN 2',\
"./Results/Data_Oscope_b1.txt" using 1:22 lw 2 with lines title 'b1 PSEC3 CHAN 3',\
"./Results/Data_Oscope_b1.txt" using 1:23 lw 2 with lines title 'b1 PSEC3 CHAN 4',\
"./Results/Data_Oscope_b1.txt" using 1:24 lw 2 with lines title 'b1 PSEC3 CHAN 5',\
"./Results/Data_Oscope_b1.txt" using 1:25 lw 2 with lines title 'b1 PSEC3 CHAN 6',\
"./Results/Data_Oscope_b1.txt" using 1:26 lw 2 with lines title 'b1 PSEC4 CHAN 1',\
"./Results/Data_Oscope_b1.txt" using 1:27 lw 2 with lines title 'b1 PSEC4 CHAN 2',\
"./Results/Data_Oscope_b1.txt" using 1:28 lw 2 with lines title 'b1 PSEC4 CHAN 3',\
"./Results/Data_Oscope_b1.txt" using 1:29 lw 2 with lines title 'b1 PSEC4 CHAN 4',\
"./Results/Data_Oscope_b1.txt" using 1:30 lw 2 with lines title 'b1 PSEC4 CHAN 5',\
"./Results/Data_Oscope_b1.txt" using 1:31 lw 2 with lines title 'b1 PSEC4 CHAN 6',\


pause 0.5
reread

