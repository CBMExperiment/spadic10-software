import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui, QtCore
import Queue
import sys
import threading
import time

INF = float('inf')

from spadic import SpadicDataClient, SpadicControlClient

class SpadicDataReader:
    def __init__(self, host):
        self.dataA_client = SpadicDataClient('A', host)
        self.dataB_client = SpadicDataClient('B', host)
        self.ctrl_client = SpadicControlClient(host)

        self.period = 25-3#100e-3
        self.dlm_sent = False

        # data, mask, expiration date
        self.data_buffer = [Queue.Queue() for _ in range(32)]
        self.data_expires = [-INF for _ in range(32)]
        self.last_data = [Queue.Queue(maxsize=1) for _ in range(32)]
        self._stop = threading.Event()
        self.groupA_reader = threading.Thread(name="groupA_reader")
        self.groupB_reader = threading.Thread(name="groupB_reader")
        self.groupA_reader.run = self._read_group_task('A')
        self.groupB_reader.run = self._read_group_task('B')
        self.groupA_reader.daemon = True
        self.groupB_reader.daemon = True
        self.groupA_reader.start()
        self.groupB_reader.start()

    def _read_group_task(self, group):
        readmethod = {'A': self.dataA_client.read_message,
                      'B': self.dataB_client.read_message}[group]
        def read_group_task():
            while not self._stop.is_set():
                m = readmethod(timeout=self.period)
                if not m or m.channel_id is None:
                    #self.ctrl_client.send_dlm(11)
                    #self.dlm_sent = True
                    print "reader: no data"
                    continue
                t = time.time()
                c = m.channel_id + {'A': 0, 'B': 16}[group]
                if t < self.data_expires[c]:
                    print "reader: data still valid"
                    continue
                mask = self.ctrl_client.control.hitlogic.read()['mask']
                if self.dlm_sent:
                    self.data_expires[c] = t
                    self.dlm_sent = False
                else:
                    self.data_expires[c] += self.period
                if self.last_data[c].full():
                    try:
                        self.last_data[c].get(block=False)
                    except Queue.Empty:
                        pass
                self.last_data[c].put((m.data(), mask))
                print "reader: put data"
        return read_group_task

    def __enter__(self):
        return self

    def __exit__(self, *args, **kwargs):
        if not self._stop.is_set():
            self._stop.set()
        for t in [self.groupA_reader, self.groupB_reader]:
            while t.is_alive():
                t.join(timeout=1)


def mask_to_x(mask):
    """
    Convert the encoded 32 bit mask to x values.

    Example: 0xF -> [28, 29, 30, 31]
    """
    return [31-i for i in reversed(range(32)) if (mask>>i)&1]

class SpadicDataMonitor:
    def __init__(self, spadic_data_reader):
        self.reader = spadic_data_reader
        self.period = 10e-3
        #self.fig = plt.figure()
        #ani_options = {'func':      self.plot_last,
        #               'frames':    self.gen,
        #               'init_func': self.plot_init,
        #               'blit':      False,
        #               'interval':  1, # milliseconds!
        #               'repeat':    False}
        #ani = animation.FuncAnimation(self.fig, **ani_options)
        #print "Press CTRL-C to exit."
        #self.fig.show()

        self.app = QtGui.QApplication([])
        self.win = pg.GraphicsWindow()#title="Basic plotting examples")
        self.win.resize(1000,600)
        self.win.setWindowTitle("SPADIC Data Monitor")

        self.plot = self.win.addPlot()
        self.curve = self.plot.plot(pen="y")
        

    def run(self):
        #QtGui.QApplication.instance().exec_()
        timer = QtCore.QTimer()
        timer.timeout.connect(self.gen)
        timer.start(self.period*1000) # milliseconds
        self.app.exec_()

    #def plot_init(self):
    #    """Initialize the plot."""
    #    self.ax = self.fig.add_subplot(111)
    #    self.ax.set_ylim(-256, 256)
    #    self.ax.set_yticks(np.linspace(-256, 256, 9))
    #    self.ax.set_xlim(0, 32)
    #    self.ax.set_xticks(np.linspace(0, 32, 9))
    #    self.ax.grid(True)
    #    self.lines = self.ax.plot([], [])
    #    # not sure if this is needed and what it does
    #    #return self.ax.lines

    def gen(self):
        """Fetch the latest data."""
        channel = 31
        try:
            (y, mask) = self.reader.last_data[channel].get(block=False)#timeout=self.period)
        except Queue.Empty:
            print "monitor: no data"
            return
        #except KeyboardInterrupt:
        #    break
        x = mask_to_x(mask)
        #yield (x, y)
        self.curve.setData(x, y)

    #def plot_last(self, data):
    #    """Plot the latest data and discard old data."""
    #    # keep the 9 latest lines
    #    self.lines = self.lines[:9]
    #    for (i, line) in enumerate(self.lines):
    #        line.set_color([i*0.1]*3) # newer = darker
    #        line.set_linewidth(1)
    #    # plot the newest data
    #    x, y = data
    #    if len(x) != len(y):
    #        L = min(len(x), len(y))
    #        x = x[:L]
    #        y = y[:L]
    #    newline = plt.Line2D(x, y)
    #    newline.set_color([0.8, 0.1, 0.1])
    #    newline.set_linewidth(2)
    #    self.lines.insert(0, newline)
    #    # update plot
    #    self.ax.lines = []
    #    for line in reversed(self.lines): # oldest first, newest last
    #        self.ax.add_line(line)
    #    # not sure if this is needed and what it does
    #    #return self.ax.lines


if __name__=='__main__':
    host = sys.argv[sys.argv.index('--host')+1]
    with SpadicDataReader(host) as reader:
        mon = SpadicDataMonitor(reader)
        mon.run()

