// TMPro::TMP_SpriteAsset* result = TMPro::TMP_SpriteAsset::New_ctor();

//             TMPro::TMP_SpriteGlyph* sprite = TMPro::TMP_SpriteGlyph::New_ctor();
//             sprite->set_index(0);
//             sprite->set_metrics(UnityEngine::TextCore::GlyphMetrics(100, 100, 25, 75, 100));
//             sprite->set_glyphRect(UnityEngine::TextCore::GlyphRect(0, 0, 100, 100));
//             sprite->sprite = Sprites::get_StarIcon();
//             result->spriteSheet = (UnityEngine::Texture *)sprite->sprite->get_texture();
            
//             sprite->set_scale(1);
//             sprite->set_atlasIndex(0);

//             if (result->m_SpriteGlyphTable == NULL) {
//                 result->m_SpriteGlyphTable = System::Collections::Generic::List_1<::TMPro::TMP_SpriteGlyph*>::New_ctor();
//             }
//             result->m_SpriteGlyphTable->Add(sprite);

//             TMPro::TMP_SpriteCharacter* character = TMPro::TMP_SpriteCharacter::New_ctor(128060, sprite);
//             character->set_name("Hui");
//             character->set_scale(1);

//             if (result->m_SpriteCharacterTable == NULL) {
//                 result->m_SpriteCharacterTable = System::Collections::Generic::List_1<::TMPro::TMP_SpriteCharacter*>::New_ctor();
//             }
//             result->m_SpriteCharacterTable->Add(character);

//             result->SortGlyphTable();
//             result->UpdateLookupTables();
            
//             UnityEngine::Shader* shader = UnityEngine::Shader::Find("TextMeshPro/Sprite");
//             UnityEngine::Material* curved = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Material*>().FirstOrDefault([](UnityEngine::Material* x) { return x->get_name() == "Teko-Medium SDF Curved Softer"; });
//             UnityEngine::Material* material = UnityEngine::Material::New_ctor(curved->get_shader());

//             UnityEngine::Shader* shader2 = curved->get_shader();
//             for (int i = 0; i < shader2->GetPropertyCount(); i++)
//             {
//                 string propertyName = shader2->GetPropertyName(i);
//                 auto propertyType = shader2->GetPropertyType(i);

//                 getLogger().info("%s", (propertyName).c_str());

//                 if (propertyType == UnityEngine::Rendering::ShaderPropertyType::Texture)
//                 {
//                     // UnityEngine::Texture* texture = material->GetTexture(propertyName);
                    
//                 }
//             }
            
//             // material->set_color(UnityEngine::Color(0.0, 0.0, 0.0, 0.0));
//             material->SetTexture("_FaceTex", (UnityEngine::Texture*)Sprites::get_StarIcon()->get_texture());
//             material->SetTexture("_MainTex", (UnityEngine::Texture*)Sprites::get_StarIcon()->get_texture());

//             result->material = material;
            
//             // material->set_hideFlags(UnityEngine::HideFlags::HideInHierarchy);
//             // UnityEngine::U2D::AssetDatabase::AddObjectToAsset(material, result);

//             playerInfo->set_spriteAsset(result);

// MAKE_HOOK_MATCH(GetCharacterFromFontAsset, &TMPro::TMP_FontAssetUtilities::GetCharacterFromFontAsset, TMPro::TMP_Character*, uint unicode, ::TMPro::TMP_FontAsset* sourceFontAsset, bool includeFallbacks, ::TMPro::FontStyles fontStyle, ::TMPro::FontWeight fontWeight, ByRef<bool> isAlternativeTypeface, ByRef<::TMPro::TMP_FontAsset*> fontAsset) {
//         TMPro::TMP_Character* result = GetCharacterFromFontAsset(unicode, sourceFontAsset, false, fontStyle, fontWeight, isAlternativeTypeface, fontAsset);
//         // getLogger().info("GetCharacterFromFontAsset");
//         return result;
//     }

//     MAKE_HOOK_MATCH(SearchForSpriteByUnicodeInternal, &TMPro::TMP_SpriteAsset::SearchForSpriteByUnicode, TMPro::TMP_SpriteAsset*, TMPro::TMP_SpriteAsset* spriteAsset, uint unicode, bool includeFallbacks, ByRef<int> spriteIndex) {
//         TMPro::TMP_SpriteAsset* result = SearchForSpriteByUnicodeInternal(spriteAsset, unicode, includeFallbacks, spriteIndex);
        
//         // if (result == NULL) {
//         //     result = spriteAsset;
            
            

//         //     *spriteIndex = 0;
//         // }
        
//         getLogger().info("SearchForSpriteByUnicodeInternal %i %ui", *spriteIndex, unicode);
//         return result;
//     }

//     MAKE_HOOK_MATCH(GetDefaultSpriteMaterial, &TMPro::TMP_SpriteAsset::GetDefaultSpriteMaterial, UnityEngine::Material*, TMPro::TMP_SpriteAsset* self) {
//         UnityEngine::Material* result = GetDefaultSpriteMaterial(self);
//         getLogger().info("GetDefaultSpriteMaterial");
//         return result;
//     }

//     MAKE_HOOK_MATCH(GetSpriteIndexFromUnicode, &TMPro::TMP_SpriteAsset::GetSpriteIndexFromUnicode, int, TMPro::TMP_SpriteAsset* self, uint unicode) {
//         int result = GetSpriteIndexFromUnicode(self, unicode);
//         getLogger().info("GetSpriteIndexFromUnicode");
//         return result;
//     }

// INSTALL_HOOK(logger, SearchForSpriteByUnicodeInternal);
//         INSTALL_HOOK(logger, GetDefaultSpriteMaterial);
//         INSTALL_HOOK(logger, GetSpriteIndexFromUnicode);
//         INSTALL_HOOK(logger, GetCharacterFromFontAsset);
