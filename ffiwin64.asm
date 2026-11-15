bits 64
default rel

section .text
global ffi_call_win64



ffi_call_win64:
    
    push rbp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    mov rbp, rsp    

   
    mov rbx, rcx      
    mov rdi, rdx      
    mov rsi, r8       

  
    mov rcx, rdi
    sub rcx, 4
    jle .no_stack_args
    shl rcx, 3         
    jmp .alloc_stack
.no_stack_args:
    xor rcx, rcx
.alloc_stack:
    add rcx, 32       
    
    
    add rcx, 8
    sub rsp, rcx

   
    xor r10, r10       
    xor r11, r11       
    mov r12, 32        
    xor r13, r13       

.loop_start:
    cmp r13, rdi
    jge .call_func

    mov rax, [rsi]    
    cmp rax, 0
    je .arg_int
    cmp rax, 1
    je .arg_float
    jmp .next_arg

.arg_int:
    cmp r10, 4
    jge .int_stack
    cmp r10, 0
    je .int_r1
    cmp r10, 1
    je .int_r2
    cmp r10, 2
    je .int_r3
    cmp r10, 3
    je .int_r4
.int_r1: mov rcx, [rsi+8]
         jmp .int_done
.int_r2: mov rdx, [rsi+8]
         jmp .int_done
.int_r3: mov r8, [rsi+8]
         jmp .int_done
.int_r4: mov r9, [rsi+8]
         jmp .int_done
.int_stack:
    mov rax, [rsi+8]
    mov [rsp+r12], rax
    add r12, 8
.int_done:
    inc r10
    jmp .next_arg

.arg_float:
   
    cmp r11, 4
    jge .float_xmm_done
    cmp r11, 0
    je .xmm0
    cmp r11, 1
    je .xmm1
    cmp r11, 2
    je .xmm2
    cmp r11, 3
    je .xmm3
.xmm0: movq xmm0, [rsi+8]
       jmp .float_xmm_done
.xmm1: movq xmm1, [rsi+8]
       jmp .float_xmm_done
.xmm2: movq xmm2, [rsi+8]
       jmp .float_xmm_done
.xmm3: movq xmm3, [rsi+8]
       jmp .float_xmm_done
.float_xmm_done:

    
    cmp r10, 4
    jge .float_gp_stack
    cmp r10, 0
    je .float_gp_r1
    cmp r10, 1
    je .float_gp_r2
    cmp r10, 2
    je .float_gp_r3
    cmp r10, 3
    je .float_gp_r4
.float_gp_r1: mov rcx, [rsi+8]
              jmp .float_gp_done
.float_gp_r2: mov rdx, [rsi+8]
              jmp .float_gp_done
.float_gp_r3: mov r8, [rsi+8]
              jmp .float_gp_done
.float_gp_r4: mov r9, [rsi+8]
              jmp .float_gp_done
.float_gp_stack:
    mov rax, [rsi+8]
    mov [rsp+r12], rax
    add r12, 8
.float_gp_done:
    inc r11
    inc r10
    jmp .next_arg

.next_arg:
    add rsi, 16
    inc r13
    jmp .loop_start

.call_func:
    mov rax, r11      
    call rbx          

   
    mov rsp, rbp       
    
 
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

section .note.GNU-stack noalloc noexec nowrite progbits