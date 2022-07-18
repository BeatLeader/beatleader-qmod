#include "Assets/BundleLoader.hpp"
#include "include/Utils/StringUtils.hpp"

#include <utility>

DEFINE_TYPE(BeatLeader, Bundle);

AssetBundle* BundleLoader::assetBundle;
BeatLeader::Bundle* BundleLoader::bundle;

custom_types::Helpers::Coroutine BundleLoader::LoadBundle(UnityEngine::GameObject* container)
{
    if (assetBundle == NULL) {
        Array<uint8_t>* bytes = Array<uint8_t>::NewLength(bl_bundle::getLength());
        
        std::copy(bl_bundle::getData(), bl_bundle::getData() + bl_bundle::getLength(), bytes->values);
        using AssetBundle_LoadFromMemoryAsync = function_ptr_t<UnityEngine::AssetBundleCreateRequest*, Array<uint8_t>*, int>;
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
    return assetBundle->LoadAsset<Material*>(newcsstr2(name));
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