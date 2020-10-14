#!/bin/sh

echo ""
echo "Start kompilacji programu SPP"
gcc -o UH UH.c pedniki.c inicjalizacja.c WKBP.c testy.c \
-lpthread -lrt -lm
echo  "Zrobione!"
echo ""

