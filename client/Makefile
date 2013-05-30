TOPDIR  := $(shell cd ..; pwd)
include $ Rules.make

APP = tempmon

all: $(APP)

$(APP): main.c	
	$(CC) main.c -o $(APP) $(CFLAGS)
	
clean:
	rm -f *.o ; rm $(APP)
