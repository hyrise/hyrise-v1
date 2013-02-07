set autoscale
set xtic auto
set ytic auto

set title "Projection"
set xlabel "Position"
set zlabel "Time in µs"
set ylabel "Width"

set ticslevel 0


splot "floating.data"  title 'Full Table Scan' with linespoints
