CC := gcc

# Targets
TARGETS = iot_client iot_client_sql iot_server

# Object files
OBJECTS = $(TARGETS:%=%.o)

# Flags
LDFLAGS = -D_REENTRANT -pthread
LDFLAGS_SQL = -D_REENTRANT -pthread -lmysqlclient

# Default rule
all: $(TARGETS)

# Link rules
iot_client: iot_client.o
	$(CC) -o $@ $^ $(LDFLAGS)

iot_client_sql: iot_client_sql.o
	$(CC) -o $@ $^ $(LDFLAGS_SQL)

iot_server: iot_server.o
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile rule
%.o: %.c
	$(CC) -c -o $@ $<

# Clean rule
clean:
	rm -f *.o $(TARGETS)

