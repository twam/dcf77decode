#!/usr/bin/env python
import sys, time
from PyQt4 import QtCore, QtGui
import pyqtgraph
import numpy
import serial
import sys
import re

class DCF77DebugApp(QtGui.QWidget):
    # DCF77_Demodulator
    DCF77_Demodulator_Phase_Value = 0
    DCF77_Demodulator_Tick_Value = 0
    DCF77_Demodulator_MaxIndex_Value = 0
    DCF77_Demodulator_Bins_Value = []

    # DCF77_Second_Decoder
    DCF77_Second_Decoder_Second_Value = 0
    DCF77_Second_Decoder_Phase_Value = 0
    DCF77_Second_Decoder_Tick_Value = 0
    DCF77_Second_Decoder_MaxIndex_Value = 0
    DCF77_Second_Decoder_Bins_Value = []

    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)

        self.setGeometry(100, 100, 1600, 600)
        self.setWindowTitle('DCF77 Debug')


        pyqtgraph.setConfigOption('background', 'w')
        pyqtgraph.setConfigOption('foreground', 'k')

        # Pens
        penBlue = pyqtgraph.mkPen('b', style=QtCore.Qt.SolidLine)
        penRed = pyqtgraph.mkPen('r', style=QtCore.Qt.SolidLine)
        penGreen = pyqtgraph.mkPen('g', style=QtCore.Qt.SolidLine)

        # DCF77_Demodulator
        self.DCF77_Demodulator_Plot_Label = QtGui.QLabel()
        self.DCF77_Demodulator_Plot_Label.setText('DCF77_Demodulator')
        self.DCF77_Demodulator_Plot = pyqtgraph.PlotWidget()
        self.DCF77_Demodulator_Bins_Curve = self.DCF77_Demodulator_Plot.plot(pen = penBlue)
        self.DCF77_Demodulator_Tick_Line = pyqtgraph.InfiniteLine(pen = penRed)
        self.DCF77_Demodulator_Plot.addItem(self.DCF77_Demodulator_Tick_Line)
        self.DCF77_Demodulator_MaxIndex_Line = pyqtgraph.InfiniteLine(pen = penGreen)
        self.DCF77_Demodulator_Plot.addItem(self.DCF77_Demodulator_MaxIndex_Line)

        # DCF77_Second_Decoder
        self.DCF77_Second_Decoder_Plot_Label = QtGui.QLabel()
        self.DCF77_Second_Decoder_Plot_Label.setText('DCF77_Second_Decoder')
        self.DCF77_Second_Decoder_Plot = pyqtgraph.PlotWidget()
        self.DCF77_Second_Decoder_Bins_Curve = self.DCF77_Second_Decoder_Plot.plot(pen = penBlue)
        self.DCF77_Second_Decoder_Tick_Line = pyqtgraph.InfiniteLine(pen = penRed)
        self.DCF77_Second_Decoder_Plot.addItem(self.DCF77_Second_Decoder_Tick_Line)

        # Layout Manager
        self.layout = QtGui.QVBoxLayout(self)
        self.layout.addWidget(self.DCF77_Demodulator_Plot_Label)
        self.layout.addWidget(self.DCF77_Demodulator_Plot)
        self.layout.addWidget(self.DCF77_Second_Decoder_Plot_Label)
        self.layout.addWidget(self.DCF77_Second_Decoder_Plot)

        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update)
        self.timer.start(10)

        # Serial connection thread
        self.serialConnectionThread = SerialConnectionThread()
        self.connect(self.serialConnectionThread, QtCore.SIGNAL('DCF77_Demodulator_Signal'), self.DCF77_Demodulator_Update)
        self.connect(self.serialConnectionThread, QtCore.SIGNAL('DCF77_Second_Decoder_Signal'), self.DCF77_Second_Decoder_Update)
        self.serialConnectionThread.start()

    def DCF77_Demodulator_Update(self, phase, tick, maxIndex, bins):
        self.DCF77_Demodulator_Phase_Value = phase
        self.DCF77_Demodulator_Tick_Value = tick
        self.DCF77_Demodulator_MaxIndex_Value = maxIndex
        self.DCF77_Demodulator_Bins_Value = bins

    def DCF77_Second_Decoder_Update(self, second, phase, tick, maxIndex, bins):
        self.DCF77_Second_Decoder_Second_Value = second
        self.DCF77_Second_Decoder_Phase_Value = phase
        self.DCF77_Second_Decoder_Tick_Value = tick
        self.DCF77_Second_Decoder_MaxIndex_Value = maxIndex
        self.DCF77_Second_Decoder_Bins_Value = bins

    def update(self):
        # DCF77_Demodulator
        self.DCF77_Demodulator_Bins_Curve.setData(y=numpy.array(self.DCF77_Demodulator_Bins_Value))
        self.DCF77_Demodulator_Tick_Line.setValue(self.DCF77_Demodulator_Tick_Value)
        self.DCF77_Demodulator_MaxIndex_Line.setValue(self.DCF77_Demodulator_MaxIndex_Value)

        # DCF77_Second_Decoder
        self.DCF77_Second_Decoder_Bins_Curve.setData(y=numpy.array(self.DCF77_Second_Decoder_Bins_Value))
        self.DCF77_Second_Decoder_Tick_Line.setValue(self.DCF77_Second_Decoder_Tick_Value)

