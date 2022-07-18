#include "Assets/BundleLoader.hpp"

#include <utility>

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
}

Material* getMaterial(std::string name, AssetBundle* assetBundle) {
    return assetBundle->LoadAsset<Material*>(std::move(name));
}

void BeatLeader::Bundle::Init(AssetBundle* assetBundle) {
    logoMaterial = getMaterial("LogoMaterial", assetBundle);
    playerAvatarMaterial = getMaterial("PlayerAvatarMaterial", assetBundle);
    UIAdditiveGlowMaterial = getMaterial("UIAdditiveGlow", assetBundle);
    scoreBackgroundMaterial = getMaterial("ScoreBackgroundMaterial", assetBundle);
    scoreUnderlineMaterial = getMaterial("ScoreUnderlineMaterial", assetBundle);
    VotingButtonMaterial = getMaterial("VotingButtonMaterial", assetBundle);
    handAccIndicatorMaterial = getMaterial("HandAccIndicatorMaterial", assetBundle);
    accGridBackgroundMaterial = getMaterial("AccGridBackgroundMaterial", assetBundle);
    accuracyGraphMaterial = getMaterial("AccuracyGraphBackground", assetBundle);
    accuracyGraphLine = getMaterial("AccuracyGraphLine", assetBundle);
    accDetailsRowMaterial = getMaterial("AccDetailsRowMaterial", assetBundle);

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