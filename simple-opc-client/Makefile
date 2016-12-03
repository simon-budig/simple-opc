all: renderer-all

LIBS=-lm `pkg-config --libs libpng`
CFLAGS=-Wall -g `pkg-config --cflags libpng`

renderer-simon: opc-client.o render-utils.o renderer-simon.c
	gcc ${CFLAGS} ${LIBS} -o renderer-simon opc-client.o render-utils.o renderer-simon.c

renderer-all: renderer-all.c opc-client.o render-utils.o renderer_astern.o renderer_ball.o renderer_pong.o
	gcc ${CFLAGS} ${LIBS} -o $@  $^

renderer-fun: renderer-fun.c
	gcc ${CFLAGS} ${LIBS} -o renderer-fun renderer-fun.c

opc-client.o: opc-client.c
	gcc ${CFLAGS} -c -o opc-client.o opc-client.c

render-utils.o: render-utils.c
	gcc ${CFLAGS} -c -o render-utils.o render-utils.c

renderer_astern.o: renderer_astern.c renderer_ball.h render-utils.o
	gcc ${CFLAGS} -c -o $@ $<
renderer_ball.o: renderer_ball.c renderer_ball.h render-utils.o
	gcc ${CFLAGS} -c -o $@ $<
renderer_pong.o: renderer_pong.c renderer_pong.h render-utils.o
	gcc ${CFLAGS} -c -o $@ $<

clean:
	rm -f *.o renderer-simon renderer-all renderer-fun
