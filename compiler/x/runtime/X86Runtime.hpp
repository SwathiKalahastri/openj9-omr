/*******************************************************************************
 * Copyright (c) 2000, 2017 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
 *******************************************************************************/

#ifndef X86RUNTIME_INCL
#define X86RUNTIME_INCL

#include "env/ProcessorInfo.hpp"

#ifdef WINDOWS
#include <intrin.h>
#define cpuid(CPUInfo, EAXValue)             __cpuid(CPUInfo, EAXValue)
#define cpuidex(CPUInfo, EAXValue, ECXValue) __cpuidex(CPUInfo, EAXValue, ECXValue)
#else
#include <cpuid.h>
#ifdef TR_HOST_64BIT
#include <emmintrin.h>
#endif
#define cpuid(CPUInfo, EAXValue)             __cpuid(EAXValue, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3] )
#define cpuidex(CPUInfo, EAXValue, ECXValue) __cpuid_count(EAXValue, ECXValue, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3] )
#endif

inline bool jitGetCPUID(TR_X86CPUIDBuffer* pBuffer)
   {
   enum
      {
      EAX = 0,
      EBX = 1,
      ECX = 2,
      EDX = 3,
      };
   int CPUInfo[4];

   // EAX = 0
   cpuid(CPUInfo, 0);
   int* VendorID = (int*)pBuffer->_vendorId;
   VendorID[0] = CPUInfo[EBX];
   VendorID[1] = CPUInfo[EDX];
   VendorID[2] = CPUInfo[ECX];

   if (CPUInfo[EAX] > 0)
      {
      // EAX = 1
      cpuid(CPUInfo, 1);
      pBuffer->_processorSignature = CPUInfo[EAX];
      pBuffer->_brandIdEtc         = CPUInfo[EBX];
      pBuffer->_featureFlags       = CPUInfo[EDX];
      pBuffer->_featureFlags2      = CPUInfo[ECX];
      // EAX = 7, ECX = 0
      cpuidex(CPUInfo, 7, 0);
      pBuffer->_featureFlags8 = CPUInfo[EBX];
      return true;
      }
   else
      {
      return false;
      }
   }

inline bool AtomicCompareAndSwap(volatile uint32_t* ptr, uint32_t old_val, uint32_t new_val)
   {
#ifdef WINDOWS
   return old_val == (uint32_t)_InterlockedCompareExchange((volatile long*)ptr, (long)new_val, (long)old_val);
#else
   return __sync_bool_compare_and_swap(ptr, old_val, new_val);
#endif
   }

inline bool AtomicCompareAndSwap(volatile uint16_t* ptr, uint16_t old_val, uint16_t new_val)
   {
#ifdef WINDOWS
   return old_val == (uint16_t)_InterlockedCompareExchange16((volatile short*)ptr, (short)new_val, (short)old_val);
#else
   return __sync_bool_compare_and_swap(ptr, old_val, new_val);
#endif
   }

inline void patchingFence16(void* addr)
   {
#ifdef TR_HOST_64BIT
   _mm_mfence();
   _mm_clflush(addr);
   _mm_clflush(static_cast<char*>(addr)+8);
   _mm_mfence();
#endif
   }

#endif
