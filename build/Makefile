CC = gcc
CFLAGS = -Wall -I./src -I./src/cJSON
LDFLAGS = -lcurl

SRC_DIR = ../src
SERVER_DIR = $(SRC_DIR)/server
SERVICES_DIR = $(SRC_DIR)/services
RUNTIME_DIR = $(SRC_DIR)/runtime
CJSON_DIR = $(SRC_DIR)/libs/cJSON

SOURCES = $(SRC_DIR)/driver.c \
          $(SERVER_DIR)/server.c \
		  $(SERVICES_DIR)/location.c \
		  $(SERVICES_DIR)/speedtest.c \
		  $(RUNTIME_DIR)/json_parser.c \
          $(CJSON_DIR)/cJSON.c \

TARGET = speedtest

default: clean build

build: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: default clean