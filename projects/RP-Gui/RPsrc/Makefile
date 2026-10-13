CFLAGS  = -Wall -std=c++20 ## -Werror
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -static -lrp-hw-can -lrp -lrp-hw-calib -lrp-hw-profiles

INCLUDE += -I/opt/redpitaya/include/api250-12
LDLIBS += -lrp-gpio -lrp-i2c
LDLIBS += -lrp-hw -lm -lstdc++ -lpthread -li2c -lsocketcan

# List of programs to compile
PRGS = WriteCoefficients

OBJS := $(patsubst %,%.o,$(PRGS))
SRC := $(patsubst %,%.cpp,$(PRGS))

all: $(PRGS)

$(PRGS): %: %.cpp
	$(CXX) $< $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	$(RM) *.o
	$(RM) $(OBJS)

clean_all: clean
	$(RM) $(PRGS)