// @CYA EDIT: add MultiMotionComponent
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Script/ScriptProperty.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

#include <EMotionFX/Source/PlayBackInfo.h>
#include <Integration/Assets/MotionAsset.h>
#include <Integration/ActorComponentBus.h>
#include <Integration/SimpleMotionComponentBus.h>
#include <Integration/MultiMotionComponentBus.h>

namespace EMotionFX
{
    namespace Integration
    {
        class MultiMotionComponent
            : public AZ::Component
            , private AZ::Data::AssetBus::MultiHandler
            , private ActorComponentNotificationBus::Handler
            , private SimpleMotionComponentRequestBus::Handler
            , private MultiMotionComponentRequestBus::Handler
        {
        public:

            friend class EditorMultiMotionComponent;

            AZ_COMPONENT(MultiMotionComponent, "{25EC370C-FDA1-4610-A8C3-C951C770E82A}");

            /**
            * Configuration struct for procedural configuration of MultiMotionComponents.
            */
            struct Configuration
            {
                AZ_TYPE_INFO(Configuration, "{418398EA-1F0F-461E-844D-EB229E3637F0}")
                Configuration();

                AZ::Data::Asset<MotionAsset>         m_motionAsset;             ///< Assigned motion asset
                EMotionBlendMode                     m_blendMode;               ///< Blend Mode
                bool                                 m_loop;                    ///< Toggles looping of the motion
                bool                                 m_retarget;                ///< Toggles retargeting of the motion
                bool                                 m_reverse;                 ///< Toggles reversing of the motion
                bool                                 m_mirror;                  ///< Toggles mirroring of the motion
                float                                m_playspeed;               ///< Determines the rate at which the motion is played
                float                                m_blendInTime;             ///< Determines the blend in time in seconds.
                float                                m_blendOutTime;            ///< Determines the blend out time in seconds.
                bool                                 m_playOnActivation;        ///< Determines if the motion should be played immediately
                bool                                 m_inPlace;                 ///< Determines if the motion should be played in-place.
                bool                                 m_freezeAtLastFrame = true;///< Determines if the motion will go to bind pose after finishing or freeze at the last frame.

                static void Reflect(AZ::ReflectContext* context);

                AZ::Crc32 GetBlendOutTimeVisibility() const;
                AZ::Crc32 GetFreezeAtLastFrameVisibility() const;
            };

            struct Channel
            {
                AZ_TYPE_INFO(Channel, "{44E0DA68-87BB-4354-87B5-58CD0AF11681}")
                Channel();

                AZ::u8 m_channelIndex;
                Configuration                               m_configuration;        ///< Component configuration.
                EMotionFX::MotionInstance*                  m_motionInstance;       ///< Motion to play on the actor
                AZ::Data::Asset<MotionAsset>                m_lastMotionAsset;      ///< Last active motion asset, kept alive for blending.
                EMotionFX::MotionInstance*                  m_lastMotionInstance;   ///< Last active motion instance, kept alive for blending.
                AZStd::vector<MotionInstanceEventHandler*>  m_eventHandlers;        ///< Save motion events list for when there is no motion instance

                static void Reflect(AZ::ReflectContext* context);
            };

            MultiMotionComponent(const AZStd::vector<Channel>* channels = nullptr);
            ~MultiMotionComponent();

            // AZ::Component interface implementation
            void Init() override;
            void Activate() override;
            void Deactivate() override;

            static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
            {
                provided.push_back(AZ_CRC("EMotionFXSimpleMotionService", 0xea7a05d8));
            }
            static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
            {
                dependent.push_back(AZ_CRC("PhysicsService", 0xa7350d22));
                dependent.push_back(AZ_CRC("MeshService", 0x71d8a455));
            }
            static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
            {
                required.push_back(AZ_CRC("EMotionFXActorService", 0xd6e8f48d));
            }
            static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
            {
                incompatible.push_back(AZ_CRC("EMotionFXAnimGraphService", 0x9ec3c819));
                incompatible.push_back(AZ_CRC("EMotionFXSimpleMotionService", 0xea7a05d8));
                incompatible.push_back(AZ_CRC_CE("NonUniformScaleService"));
            }
            static void Reflect(AZ::ReflectContext* /*context*/);

            // SimpleMotionComponentRequestBus::Handler
            void LoopMotion(bool enable) override { LoopMotion(GetLastActiveChannel(), enable); }
            bool GetLoopMotion() const override  { return GetLoopMotion(GetLastActiveChannel()); }
            void RetargetMotion(bool enable) override  { RetargetMotion(GetLastActiveChannel(), enable); }
            void ReverseMotion(bool enable) override { ReverseMotion(GetLastActiveChannel(), enable); }
            void MirrorMotion(bool enable) override { MirrorMotion(GetLastActiveChannel(), enable); }
            void SetPlaySpeed(float speed) override { SetPlaySpeed(GetLastActiveChannel(), speed); }
            float GetPlaySpeed() const override { return GetPlaySpeed(GetLastActiveChannel()); }
            void PlayTime(float time) override { PlayTime(GetLastActiveChannel(), time); }
            float GetPlayTime() const override { return GetPlayTime(GetLastActiveChannel()); }
            float GetDuration() const override { return GetDuration(GetLastActiveChannel()); }
            void Motion(AZ::Data::AssetId assetId) override { Motion(GetLastActiveChannel(), assetId); }
            AZ::Data::AssetId  GetMotion() const override { return GetMotion(GetLastActiveChannel()); }
            void BlendInTime(float time) override { BlendInTime(GetLastActiveChannel(), time); }
            float GetBlendInTime() const override { return GetBlendInTime(GetLastActiveChannel()); }
            void BlendOutTime(float time) override { BlendOutTime(GetLastActiveChannel(), time); }
            float GetBlendOutTime() const override { return GetBlendOutTime(GetLastActiveChannel()); }
            void PlayMotion() override { PlayMotion(GetLastActiveChannel()); }
            void AddEventHandler(MotionInstanceEventHandler* eventHandler) override { AddEventHandler(GetLastActiveChannel(), eventHandler); }
            void RemoveEventHandler(MotionInstanceEventHandler* eventHandler) override { RemoveEventHandler(GetLastActiveChannel(), eventHandler); }

            // MultiMotionComponentRequestBus::Handler
            AZ::u32 GetChannelsCount() const override;
            void LoopMotion(AZ::u8 channel, bool enable) override;
            bool GetLoopMotion(AZ::u8 channel) const override;
            void RetargetMotion(AZ::u8 channel, bool enable) override;
            void ReverseMotion(AZ::u8 channel, bool enable) override;
            void MirrorMotion(AZ::u8 channel, bool enable) override;
            void SetPlaySpeed(AZ::u8 channel, float speed) override;
            float GetPlaySpeed(AZ::u8 channel) const override;
            void PlayTime(AZ::u8 channel, float time) override;
            float GetPlayTime(AZ::u8 channel) const override;
            float GetDuration(AZ::u8 channel) const override;
            void Motion(AZ::u8 channel, AZ::Data::AssetId assetId) override;
            AZ::Data::AssetId  GetMotion(AZ::u8 channel) const override;
            void BlendInTime(AZ::u8 channel, float time) override;
            float GetBlendInTime(AZ::u8 channel) const override;
            void BlendOutTime(AZ::u8 channel, float time) override;
            float GetBlendOutTime(AZ::u8 channel) const override;
            void PlayMotion(AZ::u8 channel) override;
            void AddEventHandler(AZ::u8 channel, MotionInstanceEventHandler* eventHandler) override;
            void RemoveEventHandler(AZ::u8 channel, MotionInstanceEventHandler* eventHandler) override;

            const EMotionFX::MotionInstance* GetMotionInstance(AZ::u8 channel);
            void SetMotionAssetId(AZ::u8 channel, const AZ::Data::AssetId& assetId);

            // AZ::Data::AssetBus::Handler
            void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
            void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

        private:
            AZ::u8 GetLastActiveChannel() const { return m_lastActiveChannel; }

            // ActorComponentNotificationBus::Handler
            void OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance) override;
            void OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance) override;

            void RemoveMotionInstanceFromActor(EMotionFX::MotionInstance* motionInstance);

            static EMotionFX::MotionInstance* PlayMotionInternal(const EMotionFX::ActorInstance* actorInstance, const Configuration& cfg, bool deleteOnZeroWeight);

            AZ::u8 m_lastActiveChannel;
            AZStd::vector<Channel> m_channels;
            EMotionFXPtr<EMotionFX::ActorInstance> m_actorInstance;        ///< Associated actor instance (retrieved from Actor Component).
        };
    } // namespace Integration
} // namespace EMotionFX
// @CYA END
