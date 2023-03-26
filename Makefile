CC=gcc
COMMDIR=common
CFLAGS=-I$(COMMDIR) -lpthread
CLIENT_DIR=/client/
DEPS=$(COMMDIR)/common.h
OBJ= $(COMMDIR)/common.o $(CLIENT_DIR)/ftp_client.o

all: client server

common.o:
	$(CC) -c -o $(COMMDIR)/common.o $(COMMDIR)/common.c

client: common.o
	$(CC) -o client/ftp_client client/ftp_client.c $(COMMDIR)/common.o -lpthread

server: common.o
	$(CC) -o client/ftp_server server/ftp_server.c $(COMMDIR)/common.o -lpthread

.PHONY: clean
clean:
	rm common/common.o client/ftp_client server/ftp_server