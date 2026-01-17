CC = gcc
CFLAGS = -Wall -I./src -I./src/cJSON
LDFLAGS = -lcurl

SRC_DIR = ../src
SERVER_DIR = $(SRC_DIR)/server
SERVICES_DIR = $(SRC_DIR)/services
CJSON_DIR = $(SRC_DIR)/cJSON

SOURCES = $(SRC_DIR)/driver.c \
          $(SERVER_DIR)/server.c \
		  $(SERVICES_DIR)/speedtest.c \
          $(CJSON_DIR)/cJSON.c \

TARGET = speedtest

default: clean build

build: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: default clean