OBJS_DIR     = objs

OBJS         = $(OBJS_DIR)/main.o           \
			   $(OBJS_DIR)/hawkeye_config.o  \
			   $(OBJS_DIR)/hawkeye.o         \
			   $(OBJS_DIR)/cpu_stat.o       \
			   $(OBJS_DIR)/io_stat.o        \
			   $(OBJS_DIR)/load_stat.o      \
			   $(OBJS_DIR)/memory_stat.o    \
			   $(OBJS_DIR)/nginx_stat.o     \
			   $(OBJS_DIR)/traffic_stat.o   \
			   $(OBJS_DIR)/partition_stat.o \
			   $(OBJS_DIR)/processes_stat.o   \
			   $(OBJS_DIR)/report.o

CC           = gcc
CFLAGS       = -g -W -Wall -Werror -Wno-unused-parameter -Wunused-function \
			   -Wunused-variable -Wunused-value -fPIC  

LIBS         = -lcurl -leasy

MAIN_EXE     = hawkeye

main:$(OBJS)
	$(CC) -o $(MAIN_EXE) $(OBJS) $(CFLAGS) $(LIBS)
	@echo $(MAIN_EXE) is generated!

$(OBJS_DIR)/main.o:main.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/main.o main.c 

$(OBJS_DIR)/hawkeye_config.o:hawkeye_config.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/hawkeye_config.o hawkeye_config.c 

$(OBJS_DIR)/hawkeye.o:hawkeye.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/hawkeye.o hawkeye.c 

$(OBJS_DIR)/cpu_stat.o:stat/cpu_stat.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/cpu_stat.o stat/cpu_stat.c 

$(OBJS_DIR)/io_stat.o:stat/io_stat.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/io_stat.o stat/io_stat.c 

$(OBJS_DIR)/load_stat.o:stat/load_stat.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/load_stat.o stat/load_stat.c 

$(OBJS_DIR)/memory_stat.o:stat/memory_stat.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/memory_stat.o stat/memory_stat.c 

$(OBJS_DIR)/nginx_stat.o:stat/nginx_stat.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/nginx_stat.o stat/nginx_stat.c

$(OBJS_DIR)/traffic_stat.o:stat/traffic_stat.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/traffic_stat.o stat/traffic_stat.c

$(OBJS_DIR)/partition_stat.o:stat/partition_stat.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/partition_stat.o stat/partition_stat.c

$(OBJS_DIR)/processes_stat.o:stat/processes_stat.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/processes_stat.o stat/processes_stat.c

$(OBJS_DIR)/report.o:report.c
	$(CC) -c $(CFLAGS) -o $(OBJS_DIR)/report.o report.c

clean:
	rm -f $(MAIN_EXE)
	rm -f $(OBJS)
