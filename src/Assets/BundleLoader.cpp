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
        ArrayW<uint8_t> bytes(bl_bundle::getLength());
        std::copy(bl_bundle::getData(), bl_bundle::getData() + bl_bundle::getLength(), bytes.begin());

        using AssetBundle_LoadFromMemoryAsync = function_ptr_t<UnityEngine::AssetBundleCreateRequest*, ArrayW<uint8_t>, int>;
        static auto assetBundle_LoadFromMemoryAsync = reinterpret_cast<AssetBundle_LoadFromMemoryAsync>(il2cpp_functions::resolve_icall("UnityEngine.AssetBundle::LoadFromMemoryAsync_Internal"));

        auto req = assetBundle_LoadFromMemoryAsync(bytes, 0);
        req->set_allowSceneActivation(true);
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(req);

        bundle = req->get_assetBundle();
    }
    LoadAssets(bundle);
}

void BundleLoader::LoadAssets(AssetBundle* assetBundle) {
    logoMaterial = assetBundle->LoadAsset<Material*>("LogoMaterial");
    playerAvatarMaterial = assetBundle->LoadAsset<Material*>("PlayerAvatarMaterial");
    UIAdditiveGlowMaterial = assetBundle->LoadAsset<Material*>("UIAdditiveGlow");
    scoreBackgroundMaterial = assetBundle->LoadAsset<Material*>("ScoreBackgroundMaterial");
    scoreUnderlineMaterial = assetBundle->LoadAsset<Material*>("ScoreUnderlineMaterial");
    VotingButtonMaterial = assetBundle->LoadAsset<Material*>("VotingButtonMaterial");
    handAccIndicatorMaterial = assetBundle->LoadAsset<Material*>("HandAccIndicatorMaterial");
    accGridBackgroundMaterial = assetBundle->LoadAsset<Material*>("AccGridBackgroundMaterial");

    locationIcon = assetBundle->LoadAsset<Sprite*>("LocationIcon");
    rowSeparatorIcon = assetBundle->LoadAsset<Sprite*>("RowSeparatorIcon");
    beatLeaderLogoGradient = assetBundle->LoadAsset<Sprite*>("BeatLeaderLogoGradient");
    transparentPixel = assetBundle->LoadAsset<Sprite*>("TransparentPixel");
    fileError = assetBundle->LoadAsset<Sprite*>("FileError");
    modifiersIcon = assetBundle->LoadAsset<Sprite*>("ModifiersIcon");

    overviewIcon = assetBundle->LoadAsset<Sprite*>("BL_OverviewIcon");
    detailsIcon = assetBundle->LoadAsset<Sprite*>("BL_DetailsIcon");
    gridIcon = assetBundle->LoadAsset<Sprite*>("BL_GridIcon");
    graphIcon = assetBundle->LoadAsset<Sprite*>("BL_GraphIcon");
    websiteLinkIcon = assetBundle->LoadAsset<Sprite*>("BL_Website");
    discordLinkIcon = assetBundle->LoadAsset<Sprite*>("BL_Discord");
    patreonLinkIcon = assetBundle->LoadAsset<Sprite*>("BL_Patreon");

    TMP_SpriteCurved = assetBundle->LoadAsset<Shader*>("TMP_SpriteCurved");
}