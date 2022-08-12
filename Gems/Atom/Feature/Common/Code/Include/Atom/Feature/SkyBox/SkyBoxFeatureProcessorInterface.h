/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Atom/RPI.Public/FeatureProcessor.h>
#include <Atom/RPI.Reflect/Image/Image.h>
#include <Atom/Feature/CoreLights/PhotometricValue.h>
#include <AzCore/Math/Matrix4x4.h>
#include <SkyBox/SkyBoxFogSettings.h>

namespace AZ
{

// @CYA EDIT: Add SkyBoxFeatureProcessorNotificationBus to override OnSimulate
    namespace RPI
    {
        class ShaderResourceGroup;
    }
// @CYA END

    namespace Render
    {
// @CYA EDIT: Add SkyBoxFeatureProcessorNotificationBus to override OnSimulate
        class SkyBoxFeatureProcessorNotifications : public AZ::EBusTraits
        {
        public:
            //////////////////////////////////////////////////////////////////////////
            // EBusTraits overrides
            using BusIdType = AZ::EntityId;
            using MutexType = AZStd::recursive_mutex;
            static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
            static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
            //////////////////////////////////////////////////////////////////////////

            virtual ~SkyBoxFeatureProcessorNotifications() = default;

            //////////////////////////////////////////////////////////////////////////
            // Notifications
            virtual void OnSimulateSkyBox([[maybe_unused]] AZ::RPI::ShaderResourceGroup* sceneSrg){};
            virtual void OnComputeSunRGB([[maybe_unused]] AZ::Color& sunColor){};
            //////////////////////////////////////////////////////////////////////////
        };

        using SkyBoxFeatureProcessorNotificationBus = AZ::EBus<SkyBoxFeatureProcessorNotifications>;
// @CYA END

        enum class SkyBoxMode
        {
            None = 0,
            Cubemap,
            PhysicalSky
        };

        struct SunPosition
        {
            SunPosition() = default;
            SunPosition(float azimuth, float altitude) :m_azimuth(azimuth), m_altitude(altitude) {};
            SunPosition& operator=(const SunPosition& pos)
            {
                m_azimuth = pos.m_azimuth;
                m_altitude = pos.m_altitude;
                return *this;
            }
            float m_azimuth = 0.0f;
            float m_altitude = 0.0f;
        };

        class SkyBoxFeatureProcessorInterface
            : public RPI::FeatureProcessor
        {
            public:
            AZ_RTTI(AZ::Render::SkyBoxFeatureProcessorInterface, "{71061869-1190-4451-A337-E9CFF16441B4}", AZ::RPI::FeatureProcessor); 

            virtual void Enable(bool enable) = 0;
            virtual bool IsEnabled() = 0;
            virtual void SetSkyboxMode(SkyBoxMode mode) = 0;
            virtual void SetFogSettings(const SkyBoxFogSettings& fogSettings) = 0;

            // HDRiSkyBox
            virtual void SetCubemap(Data::Instance<RPI::Image> cubemap) = 0;
            virtual void SetCubemapExposure(float exposure) = 0;
            virtual void SetCubemapRotationMatrix(AZ::Matrix4x4 matrix) = 0;

            // PhysicalSky
            virtual void SetSunPosition(SunPosition sunPosition) = 0;
            virtual void SetSunPosition(float azimuth, float altitude) = 0;
            virtual void SetTurbidity(int turbidity) = 0;
            virtual void SetSkyIntensity(float intensity, PhotometricUnit type) = 0;
            virtual void SetSunIntensity(float intensity, PhotometricUnit type) = 0;
            virtual void SetSunRadiusFactor(float factor) = 0;

            // Fog Settings
            virtual void SetFogEnabled(bool enable) = 0;
            virtual bool IsFogEnabled() = 0;
            virtual void SetFogColor(const AZ::Color &color) = 0;
            virtual void SetFogTopHeight(float topHeight) = 0;
            virtual void SetFogBottomHeight(float bottomHeight) = 0;
        };
    }
}
