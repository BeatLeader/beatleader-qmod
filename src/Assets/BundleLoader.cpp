#include "Assets/BundleLoader.hpp"


AssetBundle* BundleLoader::bundle;
Material* BundleLoader::logoMaterial;
Material* BundleLoader::playerAvatarMaterial;
Material* BundleLoader::UIAdditiveGlowMaterial;
Material* BundleLoader::scoreBackgroundMaterial;
Material* BundleLoader::scoreUnderlineMaterial;
Material* BundleLoader::VotingButtonMaterial;
Material* BundleLoader::handAccIndicatorMaterial;
Material* BundleLoader::accGridBackgroundMaterial;
Sprite* BundleLoader::locationIcon;
Sprite* BundleLoader::rowSeparatorIcon;
Sprite* BundleLoader::beatLeaderLogoGradient;
Sprite* BundleLoader::transparentPixel;
Sprite* BundleLoader::fileError;
Sprite* BundleLoader::modifiersIcon;

Sprite* BundleLoader::overviewIcon;
Sprite* BundleLoader::detailsIcon;
Sprite* BundleLoader::gridIcon;
Sprite* BundleLoader::graphIcon;
Sprite* BundleLoader::websiteLinkIcon;
Sprite* BundleLoader::discordLinkIcon;
Sprite* BundleLoader::patreonLinkIcon;

Shader* BundleLoader::TMP_SpriteCurved;

custom_types::Helpers::Coroutine BundleLoader::LoadBundle()
{
    if (bundle == NULL) {
        Array<uint8_t>* bytes = Array<uint8_t>::NewLength(bl_bundle::getLength());
        
        std::copy(bl_bundle::getData(), bl_bundle::getData() + bl_bundle::getLength(), bytes->values);
        using AssetBundle_LoadFromMemoryAsync = function_ptr_t<UnityEngine::AssetBundleCreateRequest*, Array<uint8_t>*, int>;
        static auto assetBundle_LoadFromMemoryAsync = reinterpret_cast<AssetBundle_LoadFromMemoryAsync>(il2cpp_functions::resolve_icall("UnityEngine.AssetBundle::LoadFromMemoryAsync_Internal"));

        auto req = assetBundle_LoadFromMemoryAsync(bytes, 0);
        req->set_allowSceneActivation(true);
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(req);

        bundle = req->get_assetBundle();
    }
    LoadAssets(bundle);
}

void BundleLoader::LoadAssets(AssetBundle* assetBundle) {
    logoMaterial = assetBundle->LoadAsset<Material*>(il2cpp_utils::createcsstr("LogoMaterial"));
    playerAvatarMaterial = assetBundle->LoadAsset<Material*>(il2cpp_utils::createcsstr("PlayerAvatarMaterial"));
    UIAdditiveGlowMaterial = assetBundle->LoadAsset<Material*>(il2cpp_utils::createcsstr("UIAdditiveGlow"));
    scoreBackgroundMaterial = assetBundle->LoadAsset<Material*>(il2cpp_utils::createcsstr("ScoreBackgroundMaterial"));
    scoreUnderlineMaterial = assetBundle->LoadAsset<Material*>(il2cpp_utils::createcsstr("ScoreUnderlineMaterial"));
    VotingButtonMaterial = assetBundle->LoadAsset<Material*>(il2cpp_utils::createcsstr("VotingButtonMaterial"));
    handAccIndicatorMaterial = assetBundle->LoadAsset<Material*>(il2cpp_utils::createcsstr("HandAccIndicatorMaterial"));
    accGridBackgroundMaterial = assetBundle->LoadAsset<Material*>(il2cpp_utils::createcsstr("AccGridBackgroundMaterial"));

    locationIcon = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("LocationIcon"));
    rowSeparatorIcon = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("RowSeparatorIcon"));
    beatLeaderLogoGradient = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("BeatLeaderLogoGradient"));
    transparentPixel = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("TransparentPixel"));
    fileError = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("FileError"));
    modifiersIcon = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("ModifiersIcon"));

    TMP_SpriteCurved = assetBundle->LoadAsset<Shader*>(il2cpp_utils::createcsstr("TMP_SpriteCurved"));

    overviewIcon = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("BL_OverviewIcon"));
    detailsIcon = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("BL_DetailsIcon"));
    gridIcon = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("BL_GridIcon"));
    graphIcon = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("BL_GraphIcon"));
    websiteLinkIcon = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("BL_Website"));
    discordLinkIcon = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("BL_Discord"));
    patreonLinkIcon = assetBundle->LoadAsset<Sprite*>(il2cpp_utils::createcsstr("BL_Patreon"));
}