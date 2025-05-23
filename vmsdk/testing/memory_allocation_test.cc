/*
 * Copyright (c) 2025, valkey-search contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "vmsdk/src/memory_allocation.h"

#include <cstddef>
#include <cstdlib>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "vmsdk/src/memory_allocation_overrides.h"
#include "vmsdk/src/testing_infra/module.h"
#include "vmsdk/src/testing_infra/utils.h"

class MockSystemAlloc {
 public:
  // Prefixed with _ to avoid name collision with system functions.
  MOCK_METHOD(void*, _malloc, (size_t size), (noexcept));
  MOCK_METHOD(void, _free, (void* ptr), (noexcept));
  MOCK_METHOD(void*, _calloc, (size_t nmemb, size_t size), (noexcept));
  MOCK_METHOD(void*, _realloc, (void* ptr, size_t size), (noexcept));
  MOCK_METHOD(void*, _aligned_alloc, (size_t alignment, size_t size),
              (noexcept));
  MOCK_METHOD(size_t, _malloc_usable_size, (void* ptr), (noexcept));
  MOCK_METHOD(void*, _memalign, (size_t alignment, size_t size), (noexcept));
  MOCK_METHOD(int, _posix_memalign, (void** r, size_t alignment, size_t size),
              (noexcept));
  MOCK_METHOD(void*, _pvalloc, (size_t size), (noexcept));
  MOCK_METHOD(void*, _valloc, (size_t size), (noexcept));
  MOCK_METHOD(void, _cfree, (void* ptr), (noexcept));
};

MockSystemAlloc* kMockSystemAlloc;

namespace vmsdk {

namespace {
#ifndef TESTING_TMP_DISABLED
class MemoryAllocationTest : public RedisTest {
 protected:
  void SetUp() override {
    RedisTest::SetUp();
    kMockSystemAlloc = new MockSystemAlloc();
    SetRealAllocators(
        [](size_t size) { return kMockSystemAlloc->_malloc(size); },
        [](void* ptr) { kMockSystemAlloc->_free(ptr); },
        [](size_t nmemb, size_t size) {
          return kMockSystemAlloc->_calloc(nmemb, size);
        },
        [](void* ptr, size_t size) {
          return kMockSystemAlloc->_realloc(ptr, size);
        },
        [](size_t alignment, size_t size) {
          return kMockSystemAlloc->_aligned_alloc(alignment, size);
        },
        [](void** r, size_t alignment, size_t size) {
          return kMockSystemAlloc->_posix_memalign(r, alignment, size);
        },
        [](size_t size) { return kMockSystemAlloc->_valloc(size); });
    vmsdk::ResetValkeyAlloc();
  }
  void TearDown() override {
    SetRealAllocators(malloc, free, calloc, realloc, aligned_alloc,
                      posix_memalign, valloc);
    RedisTest::TearDown();
    delete kMockSystemAlloc;
    vmsdk::ResetValkeyAlloc();
  }
};

TEST_F(MemoryAllocationTest, SystemAllocIsDefault) {
  size_t size = 10;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _malloc(size))
      .WillOnce(testing::Return(test_ptr));
  EXPECT_CALL(*kMockRedisModule, Alloc(size)).Times(0);
  void* ptr = __wrap_malloc(size);
  EXPECT_EQ(ptr, test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);

  EXPECT_CALL(*kMockSystemAlloc, _free(test_ptr)).Times(1);
  __wrap_free(test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, SystemCallocIsDefault) {
  size_t size = 10;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _calloc(size, sizeof(int)))
      .WillOnce(testing::Return(test_ptr));
  EXPECT_CALL(*kMockRedisModule, Calloc(size, sizeof(int))).Times(0);
  void* ptr = __wrap_calloc(size, sizeof(int));
  EXPECT_EQ(ptr, test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);

  EXPECT_CALL(*kMockSystemAlloc, _free(test_ptr)).Times(1);
  __wrap_free(test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, SystemAlignedAllocIsDefault) {
  size_t size = 10;
  size_t align = 1024;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _aligned_alloc(align, size))
      .WillOnce(testing::Return(test_ptr));
  EXPECT_CALL(*kMockRedisModule, Alloc(align)).Times(0);
  void* ptr = __wrap_aligned_alloc(align, size);
  EXPECT_EQ(ptr, test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);

  EXPECT_CALL(*kMockSystemAlloc, _free(test_ptr)).Times(1);
  __wrap_free(test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, MallocUsableSize) {
  vmsdk::UseValkeyAlloc();
  size_t valkey_size = 20;
  void* valkey_test_ptr = reinterpret_cast<void*>(0xBADF00D1);
  EXPECT_CALL(*kMockRedisModule, Alloc(valkey_size))
      .WillOnce(testing::Return(valkey_test_ptr));
  EXPECT_CALL(*kMockRedisModule, MallocUsableSize(valkey_test_ptr))
      .Times(3)
      .WillRepeatedly(testing::Return(valkey_size));

  void* valkey_ptr = __wrap_malloc(valkey_size);
  EXPECT_EQ(valkey_ptr, valkey_test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), valkey_size);
  EXPECT_EQ(__wrap_malloc_usable_size(valkey_ptr), valkey_size);

  EXPECT_CALL(*kMockRedisModule, Free(valkey_test_ptr)).Times(1);
  __wrap_free(valkey_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, SwitchToValkeyAlloc) {
  vmsdk::UseValkeyAlloc();

  size_t size = 10;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _malloc(size)).Times(0);
  EXPECT_CALL(*kMockRedisModule, Alloc(size))
      .WillOnce(testing::Return(test_ptr));
  EXPECT_CALL(*kMockRedisModule, MallocUsableSize(test_ptr))
      .Times(2)
      .WillRepeatedly(testing::Return(size));

  void* ptr = __wrap_malloc(10);
  EXPECT_EQ(ptr, test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), size);

  EXPECT_CALL(*kMockRedisModule, Free(test_ptr)).Times(1);
  __wrap_free(test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, SwitchToValkeyCalloc) {
  size_t size = 10;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _calloc(size, sizeof(int))).Times(0);
  EXPECT_CALL(*kMockRedisModule, Calloc(size, sizeof(int)))
      .WillOnce(testing::Return(test_ptr));
  EXPECT_CALL(*kMockRedisModule, MallocUsableSize(test_ptr))
      .Times(2)
      .WillRepeatedly(testing::Return(size * sizeof(int)));

  vmsdk::UseValkeyAlloc();
  void* ptr = __wrap_calloc(size, sizeof(int));
  EXPECT_EQ(ptr, test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), size * sizeof(int));

  EXPECT_CALL(*kMockRedisModule, Free(test_ptr)).Times(1);
  __wrap_free(test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, SwitchToValkeyAlignedAlloc) {
  size_t size = 10;
  size_t align = 1024;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _aligned_alloc(align, size)).Times(0);
  EXPECT_CALL(*kMockRedisModule, Alloc(align))
      .WillOnce(testing::Return(test_ptr));
  EXPECT_CALL(*kMockRedisModule, MallocUsableSize(test_ptr))
      .Times(2)
      .WillRepeatedly(testing::Return(align));

  vmsdk::UseValkeyAlloc();
  void* ptr = __wrap_aligned_alloc(align, size);
  EXPECT_EQ(ptr, test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), align);

  EXPECT_CALL(*kMockRedisModule, Free(test_ptr)).Times(1);
  __wrap_free(test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, FreeSystemAllocAfterSwitching) {
  size_t size = 10;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _malloc(size))
      .WillOnce(testing::Return(test_ptr));
  void* ptr = __wrap_malloc(10);
  EXPECT_EQ(ptr, test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);

  vmsdk::UseValkeyAlloc();
  EXPECT_CALL(*kMockSystemAlloc, _free(test_ptr)).Times(1);
  __wrap_free(test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, SystemFreeNullptr) {
  EXPECT_CALL(*kMockSystemAlloc, _malloc_usable_size(testing::_)).Times(0);
  EXPECT_CALL(*kMockSystemAlloc, _free(testing::_)).Times(0);
  __wrap_free(nullptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, ValkeyFreeNullptr) {
  vmsdk::UseValkeyAlloc();
  EXPECT_CALL(*kMockRedisModule, MallocUsableSize(testing::_)).Times(0);
  EXPECT_CALL(*kMockRedisModule, Free(testing::_)).Times(0);
  __wrap_free(nullptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, SystemAllocReturnsNullptr) {
  size_t size = 10;
  EXPECT_CALL(*kMockSystemAlloc, _malloc(size))
      .WillOnce(testing::Return(nullptr));
  EXPECT_CALL(*kMockSystemAlloc, _malloc_usable_size(testing::_)).Times(0);
  void* ptr = __wrap_malloc(size);
  EXPECT_EQ(ptr, nullptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, ValkeyAllocReturnsNullptr) {
  size_t size = 10;
  vmsdk::UseValkeyAlloc();
  EXPECT_CALL(*kMockRedisModule, Alloc(size))
      .WillOnce(testing::Return(nullptr));
  EXPECT_CALL(*kMockRedisModule, MallocUsableSize(testing::_)).Times(0);
  void* ptr = __wrap_malloc(size);
  EXPECT_EQ(ptr, nullptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}

TEST_F(MemoryAllocationTest, SystemReallocBasic) {
  size_t initial_size = 10;
  size_t realloc_size = 20;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _malloc(initial_size))
      .WillOnce(testing::Return(test_ptr));

  void* test_ptr_2 = reinterpret_cast<void*>(0xBADF00D1);
  EXPECT_CALL(*kMockSystemAlloc, _realloc(test_ptr, realloc_size))
      .WillOnce(testing::Return(test_ptr_2));

  void* ptr = __wrap_malloc(initial_size);
  EXPECT_EQ(ptr, test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
  void* ptr_2 = __wrap_realloc(ptr, realloc_size);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
  EXPECT_EQ(ptr_2, test_ptr_2);

  EXPECT_CALL(*kMockSystemAlloc, _free(ptr_2)).Times(1);
  __wrap_free(ptr_2);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}
TEST_F(MemoryAllocationTest, SystemReallocNullptr) {
  size_t realloc_size = 20;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _realloc(nullptr, realloc_size))
      .WillOnce(testing::Return(test_ptr));

  void* ptr = __wrap_realloc(nullptr, realloc_size);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
  EXPECT_EQ(ptr, test_ptr);

  EXPECT_CALL(*kMockSystemAlloc, _free(test_ptr)).Times(1);
  __wrap_free(ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}
TEST_F(MemoryAllocationTest, SystemReallocAfterSwitch) {
  size_t initial_size = 10;
  size_t realloc_size = 20;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _malloc(initial_size))
      .WillOnce(testing::Return(test_ptr));

  void* test_ptr_2 = reinterpret_cast<void*>(0xBADF00D1);
  EXPECT_CALL(*kMockSystemAlloc, _realloc(test_ptr, realloc_size))
      .WillOnce(testing::Return(test_ptr_2));

  void* ptr = __wrap_malloc(initial_size);
  EXPECT_EQ(ptr, test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);

  vmsdk::UseValkeyAlloc();

  void* ptr_2 = __wrap_realloc(ptr, realloc_size);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
  EXPECT_EQ(ptr_2, test_ptr_2);

  EXPECT_CALL(*kMockSystemAlloc, _free(ptr_2)).Times(1);
  __wrap_free(ptr_2);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}
TEST_F(MemoryAllocationTest, ValkeyReallocBasic) {
  size_t initial_size = 10;
  size_t realloc_size = 20;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockRedisModule, Alloc(initial_size))
      .WillOnce(testing::Return(test_ptr));
  EXPECT_CALL(*kMockRedisModule, MallocUsableSize(test_ptr))
      .Times(2)
      .WillRepeatedly(testing::Return(initial_size));

  void* test_ptr_2 = reinterpret_cast<void*>(0xBADF00D1);
  EXPECT_CALL(*kMockRedisModule, Realloc(test_ptr, realloc_size))
      .WillOnce(testing::Return(test_ptr_2));
  EXPECT_CALL(*kMockRedisModule, MallocUsableSize(test_ptr_2))
      .Times(2)
      .WillRepeatedly(testing::Return(realloc_size));

  vmsdk::UseValkeyAlloc();

  void* ptr = __wrap_malloc(initial_size);
  EXPECT_EQ(ptr, test_ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), initial_size);

  void* ptr_2 = __wrap_realloc(ptr, realloc_size);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), realloc_size);
  EXPECT_EQ(ptr_2, test_ptr_2);

  EXPECT_CALL(*kMockRedisModule, Free(test_ptr_2)).Times(1);
  __wrap_free(ptr_2);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}
TEST_F(MemoryAllocationTest, ValkeyReallocNullptr) {
  size_t realloc_size = 20;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockRedisModule, MallocUsableSize(test_ptr))
      .Times(2)
      .WillRepeatedly(testing::Return(realloc_size));
  EXPECT_CALL(*kMockRedisModule, Realloc(nullptr, realloc_size))
      .WillOnce(testing::Return(test_ptr));

  vmsdk::UseValkeyAlloc();
  void* ptr = __wrap_realloc(nullptr, realloc_size);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), realloc_size);
  EXPECT_EQ(ptr, test_ptr);

  EXPECT_CALL(*kMockRedisModule, Free(ptr)).Times(1);
  __wrap_free(ptr);
  EXPECT_EQ(vmsdk::GetUsedMemoryCnt(), 0);
}
TEST_F(MemoryAllocationTest, SystemFreeUntracksPointer) {
  size_t size = 10;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _malloc(testing::_))
      .WillOnce(testing::Return(test_ptr));
  __wrap_malloc(size);
  EXPECT_CALL(*kMockSystemAlloc, _free(testing::_)).Times(1);
  __wrap_free(test_ptr);

  vmsdk::UseValkeyAlloc();

  EXPECT_CALL(*kMockRedisModule, Alloc(testing::_))
      .WillOnce(testing::Return(test_ptr));
  __wrap_malloc(size);
  EXPECT_CALL(*kMockRedisModule, Free(testing::_)).Times(1);
  __wrap_free(test_ptr);
}
TEST_F(MemoryAllocationTest, SystemFreeDefaultsDuringBootstrap) {
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _free(testing::_)).Times(1);
  __wrap_free(test_ptr);
}
TEST_F(MemoryAllocationTest, PosixMemalignOverride) {
  size_t size = 10;
  size_t align = 1024;
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _aligned_alloc(align, size))
      .WillOnce(testing::Return(test_ptr));
  void* out_ptr;
  __wrap_posix_memalign(&out_ptr, align, size);
  EXPECT_EQ(out_ptr, test_ptr);
  __wrap_free(test_ptr);
}
TEST_F(MemoryAllocationTest, VallocOverride) {
  size_t size = 10;
  size_t page_size = sysconf(_SC_PAGESIZE);
  void* test_ptr = reinterpret_cast<void*>(0xBAADF00D);
  EXPECT_CALL(*kMockSystemAlloc, _aligned_alloc(page_size, size))
      .WillOnce(testing::Return(test_ptr));
  EXPECT_EQ(__wrap_valloc(size), test_ptr);
  __wrap_free(test_ptr);
}
#endif  // TESTING_TMP_DISABLED
}  // namespace

}  // namespace vmsdk
