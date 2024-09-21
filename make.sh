rm *.gb
../gbdk/bin/lcc -c -o Go.o Go.c
../gbdk/bin/lcc -c -o square.o square.c
../gbdk/bin/lcc -Wa-l -Wl-m -Wl-yt3 -Wl-yo4 -Wl-ya4 -Wm-yC -o Go.gb Go.o square.o
rm *.asm
rm *.lst
rm *.ihx
rm *.sym
rm *.o