CC=g++
AR=ar
INCLUDES=-I./src
CFLAGS=${INCLUDES} -fpic
LDFLAGS=
LIBS=libccshare.so libccshare.a
LIB_OBJS=utils.o

libccshare.so:${LIB_OBJS}
	${CC} -shared -fpic -o $@ $^

libccshare.a:${LIB_OBJS}
	${AR} crv $@ $^

#%.o:src/%.cc
#	${CC} -o $@ $< ${CFLAGS}
#utils.o:src/utils.cc
#	${CC} -fpic -o $@ $< ${CFLAGS}
%.o:src/%.cc
	${CC} -o $@ -c $< ${CFLAGS}

#test:test.o
#	${CC} -o $@ $^ ${LDFLAGS}

libs:${LIBS}

all:libs

dist:all
	mkdir -p ccshare_dist/include &&	\
	mv libccshare.so ccshare_dist &&	\
	mv libccshare.a ccshare_dist && 	\
	cp src/*.h ccshare_dist/include &&	\
	tar -zcvf ccshare_dist.tar.gz ccshare_dist &&	\
	rm -rf ccshare_dist && make clean

clean:
	rm -f ${LIB_OBJS} ${LIBS}

.PHONY:clean all libs dist
