#include "main.hpp"
#include "Utils/WebUtils.hpp"
#include "Utils/ModConfig.hpp"

#include "libcurl/shared/curl.h"
#include "libcurl/shared/easy.h"

#include <filesystem>
#include <sstream>

#define TIMEOUT 10
#define USER_AGENT std::string(ID "/" VERSION " (BeatSaber/" + GameVersion + ") (Oculus)").c_str()
#define X_BSSB "X-BSSB: ✔"

namespace WebUtils {
    std::string GameVersion = "1.19.1";

    std::string API_URL = "";
    std::string WEB_URL = "";

    void refresh_urls() {
        if (getModConfig().ServerType.GetValue() == "Test") {
            API_URL = "https://blstage.azurewebsites.net/";
            WEB_URL = "https://agitated-ptolemy-7d772c.netlify.app/";
        } else {
            API_URL = "https://api.beatleader.xyz/";
            WEB_URL = "https://beatleader.xyz/";
        }
    }

    //https://stackoverflow.com/a/55660581
    std::string query_encode(const std::string& s)
    {
        std::string ret;

        #define IS_BETWEEN(ch, low, high) (ch >= low && ch <= high)
        #define IS_ALPHA(ch) (IS_BETWEEN(ch, 'A', 'Z') || IS_BETWEEN(ch, 'a', 'z'))
        #define IS_DIGIT(ch) IS_BETWEEN(ch, '0', '9')
        #define IS_HEXDIG(ch) (IS_DIGIT(ch) || IS_BETWEEN(ch, 'A', 'F') || IS_BETWEEN(ch, 'a', 'f'))

        for(size_t i = 0; i < s.size();)
        {
            char ch = s[i++];

            if (IS_ALPHA(ch) || IS_DIGIT(ch))
            {
                ret += ch;
            }
            else if ((ch == '%') && IS_HEXDIG(s[i+0]) && IS_HEXDIG(s[i+1]))
            {
                ret += s.substr(i-1, 3);
                i += 2;
            }
            else
            {
                switch (ch)
                {
                    case '-':
                    case '.':
                    case '_':
                    case '~':
                    case '!':
                    case '$':
                    case '&':
                    case '\'':
                    case '(':
                    case ')':
                    case '*':
                    case '+':
                    case ',':
                    case ';':
                    case '=':
                    case ':':
                    case '@':
                    case '/':
                    case '?':
                    case '[':
                    case ']':
                        ret += ch;
                        break;

                    default:
                    {
                        static const char hex[] = "0123456789ABCDEF";
                        char pct[] = "%  ";
                        pct[1] = hex[(ch >> 4) & 0xF];
                        pct[2] = hex[ch & 0xF];
                        ret.append(pct, 3);
                        break;
                    }
                }
            }
        }

        return ret;
    }


    std::size_t CurlWrite_CallbackFunc_StdString(void *contents, std::size_t size, std::size_t nmemb, std::string *s)
    {
        std::size_t newLength = size * nmemb;
        try {
            s->append((char*)contents, newLength);
        } catch(std::bad_alloc &e) {
            //handle memory problem
            getLogger().critical("Failed to allocate string of size: %lu", newLength);
            return 0;
        }
        return newLength;
    }

    std::optional<rapidjson::Document> GetJSON(std::string url) {
        std::string data;
        Get(url, data);
        rapidjson::Document document;
        document.Parse(data);
        if(document.HasParseError() || !document.IsObject())
            return std::nullopt;
        return document;
    }

    long Get(std::string url, std::string& val) {
        return Get(url, TIMEOUT, val);
    }

    long Get(std::string url, long timeout, std::string& val) {

        std::string directory = getDataDir(modInfo) + "cookies/";
        std::filesystem::create_directories(directory);
        std::string cookieFile = directory + "cookies.txt";

        // Init curl
        auto* curl = curl_easy_init();
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Accept: */*");
        headers = curl_slist_append(headers, X_BSSB);
        // Set headers
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 

        curl_easy_setopt(curl, CURLOPT_URL, query_encode(url).c_str());

        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFile.c_str());
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFile.c_str());

        // Don't wait forever, time out after TIMEOUT seconds.
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

        // Follow HTTP redirects if necessary.
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);

        long httpCode(0);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&val));
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
        auto res = curl_easy_perform(curl);
        /* Check for errors */ 
        if (res != CURLE_OK) {
            getLogger().critical("curl_easy_perform() failed: %u: %s", res, curl_easy_strerror(res));
        }
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);

