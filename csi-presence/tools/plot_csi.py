import sys
import numpy as np
import matplotlib.pyplot as plt

# cols: R, t_ms, rssi, then 52 amplitudes
d = np.genfromtxt(sys.argv[1], delimiter=',', usecols=range(1, 55), invalid_raise=False)
t = (d[:, 0] - d[0, 0]) / 1000.0
amp = d[:, 2:]

# the ring in motion.c but offline so i can move the window without reflashing
W = 32
base = amp[:60].mean(axis=0)
dist = np.abs(amp - base).sum(axis=1) / base.sum() * 100

var = np.array([dist[max(0, i-W):i+1].std() for i in range(len(dist))])

fig, ax = plt.subplots(3, 1, sharex=True, figsize=(12, 8))
ax[0].imshow(amp.T, aspect='auto', origin='lower',
             extent=[t[0], t[-1], 0, amp.shape[1]])
ax[0].set_ylabel('subcarrier')
ax[1].plot(t, dist, lw=0.6)
ax[1].set_ylabel('dist')
ax[2].plot(t, var, lw=0.8)
ax[2].axhline(1.8, ls='--')
ax[2].axhline(1.1, ls='--')
ax[2].set_ylabel('var')
ax[2].set_xlabel('s')
plt.tight_layout()
plt.show()

# fft of dist to see if breathing shows up. it did not, or i was sat too far
# f = np.fft.rfft(dist - dist.mean())
# plt.plot(np.fft.rfftfreq(len(dist), 1/70.0), abs(f)); plt.xlim(0, 2); plt.show()
