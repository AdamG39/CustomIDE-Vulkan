A useful object for synchronizing the CPU and GPU is a semaphore. A semaphore applies a constraint on GPU execution until a previous task is complete. This is achieved by setting a semaphores signal, by default a semaphore is created with its signal set to off unless explicitly stated otherwise during semaphore creation. Semaphores signals can be set to on by the command buffer once a command has completed execution allowing for commands to be halted until the current GPU command execution is complete.

To create a semaphore, call:
```c++
// Provided by VK_VERSION_1_0
VkResult vkCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore);
```

- `device` is the logical device that creates the semaphore.
- `pCreateInfo` is a pointer to a [VkSemaphoreCreateInfo](https://docs.vulkan.org/spec/latest/chapters/synchronization.html#VkSemaphoreCreateInfo) structure containing information about how the semaphore is to be created.
- `pAllocator` controls host memory allocation as described in the [Memory Allocation](https://docs.vulkan.org/spec/latest/chapters/memory.html#memory-allocation) chapter.
- `pSemaphore` is a pointer to a handle in which the resulting semaphore object is returned.

Return Codes
- `VK_SUCCESS`
- `VK_ERROR_OUT_OF_HOST_MEMORY`
- `VK_ERROR_OUT_OF_DEVICE_MEMORY`

The `VkSemaphoreCreateInfo` structure is defined as:
```c++
// Provided by VK_VERSION_1_0
typedef struct VkSemaphoreCreateInfo {
    VkStructureType           sType;
    const void*               pNext;
    VkSemaphoreCreateFlags    flags;
} VkSemaphoreCreateInfo;
```

- `sType` is a [VkStructureType](https://docs.vulkan.org/spec/latest/chapters/fundamentals.html#VkStructureType) value identifying this structure. For this structure `sType` should be `VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO`
- `pNext` is `NULL` or a pointer to a structure extending this structure.
- `flags` is reserved for future use.

