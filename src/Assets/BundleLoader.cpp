#include "Assets/BundleLoader.hpp"

#include "include/Utils/StringUtils.hpp"
#include "ModifiersCoreQuest/shared/Core/ModifiersManager.hpp"
#include "/Core/SpeedModifiers.hpp"

#include <utility>
#include "main.hpp"

DEFINE_TYPE(BeatLeader, Bundle);

AssetBundle* BundleLoader::assetBundle;
BeatLeader::Bundle* BundleLoader::bundle;

custom_types::Helpers::Coroutine BundleLoader::LoadBundle(UnityEngine::GameObject* container)
{
    if (assetBundle == NULL) {
        ArrayW<uint8_t> bytes(bl_bundle::getLength());
        std::copy(bl_bundle::getData(), bl_bundle::getData() + bl_bundle::getLength(), bytes.begin());

        using AssetBundle_LoadFromMemoryAsync = function_ptr_t<UnityEngine::AssetBundleCreateRequest*, ArrayW<uint8_t>, int>;
        static auto assetBundle_LoadFromMemoryAsync = reinterpret_cast<AssetBundle_LoadFromMemoryAsync>(il2cpp_functions::resolve_icall("UnityEngine.AssetBundle::LoadFromMemoryAsync_Internal"));

        auto req = assetBundle_LoadFromMemoryAsync(bytes, 0);
        req->set_allowSceneActivation(true);
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(req);

        assetBundle = req->get_assetBundle();
    }

    bundle = container->AddComponent<BeatLeader::Bundle*>();
    bundle->Init(assetBundle);

    // TODO make this a callback
    if (!ModifiersCoreQuest::ModifiersManager::HasModifierWithId(SpeedModifiers::get_BFS().Id)) {
        ModifiersCoreQuest::ModifiersManager::AddModifier(SpeedModifiers::get_BFS());
    }
    if (!ModifiersCoreQuest::ModifiersManager::HasModifierWithId(SpeedModifiers::get_BSF().Id)) {
        ModifiersCoreQuest::ModifiersManager::AddModifier(SpeedModifiers::get_BSF());
    }
    
    co_return;
}

Material* BeatLeader::Bundle::GetMaterial(StringW name) {
    if (materials->ContainsKey(name)) {
        return materials->get_Item(name);
    } else {
        return NULL;
    }
}

Material* BeatLeader::Bundle::GetAvatarMaterial(StringW effectName) {
    Material* result = NULL;
    
    if (((string)effectName).find("_") != string::npos) {
        result = GetMaterial(effectName);
    }
    return result != NULL ? result : defaultAvatarMaterial;
}

Sprite* BeatLeader::Bundle::GetSprite(StringW name) {
    if (sprites->ContainsKey(name)) {
        return sprites->get_Item(name);
    } else {
        return NULL;
    }
}

Sprite* BeatLeader::Bundle::GetCountryIcon(StringW country) {
    return GetSprite(toLower((string)country));
}

void BeatLeader::Bundle::Init(AssetBundle* assetBundle) {
    auto allnames = assetBundle->GetAllAssetNames();
    materials = System::Collections::Generic::Dictionary_2<StringW, Material*>::New_ctor();
    sprites = System::Collections::Generic::Dictionary_2<StringW, Sprite*>::New_ctor();

    for (size_t i = 0; i < allnames.size(); i++)
    {
        StringW name = allnames[i];
        auto material = assetBundle->LoadAsset<Material*>(name);
        if (material != NULL) {
            materials->Add(material->get_name(), material);
        }
        auto sprite = assetBundle->LoadAsset<Sprite*>(name);
        if (sprite != NULL) {
            sprites->Add(sprite->get_name(), sprite);
        }
    }

    logoMaterial = GetMaterial("LogoMaterial");
    defaultAvatarMaterial = GetMaterial("DefaultAvatar");
    UIAdditiveGlowMaterial = GetMaterial("UIAdditiveGlow");
    scoreBackgroundMaterial = GetMaterial("ScoreBackgroundMaterial");
    scoreUnderlineMaterial = GetMaterial("ScoreUnderlineMaterial");
    VotingButtonMaterial = GetMaterial("VotingButtonMaterial");
    handAccIndicatorMaterial = GetMaterial("HandAccIndicatorMaterial");
    accGridBackgroundMaterial = GetMaterial("AccGridBackgroundMaterial");
    accuracyGraphMaterial = GetMaterial("AccuracyGraphBackground");
    accuracyGraphLine = GetMaterial("AccuracyGraphLine");
    accDetailsRowMaterial = GetMaterial("AccDetailsRowMaterial");
    miniProfileBackgroundMaterial = GetMaterial("UIMiniProfileBackgroundMaterial");
    skillTriangleMaterial = GetMaterial("UISkillTriangleMaterial");
    clanTagBackgroundMaterial = GetMaterial("ClanTagBackgroundMaterial");

    locationIcon = GetSprite("LocationIcon");
    rowSeparatorIcon = GetSprite("RowSeparatorIcon");
    beatLeaderLogoGradient = GetSprite("BeatLeaderLogoGradient");
    transparentPixel = GetSprite("TransparentPixel");
    fileError = GetSprite("FileError");
    modifiersIcon = GetSprite("ModifiersIcon");
    settingsIcon = GetSprite("BL_SettingsIcon");

    overview1Icon = GetSprite("BL_Overview1Icon");
    overview2Icon = GetSprite("BL_Overview2Icon");
    detailsIcon = GetSprite("BL_DetailsIcon");
    gridIcon = GetSprite("BL_GridIcon");
    graphIcon = GetSprite("BL_GraphIcon");
    websiteLinkIcon = GetSprite("BL_Website");
    discordLinkIcon = GetSprite("BL_Discord");
    patreonLinkIcon = GetSprite("BL_Patreon");
    replayIcon = GetSprite("BL_QuestReplayIcon");

    twitterIcon = GetSprite("BL_TwitterIcon");
    twitchIcon = GetSprite("BL_TwitchIcon");
    youtubeIcon = GetSprite("BL_YoutubeIcon");
    profileIcon = GetSprite("BL_ProfileIcon");
    friendsIcon = GetSprite("BL_FriendsIcon");
    incognitoIcon = GetSprite("BL_IncognitoIcon");
    defaultAvatar = GetSprite("BL_DefaultAvatar");

    friendsSelectorIcon = GetSprite("BL_FriendsSelectorIcon");
    globeIcon = GetSprite("BL_GlobeIcon");

    generalContextIcon = GetSprite("BL_ContextGeneral");
    noModifiersIcon = GetSprite("BL_ContextNoModifiers");
    noPauseIcon = GetSprite("BL_ContextNoPause");
    golfIcon = GetSprite("BL_ContextGolf");
    scpmIcon = GetSprite("BL_ContextSCPM");

    BFSIcon = GetSprite("BL_BFSIcon");
    BSFIcon = GetSprite("BL_BSFIcon");

    TMP_SpriteCurved = assetBundle->LoadAsset<Shader*>("TMP_SpriteCurved");
}