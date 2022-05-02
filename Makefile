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

ROCKSDB_LIB = $(ROCKS_DIR)/build/librocksdb.a
ROCKSDB_TOOLS = tools

CC = g++

all: $(TARGET) $(ROCKSDB_TOOLS)

-include $(DEPS)

$(TARGET): $(OBJS) $(ROCKSDB_LIB)
	$(CC) $(INCLUDE) -L $(ROCKS_DIR)/build -o $@ $^ $(FINAL_CFLAGS) $(LDLIBS)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if [ ! -e $(OBJ_DIR) ] ; then mkdir $(OBJ_DIR) ; fi
	$(CC) $(INCLUDE) -o $@ $< $(CFLAGS) $(LDLIBS)

rocksdb:
	cd src/rocksdb && mkdir -p build && cd build && \
	cmake -DWITH_TESTS=off .. && make -j4

$(ROCKSDB_LIB): rocksdb

$(ROCKSDB_TOOLS): rocksdb
	ln -sf $(ROCKS_DIR)/build/tools $@

clean:
	rm -f $(TARGET) $(OBJ_DIR)/* $(SRC_DIR)/*~ $(INC_DIR)/*~ ./*~ $(ROCKSDB_TOOLS)
	rm -rf $(ROCKS_DIR)/build

.PHONY: all clean rocksdb
