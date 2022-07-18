#include "Assets/Sprites.hpp"
#include "Assets/Gif.hpp"
#include "Utils/StringUtils.hpp"
#include "Utils/WebUtils.hpp"

#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/SpriteMeshType.hpp"
#include "UnityEngine/ImageConversion.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

using UnityEngine::Sprite;

map<string, std::vector<uint8_t>> Sprites::iconCache;

void Sprites::get_Icon(string url, const std::function<void(UnityEngine::Sprite*)>& completion, bool nullable) {
    if (iconCache.contains(url)) {
        std::vector<uint8_t> bytes = iconCache[url];
        Array<uint8_t>* spriteArray = il2cpp_utils::vectorToArray(bytes);
        UnityEngine::Sprite* sprite;
        Gif gif = Gif(spriteArray);
        if (gif.Parse() == 0 && gif.Slurp() == 1) {
            auto tex = gif.get_frame(0);
            sprite = UnityEngine::Sprite::Create(
                tex, 
                UnityEngine::Rect(0.0, 0.0, gif.get_width(), gif.get_height()), 
                UnityEngine::Vector2(0.5, 0.5), 
                100.0, 
                0, 
                UnityEngine::SpriteMeshType::FullRect, 
                UnityEngine::Vector4(0.0, 0.0, 0.0, 0.0), 
                false);
        } else {
            sprite = QuestUI::BeatSaberUI::ArrayToSprite(spriteArray);
        }

        if (sprite != NULL || nullable) {
            completion(sprite);
        }
    } else {
        WebUtils::GetAsync(url, [completion, url, nullable](long code, string data) {
            if (code == 200) {
                iconCache[url] = {data.begin(), data.end()};
                QuestUI::MainThreadScheduler::Schedule([completion, url, nullable] {
                    get_Icon(url, completion, nullable);
                });
            } else if (nullable) {
                completion(NULL);
            }
        });
    }
}

void Sprites::get_AnimatedIcon(string url, const std::function<void(AllFramesResult)>& completion) {
    if (iconCache.contains(url)) {
        std::vector<uint8_t> bytes = iconCache[url];
        Array<uint8_t>* spriteArray = il2cpp_utils::vectorToArray(bytes);
        AllFramesResult result;
        Gif gif = Gif(spriteArray);
        bool isOK = false;
        if (gif.Parse() == 0 && gif.Slurp() == 1) {
            result = gif.get_all_frames();
            isOK = true;
            
        } else {
            Texture2D* texture = Texture2D::New_ctor(0, 0, TextureFormat::RGBA32, false, false);
            if (UnityEngine::ImageConversion::LoadImage(texture, spriteArray, false)) {
                texture->set_wrapMode(TextureWrapMode::Clamp);
                Array<UnityEngine::Texture2D*>* frames = Array<UnityEngine::Texture2D*>::NewLength(1);
                Array<float>* timings = Array<float>::NewLength(1);

                frames->values[0] = texture;
                timings->values[0] = 0;
                result = {
                    frames, timings
                };
                isOK = true;
            }
        }

        if (isOK) {
            completion(result);
        }
    } else {
        WebUtils::GetAsync(url, [completion, url](long code, string data) {
            if (code == 200) {
                iconCache[url] = {data.begin(), data.end()};
                QuestUI::MainThreadScheduler::Schedule([completion, url] {
                    get_AnimatedIcon(url, completion);
                });
            }
        });
    }
}

void Sprites::GetCountryIcon(string country, const std::function<void(UnityEngine::Sprite*)>& completion) {
    get_Icon("https://cdn.beatleader.xyz/flags/" + toLower(country) + ".png", completion);
}

void Sprites::ResetCache() {
    iconCache = {};
}

