/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/std/string/string.h>
#include <SceneAPI/SDKWrapper/NodeWrapper.h>

struct aiScene;

namespace AZ
{
// @CYA EDIT: Allow to preserve FBX pivots (pass manifest to LoadSceneFromFile)
    namespace SceneAPI::Containers
    {
        class SceneManifest;
    }
// @CYA END

    namespace SDKScene
    {
        class SceneWrapperBase
        {
        public:
            AZ_RTTI(SceneWrapperBase, "{703CD344-2C75-4F30-8CE2-6BDEF2511AFD}");
            virtual ~SceneWrapperBase() = default;

// @CYA EDIT: Allow to preserve FBX pivots (pass manifest to LoadSceneFromFile)
            virtual bool LoadSceneFromFile(const char* fileName, const SceneAPI::Containers::SceneManifest& manifest);
            virtual bool LoadSceneFromFile(const AZStd::string& fileName, const SceneAPI::Containers::SceneManifest& manifest);
// @CYA END

            virtual const std::shared_ptr<SDKNode::NodeWrapper> GetRootNode() const;
            virtual std::shared_ptr<SDKNode::NodeWrapper> GetRootNode();

            virtual void Clear();

            static const char* s_defaultSceneName;
        };

    } //namespace Scene
} //namespace AZ
