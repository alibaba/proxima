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
 *   \brief    Interface of AiLego User Thread Context (Linux)
 */

#ifndef __AILEGO_DEBUG_USER_CONTEXT_LINUX_H__
#define __AILEGO_DEBUG_USER_CONTEXT_LINUX_H__

#include <ucontext.h>

#if defined(__x86_64__)
#define UCONTEXT_GREGS(ucp, x) ((long)((ucp)->uc_mcontext.gregs[(x)]))
#define USER_CONTEXT_R8(ucp) UCONTEXT_GREGS(ucp, REG_R8)
#define USER_CONTEXT_R9(ucp) UCONTEXT_GREGS(ucp, REG_R9)
#define USER_CONTEXT_R10(ucp) UCONTEXT_GREGS(ucp, REG_R10)
#define USER_CONTEXT_R11(ucp) UCONTEXT_GREGS(ucp, REG_R11)
#define USER_CONTEXT_R12(ucp) UCONTEXT_GREGS(ucp, REG_R12)
#define USER_CONTEXT_R13(ucp) UCONTEXT_GREGS(ucp, REG_R13)
#define USER_CONTEXT_R14(ucp) UCONTEXT_GREGS(ucp, REG_R14)
#define USER_CONTEXT_R15(ucp) UCONTEXT_GREGS(ucp, REG_R15)
#define USER_CONTEXT_RDI(ucp) UCONTEXT_GREGS(ucp, REG_RDI)
#define USER_CONTEXT_RSI(ucp) UCONTEXT_GREGS(ucp, REG_RSI)
#define USER_CONTEXT_RBP(ucp) UCONTEXT_GREGS(ucp, REG_RBP)
#define USER_CONTEXT_RBX(ucp) UCONTEXT_GREGS(ucp, REG_RBX)
#define USER_CONTEXT_RDX(ucp) UCONTEXT_GREGS(ucp, REG_RDX)
#define USER_CONTEXT_RAX(ucp) UCONTEXT_GREGS(ucp, REG_RAX)
#define USER_CONTEXT_RCX(ucp) UCONTEXT_GREGS(ucp, REG_RCX)
#define USER_CONTEXT_RSP(ucp) UCONTEXT_GREGS(ucp, REG_RSP)
#define USER_CONTEXT_RIP(ucp) UCONTEXT_GREGS(ucp, REG_RIP)
#define USER_CONTEXT_RFLAGS(ucp) UCONTEXT_GREGS(ucp, REG_EFL)
#define USER_CONTEXT_CS(ucp) (UCONTEXT_GREGS(ucp, REG_CSGSFS) & 0xFFFF)
#define USER_CONTEXT_FS(ucp) ((UCONTEXT_GREGS(ucp, REG_CSGSFS) >> 16) & 0xFFFF)
#define USER_CONTEXT_GS(ucp) ((UCONTEXT_GREGS(ucp, REG_CSGSFS) >> 32) & 0xFFFF)
#define USER_CONTEXT_ERR(ucp) UCONTEXT_GREGS(ucp, REG_ERR)
#define USER_CONTEXT_TRAPNO(ucp) UCONTEXT_GREGS(ucp, REG_TRAPNO)
#elif defined(__i386__)
#define UCONTEXT_GREGS(ucp, x) ((long)((ucp)->uc_mcontext.gregs[x]))
#define USER_CONTEXT_GS(ucp) UCONTEXT_GREGS(ucp, REG_GS)
#define USER_CONTEXT_FS(ucp) UCONTEXT_GREGS(ucp, REG_FS)
#define USER_CONTEXT_ES(ucp) UCONTEXT_GREGS(ucp, REG_ES)
#define USER_CONTEXT_DS(ucp) UCONTEXT_GREGS(ucp, REG_DS)
#define USER_CONTEXT_CS(ucp) UCONTEXT_GREGS(ucp, REG_CS)
#define USER_CONTEXT_SS(ucp) UCONTEXT_GREGS(ucp, REG_SS)
#define USER_CONTEXT_EDI(ucp) UCONTEXT_GREGS(ucp, REG_EDI)
#define USER_CONTEXT_ESI(ucp) UCONTEXT_GREGS(ucp, REG_ESI)
#define USER_CONTEXT_EBP(ucp) UCONTEXT_GREGS(ucp, REG_EBP)
#define USER_CONTEXT_ESP(ucp) UCONTEXT_GREGS(ucp, REG_ESP)
#define USER_CONTEXT_EBX(ucp) UCONTEXT_GREGS(ucp, REG_EBX)
#define USER_CONTEXT_EDX(ucp) UCONTEXT_GREGS(ucp, REG_EDX)
#define USER_CONTEXT_ECX(ucp) UCONTEXT_GREGS(ucp, REG_ECX)
#define USER_CONTEXT_EAX(ucp) UCONTEXT_GREGS(ucp, REG_EAX)
#define USER_CONTEXT_EIP(ucp) UCONTEXT_GREGS(ucp, REG_EIP)
#define USER_CONTEXT_ERR(ucp) UCONTEXT_GREGS(ucp, REG_ERR)
#define USER_CONTEXT_TRAPNO(ucp) UCONTEXT_GREGS(ucp, REG_TRAPNO)
#define USER_CONTEXT_EFLAGS(ucp) UCONTEXT_GREGS(ucp, REG_EFL)
#elif defined(__aarch64__)
#define UCONTEXT_GREGS(ucp, x) ((long)((ucp)->uc_mcontext.regs[x]))
#define USER_CONTEXT_R0(ucp) UCONTEXT_GREGS(ucp, 0)
#define USER_CONTEXT_R1(ucp) UCONTEXT_GREGS(ucp, 1)
#define USER_CONTEXT_R2(ucp) UCONTEXT_GREGS(ucp, 2)
#define USER_CONTEXT_R3(ucp) UCONTEXT_GREGS(ucp, 3)
#define USER_CONTEXT_R4(ucp) UCONTEXT_GREGS(ucp, 4)
#define USER_CONTEXT_R5(ucp) UCONTEXT_GREGS(ucp, 5)
#define USER_CONTEXT_R6(ucp) UCONTEXT_GREGS(ucp, 6)
#define USER_CONTEXT_R7(ucp) UCONTEXT_GREGS(ucp, 7)
#define USER_CONTEXT_R8(ucp) UCONTEXT_GREGS(ucp, 8)
#define USER_CONTEXT_R9(ucp) UCONTEXT_GREGS(ucp, 9)
#define USER_CONTEXT_R10(ucp) UCONTEXT_GREGS(ucp, 10)
#define USER_CONTEXT_R11(ucp) UCONTEXT_GREGS(ucp, 11)
#define USER_CONTEXT_R12(ucp) UCONTEXT_GREGS(ucp, 12)
#define USER_CONTEXT_R13(ucp) UCONTEXT_GREGS(ucp, 13)
#define USER_CONTEXT_R14(ucp) UCONTEXT_GREGS(ucp, 14)
#define USER_CONTEXT_R15(ucp) UCONTEXT_GREGS(ucp, 15)
#define USER_CONTEXT_R16(ucp) UCONTEXT_GREGS(ucp, 16)
#define USER_CONTEXT_R17(ucp) UCONTEXT_GREGS(ucp, 17)
#define USER_CONTEXT_R18(ucp) UCONTEXT_GREGS(ucp, 18)
#define USER_CONTEXT_R19(ucp) UCONTEXT_GREGS(ucp, 19)
#define USER_CONTEXT_R20(ucp) UCONTEXT_GREGS(ucp, 20)
#define USER_CONTEXT_R21(ucp) UCONTEXT_GREGS(ucp, 21)
#define USER_CONTEXT_R22(ucp) UCONTEXT_GREGS(ucp, 22)
#define USER_CONTEXT_R23(ucp) UCONTEXT_GREGS(ucp, 23)
#define USER_CONTEXT_R24(ucp) UCONTEXT_GREGS(ucp, 24)
#define USER_CONTEXT_R25(ucp) UCONTEXT_GREGS(ucp, 25)
#define USER_CONTEXT_R26(ucp) UCONTEXT_GREGS(ucp, 26)
#define USER_CONTEXT_R27(ucp) UCONTEXT_GREGS(ucp, 27)
#define USER_CONTEXT_R28(ucp) UCONTEXT_GREGS(ucp, 28)
#define USER_CONTEXT_R29(ucp) UCONTEXT_GREGS(ucp, 29)
#define USER_CONTEXT_R30(ucp) UCONTEXT_GREGS(ucp, 30)
#define USER_CONTEXT_SP(ucp) ((long)((ucp)->uc_mcontext.sp))
#define USER_CONTEXT_PC(ucp) ((long)((ucp)->uc_mcontext.pc))
#define USER_CONTEXT_CPSR(ucp) ((long)((ucp)->uc_mcontext.pstate))
#define USER_CONTEXT_FAULTADDR(ucp) ((long)((ucp)->uc_mcontext.fault_address))
#elif defined(__arm__)
#define USER_CONTEXT_R0(ucp) ((long)((ucp)->uc_mcontext.arm_r0))
#define USER_CONTEXT_R1(ucp) ((long)((ucp)->uc_mcontext.arm_r1))
#define USER_CONTEXT_R2(ucp) ((long)((ucp)->uc_mcontext.arm_r2))
#define USER_CONTEXT_R3(ucp) ((long)((ucp)->uc_mcontext.arm_r3))
#define USER_CONTEXT_R4(ucp) ((long)((ucp)->uc_mcontext.arm_r4))
#define USER_CONTEXT_R5(ucp) ((long)((ucp)->uc_mcontext.arm_r5))
#define USER_CONTEXT_R6(ucp) ((long)((ucp)->uc_mcontext.arm_r6))
#define USER_CONTEXT_R7(ucp) ((long)((ucp)->uc_mcontext.arm_r7))
#define USER_CONTEXT_R8(ucp) ((long)((ucp)->uc_mcontext.arm_r8))
#define USER_CONTEXT_R9(ucp) ((long)((ucp)->uc_mcontext.arm_r9))
#define USER_CONTEXT_R10(ucp) ((long)((ucp)->uc_mcontext.arm_r10))
#define USER_CONTEXT_SP(ucp) ((long)((ucp)->uc_mcontext.arm_sp))
#define USER_CONTEXT_LR(ucp) ((long)((ucp)->uc_mcontext.arm_lr))
#define USER_CONTEXT_PC(ucp) ((long)((ucp)->uc_mcontext.arm_pc))
#define USER_CONTEXT_CPSR(ucp) ((long)((ucp)->uc_mcontext.arm_cpsr))
#define USER_CONTEXT_IP(ucp) ((long)((ucp)->uc_mcontext.arm_ip))
#define USER_CONTEXT_FP(ucp) ((long)((ucp)->uc_mcontext.arm_fp))
#define USER_CONTEXT_TRAPNO(ucp) ((long)((ucp)->uc_mcontext.trap_no))
#define USER_CONTEXT_FAULTADDR(ucp) ((long)((ucp)->uc_mcontext.fault_address))
#endif

#endif  // __AILEGO_DEBUG_USER_CONTEXT_LINUX_H__
