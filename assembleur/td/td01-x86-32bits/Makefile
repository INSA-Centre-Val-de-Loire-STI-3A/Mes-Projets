AS      = nasm
LD      = ld

# les sources vides (exercices pas encore faits) sont ignorées
SRC     = $(shell find src -name '*.asm' -size +0)
BIN     = $(patsubst src/%.asm,bin/%,$(SRC))

all: $(BIN)

# code 16 bits du 8086 : adresses de chargement imposées par l'énoncé
bin/ex1_2_modes_adressage: src/ex1_2_modes_adressage.asm | bin
	$(AS) -f elf32 $< -o $@.o
	$(LD) -m elf_i386 -Ttext 0x7c00 -Tbss 0x8100 $@.o -o $@

# 64 bits : fichiers suffixés _x64
bin/%_x64: src/%_x64.asm | bin
	$(AS) -f elf64 -g -F dwarf $< -o $@.o
	$(LD) $@.o -o $@

# 32 bits (mode protégé)
bin/%: src/%.asm | bin
	$(AS) -f elf32 -g -F dwarf $< -o $@.o
	$(LD) -m elf_i386 $@.o -o $@

bin:
	mkdir -p bin

clean:
	rm -rf bin

.PHONY: all clean
