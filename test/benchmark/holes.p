set autoscale
set xtic auto
set ytic auto

set title "Projection"
set xlabel "Holes"
set zlabel "Time in µs"
set ylabel "Block Size"

set ticslevel 0


splot "holes.data"  title 'Full Table Scan'
