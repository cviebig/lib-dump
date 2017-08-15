#!/bin/sh
bin/dumpTest
lualatex tikz.tex
lualatex tikz_sparse.tex
lualatex forest.tex
lualatex forest_sparse.tex
convert -density 150 tikz.pdf -quality 90 tikz.png
convert -density 150 tikz_sparse.pdf -quality 90 tikz_sparse.png
convert -density 150 forest.pdf -quality 90 forest.png
convert -density 150 forest_sparse.pdf -quality 90 forest_sparse.png
