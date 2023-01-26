// @CYA EDIT: add MultiMotionComponent
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Asset/AssetCommon.h>

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <Integration/Components/MultiMotionComponent.h>
#include <Integration/Components/ActorComponent.h>
#include <Integration/SimpleMotionComponentBus.h>
#include <Integration/MultiMotionComponentBus.h>
#include <Integration/EditorSimpleMotionComponentBus.h>
#include <Integration/EditorMultiMotionComponentBus.h>

namespace EMotionFX
{
    namespace Integration
    {
        class EditorMultiMotionComponent
            : public AzToolsFramework::Components::EditorComponentBase
            , private AZ::Data::AssetBus::MultiHandler
            , private ActorComponentNotificationBus::Handler
            , private SimpleMotionComponentRequestBus::Handler
            , private MultiMotionComponentRequestBus::Handler
            , private EditorSimpleMotionComponentRequestBus::Handler
            , private EditorMultiMotionComponentRequestBus::Handler
        {
        public:

            AZ_EDITOR_COMPONENT(EditorMultiMotionComponent, "{28889058-7ED2-4A62-A83C-65E07B38F998}");

            EditorMultiMotionComponent();
            ~EditorMultiMotionComponent() override;

            // AZ::Component interface implementation
            void Activate() override;
            void Deactivate() override;

            // ActorComponentNotificationBus::Handler
            void OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance) override;
            void OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance) override;

            // AZ::Data::AssetBus::Handler
            void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
            void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

            static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
            {
                MultiMotionComponent::GetProvidedServices(provided);
            }

            static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
            {
                MultiMotionComponent::GetDependentServices(dependent);
            }

            static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
            {
                MultiMotionComponent::GetRequiredServices(required);
            }

            static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
            {
                MultiMotionComponent::GetIncompatibleServices(incompatible);
            }

            static void Reflect(AZ::ReflectContext* context);

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

            // EditorSimpleMotionComponentRequestBus::Handler
            void SetPreviewInEditor(bool enable) override;
            bool GetPreviewInEditor() const override;
            float GetAssetDuration(const AZ::Data::AssetId& assetId) override;

            AZ::u8 GetLastActiveChannel() const { return m_lastActiveChannel; }

        private:
            EditorMultiMotionComponent(const EditorMultiMotionComponent&) = delete;

            void RemoveMotionInstanceFromActor(EMotionFX::MotionInstance* motionInstance);
            void BuildGameEntity(AZ::Entity* gameEntity) override;
            void VerifyMotionAssetState(AZ::u8 channel);
            AZ::Crc32 OnEditorPropertyChanged();

            bool m_previewInEditor; ///< Plays motion in Editor.
            AZ::u8 m_lastActiveChannel;
            AZStd::vector<MultiMotionComponent::Channel> m_channels;
            EMotionFX::ActorInstance* m_actorInstance; ///< Associated actor instance (retrieved from Actor Component).
        };
    }
}
// @CYA END
