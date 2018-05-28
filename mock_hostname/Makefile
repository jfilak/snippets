all:libmock_hostname.so

libmock_hostname.so: mock_hostname.c
	$(CC) -shared -std=c99 -Wall -Wextra -fPIC -mtune=generic $^ -o $@ -ldl

.PHONY=clean
clean:
	rm -f libmock_hostname.so
