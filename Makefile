PROG = hcc1.exe

all:$(PROG)

UNAME := $(shell uname)
ERROR_CPP = error.cpp
WARNING_CPP = warning.cpp
ifeq ($(LANG),ja_JP.eucJP)
	ERROR_CPP = error_euc.cpp
	WARNING_CPP = warning_euc.cpp
endif
ifeq ($(LANG),ja_JP.UTF-8)
	ERROR_CPP = error_utf.cpp
	WARNING_CPP = warning_utf.cpp
endif
ifneq (,$(findstring Darwin,$(UNAME)))
	ERROR_CPP = error_utf.cpp
	WARNING_CPP = warning_utf.cpp
endif

SRCS =	c_l.cpp \
	c_y.cpp \
	cmdline.cpp \
	decl.cpp \
	dump.cpp \
	$(ERROR_CPP) \
	expr0.cpp \
	expr1.cpp \
	expr2.cpp \
	expr3.cpp \
	expr4.cpp \
	expr5.cpp \
	expr6.cpp \
	generator.cpp \
	initializer.cpp \
	main.cpp \
	optimize.cpp \
	prof.cpp \
	scope.cpp \
	scope2.cpp \
	stmt.cpp \
	type.cpp \
	$(WARNING_CPP) \

error_euc.cpp:error.cpp
	sjis2euc.exe < $< > $@

error_utf.cpp:error_euc.cpp
	euc2utf.exe < $< > $@

warning_euc.cpp:warning.cpp
	sjis2euc.exe < $< > $@

warning_utf.cpp:warning_euc.cpp
	euc2utf.exe < $< > $@

OBJS = $(SRCS:.cpp=.o)

c_y.cpp:c.y
	./bison_script c.y

c_l.cpp:c.l
	./flex_script c.l

$(OBJS):c_y.cpp

DEBUG_FLAG = -g
#DEBUG_FLAG = -O3
CXXFLAGS = -w $(DEBUG_FLAG)

debian = $(if $(wildcard /etc/debian_version),1,0)
ifeq ($(debian),1)
  CXXFLAGS += -DDEBIAN
endif

DYNAMIC_LOADING_LIBRARY_FLAGS = -ldl
ifneq (,$(findstring CYGWIN,$(UNAME)))
	DYNAMIC_LOADING_LIBRARY_FLAGS =
endif

$(PROG):$(OBJS)
	$(CXX) $(DEBUG_FLAG) -o $(PROG) $(OBJS) $(DYNAMIC_LOADING_LIBRARY_FLAGS)

clean:
	$(RM) $(PROG) *.o gmon.out c_y.cpp* c_y.h* c_l.cpp* *~
	$(RM) error_euc.* warning_euc.* error_utf.* warning_utf.*
	$(RM) -r .vs x64 Debug Release

