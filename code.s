# Program
.data
printstr: .asciz "%d\n"

.text
.global Main_main
# Class
# Method
classA_classA:
push %ebp
movl %esp, %ebp
sub $0, %esp
push %ebx
push %esi
push %edi
# Assginment
# Variable
movl 12(%ebp), %eax
push %eax
pop %eax
movl 8(%ebp), %edx
movl %eax, 0(%edx)
pop %edi
pop %esi
pop %ebx
add $0, %esp
movl %ebp, %esp
pop %ebp
ret
# Method
classA_double:
push %ebp
movl %esp, %ebp
sub $0, %esp
push %ebx
push %esi
push %edi
# Assginment
# Variable
movl 8(%ebp), %edx
movl 0(%edx), %eax
push %eax
# Integer Literal
mov $2, %eax
push %eax
# TIMES
pop %ebx
pop %eax
movl $0, %edx
imul %ebx
push %eax
pop %eax
movl 8(%ebp), %edx
movl %eax, 0(%edx)
pop %edi
pop %esi
pop %ebx
add $0, %esp
movl %ebp, %esp
pop %ebp
ret
# Class
# Method
classB_classB:
push %ebp
movl %esp, %ebp
sub $0, %esp
push %ebx
push %esi
push %edi
# Method Call
push %eax
push %ecx
push %edx
# Variable
movl 12(%ebp), %eax
push %eax
movl 8(%ebp), %eax
push %eax
call classA_classA
add $4, %esp
add $4, %esp
movl %eax, %esi
pop %edx
pop %ecx
pop %eax
push %esi
add $4, %esp
# Assginment
# Variable
movl 16(%ebp), %eax
push %eax
pop %eax
movl 8(%ebp), %edx
movl %eax, 0(%edx)
pop %edi
pop %esi
pop %ebx
add $0, %esp
movl %ebp, %esp
pop %ebp
ret
# Method
classB_double:
push %ebp
movl %esp, %ebp
sub $0, %esp
push %ebx
push %esi
push %edi
# Assginment
# Variable
movl 8(%ebp), %edx
movl 0(%edx), %eax
push %eax
# Integer Literal
mov $2, %eax
push %eax
# TIMES
pop %ebx
pop %eax
movl $0, %edx
imul %ebx
push %eax
# Integer Literal
mov $1, %eax
push %eax
# MINUS
pop %edx
pop %eax
sub %edx, %eax
push %eax
pop %eax
movl 8(%ebp), %edx
movl %eax, 0(%edx)
pop %edi
pop %esi
pop %ebx
add $0, %esp
movl %ebp, %esp
pop %ebp
ret
# Class
# Method
CastClass_BtoA:
push %ebp
movl %esp, %ebp
sub $0, %esp
push %ebx
push %esi
push %edi
# Variable
movl 12(%ebp), %eax
push %eax
pop %eax
pop %edi
pop %esi
pop %ebx
add $0, %esp
movl %ebp, %esp
pop %ebp
ret
# Class
# Method
Main_main:
push %ebp
movl %esp, %ebp
sub $12, %esp
push %ebx
push %esi
push %edi
# Assginment
# New
push $8
call malloc
add $4, %esp
push %eax
push %ecx
push %edx
movl %eax, %esi
# Integer Literal
mov $5, %eax
push %eax
# Integer Literal
mov $4, %eax
push %eax
push %esi
call classB_classB
add $4, %esp
add $4, %esp
add $4, %esp
pop %edx
pop %ecx
pop %eax
push %esi
pop %eax
movl %eax, -4(%ebp)
# Assginment
# Method Call
push %eax
push %ecx
push %edx
# Variable
movl -4(%ebp), %eax
push %eax
movl -12(%ebp), %eax
push %eax
call CastClass_BtoA
add $4, %esp
add $4, %esp
movl %eax, %esi
pop %edx
pop %ecx
pop %eax
push %esi
pop %eax
movl %eax, -8(%ebp)
# Print
# Member Access
movl -8(%ebp), %eax
push %eax
pop %edx
movl 0(%edx), %eax
push %eax
push $printstr
call printf
add $4, %esp
add $4, %esp
# Print
# Member Access
movl -4(%ebp), %eax
push %eax
pop %edx
movl 0(%edx), %eax
push %eax
push $printstr
call printf
add $4, %esp
add $4, %esp
# Method Call
push %eax
push %ecx
push %edx
movl -8(%ebp), %eax
push %eax
call classA_double
add $4, %esp
movl %eax, %esi
pop %edx
pop %ecx
pop %eax
push %esi
add $4, %esp
# Print
# Member Access
movl -8(%ebp), %eax
push %eax
pop %edx
movl 0(%edx), %eax
push %eax
push $printstr
call printf
add $4, %esp
add $4, %esp
# Print
# Member Access
movl -4(%ebp), %eax
push %eax
pop %edx
movl 0(%edx), %eax
push %eax
push $printstr
call printf
add $4, %esp
add $4, %esp
# Method Call
push %eax
push %ecx
push %edx
movl -4(%ebp), %eax
push %eax
call classB_double
add $4, %esp
movl %eax, %esi
pop %edx
pop %ecx
pop %eax
push %esi
add $4, %esp
# Print
# Member Access
movl -8(%ebp), %eax
push %eax
pop %edx
movl 0(%edx), %eax
push %eax
push $printstr
call printf
add $4, %esp
add $4, %esp
# Print
# Member Access
movl -4(%ebp), %eax
push %eax
pop %edx
movl 0(%edx), %eax
push %eax
push $printstr
call printf
add $4, %esp
add $4, %esp
pop %edi
pop %esi
pop %ebx
add $12, %esp
movl %ebp, %esp
pop %ebp
ret
