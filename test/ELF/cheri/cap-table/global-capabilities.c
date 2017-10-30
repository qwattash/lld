
// REQUIRES: clang

// RUN: %cheri128_cc1 -emit-obj -O2 -target-abi purecap -mllvm -cheri-cap-table %s -o %t-128.o
// RUN: %cheri256_cc1 -emit-obj -O2 -target-abi purecap -mllvm -cheri-cap-table %s -o %t-256.o
// RUN: llvm-readobj -r %t-128.o | FileCheck %s -check-prefix RELOCS
// RUN: llvm-readobj -r %t-256.o | FileCheck %s -check-prefix RELOCS
// RUN: ld.lld -o %t-128.exe %t-128.o
// RUNNOT: ld.lld -o %t-256.exe %t-256.o
// RUN: llvm-objdump -d -r -C -t %t-128.exe | FileCheck %s -check-prefixes EXE,EXE128
// RUNNOT: llvm-objdump -d -r -C -t %t-256.exe | FileCheck %s -check-prefixes EXE,EXE256




// RELOCS-LABEL: Section (3) .rela.text {
// RELOCS-NEXT:   0x28 R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE functions 0x0
// RELOCS-NEXT:   0x2C R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE functions 0x0
// RELOCS-NEXT:   0x60 R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE string 0x0
// RELOCS-NEXT:   0x64 R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE string 0x0
// RELOCS-NEXT:  }

// RELOCS-LABEL: Section (8) .rela.data {
// RELOCS-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE func 0x0
// RELOCS-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str 0x0
// RELOCS-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str.1 0x0
// RELOCS-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE func1 0x0
// RELOCS-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str.2 0x0
// RELOCS-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE func2 0x0
// RELOCS-NEXT:  }

// EXE-LABEL: CAPABILITY RELOCATION RECORDS:
// EXE-NEXT: 0x0000000120020000	Base: func (0x0000000120010000)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
// EXE-NEXT: 0x0000000120020010	Base: <Unnamed symbol> (0x0000000120000196)	Offset: 0x0000000000000000	Length: 0x000000000000000e	Permissions: 0x00000000
// EXE-NEXT: 0x0000000120020020	Base: <Unnamed symbol> (0x0000000120000190)	Offset: 0x0000000000000000	Length: 0x0000000000000006	Permissions: 0x00000000
// EXE-NEXT: 0x0000000120020030	Base: func1 (0x0000000120010008)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
// EXE-NEXT: 0x0000000120020040	Base: <Unnamed symbol> (0x00000001200001a4)	Offset: 0x0000000000000000	Length: 0x0000000000000007	Permissions: 0x00000000
// EXE-NEXT: 0x0000000120020050	Base: func2 (0x0000000120010010)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x8000000000000000 (Function)
// EXE-NEXT: 0x0000000120030000	Base: functions (0x0000000120020020)	Offset: 0x0000000000000000	Length: 0x0000000000000040	Permissions: 0x00000000
// EXE-NEXT: 0x0000000120030010	Base: string (0x0000000120020010)	Offset: 0x0000000000000000	Length: 0x0000000000000010	Permissions: 0x00000000


void func(int a) {

}
void func1(int a) {

}
void func2(int a) {

}

typedef void(*fnptr)(int);

fnptr x = &func;
char* string = "Hello, World!";

struct table {
    const char* name;
    fnptr func;
};

struct table functions[] = {
  {"first", &func1},
  {"second", &func2},
};


int __start() {
  functions[0].func(1);
  functions[1].func(2);
  return string[1] > 'c';
}
