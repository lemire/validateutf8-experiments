load "linespointsstyle.gnuplot"


set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey
set grid back linestyle 81
set term pdfcairo fontscale 0.75
#set term png fontscale
set xlabel "input size (KB)"
set ylabel "speed (GB/s)"

set xrange [:48]
set yrange [0:14]
set key center right


set out "skylake.pdf"

plot "skylake1000.txt" using ($1/1000):7 title "branchy" ls 1 w lines,\
 "" using ($1/1000):10 title "finite-state"  ls 2  w lines,\
 "" using ($1/1000):13  title "lookup"  ls 3  w lines

set yrange [0:350]

set out "skylake_mis.pdf"
set ylabel "mispredicted branch (/KB)"

plot "skylake1000.txt" using ($1/1000):6 title "branchy" ls 1 w lines,\
 "" using ($1/1000):9 title "finite-state"  ls 2  w lines,\
 "" using ($1/1000):12  title "lookup"  ls 3  w lines
