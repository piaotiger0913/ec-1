
CC := gcc
CXX := g++
CFLAGS += -Isrc -Wall -std=gnu99
CXXFLAGS += -Isrc -std=c++0x -Wall -Wno-sign-compare \
    -Wno-unused-local-typedefs -Winit-self -rdynamic \
    -DHAVE_POSIX_MEMALIGN
LDLIBS += -ldl -lm -Wl,-Bstatic -Wl,-Bdynamic -lrt -lpthread \
    -lasound $(shell pkg-config --libs speexdsp)


# Set optimization level.
CFLAGS += -O3
CXXFLAGS += -O3


COMMON_OBJ = src/audio.o src/fifo.o src/pa_ringbuffer.o src/util.o
COMMON_OBJ1 = cal_corr/cal_corr_data.o cal_corr/rt_nonfinite.o cal_corr/rtGetNaN.o cal_corr/rtGetInf.o cal_corr/cal_corr_initialize.o cal_corr/cal_corr_terminate.o cal_corr/cal_corr.o cal_corr/fft.o cal_corr/FFTImplementationCallback.o cal_corr/cal_corr_emxutil.o cal_corr/cal_corr_emxAPI.o
EC_OBJ = $(COMMON_OBJ) src/ec.o
EC_LOOPBACK_OBJ = $(COMMON_OBJ) src/ec_hw.o
TESTECHO_OBJ = src/testecho.o
CALDELAY_OBJ = $(COMMON_OBJ1) cal_corr/examples/caldelay.o
all: ec ec_hw testecho caldelay

ec: $(EC_OBJ)
	$(CXX) $(EC_OBJ) $(LDLIBS) -o ec

ec_hw: $(EC_LOOPBACK_OBJ)
	$(CXX) $(EC_LOOPBACK_OBJ) $(LDLIBS) -o ec_hw
testecho: $(TESTECHO_OBJ)
	$(CXX) $(TESTECHO_OBJ) $(LDLIBS) -o testecho
caldelay: $(CALDELAY_OBJ)
	$(CXX) $(CALDELAY_OBJ) $(LDLIBS) -o caldelay

clean:
	-rm -f src/*.o ec ec_hw testecho caldelay
