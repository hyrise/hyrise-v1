set autoscale
set xtic auto
set ytic auto

set title "Aggregate Scan"
set xlabel "Table Type"
set ylabel "Time in µs"

sw(x,S)=1/(x*x*S)

plot "data3.dat" using 1:4 title 'Aggregate Scan sum(18)' with linespoints