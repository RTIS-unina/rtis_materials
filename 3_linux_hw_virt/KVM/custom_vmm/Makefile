CFLAGS = -Wall -Wextra -Werror -O2

.PHONY: run
run: custom_vmm
#	./custom_vmm
#	./custom_vmm -s
#	./custom_vmm -p
	./custom_vmm -l

custom_vmm: custom_vmm.o payload.o
	$(CC) $^ -o $@

payload.o: payload.ld guest16.o guest32.img.o guest64.img.o
	$(LD) -r -T $< -o $@

guest64.o: guest.c
	$(CC) $(CFLAGS) -m64 -ffreestanding -fno-pic -c -o $@ $^

guest64.img: guest64.o
	$(LD) -T guest.ld $^ -o $@

guest32.o: guest.c
	$(CC) $(CFLAGS) -m32 -ffreestanding -fno-pic -c -o $@ $^

guest32.img: guest32.o
	$(LD) -T guest.ld -m elf_i386 $^ -o $@

%.img.o: %.img
	$(LD) -b binary -r $^ -o $@

.PHONY: clean
clean:
	$(RM) custom_vmm custom_vmm.o payload.o guest16.o \
		guest32.o guest32.img guest32.img.o \
		guest64.o guest64.img guest64.img.o
