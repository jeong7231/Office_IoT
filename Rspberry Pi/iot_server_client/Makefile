CC := gcc

# Targets
TARGETS = iot_client iot_client_sql iot_server iot_client_bluetooth

# Object files
OBJECTS = $(TARGETS:%=%.o)

# Flags
LDFLAGS = -D_REENTRANT -pthread
LDFLAGS_SQL = -D_REENTRANT -pthread -lmysqlclient
LDFLAGS_BLT = -D_REENTRANT -lbluetooth -pthread

# Default rule
all: $(TARGETS)

# Link rules
iot_client: iot_client.o
	$(CC) -o $@ $^ $(LDFLAGS)

iot_client_sql: iot_client_sql.o
	$(CC) -o $@ $^ $(LDFLAGS_SQL)

iot_server: iot_server.o
	$(CC) -o $@ $^ $(LDFLAGS)

iot_client_bluetooth: iot_client_bluetooth.o
	$(CC) -o $@ $^ $(LDFLAGS_BLT)

# Compile rule
%.o: %.c
	$(CC) -c -o $@ $<

# Clean rule
clean:
	rm -f *.o $(TARGETS)

