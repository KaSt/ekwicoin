#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/ekwicoin.png
ICON_DST=../../src/qt/res/icons/ekwicoin.ico
convert ${ICON_SRC} -resize 16x16 ekwicoin-16.png
convert ${ICON_SRC} -resize 32x32 ekwicoin-32.png
convert ${ICON_SRC} -resize 48x48 ekwicoin-48.png
convert ekwicoin-16.png ekwicoin-32.png ekwicoin-48.png ${ICON_DST}

