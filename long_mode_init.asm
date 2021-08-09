global long_mode_start
global read_port
global write_port

section .text
bits 64

extern main
extern isr1_handler
extern IDT
extern MarkLines


%macro PUSHALL 0
    push rax
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11
%endmacro

%macro POPALL 0
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rax
%endmacro
    

idtDescriptor:
    dw 4095
    dq IDT
isr1:
    PUSHALL
    call isr1_handler
    call    MarkLines
    POPALL
    iretq
    GLOBAL isr1
LoadIDT:
    lidt[idtDescriptor]
    sti
    ret
    GLOBAL LoadIDT
    



long_mode_start:
    ; print `OKAY` to screen
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    
    mov rax, 0x2f592f412f4b2f4f
    mov qword [0xb8000], rax
    
    call main
   
    hlt
