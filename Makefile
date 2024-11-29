a.out: 
	gcc src/interface.c -lsqlite3 -L/usr/lib/x86_64-linux-gnu/ -lglib-2.0 -g -o a.out -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

clean:
	rm a.out