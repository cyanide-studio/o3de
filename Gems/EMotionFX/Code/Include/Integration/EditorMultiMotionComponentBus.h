// @CYA EDIT: add MultiMotionComponent
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Component/ComponentBus.h>

namespace EMotionFX
{
    namespace Integration
    {
        class EditorMultiMotionComponentRequests
            : public AZ::ComponentBus
        {
        public:
            virtual void SetPreviewInEditor(bool enable) = 0;
            virtual bool GetPreviewInEditor() const = 0;
            virtual float GetAssetDuration(const AZ::Data::AssetId& assetId) = 0;
        };
        using EditorMultiMotionComponentRequestBus = AZ::EBus<EditorMultiMotionComponentRequests>;
    }
}
// @CYA END