const string Sprites::UpB64 = "iVBORw0KGgoAAAANSUhEUgAAAJYAAABfCAMAAAA9OM6sAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAhGVYSWZNTQAqAAAACAAFARIAAwAAAAEAAQAAARoABQAAAAEAAABKARsABQAAAAEAAABSASgAAwAAAAEAAgAAh2kABAAAAAEAAABaAAAAAAAAAGAAAAABAAAAYAAAAAEAA6ABAAMAAAABAAEAAKACAAQAAAABAAAAlqADAAQAAAABAAAAXwAAAADvKUNOAAAACXBIWXMAAA7EAAAOxAGVKw4bAAABWWlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNi4wLjAiPgogICA8cmRmOlJERiB4bWxuczpyZGY9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkvMDIvMjItcmRmLXN5bnRheC1ucyMiPgogICAgICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgICAgICAgICB4bWxuczp0aWZmPSJodHRwOi8vbnMuYWRvYmUuY29tL3RpZmYvMS4wLyI+CiAgICAgICAgIDx0aWZmOk9yaWVudGF0aW9uPjE8L3RpZmY6T3JpZW50YXRpb24+CiAgICAgIDwvcmRmOkRlc2NyaXB0aW9uPgogICA8L3JkZjpSREY+CjwveDp4bXBtZXRhPgoZXuEHAAACiFBMVEUAAAAAAACAgIBVVVVAQEAzMzNmZmZVVVVJSUlAQEBgYGBVVVVNTU1GRkZVVVVOTk5JSUlVVVVQUFBLS0tVVVVRUVFNTU1RUVFOTk5VVVVOTk5SUlJPT09SUlJNTU1VVVVTU1NQUFBOTk5TU1NRUVFOTk5TU1NRUVFPT09TU1NRUVFPT09OTk5TU1NRUVFQUFBOTk5TU1NSUlJQUFBTU1NSUlJQUFBPT09TU1NSUlJRUVFPT09SUlJRUVFPT09SUlJRUVFQUFBSUlJRUVFQUFBPT09RUVFQUFBPT09TU1NRUVFQUFBPT09TU1NSUlJRUVFPT09SUlJRUVFSUlJRUVFSUlJQUFBQUFBRUVFQUFBRUVFRUVFQ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";

UnityEngine::Sprite* Sprites::get_UpIcon() {
    return QuestUI::BeatSaberUI::Base64ToSprite(const_cast<string&>(UpB64));
}

