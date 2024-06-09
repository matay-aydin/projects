CC = gcc
CFLAGS = -Wall -Werror -Wextra -g
SRCDIR = src
INCDIR = include
BINDIR = bin
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(BINDIR)/%.o)
EXEC = main
INCLUDE_DIRS = -I $(INCDIR)

$(BINDIR)/$(EXEC): $(OBJS)
	$(CC) $(CFALGS) $(INCLUDE_DIRS) $^ -o $@
	
$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@
	
run: $(BINDIR)/$(EXEC)
	./$(BINDIR)/$(EXEC)
	
clean:
	rm -f $(OBJS) $(BINDIR)/$(EXEC)
