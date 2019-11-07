PREFIX = $(MIX_APP_PATH)/priv
OBJDIR  = $(MIX_APP_PATH)/obj
TARGET = $(PREFIX)/irex_nif.so

CFLAGS ?= -O2 -Wall -Wextra -Wno-unused-parameter -pedantic -fPIC
LDFLAGS += -fPIC -shared

ERL_CFLAGS ?= -I$(ERL_EI_INCLUDE_DIR)
ERL_LDFLAGS ?= -L$(ERL_EI_LIBDIR) -lei

SRC = src/receiver.c src/irex_nif.c
HEADERS = $(wildcard src/*.h)
OBJ = $(SRC:src/%.c=$(OBJDIR)/%.o)

.PHONY: all clean default

default:
	mix compile

all: $(PREFIX) $(OBJDIR) $(TARGET)

$(OBJ): $(HEADERS) Makefile

$(OBJDIR)/%.o: src/%.c
	$(CC) -c $(ERL_CFLAGS) $(CFLAGS) -o $@ $<

$(TARGET): $(OBJ)
	$(CC) -o $@ $(ERL_LDFLAGS) $(LDFLAGS) $^

$(PREFIX):
	mkdir -p $@

$(OBJDIR):
	mkdir -p $@

clean:
	$(RM) $(TARGET) $(OBJDIR)/*.o