const string Sprites::DownB64 = "iVBORw0KGgoAAAANSUhEUgAAAJYAAABfCAYAAAAK5j6eAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAhGVYSWZNTQAqAAAACAAFARIAAwAAAAEAAQAAARoABQAAAAEAAABKARsABQAAAAEAAABSASgAAwAAAAEAAgAAh2kABAAAAAEAAABaAAAAAAAAAGAAAAABAAAAYAAAAAEAA6ABAAMAAAABAAEAAKACAAQAAAABAAAAlqADAAQAAAABAAAAXwAAAADvKUNOAAAACXBIWXMAAA7EAAAOxAGVKw4bAAABWWlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNi4wLjAiPgogICA8cmRmOlJERiB4bWxuczpyZGY9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkvMDIvMjItcmRmLXN5bnRheC1ucyMiPgogICAgICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgICAgICAgICB4bWxuczp0aWZmPSJodHRwOi8vbnMuYWRvYmUuY29tL3RpZmYvMS4wLyI+CiAgICAgICAgIDx0aWZmOk9yaWVudGF0aW9uPjE8L3RpZmY6T3JpZW50YXRpb24+CiAgICAgIDwvcmRmOkRlc2NyaXB0aW9uPgogICA8L3JkZjpSREY+CjwveDp4bXBtZXRhPgoZXuEHAAASiklEQVR4Ae2dW2wc1RnHvb6GxDE2EBS1QmpVKGoDpDRcU5w4VxKb3JNSCr2CeKtU9aGiqtTyVB76UIk+VEK9pLSlTdNcnWAIJHEu3BtIgASUSuWBi4DmDiS+u7//eL7N8Xg33nW8u3N2d6TxjGdnZs/5vv/5f//zzZmziYrslkRLS0tVZ2dnP5cN2qVr1qyZ2tfX1zQwMNBUWVn5WSKROF5VVfXx+vXre+wcrquOXmeflbf5sQB+qpo2bdrgww8/PGDfuGLFiiv7+/ub8NdlbLuqq6tPNDQ0fLR27douO0e+Yx1wr7PP0m0T6T6IHuemlYcPH04AFoGqggLNZrOIdR7rF1gnAaiJg4ODveyfYXuM//ey7qCg26ygIcD6OKe85MkC8h2NWmtg95UrV94MCbTim/kU4RpW+a4en8m3Z1hPsP882x01NTXt+Pw0+xXZ+C4jYLk3BFB38B2/gJkWgO4KChisFKRCKwVMrpyj8lT09va+zeaRTZs2Pa7/H3zwwZrHHntMACwvObaAWMohg+l83c/x0RoAk6nv3sWvvz516tTvBMxMfTcqsFxQLV++/DeA6ccCDGARnQrhQo/u497LwmRAudBsNWtFT0/Pbuj2nq1bt36UaQG5b3kZowVc30EID3GbR2pra+UH3VENO/CdyADwmP/S+e4g598NORzNxHd2M33RiMVusHTp0smAaX1dXd2dYaFUstoRF6Q/IAoepFI1XP8elVi+efPmA7SmWleHpb+8/Em2FnBAlQBUj2P7+yADAaibe9VlcT+RRz8MV8v1Z4hQq7Zs2fKsYSPdfdICy5zO9jKEeScFux5QqFDVrFXpbniB44O0jG6YawL362J//saNG59bvHhxXUdHh+5bXsbJAgYqaatDhw51QAgLu7u7jaHG4juVTL6rI+JofwnMtc0wogPRZUgERY4KjWKStra2JkCwB7QaqIT0sRYsQWsRqHoELu7z7OrVq2cKVCpgpAjlf8doAQMVlycA1VMQgkClCDNWQrCSCFS9oW5upwPQJoyk890IYBnFCVQUSqC6DgpUwbKhTytMqm0tdKoCTqCgO5ctWzbzQgVMdYPysdQWcEClXrtAtYAoY7IlbXRKfbeUR2vwXZ/ABUlsW7VqVat8J8xEzx72ZQYqRHojoUqguiEEVS4YReBSQc9RqHlQ6wvlsBh1T+b/O6BK4D+FP+lhA1XmN8rgTLDRyyrfqWfZhuZ60rBjlyeBZQXDuQ0TJkzYF4IqW6Fn9810q7BYC3Odo6Bz0VwvXihuZ3rTUjvPfKd6w1QdMNWiXIHKsa2Yq1rggr1a6Yx1uOCyUJiw5Bmg2gjaxVS5BpXKKFD1Aq5LKNwu4vZt5bDouC6D3QKBSiWrxmeB5oIU2mHJGcpNWlgMGAuWCJJofPgooPpRHtAeNZnAVQPIztIK5m7YsOGlMnNFTTTy/wKCyi1MD7nNWjpl73LwJiTNx8JTZejAfql8wp9ApaTmCDHm3ikH+wKVwuJEqHUXgv5WMZc0Vw6+qyhu6YIK323PU/hLZTtFnS4I6SoY7FGdoOeRSY1FbH4J5N2i0MQJ+QaWFdg011nKMIe4/XKZucw057cuqIgy23FqawGizPkCDQ1IUJ6yEr81w1r7A40FqNYIVNDZAB8o31GoJUhFiLkIidJcN5c113BXxBBUKqAIagAMaV+PjoJnRVL1y3Gk/tejlySL6UC+F8qisChROImwuNvAVQ6Lw0cXQAbbYsBULjyqICb934yUuUoa61IobCbO1EHrJWq/kEuguWgBkwDaLhJxN5V6ht5lKkDVjqZqK3D4G4EPfDVAtGlgba4krXA1wPpCzIClQlsqol7Mpe5sqYZFF1Qw+FZAdReg0rO/XCSuRwAmwwNBOFTkA2C3SmxdAcJ0rXqDBQ2DKSoQhEXKVw/4SxJcLqhoXFvpuS8JmapQHawUbhpxaCoAq/wcTtMnGocTN2CpXJaKmCxw0WK/XiqpCBdUhL8taCoDVZyYSj6yJaFMPMvnxVh9rPZBXLcKi0pFTKasnQJXsWuuCKg2E/6Wxk1TpQJLSFJ9ley8H54gtoozwkxzCVy7EfQ3FqvmSgGqZT6ASvgJgfWeeoHHw26i9uMMLIoXhEU9/mmAwTrRHF8rtrDoggpm3gRT+QIq+SdYRFaVdOmP8t9bYXIryDmEn8d1Y6mIBioQgEthsRjyXC6o0FQbEerLPRDqhhORUpWyC0SU5ytp8RoPtV/dRJa4M5bKqMU0l3JwnThhuu+aKwoqmGoFoLKUQhw7VUOeOP93EAxJvCsCPmcJ0fVhOFRvI5D158+P7Z5prkspYQAuXzVXBFQbQlBpkF6cUwpRYPSHaavt7e3txwSsBA8NnwFpT0G9OtmHcGiVslREIwc6EfQ3+Ka5XFChqf4FqFZ6FP4CPxA1ghExJNv7Ya1HdLCSgVnBk0Pi4i/FWpwkdPnCWqqDhcVGGscenlNd70tYdEFFOF9Pw17lWfiT/aWpesmxafdRxtK9HYzH0qg/VVBDVDjhJ6GIF7B80VuqkIXFRlpMAK64h8UoqGCq1SGoCjm6RLbMdummQdR1dXW9dPLkyZ/q4mHjsfg/yGNBx7+nkveHrwyJvXwQjqqPlmAkKsx7EuadRYh/U71FMdjQx/H4GwHVP7H3Gif8+WRvGz36Ib3BGbzh/sGMGTNqDhw40GvivQL6CvZ5oeEBQLWFykrIq1fiE3NZKqKJcu8hvFwXt1REBFTrQlDJzr414mBQJo34GBJknkCl8e4CFXUZzkaRSgtceoxgldb5vizWkk4QGmcR9w/TcAr+On/EvgLVN53w5yNTHQdUs3j964hA5U70MqIykcr7DK5e9GINLSpZ+UKCy7UrTwz+gdi921dQkVaQpj0BqJoFqlR2HQEsUZJrBDRXdKhGymtiSmU2hj4JrkJoLteehOe/w1Tf8h1UFgnS2TOpsVxg2DxIOobmUjjchjFMc7mnxn1fLasb5rpcvUWc+pV8pyIioHqiGECF02ebvEjXMbog+0SMInBpOKyPmsvC4jGMot7iW6noe7xbCd/hTnomUN3jOVOdDO335mj2uyCwZOg04LLHDaNeP97Ouoj7Gbj+F9L42+lo/CK+I3mpCyo01d/QVN/2HFSnQqH+xmigkhEyAoblJnQBRorDe2wqylgWm+Mpp+CKNMa/wlT3eg6qkzTGFsLf65k2xpQaK+ox5SbUndRxMvQKh5p4QppLzOXTojmelIqYQuvbw0yF14635ioiUCnZLI16ykAlpkqnqaIgyIix7KKI0Z4EW4vDlhiAzs7zYGth8WPKqjd3j2ZC76PVyw1/dBT+gn3u85SpbC6N06FseD1b+2QFLBnWAZfmthS4bMoc7zLHMJcms0iCK1OaTwUwF1TIhcfRVN/xFFSWotEU3LNpdIfGYpeMQqFrSCcVMciXtvL452nA5WUqAlApFXEl9dvLkJtrxhoW1dh46B0MN6Kx/dljUFn4O82AhBaBSkyVafhzcZI1Y9nFjqBPkETt4Am3ZpDzMRVhj38+DGn/P9nQvstUAhVt7LueMpWFv08A1Sy09MFs7GC4sG3WjGUXStCrpfL/IElUaa0dGFXh0DdBr3AozTUVQb8XQX91pkNuIqBaWwygQqzPvlhQCSNjBpYudsPi9OnTBa5nMK7CYqyGqaisoyx6pqiwOJWe0D4Dl7RFuutcUKGp/kS1v+c5U51hqFELoxReU73VuNLVPZPjFwUsfYFND8ic4gOAS0Je4JJDLqpgmRR+nM+pA1wKiwLXXsDypXSaKwoqNNX3PQeVwp/mgH1V4W8smirqizFrrOiNrLeoSesPHjz4NMaeHxrb11TEB2iuZpKC/5WxrQW7oEJT/ZFG9APGemuyOsmCcbNn1L45+N801acS6oS/cf2lkHE1hIFLxqf1q7c4D3CJuXxNRQwDF/Xod3p/f6B+PyyDKjXkxxVY+gob8BUBVz5mYE5dw7EfleZSpv59QmMzgHqHW8legxq+TS/4ft9BhaZS+HvFZeSxm2v4leMOLN0+Ai71FueGzCVh79NiqYgPYOA7eF/uHQt/1EfT12n+p5zYMEdGCsIfvd/PuP+cXIFKZc+ZUSwsatvU1CRwzfEUXL2wk3617Agt/DV274Wp9BaTbJcz+8k547yYpjqr3l8uQaVy59QwxlwRcPkYFoNfYUDMV8BcyrCrN51T28k547gkmQq2motQz/ls1GP9Ja+M6kwSdUDgomfVN2nSpCfq6+ubYa6rNcKAG+T0uzMqYOYnaR6xfpwipvK193eW4s9jjHrOQSWzXnQeazTf2AuxytRPmTJlISFlD+Dy8dmiGoJvL5Na+DsnpgJUefvFj5wDS8BThl7hUCADXPqpM4HLx8c/o7WjOH2eBJV6f/kElYyQF2DpiwQu01x048Vce0Pm8u3xj6oT9yUJKgo6D6Ge919Vyxuw5Al7/KMsNuBawJCbfYDLx8c/cQaWCfUugYqhLy/kIk81mgEK0rNRWBSD6WEnP2OnZ4vNMJgEvW95rtHsm+/P7cdFu0NN9XwhQKVK55WxzMoWFvWwk1lKpLn2l8OiWWfM2wBU9F71Q+7q/RUMVKpBQRjLTGetKWSunYDrG2XmMutktU0ylUCFpnrObJvVXcbx5ILmko4cOdIvQb9u3bqeqVOnPgGw5rB+0cM81zi6JOtbGagkJeajqQoOKtWgoIxlJjTNxXZCY2PjTobczCwzl1nnglsXVBLq+wvNVFbagmgs+3LbSnPJIGy7AJaG2rzAMzkfR6JalfKxTYIKoS6mig2oVPlYMJZ5wVob20t4JifNdXuZucw6w7YGql401Xw01V6z3bCzCvhPrIAlO9jbPyG4dgGu28rgGoaQAFT0/vRGVCxBpdLGIhS6ZrPX+UminoO1FBZfBFzlsDhkpCRTAawFcWQq82XsGMsKZtS+ZMmSiYzgFHPdyjioUk6iJpkKTbWQoS+dZiOzWZy2sWMsM44e+ygVwajNs6Qf5gKql0PmErhKbTGm0tCd2INKzoktYxlyTHMtXLiQIV2TxFy3lJLmQpxLoNcAqD62Cwl/u+PMVOa32DKWFVCaS4bcsWPHZywaO/9KCaUikqDCHnf6Air5LvbAUiEVFg1cjIpQWBS4in1URDL8IdQXkafa5QNTyV9avACWCmqai+2nAhfM9e8i1lwaY6/wNwCo7kSo7/QJVPJX7DWWCukuNlgQQ9eTjuiEuWbAYD6+oOFWy90PXtwAUxLqixil8KxvoFJlvAOWCm2GZvKOyaQidhcRuCz86aUNhb9nrK6qt0+LN6HQNaqFRWZG+YRUxBwY61XfNZd6f2H4028XeQ0q+cpLxjKQWSqC8VwNjIjoZL3R01SEhT8k1eAiNNUOX5nKfOMlY1nhLRXBSNQzjJ9vYdWbyl49/gmZqlo6nXVxMYBK/vEaWKqApSIELkClsHjQo7BoeSr9UPdihPrTvjOVfKLFe2CpEqa52J4mFdESgkvMFefHP5ZSUBVaeVu8aEClCnmtsVQBd3FSEZeSitgDc02PaSrCNJWKv5je31PFwlTmj6ICliplDmKqx0b0i8B1Q5zAJU3FquSnfqS7FU3VYWU2pxTDtuiA5YKrra2tiey8kqgCV8GH3LigAlhtaKonixFU8kFRaKxoCzfNtX379pOkH6S53gBchdZcGp0QMBVCvahBJX8UJWMZ0IwNQuZSWLy+QGExqakA112MUthuZbOyFtu2KBnLnGSpCDFX2Ft8E3DldVREGP6Up1KxlpQCqFTRogaWKmhhke0JwDUbxjqcx7CoeUptOvIl9P62FTtTyeZaijoUDlVx6K+lIpic9nKOKCxOy7Ggl6YKJmrjeeZShHp7qYBKFi96xhqC1dAUSnIsrHGcY2KuIyFz5WJ+riSo+K5lApWALfa08hT7tmQYyxxprMHbP1cQGsVcXx1nQR/kqfR95KmWA+Qt9p1WhlLYlgxjmTNNc/H2zzGy84HmItdlgl5DVi5m6dHQF93AQFVqTGXGKzlgqeI2s6DAhf7Ra/wacqM8lxhcbxhryu1MQabunkR6v0IrvT9NJNsqpjJdx2clt5RcKHQ97DqenzH5FSzzM8ChudzFOAJX8Kup4TVmKxdwOlZDSK1AqFcQUvfx/wOA6qh77/D6ktqYsUqq0m5l0T9VhMcAQAx1vpGhzg/x+V0AbKLAAsCC022rY7aEYNI5r7L+lud+a/WZDUC080pxe95KpVj783VOMDdXlaZT0iFSEl9mM5f1dsBzLaCRbqpn1UR1n7L2cewj9NSL7O8h6SmmChDoApVj5aVsgaEfl9LvLaayheaQ0Msb6T5Xz4/ryg01NN7/AVc/vUYekbmIAAAAAElFTkSuQmCC";

