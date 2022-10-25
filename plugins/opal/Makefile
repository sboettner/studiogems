#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = OpalChorus

# --------------------------------------------------------------
# Files to build

FILES_DSP = PluginOpal.cpp

FILES_UI = UIOpal.cpp

# --------------------------------------------------------------
# Do some magic

UI_TYPE = generic
FILE_BROWSER_DISABLED = true
include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Extra flags

BASE_FLAGS += -pthread
LINK_FLAGS += -pthread

# --------------------------------------------------------------
# Enable all possible plugin types

all: ladspa dssi

# --------------------------------------------------------------
