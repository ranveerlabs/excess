# dumps the R, lines to a csv. set print_raw in csi_rx.c first or you get nothing
import sys, serial, time

port = sys.argv[1] if len(sys.argv) > 1 else 'COM5'
out  = sys.argv[2] if len(sys.argv) > 2 else 'cap_%d.csv' % int(time.time())

s = serial.Serial(port, 921600, timeout=1)
f = open(out, 'w')
n = 0
try:
    while True:
        l = s.readline().decode('utf8', 'ignore').strip()
        if not l.startswith('R,'):
            if l: print(l)      # log lines, let them thru
            continue
        f.write(l + '\n')
        n += 1
        if n % 200 == 0:
            print(n, end='\r')
            f.flush()
except KeyboardInterrupt:
    pass
f.close()
print('\n', n, 'rows ->', out)
