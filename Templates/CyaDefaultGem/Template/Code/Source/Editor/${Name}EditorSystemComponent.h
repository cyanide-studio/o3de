// {BEGIN_LICENSE}
/*
 * Copyright (c) Cyanide Studio 2022
 *
 */
 // {END_LICENSE}

#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Game/${Name}SystemComponent.h>

namespace ${SanitizedCppName}
{
    /// System component for ${SanitizedCppName} editor
    class ${SanitizedCppName}EditorSystemComponent
        : public ${SanitizedCppName}SystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = ${SanitizedCppName}SystemComponent;
    public:
        AZ_COMPONENT(${SanitizedCppName}EditorSystemComponent, "${EditorSysCompClassId}", BaseSystemComponent);
        static void Reflect(AZ::ReflectContext* context);

        ${SanitizedCppName}EditorSystemComponent();
        ~${SanitizedCppName}EditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace ${SanitizedCppName}
