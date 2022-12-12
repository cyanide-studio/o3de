/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Script/ScriptContext.h>

#include <Integration/Components/MultiMotionComponent.h>
#include <MCore/Source/AttributeString.h>
#include <EMotionFX/Source/ActorInstance.h>
#include <EMotionFX/Source/MotionSystem.h>
#include <EMotionFX/Source/MotionInstance.h>

namespace EMotionFX
{
    namespace Integration
    {
        void MultiMotionComponent::Configuration::Reflect(AZ::ReflectContext *context)
        {
            auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
            if (serializeContext)
            {
                serializeContext->Class<Configuration>()
                    ->Version(4)
                    ->Field("MotionAsset", &Configuration::m_motionAsset)
                    ->Field("BlendMode", &Configuration::m_blendMode)
                    ->Field("Loop", &Configuration::m_loop)
                    ->Field("Retarget", &Configuration::m_retarget)
                    ->Field("Reverse", &Configuration::m_reverse)
                    ->Field("Mirror", &Configuration::m_mirror)
                    ->Field("PlaySpeed", &Configuration::m_playspeed)
                    ->Field("BlendIn", &Configuration::m_blendInTime)
                    ->Field("BlendOut", &Configuration::m_blendOutTime)
                    ->Field("PlayOnActivation", &Configuration::m_playOnActivation)
                    ->Field("InPlace", &Configuration::m_inPlace)
                    ->Field("FreezeAtLastFrame", &Configuration::m_freezeAtLastFrame)
                    ;

                AZ::EditContext* editContext = serializeContext->GetEditContext();
                if (editContext)
                {
                    editContext->Class<Configuration>( "Configuration", "Settings for this Simple Motion")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_motionAsset, "Motion", "EMotion FX motion to be loaded for this actor")
                        ->DataElement(AZ::Edit::UIHandlers::ComboBox, &Configuration::m_blendMode, "Blend Mode", "Blend Mode")
                            ->EnumAttribute(EMotionFX::BLENDMODE_OVERWRITE, "Overwrite")
                            ->EnumAttribute(EMotionFX::BLENDMODE_ADDITIVE, "Additive")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_loop, "Loop motion", "Toggles looping of the animation")
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_retarget, "Retarget motion", "Toggles retargeting of the animation")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_reverse, "Reverse motion", "Toggles reversing of the animation")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_mirror, "Mirror motion", "Toggles mirroring of the animation")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_playspeed, "Play speed", "Determines the rate at which the motion is played")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                            ->Attribute(AZ::Edit::Attributes::Max, 100.0f)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_blendInTime, "Blend In Time", "Determines the blend in time in seconds")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                            ->Attribute(AZ::Edit::Attributes::Max, 100.0f)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_blendOutTime, "Blend Out Time", "Determines the blend out time in seconds")
                            ->Attribute(AZ::Edit::Attributes::Visibility, &Configuration::GetBlendOutTimeVisibility)
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                            ->Attribute(AZ::Edit::Attributes::Max, 100.0f)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_playOnActivation, "Play on active", "Playing animation immediately after activation.")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_inPlace, "In-place",
                            "Plays the animation in-place and removes any positional and rotational changes from root joints.")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Configuration::m_freezeAtLastFrame, "Freeze At Last Frame",
                            "Go back to bind pose after finishing the motion or freeze at the last frame. This only applies for non-looping motions.")
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                            ->Attribute(AZ::Edit::Attributes::Visibility, &Configuration::GetFreezeAtLastFrameVisibility)
                        ;
                }
            }
        }
        
        void MultiMotionComponent::Channel::Reflect(AZ::ReflectContext *context)
        {
            auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
            if (serializeContext)
            {
                serializeContext->Class<Channel>()
                    ->Version(1)
                    ->Field("Channel", &Channel::m_channelIndex)
                    ->Field("Configuration", &Channel::m_configuration)
                    ;

                AZ::EditContext* editContext = serializeContext->GetEditContext();
                if (editContext)
                {
                    editContext->Class<Channel>( "Channel", "Channel")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &Channel::m_configuration, "Data", "")
                        ;
                }
            }
        }

        AZ::Crc32 MultiMotionComponent::Configuration::GetBlendOutTimeVisibility() const
        {
            if (!m_loop && !m_freezeAtLastFrame)
            {
                return AZ::Edit::PropertyVisibility::Show;
            }

            return AZ::Edit::PropertyVisibility::Hide;
        }

        AZ::Crc32 MultiMotionComponent::Configuration::GetFreezeAtLastFrameVisibility() const
        {
            return m_loop ? AZ::Edit::PropertyVisibility::Hide : AZ::Edit::PropertyVisibility::Show;
        }

        void MultiMotionComponent::Reflect(AZ::ReflectContext* context)
        {
            Configuration::Reflect(context);
            Channel::Reflect(context);

            auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
            if (serializeContext)
            {
                serializeContext->Class<MultiMotionComponent, AZ::Component>()
                    ->Version(1)
                    ->Field("Channels", &MultiMotionComponent::m_channels)
                    ;
            }

            auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
            if (behaviorContext)
            {
                behaviorContext->EBus<MultiMotionComponentRequestBus>("MultiMotionComponentRequestBus")
                    ->Event("GetChannelsCount", &MultiMotionComponentRequestBus::Events::GetChannelsCount)
                    ->Event("LoopMotion", &MultiMotionComponentRequestBus::Events::LoopMotion)
                    ->Event("GetLoopMotion", &MultiMotionComponentRequestBus::Events::GetLoopMotion)
                        ->Attribute("Hidden", AZ::Edit::Attributes::PropertyHidden)
                    ->Event("RetargetMotion", &MultiMotionComponentRequestBus::Events::RetargetMotion)
                    ->Event("ReverseMotion", &MultiMotionComponentRequestBus::Events::ReverseMotion)
                    ->Event("MirrorMotion", &MultiMotionComponentRequestBus::Events::MirrorMotion)
                    ->Event("SetPlaySpeed", &MultiMotionComponentRequestBus::Events::SetPlaySpeed)
                    ->Event("GetPlaySpeed", &MultiMotionComponentRequestBus::Events::GetPlaySpeed)
                        ->Attribute("Hidden", AZ::Edit::Attributes::PropertyHidden)
                    ->Event("PlayTime", &MultiMotionComponentRequestBus::Events::PlayTime)
                    ->Event("GetPlayTime", &MultiMotionComponentRequestBus::Events::GetPlayTime)
                        ->Attribute("Hidden", AZ::Edit::Attributes::PropertyHidden)
                    ->Event("Motion", &MultiMotionComponentRequestBus::Events::Motion)
                    ->Event("GetMotion", &MultiMotionComponentRequestBus::Events::GetMotion)
                    ->Event("BlendInTime", &MultiMotionComponentRequestBus::Events::BlendInTime)
                    ->Event("GetBlendInTime", &MultiMotionComponentRequestBus::Events::GetBlendInTime)
                        ->Attribute("Hidden", AZ::Edit::Attributes::PropertyHidden)
                    ->Event("BlendOutTime", &MultiMotionComponentRequestBus::Events::BlendOutTime)
                    ->Event("GetBlendOutTime", &MultiMotionComponentRequestBus::Events::GetBlendOutTime)
                        ->Attribute("Hidden", AZ::Edit::Attributes::PropertyHidden)
                    ->Event("PlayMotion", &MultiMotionComponentRequestBus::Events::PlayMotion)
                    ;

                behaviorContext->Class<MultiMotionComponent>()->RequestBus("MultiMotionComponentRequestBus");
            }
        }

        MultiMotionComponent::Configuration::Configuration()
            : m_blendMode(EMotionFX::EMotionBlendMode::BLENDMODE_ADDITIVE)
            , m_loop(false)
            , m_retarget(false)
            , m_reverse(false)
            , m_mirror(false)
            , m_playspeed(1.f)
            , m_blendInTime(0.0f)
            , m_blendOutTime(0.0f)
            , m_playOnActivation(true)
            , m_inPlace(false)
        {
        }

        MultiMotionComponent::Channel::Channel()
            : m_channelIndex(0)
            , m_motionInstance(nullptr)
            , m_lastMotionInstance(nullptr)
        {

        }

        MultiMotionComponent::MultiMotionComponent(const AZStd::vector<Channel>* channels)
            : m_actorInstance(nullptr)
            , m_lastActiveChannel(0)
        {
            if (channels)
            {
                m_channels = *channels;
            }
        }

        MultiMotionComponent::~MultiMotionComponent()
        {
        }

        void MultiMotionComponent::Init()
        {
        }

        void MultiMotionComponent::Activate()
        {
            for(auto& channel : m_channels)
                channel.m_motionInstance = nullptr;

            AZ::Data::AssetBus::MultiHandler::BusDisconnect();
            
            SimpleMotionComponentRequestBus::Handler::BusConnect(GetEntityId());
            MultiMotionComponentRequestBus::Handler::BusConnect(GetEntityId());

            for (auto& channel : m_channels)
            {
                auto& cfg = channel.m_configuration;

                if (cfg.m_motionAsset.GetId().IsValid())
                {
                    AZ::Data::AssetBus::MultiHandler::BusConnect(cfg.m_motionAsset.GetId());
                    cfg.m_motionAsset.QueueLoad();
                }
            }

            ActorComponentNotificationBus::Handler::BusConnect(GetEntityId());
        }

        void MultiMotionComponent::Deactivate()
        {
            MultiMotionComponentRequestBus::Handler::BusDisconnect();
            SimpleMotionComponentRequestBus::Handler::BusDisconnect();
            ActorComponentNotificationBus::Handler::BusDisconnect();
            AZ::Data::AssetBus::MultiHandler::BusDisconnect();
            
            for (auto& channel : m_channels)
            {
                RemoveMotionInstanceFromActor(channel.m_motionInstance);
                channel.m_motionInstance = nullptr;
                RemoveMotionInstanceFromActor(channel.m_lastMotionInstance);
                channel.m_lastMotionInstance = nullptr;
                channel.m_configuration.m_motionAsset.Release();
                channel.m_lastMotionAsset.Release();
            }
            m_actorInstance.reset();
        }

        const MotionInstance* MultiMotionComponent::GetMotionInstance(AZ::u8 channel)
        {
            if (m_channels.size() <= channel)
                return nullptr;
            return m_channels[channel].m_motionInstance;
        }

        void MultiMotionComponent::SetMotionAssetId(AZ::u8 channel, const AZ::Data::AssetId& assetId)
        {
            if (m_channels.size() <= channel)
                return;

           m_channels[channel]. m_configuration.m_motionAsset = AZ::Data::Asset<MotionAsset>(assetId, azrtti_typeid<MotionAsset>());
        }

        void MultiMotionComponent::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
        {
            for (auto& channel : m_channels)
            {
                auto& cfg = channel.m_configuration;

                if (asset.GetId() == cfg.m_motionAsset.GetId())
                {
                    cfg.m_motionAsset = asset;
                    if (cfg.m_playOnActivation)
                    {
                        PlayMotion(channel.m_channelIndex);
                    }
                }
            }
        }

        void MultiMotionComponent::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
        {
            OnAssetReady(asset);
        }

        void MultiMotionComponent::OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance)
        {
            m_actorInstance = actorInstance;
            for (auto& channel : m_channels)
            {
                if (channel.m_configuration.m_playOnActivation)
                {
                    PlayMotion(channel.m_channelIndex);
                }
            }
        }

        void MultiMotionComponent::OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance)
        {
            AZ_UNUSED(actorInstance);
            for (auto& channel : m_channels)
            {
                RemoveMotionInstanceFromActor(channel.m_motionInstance);
                channel.m_motionInstance = nullptr;
                RemoveMotionInstanceFromActor(channel.m_lastMotionInstance);
                channel.m_lastMotionInstance = nullptr;
            }
            m_actorInstance.reset();
        }

        void MultiMotionComponent::PlayMotion(AZ::u8 channel)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_motionInstance = PlayMotionInternal(m_actorInstance.get(), m_channels[channel].m_configuration, /*deleteOnZeroWeight*/true);
            m_lastActiveChannel = channel;

            for (auto* handler : m_channels[channel].m_eventHandlers)
            {
                m_channels[channel].m_motionInstance->AddEventHandler(handler);
            }
        }

        void MultiMotionComponent::AddEventHandler(AZ::u8 channel, MotionInstanceEventHandler* eventHandler)
        {
            if (m_channels.size() <= channel)
                return;

            if (m_channels[channel].m_motionInstance)
                m_channels[channel].m_motionInstance->AddEventHandler(eventHandler);

            m_channels[channel].m_eventHandlers.push_back(eventHandler);
        }

        void MultiMotionComponent::RemoveEventHandler(AZ::u8 channel, MotionInstanceEventHandler* eventHandler)
        {
            if (m_channels.size() <= channel)
                return;

            if (m_channels[channel].m_motionInstance)
                m_channels[channel].m_motionInstance->RemoveEventHandler(eventHandler);

            m_channels[channel].m_eventHandlers.erase(std::remove(m_channels[channel].m_eventHandlers.begin(), m_channels[channel].m_eventHandlers.end(), eventHandler), m_channels[channel].m_eventHandlers.end());
        }

        void MultiMotionComponent::RemoveAllEventHandlers(AZ::u8 channel)
        {
            if (m_channels.size() <= channel)
                return;

            if (m_channels[channel].m_motionInstance)
                m_channels[channel].m_motionInstance->RemoveAllEventHandlers();

            m_channels[channel].m_eventHandlers.clear();
        }

        void MultiMotionComponent::RemoveMotionInstanceFromActor(EMotionFX::MotionInstance* motionInstance)
        {
            if (motionInstance)
            {
                if (m_actorInstance && m_actorInstance->GetMotionSystem())
                {
                    m_actorInstance->GetMotionSystem()->RemoveMotionInstance(motionInstance);
                }
            }
        }

        AZ::u32 MultiMotionComponent::GetChannelsCount() const
        {
            return (AZ::u32)m_channels.size();
        }

        void MultiMotionComponent::LoopMotion(AZ::u8 channel, bool enable)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_loop = enable;
            if (m_channels[channel].m_motionInstance)
            {
                m_channels[channel].m_motionInstance->SetMaxLoops(enable ? EMFX_LOOPFOREVER : 1);
            }
        }

        bool MultiMotionComponent::GetLoopMotion(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return false;

            return m_channels[channel].m_configuration.m_loop;
        }

        void MultiMotionComponent::RetargetMotion(AZ::u8 channel, bool enable)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_retarget = enable;
            if (m_channels[channel].m_motionInstance)
            {
                m_channels[channel].m_motionInstance->SetRetargetingEnabled(enable);
            }
        }

        void MultiMotionComponent::ReverseMotion(AZ::u8 channel, bool enable)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_reverse = enable;
            if (m_channels[channel].m_motionInstance)
            {
                m_channels[channel].m_motionInstance->SetPlayMode(enable ? EMotionFX::EPlayMode::PLAYMODE_BACKWARD : EMotionFX::EPlayMode::PLAYMODE_FORWARD);
            }
        }

        void MultiMotionComponent::MirrorMotion(AZ::u8 channel, bool enable)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_mirror = enable;
            if (m_channels[channel].m_motionInstance)
            {
                m_channels[channel].m_motionInstance->SetMirrorMotion(enable);
            }
        }

        void MultiMotionComponent::SetPlaySpeed(AZ::u8 channel, float speed)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_playspeed = speed;
            if (m_channels[channel].m_motionInstance)
            {
                m_channels[channel].m_motionInstance->SetPlaySpeed(speed);
            }
        }

        float MultiMotionComponent::GetPlaySpeed(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return 0.f;

            return m_channels[channel].m_configuration.m_playspeed;
        }

        void MultiMotionComponent::PlayTime(AZ::u8 channel, float time)
        {
            if (m_channels.size() <= channel)
                return;

            if (m_channels[channel].m_motionInstance)
            {
                float delta = time - m_channels[channel].m_motionInstance->GetLastCurrentTime();
                m_channels[channel].m_motionInstance->SetCurrentTime(time, false);

                // Apply the same time step to the last animation
                // so blend out will be good. Otherwise we are just blending
                // from the last frame played of the last animation.
                if (m_channels[channel].m_lastMotionInstance && m_channels[channel].m_lastMotionInstance->GetIsBlending())
                {
                    m_channels[channel].m_lastMotionInstance->SetCurrentTime(m_channels[channel].m_lastMotionInstance->GetLastCurrentTime() + delta, false);
                }
            }
        }

        float MultiMotionComponent::GetPlayTime(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return 0.f;

            float result = 0.0f;
            if (m_channels[channel].m_motionInstance)
            {
                result = m_channels[channel].m_motionInstance->GetCurrentTimeNormalized();
            }
            return result;
        }
        
        float MultiMotionComponent::GetDuration(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return 0.f;

            float result = 0.0f;
            if (m_channels[channel].m_motionInstance)
            {
                result = m_channels[channel].m_motionInstance->GetDuration();
            }
            return result;
        }

        void MultiMotionComponent::Motion(AZ::u8 channel, AZ::Data::AssetId assetId)
        {
            if (m_channels.size() <= channel)
                return;

            if (m_channels[channel].m_configuration.m_motionAsset.GetId() != assetId)
            {
                // Disconnect the old asset bus
                if (AZ::Data::AssetBus::MultiHandler::BusIsConnectedId(m_channels[channel].m_configuration.m_motionAsset.GetId()))
                {
                    AZ::Data::AssetBus::MultiHandler::BusDisconnect(m_channels[channel].m_configuration.m_motionAsset.GetId());
                }

                // Save the motion asset that we are about to be remove in case it can be reused.
                AZ::Data::Asset<MotionAsset> oldLastMotionAsset = m_channels[channel].m_lastMotionAsset;

                if (m_channels[channel].m_lastMotionInstance)
                {
                    RemoveMotionInstanceFromActor(m_channels[channel].m_lastMotionInstance);
                }

                // Store the current motion asset as the last one for possible blending.
                // If we don't keep a reference to the motion asset, the motion instance will be
                // automatically released.
                if (m_channels[channel].m_configuration.m_motionAsset.GetId().IsValid())
                {
                    m_channels[channel].m_lastMotionAsset = m_channels[channel].m_configuration.m_motionAsset;
                }

                // Set the current motion instance as the last motion instance. The new current motion
                // instance will then be set when the load is complete.
                m_channels[channel].m_lastMotionInstance = m_channels[channel].m_motionInstance;
                m_channels[channel].m_motionInstance = nullptr;

                // Start the fade out if there is a blend out time. Otherwise just leave the
                // m_lastMotionInstance where it is at so the next anim can blend from that frame.
                if (m_channels[channel].m_lastMotionInstance && m_channels[channel].m_configuration.m_blendOutTime > 0.0f)
                {
                    m_channels[channel].m_lastMotionInstance->Stop(m_channels[channel].m_configuration.m_blendOutTime);
                    m_channels[channel].m_lastMotionInstance->SetDeleteOnZeroWeight(true);
                }

                // Reuse the old, last motion asset if possible. Otherwise, request a load.
                if (assetId.IsValid() && oldLastMotionAsset.GetData() && assetId == oldLastMotionAsset.GetId())
                {
                    // Even though we are not calling GetAsset here, OnAssetReady
                    // will be fired when the bus is connected because this asset is already loaded.
                    m_channels[channel].m_configuration.m_motionAsset = oldLastMotionAsset;
                }
                else
                {
                    // Won't be able to reuse oldLastMotionAsset, release it.
                    oldLastMotionAsset.Release();

                    // Clear the old asset.
                    m_channels[channel].m_configuration.m_motionAsset.Release();
                    m_channels[channel].m_configuration.m_motionAsset = AZ::Data::Asset<MotionAsset>(assetId, AZ::Uuid("{00494B8E-7578-4BA2-8B28-272E90680787}"));

                    // Create a new asset
                    if (assetId.IsValid())
                    {
                        m_channels[channel].m_configuration.m_motionAsset = AZ::Data::AssetManager::Instance().GetAsset<MotionAsset>(assetId, m_channels[channel].m_configuration.m_motionAsset.GetAutoLoadBehavior());
                    }
                }

                // Connect the bus if the asset is is valid.
                if (m_channels[channel].m_configuration.m_motionAsset.GetId().IsValid())
                {
                    AZ::Data::AssetBus::MultiHandler::BusConnect(m_channels[channel].m_configuration.m_motionAsset.GetId());
                }

            }
        }

        AZ::Data::AssetId MultiMotionComponent::GetMotion(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return {};

            return m_channels[channel].m_configuration.m_motionAsset.GetId();
        }

        void MultiMotionComponent::BlendInTime(AZ::u8 channel, float time)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_blendInTime = time;
        }

        float MultiMotionComponent::GetBlendInTime(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return 0.f;

            return m_channels[channel].m_configuration.m_blendInTime;
        }

        void MultiMotionComponent::BlendOutTime(AZ::u8 channel, float time)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_blendOutTime = time;
        }

        float MultiMotionComponent::GetBlendOutTime(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return 0.f;

            return m_channels[channel].m_configuration.m_blendOutTime;
        }

        EMotionFX::MotionInstance* MultiMotionComponent::PlayMotionInternal(const EMotionFX::ActorInstance* actorInstance, const Configuration& cfg, bool deleteOnZeroWeight)
        {
            if (!actorInstance || !cfg.m_motionAsset.IsReady())
            {
                AZ_Error("EMotionFX", cfg.m_motionAsset.IsError(), "Motion asset %s is in error state (%s).", cfg.m_motionAsset.GetId().ToString<AZStd::string>().c_str(), cfg.m_motionAsset.GetHint().c_str());
                return nullptr;
            }

            if (!actorInstance->GetMotionSystem())
            {
                return nullptr;
            }

            auto* motionAsset = cfg.m_motionAsset.GetAs<MotionAsset>();
            if (!motionAsset)

            {
                AZ_Error("EMotionFX", motionAsset, "Motion asset is not valid.");
                return nullptr;
            }
            //init the PlaybackInfo based on our config
            EMotionFX::PlayBackInfo info;
            info.m_numLoops = cfg.m_loop ? EMFX_LOOPFOREVER : 1;
            info.m_retarget = cfg.m_retarget;
            info.m_playMode = cfg.m_reverse ? EMotionFX::EPlayMode::PLAYMODE_BACKWARD : EMotionFX::EPlayMode::PLAYMODE_FORWARD;
            info.m_freezeAtLastFrame = info.m_numLoops == 1;
            info.m_mirrorMotion = cfg.m_mirror;
            info.m_playSpeed = cfg.m_playspeed;
            info.m_playNow = true;
            info.m_deleteOnZeroWeight = deleteOnZeroWeight;
            info.m_canOverwrite = false;
            info.m_blendInTime = cfg.m_blendInTime;
            info.m_blendOutTime = cfg.m_blendOutTime;
            info.m_inPlace = cfg.m_inPlace;
            info.m_freezeAtLastFrame = cfg.m_freezeAtLastFrame;
            info.m_blendMode = cfg.m_blendMode;
            info.m_mix = cfg.m_blendMode == EMotionFX::EMotionBlendMode::BLENDMODE_ADDITIVE;
            
            return actorInstance->GetMotionSystem()->PlayMotion(motionAsset->m_emfxMotion.get(), &info);
        }

    } // namespace integration
} // namespace EMotionFX

