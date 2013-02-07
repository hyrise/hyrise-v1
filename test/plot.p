set autoscale
set xtic auto
set ytic auto

set title "Full Table Scan"
set xlabel "Table Type"
set ylabel "Time in µs"

sw(x,S)=1/(x*x*S)

plot "data3.dat" using 1:2 title 'Full Table Scan' with linespoints