        return httpCode;
    }

    struct ProgressUpdateWrapper {
        std::function<void(float)> progressUpdate;
        long length;
    };

    void GetAsync(std::string url, std::function<void(long, std::string)> finished, std::function<void(float)> progressUpdate) {
        GetAsync(url, TIMEOUT, finished, progressUpdate);
    }

    void GetAsync(std::string url, long timeout, std::function<void(long, std::string)> finished, std::function<void(float)> progressUpdate) {
        std::thread t (
            [url, timeout, progressUpdate, finished] {
                std::string directory = getDataDir(modInfo) + "cookies/";
                std::filesystem::create_directories(directory);
                std::string cookieFile = directory + "cookies.txt";
                std::string val;
                // Init curl
                auto* curl = curl_easy_init();
                struct curl_slist *headers = NULL;
                headers = curl_slist_append(headers, "Accept: */*");
                headers = curl_slist_append(headers, X_BSSB);
                // Set headers
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 

                curl_easy_setopt(curl, CURLOPT_URL, query_encode(url).c_str());

                curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFile.c_str());
                curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFile.c_str());

                // Don't wait forever, time out after TIMEOUT seconds.
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

                // Follow HTTP redirects if necessary.
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);

                ProgressUpdateWrapper* wrapper = new ProgressUpdateWrapper { progressUpdate };
                if(progressUpdate) {
                    // Internal CURL progressmeter must be disabled if we provide our own callback
                    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
                    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, wrapper);
                    // Install the callback function
                    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, 
                        +[] (void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
                            float percentage = (float)dlnow / (float)dltotal * 100.0f;
                            if(isnan(percentage))
                                percentage = 0.0f;
                            reinterpret_cast<ProgressUpdateWrapper*>(clientp)->progressUpdate(percentage);
                            return 0;
                        }
                    );
                }

                long httpCode(0);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &val);
                curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            
                auto res = curl_easy_perform(curl);
                /* Check for errors */ 
                if (res != CURLE_OK) {
                    getLogger().critical("curl_easy_perform() failed: %u: %s", res, curl_easy_strerror(res));
                }
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
                curl_easy_cleanup(curl);
                delete wrapper;
                finished(httpCode, val);
            }
        );
        t.detach();
    }

    void GetJSONAsync(std::string url, std::function<void(long, bool, rapidjson::Document&)> finished) {
        GetAsync(url,
            [finished] (long httpCode, std::string data) { 
                rapidjson::Document document;
                document.Parse(data);
                finished(httpCode, document.HasParseError() || !document.IsObject(), document);
            }
        );
    }

    void PostJSONAsync(std::string url, std::string data, std::function<void(long, std::string)> finished) {
        PostJSONAsync(url, data, TIMEOUT, finished);
    }

    void PostJSONAsync(std::string url, std::string data, long timeout, std::function<void(long, std::string)> finished) {
        std::thread t(
            [url, timeout, data, finished] {
                std::string val;
                // Init curl
                auto* curl = curl_easy_init();
                //auto form = curl_mime_init(curl);
                struct curl_slist* headers = NULL;
                headers = curl_slist_append(headers, "Accept: */*");
                headers = curl_slist_append(headers, X_BSSB);
                headers = curl_slist_append(headers, "Content-Type: application/json");
                // Set headers
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                curl_easy_setopt(curl, CURLOPT_URL, query_encode(url).c_str());

                // Don't wait forever, time out after TIMEOUT seconds.
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

                // Follow HTTP redirects if necessary.
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);

                long httpCode(0);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &val);
                curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, &data);

                CURLcode res = curl_easy_perform(curl);
                /* Check for errors */
                if (res != CURLE_OK) {
                    getLogger().critical("curl_easy_perform() failed: %u: %s", res, curl_easy_strerror(res));
                }
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
                curl_easy_cleanup(curl);
                //curl_mime_free(form);
                finished(httpCode, val);
            }
        );
        t.detach();
    }

    void PostFormAsync(std::string url, std::string action, std::string login, std::string password, std::function<void(long, std::string)> finished) {
        std::thread t(
            [url, action, login, password, finished] {
                long timeout = TIMEOUT;
                std::string directory = getDataDir(modInfo) + "cookies/";
                std::filesystem::create_directories(directory);
                std::string cookieFile = directory + "cookies.txt";
                std::string val;
                // Init curl
                auto* curl = curl_easy_init();
                //auto form = curl_mime_init(curl);
                struct curl_slist* headers = NULL;
                headers = curl_slist_append(headers, "Accept: */*");
                headers = curl_slist_append(headers, X_BSSB);
                headers = curl_slist_append(headers, "Content-Type: multipart/form-data");
                // Set headers
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                curl_easy_setopt(curl, CURLOPT_URL, query_encode(url).c_str());

                curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFile.c_str());
                curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFile.c_str());

                // Don't wait forever, time out after TIMEOUT seconds.
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

                // Follow HTTP redirects if necessary.
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);

                struct curl_httppost *formpost=NULL;
                struct curl_httppost *lastptr=NULL;

                curl_formadd(&formpost,
                    &lastptr,
                    CURLFORM_COPYNAME, "action",
                    CURLFORM_COPYCONTENTS, action.c_str(),
                    CURLFORM_END);
                    curl_formadd(&formpost,
                    &lastptr,
                    CURLFORM_COPYNAME, "login",
                    CURLFORM_COPYCONTENTS, login.c_str(),
                    CURLFORM_END);
                    curl_formadd(&formpost,
                    &lastptr,
                    CURLFORM_COPYNAME, "password",
                    CURLFORM_COPYCONTENTS, password.c_str(),
                    CURLFORM_END);
                curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

                long httpCode(0);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &val);
                curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

                CURLcode res = curl_easy_perform(curl);
                /* Check for errors */
                if (res != CURLE_OK) {
                    getLogger().critical("curl_easy_perform() failed: %u: %s", res, curl_easy_strerror(res));
                }
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
                curl_easy_cleanup(curl);
                curl_formfree(formpost);
                //curl_mime_free(form);
                finished(httpCode, val);
            }
        );
        t.detach();
    }

    struct input {
        FILE *in;
        size_t bytes_read; /* count up */
        CURL *hnd;
    };

    static size_t read_callback(char *ptr, size_t size, size_t nmemb, void *userp)
    {
        struct input *i = (struct input *)userp;
        size_t retcode = fread(ptr, size, nmemb, i->in);
        i->bytes_read += retcode;
        return retcode;
    }

    void PostFileAsync(std::string url, FILE* data, long length, long timeout, std::function<void(long, std::string)> finished, std::function<void(float)> progressUpdate) {
        std::thread t(
            [url, timeout, data, finished, length, progressUpdate] {
                std::string val;
                std::string directory = getDataDir(modInfo) + "cookies/";
                std::filesystem::create_directories(directory);
                std::string cookieFile = directory + "cookies.txt";

                
                // Init curl
                auto* curl = curl_easy_init();

                struct input i;
                i.in = data;
                i.hnd = curl;

                //auto form = curl_mime_init(curl);
                struct curl_slist* headers = NULL;
                headers = curl_slist_append(headers, "Accept: */*");
                headers = curl_slist_append(headers, X_BSSB);
                headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
                // Set headers
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                curl_easy_setopt(curl, CURLOPT_URL, query_encode(url).c_str());
                curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

                curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFile.c_str());
                curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFile.c_str());

                // Don't wait forever, time out after TIMEOUT seconds.
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

                ProgressUpdateWrapper* wrapper = new ProgressUpdateWrapper { progressUpdate, length };
                if (progressUpdate) {
                    // Internal CURL progressmeter must be disabled if we provide our own callback
                    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
                    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, wrapper);
                    // Install the callback function
                    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, 
                        +[] (void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
                            float percentage = (ulnow / (float)reinterpret_cast<ProgressUpdateWrapper*>(clientp)->length) * 100.0f;
                            if(isnan(percentage))
                                percentage = 0.0f;
                            reinterpret_cast<ProgressUpdateWrapper*>(clientp)->progressUpdate(percentage);
                            return 0;
                        }
                    );
                }

                // Follow HTTP redirects if necessary.
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);

                long httpCode(0);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &val);
                curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

                curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
                /* size of the POST data */
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, length);
                /* binary data */
                curl_easy_setopt(curl, CURLOPT_READDATA, &i);

                CURLcode res = curl_easy_perform(curl);
                /* Check for errors */
                if (res != CURLE_OK) {
                    getLogger().critical("curl_easy_perform() failed: %u: %s", res, curl_easy_strerror(res));
                }
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
                curl_easy_cleanup(curl);
                //curl_mime_free(form);
                finished(httpCode, val);
            }
        );
        t.detach();
    }

}