UnityEngine::Sprite* Sprites::get_DownIcon() {
    return QuestUI::BeatSaberUI::Base64ToSprite(const_cast<string&>(DownB64));
}

const string Sprites::StarB64 = "iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAMAAAD04JH5AAAB5lBMVEX////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////ZkfksAAAAoXRSTlMAAQIDBAUGCAkKCwwNEBESExUWFxgZGxweHyQlJicrLC0vMTQ2Nzg5Oj9AQUJFSElKS0xOUlNUVldYWlxeX2BhYmRlaGtsbm9xcnR1dnd6e35/hYeIiouOk5SWmJmcn6Gio6Slp6ipra6xsrO1t7u8vb6/wcLDxMXGx8jJysvO0NHW19jZ2tvd3t/i5OXm6Ovt7/Dx8vP09fb3+Pn6+/z9/nFfuYEAAAQpSURBVHgB7dkJN1RhAwfw/8VQSCVFjaRQoSQtKbQv0hJlSdGuRKSlSEUhRUiNlAzm/03f88qZY5Z7Z5773Pvc9z2n39f44Z//byk3R0dvpsAxrmck+cwFp1RyUSUcEjvCRSOxcMZ+LtkPZ7zikldwRA79cuCE+/S7Dwds8NLPuwHqXeMy16DcykkuM7kSqp1igFNQTBtkgEENahUzSDHU6mSQTii1hSG2QKVmhmiGQmtnGGJmLdSpZhjVUCZ+gmFMxEOVCoZVAVX6GVY/FCmkjkKo0UYdbVDC7aMOnxsqXKeu61Bg1TR1Ta+C/S7QwAXYLm6UBkbjYLdSGiqF3XppqBc2284ItsNeDxnBQ9gqY54RzGfATvWMqB42SvrOiL4nwT5nGIUzsE3MMKMwHAO77GNU9sEWcem53YxKd256HKyipW7be7Kmqb1vYoECFib62ptqTu7dlqrBlJSs3RWXG1t7R72U5B3tbW28XLE7KwWRJboLjlQ1tLz8/Js2+P35ZUtD1ZECdyKCZOw8eLb2bteAh4p4Brru1p49uDMD/1X+gY75UA4U+uggXyHe0FFvMERHDaGLjupC9hwdNJcNHJ6lY2YPA8AuDx3i2YVFW8fpiPGtWLJxgA4Y2Ai/NT1UrmcNllnxmIo9XoEAsU1UqikWwaqpUDXCODFPReZPIKySX1TiVwl05H2jAt/yoCvzC233JRMG0vpos740GErupK06kxGB6x5tdM+FiLQ62qZOQzTO+WgL3zlE6dAf2uDPIUSt4Act96MAArK/0mJfsyEk4yMt9TEDglJe00KvUyAsoZWWaU2ACTE3aJEbMTDnCi1xBaYdm6O0uWOQsGeakqb3QEoxJRVDTikllUJOAyU1QE43JXVDimuGkmZckJFDaTmQcZrSTkPGHUq7AxmfKO0TJKymBVbDvCJaoAjm1dACNTCvgxbogHmTtMAkTHPTEm6YVUZLlMGsRlqiEWa9pyXew6QELy3hTYA5O2iRHTCnkhaphDkttEgLzBmhRUZgyjpGwetlFNbZlvUdmzd32Bb6VxnJcAkAlAwzkqsw4ymN/bwUj0Xxl37S2FOYEOOhEd/t9fBbf9tHI54YiMuikXd5CJD3jkayIO449U0c1RBEOzpBfcchrol6vPXJCCO53ks9TRDXTx1PMqEj8wl19ENY4jzDGiqGgeIhhjWfCFH5DGeqKh6G4qumGE4+RF1kKN+tNESUdsvHUBch6hFD9OYiKrm9DPEIosYYZLxcQ5S08nEGGYOgdAaarU2GgOTaWQZKh5gDDNC+CYI2tTPAAYip4zKDRTChaJDL1EHMc/p5zrtgiuu8h37PIeYFlyw0p8K01OYFLnkBMQ/4V08OpOT08K8HEOOeIsmxMg2StLIxkpxyQ1B+2/Tb6iRYIKn67XRbPv5X/fMfwXx1itpIs6EAAAAASUVORK5CYII=";

Sprite* Sprites::get_StarIcon() {
    return QuestUI::BeatSaberUI::Base64ToSprite(const_cast<string&>(StarB64));
}

const string Sprites::PixelB64 = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAANSURBVBhXY2BgYGAAAAAFAAGKM+MAAAAAAElFTkSuQmCC";

Sprite* Sprites::get_TransparentPixel() {
    return QuestUI::BeatSaberUI::Base64ToSprite(const_cast<string&>(PixelB64));
}

const string Sprites::GraphB64 = "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";

Sprite* Sprites::get_GraphIcon() {
    return QuestUI::BeatSaberUI::Base64ToSprite(const_cast<string&>(GraphB64));
}

const string Sprites::ReplayB64 = "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";

Sprite* Sprites::get_ReplayIcon() {
    return QuestUI::BeatSaberUI::Base64ToSprite(const_cast<string&>(ReplayB64));
}
