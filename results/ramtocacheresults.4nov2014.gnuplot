load "linespointsstyle.gnuplot"

! ./projection4nov2004.py > tmp.data

set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey
set grid back linestyle 81
set term pdfcairo fontscale 0.75
#set term png fontscale 0.6
set xlabel "bits per integer"
set ylabel "decoding speed (mis)"

set xrange [8:15.5]
set key top right


set out "ramtocacheresults.4nov2014.pdf"


plot "tmp.data" u 1:2 title "Masked VByte" with linespoints ls 1, "" u 1:4 title " VByte" with linespoints  ls 2



set out "ramtocacheresults.4nov2014.ratio.pdf"
set key  bottom

set yrange [1.9:4]
set ylabel "relative speed"


plot "tmp.data" u 1:($2/$4) title "Masked VByte vs.  VByte" with linespoints ls 4


