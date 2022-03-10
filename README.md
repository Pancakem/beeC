# beeC (WIP)
A compiler in C


## Progress

Currently we can generate the following x86 asm from the code below:
```
int foo() { return 5; }

int main() { return 0; }

```

```asm
.intel_syntax noprefix

.data

.text

.global foo
foo:
 push rbp
 mov rbp, rsp
 sub rsp, 0
 push 5
 pop rdi
 pop rax
 push rax
 pop rax
 jmp .Lreturn.(null)
.Lreturn.foo:
 mov rsp, rbp
 pop rbp
 ret
.global main
main:
 push rbp
 mov rbp, rsp
 sub rsp, 0
 push 0
 pop rdi
 pop rax
 push rax
 pop rax
 jmp .Lreturn.(null)
.Lreturn.main:
 mov rsp, rbp
 pop rbp
 ret
```
