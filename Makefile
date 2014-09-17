NDroutes: NDroutes.c
	gcc NDroutes.c gfx.c -o NDroutes -lX11 -lm
clean:
	rm -f NDroutes
