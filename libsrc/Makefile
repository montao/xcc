LIBSRC_DIR:=../libsrc
LIBOBJ_DIR:=../obj/lib
LIB_DIR:=../lib
INC_DIR:=../include

LIBS:=$(LIB_DIR)/crt0.a $(LIB_DIR)/libc.a

UNAME:=$(shell uname)
ifeq ("$(UNAME)", "Darwin")
CFLAGS:=-I$(INC_DIR) -Wall -Wextra -Werror \
	-Wno-incompatible-library-redeclaration
else
CFLAGS:=-I$(INC_DIR) -Wall -Wextra -Werror \
	-Wno-builtin-declaration-mismatch \
	-Wno-format-truncation
endif

### Library

CRT0_SRCS:=$(wildcard $(LIBSRC_DIR)/crt0/*.c)

LIBC_SRCS:=\
	$(wildcard $(LIBSRC_DIR)/math/*.c) \
	$(wildcard $(LIBSRC_DIR)/misc/*.c) \
	$(wildcard $(LIBSRC_DIR)/stdio/*.c) \
	$(wildcard $(LIBSRC_DIR)/stdlib/*.c) \
	$(wildcard $(LIBSRC_DIR)/string/*.c) \
	$(wildcard $(LIBSRC_DIR)/unistd/*.c) \

CRT0_OBJS:=$(addprefix $(LIBOBJ_DIR)/,$(notdir $(CRT0_SRCS:.c=.o)))
LIBC_OBJS:=$(addprefix $(LIBOBJ_DIR)/,$(notdir $(LIBC_SRCS:.c=.o)))

.PHONY: libs
libs: $(LIBS)

.PHONY: clean
clean:	clean-test
	rm -rf $(LIBOBJ_DIR) $(LIB_DIR)

$(LIB_DIR)/crt0.a:	$(CRT0_OBJS)
	@mkdir -p $(LIB_DIR)
	$(AR) r $@ $^

$(LIB_DIR)/libc.a:	$(LIBC_OBJS)
	@mkdir -p $(LIB_DIR)
	$(AR) r $@ $^

$(LIBOBJ_DIR)/%.o: $(LIBSRC_DIR)/**/%.c
	@mkdir -p $(LIBOBJ_DIR)
	$(CC) -c -o $@ -nostdinc -Werror -ffreestanding $(CFLAGS) $<

### Test

.PHONY:	test
test:	test-printf test-math

.PHONY: clean-test
clean-test:
	rm -rf printf_test math_test

.PHONY: test-printf
test-printf:	printf_test
	@echo '## printf test'
	@./printf_test
	@echo ''

STDIO_SRCS:=$(wildcard stdio/*.c)
STDIO_OBJS:=$(addprefix $(LIBOBJ_DIR)/,$(notdir $(STDIO_SRCS:.c=.o)))
printf_test:	tests/printf_test.c $(STDIO_OBJS)
	$(CC) -o$@ $(CFLAGS) -I$(INC_DIR) -DUNIT_TEST -ffreestanding $^

.PHONY: test-math
test-math:	math_test
	@echo '## math test'
	@./math_test
	@echo ''

MATH_SRCS:=$(wildcard math/*.c)
MATH_OBJS:=$(addprefix $(LIBOBJ_DIR)/,$(notdir $(MATH_SRCS:.c=.o)))
math_test:	tests/math_test.c $(MATH_OBJS)
	$(CC) -o$@ $(CFLAGS) -I$(INC_DIR) -DUNIT_TEST -ffreestanding $^
