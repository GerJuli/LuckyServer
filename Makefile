CC_local=gcc
CFLAGS_local= -Wall
CFLAGS_ALL= -pthread
objects = main.c

LuckyServer: $(objects)
	$(CC_local) -o $@ $(CFLAGS_local) $(CFLAGS_ALL) $(objects)	\
	-I"/home/juli/eclipse-workspace/SRI/" \
	-L"/home/juli/eclipse-workspace/SRI/" \
	-l"SRI"

# In order to make this work you need to export the variables of the Toradex Toolchain
# $ . /usr/local/oecore-x86_64/environment-setup-armv7at2hf-neon-angstrom-linux-gnueabi
ToradexLS:
	${CC} -o $@ ${CFLAGS} $(CFLAGS_ALL) $(objects)	\
	-I"/home/juli/eclipse-workspace/SRI/" \
	-L"/home/juli/eclipse-workspace/SRI/" \
	-l"ToradexSRI"
clean:
	rm LuckyServer ToradexLS
