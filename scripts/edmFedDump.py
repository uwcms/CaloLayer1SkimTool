#!/usr/bin/env python
# Nick Smith <nick.smith@cern.ch>
# coding: utf-8
import argparse

parser = argparse.ArgumentParser(description='Reads EDM files and dumps raw data from list of FEDs to text files.')
parser.add_argument('--input', '-i', help='EDM File (Note, only reads first event in file, so use edmPickEvents to get right event)', required=True)
parser.add_argument('--output', '-o', help='Output file prefix (will be appended with "_fedN.txt" for N FEDs dumped)', required=True)
parser.add_argument('--feds', '-f', help='List of FEDs to dump, will write one text file per FED', required=True)
parser.add_argument('--label', help='Label in EDM file of FEDRawDataCollection', default='rawDataCollector')
parser.add_argument('--event', help='Event offset in file', default=0)
args = parser.parse_args()

import os
if not 'CMSSW_BASE' in os.environ :
    print 'Error: I need a CMSSW environment available to run!'
    exit(1)

import ROOT
ROOT.gROOT.SetBatch(True)
ROOT.PyConfig.IgnoreCommandLineOptions = True
from DataFormats.FWLite import Handle, Events

fedHandle = Handle('FEDRawDataCollection')

events = Events(args.input)

if events.size() > 1 :
    print 'Warning, this script is designed for pre-picked EDM files (i.e. 1 event!)'

events.to(args.event)
if not events.getByLabel(args.label, fedHandle) :
    print "Sorry, I couldn't load the FED collection for event %d :<" % args.event
    exit(1)

fedsToPrint = map(int, args.feds.split(','))
for iFed in fedsToPrint :
    fedEDMObject = fedHandle.product().FEDData(iFed)
    fedData = fedEDMObject.data() # no worries about executor for unknown type ...
    fedData.SetSize(fedEDMObject.size()/8) # size in bytes, we read in long (8 bytes)
    with open(args.output+'_fed%d.txt'%iFed,'w') as fout :
        for word in fedData :
            # Signed to unsigned word conversion
            fout.write('%016x\n' % (word & (2**64-1)))

