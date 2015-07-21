default:
	gcc -g -o cpu_stat       cpu_stat.c       -leasy -D_DEBUG
	gcc -g -o load_stat      load_stat.c      -leasy -D_DEBUG
	gcc -g -o memory_stat    memory_stat.c    -leasy -D_DEBUG
	gcc -g -o nginx_stat     nginx_stat.c     -leasy -lcurl -D_DEBUG
	gcc -g -o traffic_stat   traffic_stat.c   -leasy -D_DEBUG
	gcc -g -o io_stat        io_stat.c        -leasy -D_DEBUG
	gcc -g -o partition_stat partition_stat.c -leasy -D_DEBUG
	gcc -g -o processes_stat   processes_stat.c   -leasy -D_DEBUG

clean:
	rm -f cpu_stat
	rm -f load_stat
	rm -f memory_stat
	rm -f nginx_stat
	rm -f traffic_stat
	rm -f io_stat
	rm -f partition_stat
	rm -f processes_stat
