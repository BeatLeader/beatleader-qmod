#include "Assets/BundleLoader.hpp"

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
    
    co_return;
}

Material* getMaterial(std::string name, AssetBundle* assetBundle) {
    return assetBundle->LoadAsset<Material*>(std::move(name));
}

void BeatLeader::Bundle::Init(AssetBundle* assetBundle) {
    logoMaterial = getMaterial("LogoMaterial", assetBundle);
    defaultAvatarMaterial = getMaterial("DefaultAvatar", assetBundle);
    UIAdditiveGlowMaterial = getMaterial("UIAdditiveGlow", assetBundle);
    scoreBackgroundMaterial = getMaterial("ScoreBackgroundMaterial", assetBundle);
    scoreUnderlineMaterial = getMaterial("ScoreUnderlineMaterial", assetBundle);
    VotingButtonMaterial = getMaterial("VotingButtonMaterial", assetBundle);
    handAccIndicatorMaterial = getMaterial("HandAccIndicatorMaterial", assetBundle);
    accGridBackgroundMaterial = getMaterial("AccGridBackgroundMaterial", assetBundle);
    accuracyGraphMaterial = getMaterial("AccuracyGraphBackground", assetBundle);
    accuracyGraphLine = getMaterial("AccuracyGraphLine", assetBundle);
    accDetailsRowMaterial = getMaterial("AccDetailsRowMaterial", assetBundle);
    miniProfileBackgroundMaterial = getMaterial("UIMiniProfileBackgroundMaterial", assetBundle);
    skillTriangleMaterial = getMaterial("UISkillTriangleMaterial", assetBundle);

    locationIcon = assetBundle->LoadAsset<Sprite*>("LocationIcon");
    rowSeparatorIcon = assetBundle->LoadAsset<Sprite*>("RowSeparatorIcon");
    beatLeaderLogoGradient = assetBundle->LoadAsset<Sprite*>("BeatLeaderLogoGradient");
    transparentPixel = assetBundle->LoadAsset<Sprite*>("TransparentPixel");
    fileError = assetBundle->LoadAsset<Sprite*>("FileError");
    modifiersIcon = assetBundle->LoadAsset<Sprite*>("ModifiersIcon");
    settingsIcon = assetBundle->LoadAsset<Sprite*>("BL_SettingsIcon");

    overview1Icon = assetBundle->LoadAsset<Sprite*>("BL_Overview1Icon");
    overview2Icon = assetBundle->LoadAsset<Sprite*>("BL_Overview2Icon");
    detailsIcon = assetBundle->LoadAsset<Sprite*>("BL_DetailsIcon");
    gridIcon = assetBundle->LoadAsset<Sprite*>("BL_GridIcon");
    graphIcon = assetBundle->LoadAsset<Sprite*>("BL_GraphIcon");
    websiteLinkIcon = assetBundle->LoadAsset<Sprite*>("BL_Website");
    discordLinkIcon = assetBundle->LoadAsset<Sprite*>("BL_Discord");
    patreonLinkIcon = assetBundle->LoadAsset<Sprite*>("BL_Patreon");
    replayIcon = assetBundle->LoadAsset<Sprite*>("BL_QuestReplayIcon");

    twitterIcon = assetBundle->LoadAsset<Sprite*>("BL_TwitterIcon");
    twitchIcon = assetBundle->LoadAsset<Sprite*>("BL_TwitchIcon");
    youtubeIcon = assetBundle->LoadAsset<Sprite*>("BL_YoutubeIcon");
    profileIcon = assetBundle->LoadAsset<Sprite*>("BL_ProfileIcon");
    friendsIcon = assetBundle->LoadAsset<Sprite*>("BL_FriendsIcon");
    incognitoIcon = assetBundle->LoadAsset<Sprite*>("BL_IncognitoIcon");
    defaultAvatar = assetBundle->LoadAsset<Sprite*>("BL_DefaultAvatar");

    friendsSelectorIcon = assetBundle->LoadAsset<Sprite*>("BL_FriendsSelectorIcon");
    globeIcon = assetBundle->LoadAsset<Sprite*>("BL_GlobeIcon");

    generalContextIcon = assetBundle->LoadAsset<Sprite*>("BL_ContextGeneral");
    noModifiersIcon = assetBundle->LoadAsset<Sprite*>("BL_ContextNoModifiers");
    noPauseIcon = assetBundle->LoadAsset<Sprite*>("BL_ContextNoPause");
    golfIcon = assetBundle->LoadAsset<Sprite*>("BL_ContextGolf");
    TMP_SpriteCurved = assetBundle->LoadAsset<Shader*>("TMP_SpriteCurved");
}

Material* BeatLeader::Bundle::GetAvatarMaterial(StringW effectName) {
    Material* result;
    if (((string)effectName).find("_") != string::npos) {
        result = getMaterial(effectName, BundleLoader::assetBundle);
    }
    return result != NULL ? result : defaultAvatarMaterial;
}

Sprite* BeatLeader::Bundle::GetCountryIcon(StringW country) {
    return BundleLoader::assetBundle->LoadAsset<Sprite*>(country);
}