/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Memory/PoolAllocator.h>
#include <RHI/MemoryView.h>

// @CYA EDIT: Replace O3DE allocator by VMA
#include <vma/vk_mem_alloc.h>
// @CYA END

namespace AZ
{
    namespace Vulkan
    {
        //! A BufferMemory represents a Vulkan's Buffer that will be sub allocated into one
        //! or multiple RHI Buffer Resources. Each of the RHI Buffers reference a section of the Vulkan Buffer.
        //! The Vulkan Buffer sits on top of a Vulkan Memory Region.
        //! When it is removed, the native buffer will be freed.
        //!      _______________________________________________________________
        //!     |                           VkMemory                            |
        //!     |_______________________________________________________________|
        //!      _______________________________  ______________________________
        //!     |  VkBuffer (BufferMemoryView)  ||  VkBuffer (BufferMemoryView) |
        //!     |_______________________________||______________________________|
        //!      _______________  ______________  ______________________________
        //!     |   RHI Buffer  ||  RHI Buffer  ||         RHI Buffer           |
        //!     |_______________||______________||______________________________|
        //!
        class BufferMemory
            : public RHI::DeviceObject
        {
            using Base = RHI::DeviceObject;

        public:
            AZ_CLASS_ALLOCATOR(BufferMemory, AZ::ThreadPoolAllocator, 0);
            AZ_RTTI(BufferMemory, "{39053FBD-CE0E-44E8-A9BF-29C4014C3958}", Base);

            using Descriptor = RHI::BufferDescriptor;

            ~BufferMemory() = default;

            static RHI::Ptr<BufferMemory> Create();
// @CYA EDIT: Replace O3DE allocator by VMA
            RHI::ResultCode Init(Device& device, const Descriptor& descriptor);
// @CYA END
            RHI::ResultCode Init(Device& device, const MemoryView& memoryView, const Descriptor& descriptor);
            RHI::ResultCode Init(Device& device, VkBuffer vkBuffer, const MemoryView& memoryView, const Descriptor& descriptor);

// @CYA EDIT: Replace O3DE allocator by VMA
            bool IsVmaAllocated() const;

            const VmaAllocationInfo& GetVmaAllocationInfo() const;
// @CYA END

            // It maps a memory and returns its mapped address.
            CpuVirtualAddress Map(size_t offset, size_t size, RHI::HostMemoryAccess hostAccess);

            // Unmap have to be called after Map is called for memory.
            void Unmap(size_t offset, RHI::HostMemoryAccess hostAccess);

            const VkBuffer GetNativeBuffer();
            const Descriptor& GetDescriptor() const;

        private:
            BufferMemory() = default;

            //////////////////////////////////////////////////////////////////////////
            // RHI::Object
            void SetNameInternal(const AZStd::string_view& name) override;
            //////////////////////////////////////////////////////////////////////////

            //////////////////////////////////////////////////////////////////////////
            // RHI::DeviceObject
            void Shutdown() override;
            //////////////////////////////////////////////////////////////////////////

            Descriptor m_descriptor;
            VkBuffer m_vkBuffer = VK_NULL_HANDLE;
            MemoryView m_memoryView;
// @CYA EDIT: Replace O3DE allocator by VMA
            VmaAllocation m_vmaAllocation = VK_NULL_HANDLE;
            VmaAllocationInfo m_vmaAllocationInfo;
// @CYA END
        };
    }
}