class SerialConnectionThread(QtCore.QThread):
    def __init__(self):
        QtCore.QThread.__init__(self)

    def __del__(self):
        self.port.close()
        self.wait()

    def run(self):
        self.port = serial.Serial(port ='/dev/tty.usbserial-FTGNNGRP', baudrate=115200, timeout=1)

        while (True):
            line = self.port.readline().strip()

            if line[0:17] == 'DCF77_Demodulator':
                matches = re.match('^DCF77_Demodulator: Phase: (0x[0-9A-Z]+) Tick: ([0-9]+) Quality: ([0-9]+)-([0-9]+) MaxIndex: ([0-9]+) > (.*)$', line)
                if matches:
                    phase = int(matches.group(1),16)
                    tick = int(matches.group(2))
                    quality1 = int(matches.group(3))
                    quality2 = int(matches.group(4))
                    maxIndex = int(matches.group(5))
                    bins = [int(x,16) for x in re.findall('(0x[0-9A-F]+)', matches.group(6))]
                    self.emit(QtCore.SIGNAL('DCF77_Demodulator_Signal'), phase, tick, maxIndex, bins)

                else:
                    print 'no match:', line

            elif line[0:20] == 'DCF77_Second_Decoder':

                matches = re.match('^DCF77_Second_Decoder: Second: ([0-9]+) SyncMarkIndex: (0x[0-9A-Z]+) Tick: ([0-9]+) Quality: ([0-9]+)-([0-9]+) MaxIndex: ([0-9]+) > (.*)$', line)
                if matches:
                    second = int(matches.group(1))
                    synxmarkindex = int(matches.group(2),16)
                    tick = int(matches.group(3))
                    quality1 = int(matches.group(4))
                    quality2 = int(matches.group(5))
                    maxindex = int(matches.group(6))
                    bins = [int(x,16) for x in re.findall('(0x[0-9A-F]+)', matches.group(7))]

                    self.emit(QtCore.SIGNAL('DCF77_Second_Decoder_Signal'), second, phase, tick, maxIndex, bins)

                else:
                    print 'no match:', line
            else:
                print 'RAW:', line

            # if len(data_split) >= 2:
            #     try:
            #         self.emit( QtCore.SIGNAL('update(double,double)'), float(data_split[0]),float(data_split[1]) )
            #     except:
            #         continue

        return

# run
app = QtGui.QApplication(sys.argv)
dcf77debugapp = DCF77DebugApp()
dcf77debugapp.show()
dcf77debugapp.raise_()
app.exec_()


