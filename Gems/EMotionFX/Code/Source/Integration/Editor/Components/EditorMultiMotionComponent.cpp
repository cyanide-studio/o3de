// @CYA EDIT: add Multi Motion Component
#include <AzCore/Component/Entity.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Asset/AssetManager.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Integration/Assets/ActorAsset.h>
#include <Integration/Editor/Components/EditorMultiMotionComponent.h>
#include <Integration/SimpleMotionComponentBus.h>
#include <EMotionFX/Source/MotionSystem.h>
#include <EMotionFX/Source/MotionInstance.h>

namespace EMotionFX
{
    namespace Integration
    {
        void EditorMultiMotionComponent::Reflect(AZ::ReflectContext* context)
        {
            auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
            if (serializeContext)
            {
                serializeContext->Class<EditorMultiMotionComponent, AzToolsFramework::Components::EditorComponentBase>()
                    ->Version(3)
                    ->Field("PreviewInEditor", &EditorMultiMotionComponent::m_previewInEditor)
                    ->Field("Channels", &EditorMultiMotionComponent::m_channels)
                    ;

                AZ::EditContext* editContext = serializeContext->GetEditContext();
                if (editContext)
                {
                    editContext->Class<EditorMultiMotionComponent>(
                        "Multi Motion", "")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Animation")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/SimpleMotion.svg")
                        ->Attribute(AZ::Edit::Attributes::PrimaryAssetType, azrtti_typeid<MotionAsset>())
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/Mannequin.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://o3de.org/docs/user-guide/components/reference/animation/simple-motion/")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(0, &EditorMultiMotionComponent::m_previewInEditor, "Preview In Editor", "Plays motion in Editor")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorMultiMotionComponent::OnEditorPropertyChanged)
                        ->DataElement(0, &EditorMultiMotionComponent::m_channels, "Channels", "Animation Channels")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorMultiMotionComponent::OnEditorPropertyChanged)
                        ;
                }
            }

            AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
            if (behaviorContext)
            {
                behaviorContext->EBus<EditorMultiMotionComponentRequestBus>("EditorMultiMotionComponentRequestBus")
                    ->Event("SetPreviewInEditor", &EditorMultiMotionComponentRequestBus::Events::SetPreviewInEditor)
                    ->Event("GetPreviewInEditor", &EditorMultiMotionComponentRequestBus::Events::GetPreviewInEditor)
                    ->Attribute("Hidden", AZ::Edit::Attributes::PropertyHidden)
                    ->VirtualProperty("PreviewInEditor", "GetPreviewInEditor", "SetPreviewInEditor")
                    ->Event("GetAssetDuration", &EditorMultiMotionComponentRequestBus::Events::GetAssetDuration)
                    ;

                behaviorContext->Class<EditorMultiMotionComponent>()
                    ->RequestBus("MultiMotionComponentRequestBus")
                    ->RequestBus("EditorMultiMotionComponentRequestBus")
                    ;
            }
        }

        EditorMultiMotionComponent::EditorMultiMotionComponent()
            : m_previewInEditor(false)
            , m_actorInstance(nullptr)
        {
        }

        EditorMultiMotionComponent::~EditorMultiMotionComponent()
        {
        }

        void EditorMultiMotionComponent::Activate()
        {
            AZ::Data::AssetBus::MultiHandler::BusDisconnect();
            SimpleMotionComponentRequestBus::Handler::BusConnect(GetEntityId());
            MultiMotionComponentRequestBus::Handler::BusConnect(GetEntityId());
            EditorMultiMotionComponentRequestBus::Handler::BusConnect(GetEntityId());

            //check if our motion has changed
            for (auto& channel : m_channels)
            {
                VerifyMotionAssetState(channel.m_channelIndex);
            }

            ActorComponentNotificationBus::Handler::BusConnect(GetEntityId());
        }

        void EditorMultiMotionComponent::Deactivate()
        {
            ActorComponentNotificationBus::Handler::BusDisconnect();
            EditorMultiMotionComponentRequestBus::Handler::BusDisconnect();
            MultiMotionComponentRequestBus::Handler::BusDisconnect();
            SimpleMotionComponentRequestBus::Handler::BusDisconnect();
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
            m_actorInstance = nullptr;
        }

        void EditorMultiMotionComponent::VerifyMotionAssetState(AZ::u8 channel)
        {
            if (m_channels[channel].m_configuration.m_motionAsset.GetId().IsValid())
            {
                AZ::Data::AssetBus::MultiHandler::BusConnect(m_channels[channel].m_configuration.m_motionAsset.GetId());
                m_channels[channel].m_configuration.m_motionAsset.QueueLoad();
            }
        }

        void EditorMultiMotionComponent::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
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
            AZ::Data::AssetBus::MultiHandler::BusDisconnect(asset.GetId());
        }

        void EditorMultiMotionComponent::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
        {
            OnAssetReady(asset);
        }

        void EditorMultiMotionComponent::OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance)
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

        void EditorMultiMotionComponent::OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance)
        {
            AZ_UNUSED(actorInstance);
            for (auto& channel : m_channels)
            {
                RemoveMotionInstanceFromActor(channel.m_motionInstance);
                channel.m_motionInstance = nullptr;
                RemoveMotionInstanceFromActor(channel.m_lastMotionInstance);
                channel.m_lastMotionInstance = nullptr;
            }
            m_actorInstance = nullptr;
        }

        void EditorMultiMotionComponent::PlayMotion(AZ::u8 channel)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_motionInstance = MultiMotionComponent::PlayMotionInternal(m_actorInstance, m_channels[channel].m_configuration, /*deleteOnZeroWeight*/true);
            m_lastActiveChannel = channel;

            for (auto* handler : m_channels[channel].m_eventHandlers)
            {
                m_channels[channel].m_motionInstance->AddEventHandler(handler);
            }
        }

        void EditorMultiMotionComponent::AddEventHandler(AZ::u8 channel, MotionInstanceEventHandler* eventHandler)
        {
            if (m_channels.size() <= channel)
                return;

            if (m_channels[channel].m_motionInstance)
                m_channels[channel].m_motionInstance->AddEventHandler(eventHandler);

            m_channels[channel].m_eventHandlers.push_back(eventHandler);
        }

        void EditorMultiMotionComponent::RemoveEventHandler(AZ::u8 channel, MotionInstanceEventHandler* eventHandler)
        {
            if (m_channels.size() <= channel)
                return;

            if (m_channels[channel].m_motionInstance)
                m_channels[channel].m_motionInstance->RemoveEventHandler(eventHandler);

            m_channels[channel].m_eventHandlers.erase(std::remove(m_channels[channel].m_eventHandlers.begin(), m_channels[channel].m_eventHandlers.end(), eventHandler),m_channels[channel].m_eventHandlers.end());
        }

        void EditorMultiMotionComponent::RemoveMotionInstanceFromActor(EMotionFX::MotionInstance* motionInstance)
        {
            if (motionInstance)
            {
                if (m_actorInstance && m_actorInstance->GetMotionSystem())
                {
                    m_actorInstance->GetMotionSystem()->RemoveMotionInstance(motionInstance);
                }
            }
        }

        void EditorMultiMotionComponent::BuildGameEntity(AZ::Entity* gameEntity)
        {
            gameEntity->AddComponent(aznew MultiMotionComponent(&m_channels));
        }

        AZ::u32 EditorMultiMotionComponent::GetChannelsCount() const
        {
            return (AZ::u32)m_channels.size();
        }

        void EditorMultiMotionComponent::LoopMotion(AZ::u8 channel, bool enable)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_loop = enable;
            if (m_channels[channel].m_motionInstance)
            {
                m_channels[channel].m_motionInstance->SetMaxLoops(enable ? EMFX_LOOPFOREVER : 1);
            }
        }

        bool EditorMultiMotionComponent::GetLoopMotion(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return false;

            return m_channels[channel].m_configuration.m_loop;
        }

        void EditorMultiMotionComponent::RetargetMotion(AZ::u8 channel, bool enable)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_retarget = enable;
            if (m_channels[channel].m_motionInstance)
            {
                m_channels[channel].m_motionInstance->SetRetargetingEnabled(enable);
            }
        }

        void EditorMultiMotionComponent::ReverseMotion(AZ::u8 channel, bool enable)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_reverse = enable;
            if (m_channels[channel].m_motionInstance)
            {
                m_channels[channel].m_motionInstance->SetPlayMode(enable ? EMotionFX::EPlayMode::PLAYMODE_BACKWARD : EMotionFX::EPlayMode::PLAYMODE_FORWARD);
            }
        }

        void EditorMultiMotionComponent::MirrorMotion(AZ::u8 channel, bool enable)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_mirror = enable;
            if (m_channels[channel].m_motionInstance)
            {
                m_channels[channel].m_motionInstance->SetMirrorMotion(enable);
            }
        }

        void EditorMultiMotionComponent::SetPlaySpeed(AZ::u8 channel, float speed)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_playspeed = speed;
            if (m_channels[channel].m_motionInstance)
            {
                m_channels[channel].m_motionInstance->SetPlaySpeed(speed);
            }
        }

        float EditorMultiMotionComponent::GetPlaySpeed(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return 0.f;

            return m_channels[channel].m_configuration.m_playspeed;
        }
        
        float EditorMultiMotionComponent::GetDuration(AZ::u8 channel) const
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

        float EditorMultiMotionComponent::GetAssetDuration(const AZ::Data::AssetId& assetId)
        {
            float result = 1.0f;

            // Do a blocking load of the asset.
            AZ::Data::Asset<MotionAsset> motionAsset = AZ::Data::AssetManager::Instance().GetAsset<MotionAsset>(assetId, AZ::Data::AssetLoadBehavior::Default);
            motionAsset.BlockUntilLoadComplete();

            if (motionAsset && motionAsset.Get()->m_emfxMotion)
            {
                result = motionAsset.Get()->m_emfxMotion.get()->GetDuration();
            }

            motionAsset.Release();

            return result;
        }

        void EditorMultiMotionComponent::PlayTime(AZ::u8 channel, float time)
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

        float EditorMultiMotionComponent::GetPlayTime(AZ::u8 channel) const
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

        void EditorMultiMotionComponent::Motion(AZ::u8 channel, AZ::Data::AssetId assetId)
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

                    // Create a new asset
                    if (assetId.IsValid())
                    {
                        m_channels[channel].m_configuration.m_motionAsset = AZ::Data::AssetManager::Instance().GetAsset<MotionAsset>(assetId, m_channels[channel].m_configuration.m_motionAsset.GetAutoLoadBehavior());
                    }
                }

                // Connect the bus if the asset is valid.
                if (m_channels[channel].m_configuration.m_motionAsset.GetId().IsValid())
                {
                    AZ::Data::AssetBus::MultiHandler::BusConnect(m_channels[channel].m_configuration.m_motionAsset.GetId());
                }
            }
        }

        AZ::Data::AssetId EditorMultiMotionComponent::GetMotion(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return {};

            return m_channels[channel].m_configuration.m_motionAsset.GetId();
        }

        void EditorMultiMotionComponent::SetPreviewInEditor(bool enable)
        {
            if (m_previewInEditor != enable)
            {
                m_previewInEditor = enable;
                OnEditorPropertyChanged();
            }
        }

        bool EditorMultiMotionComponent::GetPreviewInEditor() const
        {
            return m_previewInEditor;
        }

        void EditorMultiMotionComponent::BlendInTime(AZ::u8 channel, float time)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_blendInTime = time;
        }

        float EditorMultiMotionComponent::GetBlendInTime(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return 0.f;

            return m_channels[channel].m_configuration.m_blendInTime;
        }

        void EditorMultiMotionComponent::BlendOutTime(AZ::u8 channel, float time)
        {
            if (m_channels.size() <= channel)
                return;

            m_channels[channel].m_configuration.m_blendOutTime = time;
        }

        float EditorMultiMotionComponent::GetBlendOutTime(AZ::u8 channel) const
        {
            if (m_channels.size() <= channel)
                return 0.f;

            return m_channels[channel].m_configuration.m_blendOutTime;
        }

        AZ::Crc32 EditorMultiMotionComponent::OnEditorPropertyChanged()
        {
            AZ::u8 index = 0;
            for (auto& channel : m_channels)
            {
                channel.m_channelIndex = index++;
                RemoveMotionInstanceFromActor(channel.m_lastMotionInstance);
                channel.m_lastMotionInstance = nullptr;
                RemoveMotionInstanceFromActor(channel.m_motionInstance);
                channel.m_motionInstance = nullptr;
                channel.m_configuration.m_motionAsset.Release();
                VerifyMotionAssetState(channel.m_channelIndex);
            }
            return AZ::Edit::PropertyRefreshLevels::None;
        }
    }
}
// @CYA END
