#!/bin/sh

# TODO check if files were already patched !!

if [ -d $1 ]; then
  cd $1

  for file in *.html *.php ; do
    sed -e 's:></HEAD:><link rel="STYLESHEET" href="/css/default.css"></HEAD:g' $file > tmp
    sed -e 's:/docbook-dsssl/:../images/:g' tmp > tmp2
    rm tmp
    ../add_header_footer.pl tmp2 > $file
    rm tmp2
#    mv $file `basename $file .html`.php
  done
else
  sed -e 's:></HEAD:><link rel="STYLESHEET" href="/css/default.css"></HEAD:g' $file > tmp
  sed -e 's:/docbook-dsssl/:../images/:g' tmp > tmp2
  rm tmp
  mv tmp2 $file
  ../add_header_footer.pl tmp2 > $file
  rm tmp2
#  mv $file `basename $file .html`.php
fi
