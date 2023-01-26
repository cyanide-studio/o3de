// @CYA EDIT: add MultiMotionComponent
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Component/ComponentBus.h>

namespace EMotionFX
{
    class MotionInstance;
    class MotionInstanceEventHandler;

    namespace Integration
    {
        class MultiMotionComponentRequests
            : public AZ::ComponentBus
        {
        public:

            static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

            virtual AZ::u32 GetChannelsCount() const = 0;
            virtual void LoopMotion(AZ::u8 channel, bool enable) = 0;
            virtual bool GetLoopMotion(AZ::u8 channel) const = 0;
            virtual void RetargetMotion(AZ::u8 channel, bool enable) = 0;
            virtual void ReverseMotion(AZ::u8 channel, bool enable) = 0;
            virtual void MirrorMotion(AZ::u8 channel, bool enable) = 0;
            virtual void SetPlaySpeed(AZ::u8 channel, float speed) = 0;
            virtual float GetPlaySpeed(AZ::u8 channel) const = 0;
            virtual void PlayTime(AZ::u8 channel, float time) = 0;
            virtual float GetPlayTime(AZ::u8 channel) const = 0;
            virtual float GetDuration(AZ::u8 channel) const = 0;
            virtual void Motion(AZ::u8 channel, AZ::Data::AssetId assetId) = 0;
            virtual AZ::Data::AssetId  GetMotion(AZ::u8 channel) const = 0;
            virtual void BlendInTime(AZ::u8 channel, float time) = 0;
            virtual float GetBlendInTime(AZ::u8 channel) const = 0;
            virtual void BlendOutTime(AZ::u8 channel, float time) = 0;
            virtual float GetBlendOutTime(AZ::u8 channel) const = 0;
            virtual void PlayMotion(AZ::u8 channel) = 0;

            // Motion event handler functions
            virtual void AddEventHandler(AZ::u8 channel, MotionInstanceEventHandler* eventHandler) = 0;
            virtual void RemoveEventHandler(AZ::u8 channel, MotionInstanceEventHandler* eventHandler) = 0;
        };
        using MultiMotionComponentRequestBus = AZ::EBus<MultiMotionComponentRequests>;
    }
}
// @CYA END
