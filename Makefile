# Chad Voegele
# status_bar

PROG_NAME=status_bar

CXX=gcc
CXX_FLAGS=-Wall

SRCDIR=src
BINDIR=bin

INCLUDES=-I${SRCDIR} `pkg-config --cflags glib-2.0 libcurl`
LIBS=`pkg-config --libs glib-2.0 libcurl`

SRC=$(wildcard ${SRCDIR}/*.c)
OBJS=$(patsubst ${SRCDIR}/%.c,${BINDIR}/%.o,$(SRC))

${BINDIR}/${PROG_NAME}: ${OBJS}
	@${CXX} ${LIBS} ${OBJS}  -o $@

$(OBJS): $(BINDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p ${BINDIR}
	@${CXX} ${CXX_FLAGS} ${INCLUDES} -c $< -o $@

clean:
	rm bin/*

install:${BINDIR}/${PROG_NAME}
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f ${BINDIR}/${PROG_NAME} ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/${PROG_NAME}
