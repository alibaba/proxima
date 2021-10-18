/**
 *   Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
 * 
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *   
 *       http://www.apache.org/licenses/LICENSE-2.0
 *   
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.

 *   \author   Hechong.xyf
 *   \date     Dec 2020
 *   \brief    Interface of AiLego User Thread Context (Darwin)
 */

#ifndef __AILEGO_DEBUG_USER_CONTEXT_DARWIN_H__
#define __AILEGO_DEBUG_USER_CONTEXT_DARWIN_H__

#include <sys/ucontext.h>

#if defined(__x86_64__)
#define USER_CONTEXT_RAX(uc) ((long)(uc)->uc_mcontext->__ss.__rax)
#define USER_CONTEXT_RBX(uc) ((long)(uc)->uc_mcontext->__ss.__rbx)
#define USER_CONTEXT_RCX(uc) ((long)(uc)->uc_mcontext->__ss.__rcx)
#define USER_CONTEXT_RDX(uc) ((long)(uc)->uc_mcontext->__ss.__rdx)
#define USER_CONTEXT_RDI(uc) ((long)(uc)->uc_mcontext->__ss.__rdi)
#define USER_CONTEXT_RSI(uc) ((long)(uc)->uc_mcontext->__ss.__rsi)
#define USER_CONTEXT_RBP(uc) ((long)(uc)->uc_mcontext->__ss.__rbp)
#define USER_CONTEXT_RSP(uc) ((long)(uc)->uc_mcontext->__ss.__rsp)
#define USER_CONTEXT_R8(uc) ((long)(uc)->uc_mcontext->__ss.__r8)
#define USER_CONTEXT_R9(uc) ((long)(uc)->uc_mcontext->__ss.__r9)
#define USER_CONTEXT_R10(uc) ((long)(uc)->uc_mcontext->__ss.__r10)
#define USER_CONTEXT_R11(uc) ((long)(uc)->uc_mcontext->__ss.__r11)
#define USER_CONTEXT_R12(uc) ((long)(uc)->uc_mcontext->__ss.__r12)
#define USER_CONTEXT_R13(uc) ((long)(uc)->uc_mcontext->__ss.__r13)
#define USER_CONTEXT_R14(uc) ((long)(uc)->uc_mcontext->__ss.__r14)
#define USER_CONTEXT_R15(uc) ((long)(uc)->uc_mcontext->__ss.__r15)
#define USER_CONTEXT_RIP(uc) ((long)(uc)->uc_mcontext->__ss.__rip)
#define USER_CONTEXT_RFLAGS(uc) ((long)(uc)->uc_mcontext->__ss.__rflags)
#define USER_CONTEXT_CS(uc) ((long)(uc)->uc_mcontext->__ss.__cs)
#define USER_CONTEXT_FS(uc) ((long)(uc)->uc_mcontext->__ss.__fs)
#define USER_CONTEXT_GS(uc) ((long)(uc)->uc_mcontext->__ss.__gs)
#define USER_CONTEXT_ERR(uc) ((long)(uc)->uc_mcontext->__es.__err)
#define USER_CONTEXT_TRAPNO(uc) ((long)(uc)->uc_mcontext->__es.__trapno)
#elif defined(__i386__)
#define USER_CONTEXT_GS(uc) ((long)(uc)->uc_mcontext->__ss.__gs)
#define USER_CONTEXT_FS(uc) ((long)(uc)->uc_mcontext->__ss.__fs)
#define USER_CONTEXT_ES(uc) ((long)(uc)->uc_mcontext->__ss.__es)
#define USER_CONTEXT_DS(uc) ((long)(uc)->uc_mcontext->__ss.__ds)
#define USER_CONTEXT_CS(uc) ((long)(uc)->uc_mcontext->__ss.__cs)
#define USER_CONTEXT_SS(uc) ((long)(uc)->uc_mcontext->__ss.__ss)
#define USER_CONTEXT_EDI(uc) ((long)(uc)->uc_mcontext->__ss.__edi)
#define USER_CONTEXT_ESI(uc) ((long)(uc)->uc_mcontext->__ss.__esi)
#define USER_CONTEXT_EBP(uc) ((long)(uc)->uc_mcontext->__ss.__ebp)
#define USER_CONTEXT_ESP(uc) ((long)(uc)->uc_mcontext->__ss.__esp)
#define USER_CONTEXT_EBX(uc) ((long)(uc)->uc_mcontext->__ss.__ebx)
#define USER_CONTEXT_EDX(uc) ((long)(uc)->uc_mcontext->__ss.__edx)
#define USER_CONTEXT_ECX(uc) ((long)(uc)->uc_mcontext->__ss.__ecx)
#define USER_CONTEXT_EAX(uc) ((long)(uc)->uc_mcontext->__ss.__eax)
#define USER_CONTEXT_EIP(uc) ((long)(uc)->uc_mcontext->__ss.__eip)
#define USER_CONTEXT_ERR(uc) ((long)(uc)->uc_mcontext->__es.__err)
#define USER_CONTEXT_TRAPNO(uc) ((long)(uc)->uc_mcontext->__es.__trapno)
#define USER_CONTEXT_EFLAGS(uc) ((long)(uc)->uc_mcontext->__ss.__eflags)
#elif defined(__aarch64__)
#error "Unsupported platform !!!"
#endif

#endif  // __AILEGO_DEBUG_USER_CONTEXT_DARWIN_H__
