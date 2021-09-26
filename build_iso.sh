#!/bin/bash

make && mkpsxiso -y buzzy.xml && pcsx-redux -run -iso buzzy.cue
