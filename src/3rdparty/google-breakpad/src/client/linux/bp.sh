rm -f *dmp
g++ -g -O2 -I../.. bp_test.cc -c
g++ -g bp_test.o libbreakpad.a -o bp_test

../../tools/linux/dump_syms/dump_syms bp_test > temp
DIR=$(head -n 1 temp|awk '{print "symbols/" $5 "/" $4}')
mkdir -p $DIR
mv temp $DIR/bp_test.sym

./bp_test

for i in *.dmp;
do
../../processor/minidump_stackwalk $i symbols
done
# ../../processor/minidump_dump

