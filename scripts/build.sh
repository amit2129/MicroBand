rm -f microband
rm -rf /tmp/microband
mkdir /tmp/microband

cp infiniband/* /tmp/microband/
cp MicroBand.ino /tmp/microband/microband.c
gcc /tmp/microband/*.c -o microband
echo "run with ./microband"
