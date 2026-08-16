#pragma once

#include <mutex>
#include <vector>
#include "Loader/DragonWildsModLoaderBase.h"
#include "nlohmann/json.hpp"

namespace DragonWilds {
    class DragonWildsAssetModLoader : public DragonWildsModLoaderBase {
        struct PendingAsset {
            RC::StringType Target;
            RC::StringType ObjectPath;
            RC::StringType ModName;
            nlohmann::json Properties;
        };

        struct LoadResult {
            int PropertiesWritten = 0;
            int ErrorCount = 0;
        };
    public:
        DragonWildsAssetModLoader();

        ~DragonWildsAssetModLoader();
    protected:
        virtual void OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        virtual void OnAutoReload(const std::filesystem::path::string_type& modName, const std::filesystem::path& modFilePath) override final;

        virtual bool CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        virtual bool OnInitialize() override final;
    private:
        std::mutex m_mutex;
        std::vector<PendingAsset> m_pendingAssets;
        RC::Unreal::UClass* m_dataAssetClass = nullptr;
        RC::Unreal::UClass* m_curveBaseClass = nullptr;

        void QueueData(const nlohmann::json& data, const RC::StringType& modName);
        void Apply(RC::Unreal::UObject* object, const PendingAsset& pendingAsset, LoadResult& outResult);
        void AppendProperties(RC::Unreal::UObject* object, RC::Unreal::UClass* objectClass, const nlohmann::json& appendData, LoadResult& outResult);
        void TryApplyPending();
        void ReportUnresolvedAssets();

        RC::Unreal::UObject* Resolve(const PendingAsset& pendingAsset);
        RC::StringType NormalizeObjectPath(const RC::StringType& target) const;

        bool IsSupportedTarget(RC::Unreal::UObject* object) const;
        bool IsReadyForPatch(RC::Unreal::UObject* object) const;
    };
}

