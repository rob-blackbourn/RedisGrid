# Copyright (c) 2018, Rob Blackbourn
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# Set environment variable RM_INCLUDE_DIR to the location of redismodule.h
ifndef RM_INCLUDE_DIR
	RM_INCLUDE_DIR=$(shell pwd)
endif

# Setting the DEBUG env variable to 1 will cause us to build with -O0
ifndef DEBUG
	DEBUG = 0
endif
DEBUGFLAGS = -g -ggdb -O2
ifeq ($(DEBUG), 1)
	DEBUGFLAGS = -g -ggdb -O0
endif

# Find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
INCLUDES = -I"$(RM_INCLUDE_DIR)"
CFLAGS = $(INCLUDES) -Wall $(DEBUGFLAGS) -fPIC -std=gnu99  -D_GNU_SOURCE
CC:=$(shell sh -c 'type $(CC) >/dev/null 2>/dev/null && echo $(CC) || echo gcc')

# Compile flags for linux / osx
ifeq ($(uname_S),Linux)
	SHOBJ_CFLAGS ?=  -fno-common -g -ggdb
	SHOBJ_LDFLAGS ?= -shared -Bsymbolic -Bsymbolic-functions
else
	CFLAGS += -mmacosx-version-min=10.6
	SHOBJ_CFLAGS ?= -dynamic -fno-common -g -ggdb
	SHOBJ_LDFLAGS ?= -dylib -exported_symbol _RedisModule_OnLoad -macosx_version_min 10.6
endif
export CFLAGS
export DEBUGFLAGS

all: grid.so

grid.so: grid.o utils.o array_grid.o row_grid.o
	$(LD) -o $@ $^ $(LIBS) $(SHOBJ_LDFLAGS) -lc -lm

clean:
	rm -rvf *.so *.o

grid.c: utils.h array_grid.h row_grid.h
utils.c: utils.h
array_grid.c: array_grid.h utils.h
row_grid.c: row_grid.h utils.h
