# Makefile

CFLAGS = -c -g -O3 -Wall -std=c++17 -flto -MMD -MP
FINAL_CFLAGS = -g -O3 -Wall -std=c++17 -flto
LDLIBS = -lboost_program_options -lrocksdb -lpthread

TARGET = rocks_eval

SRC_DIR = ./src
INC_DIR = ./include
ROCKS_DIR = ./src/rocksdb
ROCKS_INC_DIR = $(ROCKS_DIR)/include
OBJ_DIR = ./obj

SRCS = $(shell ls $(SRC_DIR)/*.cpp)
OBJS = $(subst $(SRC_DIR), $(OBJ_DIR), $(SRCS:.cpp=.o))
DEPS = $(OBJS:.o=.d)

INCLUDE = -I $(INC_DIR) -I $(ROCKS_INC_DIR)

ROCKSDBLIB = $(ROCKS_DIR)/build/librocksdb.a

CC = g++

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS) $(ROCKSDBLIB)
	$(CC) $(INCLUDE) -L $(ROCKS_DIR)/build -o $@ $^ $(FINAL_CFLAGS) $(LDLIBS)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if [ ! -e $(OBJ_DIR) ] ; then mkdir $(OBJ_DIR) ; fi
	$(CC) $(INCLUDE) -o $@ $< $(CFLAGS) $(LDLIBS)

$(ROCKSDBLIB):
	cd src/rocksdb && mkdir -p build && cd build \
	cmake .. && make -j4

clean:
	rm -f $(TARGET) $(OBJ_DIR)/* $(SRC_DIR)/*~ $(INC_DIR)/*~ ./*~
	rm -f $(ROCKS_DIR)/build

.PHONY: all clean
