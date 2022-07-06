#include "Assets/BundleLoader.hpp"
#include "include/Utils/StringUtils.hpp"


AssetBundle* BundleLoader::bundle;
Material* BundleLoader::logoMaterial;
Material* BundleLoader::playerAvatarMaterial;
Material* BundleLoader::UIAdditiveGlowMaterial;
Material* BundleLoader::scoreBackgroundMaterial;
Material* BundleLoader::scoreUnderlineMaterial;
Material* BundleLoader::VotingButtonMaterial;
Material* BundleLoader::handAccIndicatorMaterial;
Material* BundleLoader::accGridBackgroundMaterial;
Material* BundleLoader::accuracyGraphMaterial;
Material* BundleLoader::accuracyGraphLine;
Material* BundleLoader::accDetailsRowMaterial;

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
    logoMaterial = assetBundle->LoadAsset<Material*>(newcsstr2("LogoMaterial"));
    playerAvatarMaterial = assetBundle->LoadAsset<Material*>(newcsstr2("PlayerAvatarMaterial"));
    UIAdditiveGlowMaterial = assetBundle->LoadAsset<Material*>(newcsstr2("UIAdditiveGlow"));
    scoreBackgroundMaterial = assetBundle->LoadAsset<Material*>(newcsstr2("ScoreBackgroundMaterial"));
    scoreUnderlineMaterial = assetBundle->LoadAsset<Material*>(newcsstr2("ScoreUnderlineMaterial"));
    VotingButtonMaterial = assetBundle->LoadAsset<Material*>(newcsstr2("VotingButtonMaterial"));
    handAccIndicatorMaterial = assetBundle->LoadAsset<Material*>(newcsstr2("HandAccIndicatorMaterial"));
    accGridBackgroundMaterial = assetBundle->LoadAsset<Material*>(newcsstr2("AccGridBackgroundMaterial"));
    accuracyGraphMaterial = assetBundle->LoadAsset<Material*>(newcsstr2("AccuracyGraphBackground"));
    accuracyGraphLine = assetBundle->LoadAsset<Material*>(newcsstr2("AccuracyGraphLine"));
    accDetailsRowMaterial = assetBundle->LoadAsset<Material*>(newcsstr2("AccDetailsRowMaterial"));

    locationIcon = assetBundle->LoadAsset<Sprite*>(newcsstr2("LocationIcon"));
    rowSeparatorIcon = assetBundle->LoadAsset<Sprite*>(newcsstr2("RowSeparatorIcon"));
    beatLeaderLogoGradient = assetBundle->LoadAsset<Sprite*>(newcsstr2("BeatLeaderLogoGradient"));
    transparentPixel = assetBundle->LoadAsset<Sprite*>(newcsstr2("TransparentPixel"));
    fileError = assetBundle->LoadAsset<Sprite*>(newcsstr2("FileError"));
    modifiersIcon = assetBundle->LoadAsset<Sprite*>(newcsstr2("ModifiersIcon"));

    TMP_SpriteCurved = assetBundle->LoadAsset<Shader*>(newcsstr2("TMP_SpriteCurved"));

    overviewIcon = assetBundle->LoadAsset<Sprite*>(newcsstr2("BL_OverviewIcon"));
    detailsIcon = assetBundle->LoadAsset<Sprite*>(newcsstr2("BL_DetailsIcon"));
    gridIcon = assetBundle->LoadAsset<Sprite*>(newcsstr2("BL_GridIcon"));
    graphIcon = assetBundle->LoadAsset<Sprite*>(newcsstr2("BL_GraphIcon"));
    websiteLinkIcon = assetBundle->LoadAsset<Sprite*>(newcsstr2("BL_Website"));
    discordLinkIcon = assetBundle->LoadAsset<Sprite*>(newcsstr2("BL_Discord"));
    patreonLinkIcon = assetBundle->LoadAsset<Sprite*>(newcsstr2("BL_Patreon"));
}