set autoscale
set xtic auto
set ytic auto

set title "Projection Scan"
set xlabel "Table Type"
set ylabel "Time in µs"

sw(x,S)=1/(x*x*S)

plot "data3.dat" using 1:3 title 'Projection Scan' with linespoints