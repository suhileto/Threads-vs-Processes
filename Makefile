all:process_sync thread.c 
process_sync:
            gcc process_sync.c -lpthread -lrt 
thread_sync:
            gcc thread_sync.c -lpthread -lrt
