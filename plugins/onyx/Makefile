#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = onyx

# --------------------------------------------------------------
# Files to build

FILES_DSP = PluginOnyx.cpp

FILES_UI = UIOnyx.cpp

# --------------------------------------------------------------
# Do some magic

UI_TYPE = cairo
FILE_BROWSER_DISABLED = true
include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Extra flags

#BASE_FLAGS += -pthread
#LINK_FLAGS += -pthread

BUILD_CXX_FLAGS += -I../../ui
LINK_FLAGS += -L../../build/ui -lui

BUILD_CXX_FLAGS += `pkg-config --cflags pangocairo`
LINK_FLAGS += `pkg-config --libs pangocairo`

LINK_FLAGS += `pkg-config --libs fontconfig`

# --------------------------------------------------------------
# Enable all possible plugin types

all: dssi

$(dssi_ui): ../../build/ui/libui.a

# --------------------------------------------------------------
