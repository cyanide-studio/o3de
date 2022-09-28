// {BEGIN_LICENSE}
/*
 * Copyright (c) Cyanide Studio 2022
 *
 */
// {END_LICENSE}


#include <${Name}ModuleInterface.h>
#include "${Name}SystemComponent.h"

namespace ${SanitizedCppName}
{
    class ${SanitizedCppName}Module
        : public ${SanitizedCppName}ModuleInterface
    {
    public:
        AZ_RTTI(${SanitizedCppName}Module, "${ModuleClassId}", ${SanitizedCppName}ModuleInterface);
        AZ_CLASS_ALLOCATOR(${SanitizedCppName}Module, AZ::SystemAllocator, 0);
    };
}// namespace ${SanitizedCppName}

AZ_DECLARE_MODULE_CLASS(Gem_${SanitizedCppName}, ${SanitizedCppName}::${SanitizedCppName}Module)
