#include <atomic>
#include "Mod/CppUserModBase.hpp"
#include "UE4SSProgram.hpp"
#include "Generator/JsonSchemaGenerator.h"
#include "Loader/DragonWildsMainLoader.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "SDK/DragonWildsSignatures.h"
#include "SDK/UnrealOffsets.h"

using namespace RC;
using namespace RC::Unreal;

class RuneSchema : public RC::CppUserModBase
{
public:
    RuneSchema() : CppUserModBase()
    {
        ModName = STR("RuneSchema");
        ModVersion = STR("0.6.0");
        ModDescription = STR("Allows modifying of DragonWilds's assets dynamically.");
        ModAuthors = STR("Okaetsu");

        auto config = PS::PSConfig::Get();
        config->Load();

        PS::Log<LogLevel::Verbose>(STR("Initializing SignatureManager...\n"));
        DragonWilds::SignatureManager::Initialize();

        PS::Log<LogLevel::Verbose>(STR("Initializing UnrealOffsets...\n"));
        DragonWilds::UnrealOffsets::Initialize();

        PS::Log<LogLevel::Verbose>(STR("Preparing to pre-initialize RuneSchema...\n"));
        MainLoader.PreInitialize();

        PS::Log<RC::LogLevel::Normal>(STR("{} v{} by {} loaded.\n"), ModName, ModVersion, ModAuthors);
    }

    ~RuneSchema() override
    {
    }

    auto on_ui_init() -> void override
    {
        if (!UE4SSProgram::settings_manager.Debug.DebugConsoleEnabled)
        {
            return;
        }

        PS::Log<LogLevel::Verbose>(STR("GUI Console is enabled, enabling ImGui for RuneSchema...\n"));

        UE4SS_ENABLE_IMGUI()

        PS::Log<LogLevel::Verbose>(STR("Registering RuneSchema tab in GUI Console...\n"));
        register_tab(STR("RuneSchema"), [](CppUserModBase* instance) {
            auto mod = dynamic_cast<RuneSchema*>(instance);
            if (!mod)
            {
                return;
            }

            ImGui::SeparatorText("Generators");
            mod->render_schema_generator();
        });

        PS::Log<LogLevel::Verbose>(STR("Finished registering RuneSchema tab for GUI Console.\n"));
    }

    auto render_schema_generator() -> void
    {
        if (ImGui::Button("Generate JSON Schema Files"))
        {
            bool expected = false;
            m_generateSchemas.compare_exchange_strong(expected, true);
        }

        if (m_generateSchemas)
        {
            ImGui::ProgressBar(-0.5f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "Generating...");
        }

        ImGui::TextWrapped("Writes editor autocomplete schemas for mod JSON files to Mods/RuneSchema/schemas. "
                           "Data tables load with the world, so run this after entering one for full coverage.");
    }

    auto on_update() -> void override
    {
        if (m_generateSchemas)
        {
            PS::JsonSchemaGenerator::GenerateSchemaFiles();
            m_generateSchemas = false;
        }
    }

    auto on_program_start() -> void override
    {
    }

    auto on_unreal_init() -> void override
    {
        MainLoader.Initialize();
    }

private:
    DragonWilds::DragonWildsMainLoader MainLoader;
    std::atomic<bool> m_generateSchemas = false;
};

#define RuneSchema_API __declspec(dllexport)
extern "C"
{
    RuneSchema_API RC::CppUserModBase* start_mod()
    {
        return new RuneSchema();
    }

    RuneSchema_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
