set autoscale
set xtic auto
set ytic auto

set title "Projection"
set xlabel "Columns Selected"
set ylabel "Time in µs"

sw(x,S)=1/(x*x*S)

plot "projection.data" using 1:3 title 'Full Table Scan' with linespoints, \
"projection.data" using 1:2 title 'Projection' with linespoints
