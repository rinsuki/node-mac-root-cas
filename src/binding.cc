#define NAPI_VERSION 3
#include <node_api.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

// for check memory leak
// #define MACROOTCAS_CF_RELEASE_COUNTER_CHECK

namespace macrootcas {
    napi_value listCertificates(napi_env env, napi_callback_info args) {
        #ifndef MACROOTCAS_CF_RELEASE_COUNTER_CHECK
        napi_value pems;
        if (napi_ok != napi_create_array(env, &pems)) {
            return nullptr;
        }
        #endif
        // napi_status status = napi_create_string_utf8(env, "HELLO~", NAPI_AUTO_LENGTH, &greeting);
        // if (status != napi_ok) {
        //     return nullptr;
        // }
        CFMutableDictionaryRef query = CFDictionaryCreateMutable(kCFAllocatorDefault, 3, NULL, NULL);
        CFDictionaryAddValue(query, kSecClass, kSecClassCertificate);
        CFDictionaryAddValue(query, kSecMatchLimit, kSecMatchLimitAll);
        CFDictionaryAddValue(query, kSecMatchTrustedOnly, kCFBooleanTrue);

        CFTypeRef resultType;
        auto secStatus = SecItemCopyMatching(query, &resultType);
        CFRelease(query);
        if (CFArrayGetTypeID() != CFGetTypeID(resultType)) {
            napi_throw_error(env, NULL, "macrootcas: internal: SecItemCopyMatching didn't return CFArray");
            return nullptr;
        }
        CFArrayRef result = (CFArrayRef) resultType;

        SecPolicyRef ssl_policy = SecPolicyCreateSSL(true, NULL);

        bool is_fail = true;
        uint32_t result_i = 0;
        for (uint32_t i=0; i<(uint32_t) CFArrayGetCount(result); i++) {
            SecCertificateRef cert = (SecCertificateRef) CFArrayGetValueAtIndex(result, (CFIndex) i);
            if (SecCertificateGetTypeID() != CFGetTypeID(cert)) {
                napi_throw_error(env, NULL, "macrootcas: internal: some of SecItems are not SecCertificate");
                goto fail;
            }
            SecKeyRef publicKey = SecCertificateCopyKey(cert);

            SecTrustRef trust;
            OSStatus os_status = SecTrustCreateWithCertificates(cert, ssl_policy, &trust);
            bool is_trusted = SecTrustEvaluateWithError(trust, NULL);
            CFRelease(trust);
            CFRelease(publicKey);
            if (!is_trusted) {
                continue;
            }

            #if false
            NSLog(@"result = %@", cert);
            #endif
            CFDataRef certData;
            os_status = SecItemExport(cert, kSecFormatPEMSequence, 0, NULL, &certData);
            if (os_status != noErr) {
                fprintf(stderr, "SecItemExport fail: %d\n", os_status);
                napi_throw_error(env, NULL, "macrootcas: internal: failed to some SecItemExport");
                goto fail;
            }

            #ifndef MACROOTCAS_CF_RELEASE_COUNTER_CHECK
            napi_value pem_string;
            if (napi_ok != napi_create_string_utf8(env, (const char*)CFDataGetBytePtr(certData), CFDataGetLength(certData), &pem_string)) {
                CFRelease(certData);
                goto fail;
            }
            #endif
            CFRelease(certData);
            #ifndef MACROOTCAS_CF_RELEASE_COUNTER_CHECK
            if (napi_ok != napi_set_element(env, pems, result_i++, pem_string)) {
                goto fail;
            }
            #endif
        }
        is_fail = false;

        fail:
        CFRelease(result);
        CFRelease(ssl_policy);
        if (is_fail) {
            return nullptr;
        } else {
            #ifndef MACROOTCAS_CF_RELEASE_COUNTER_CHECK
            return pems;
            #else
            return nullptr;
            #endif
        }
    }

    napi_value init(napi_env env, napi_value exports) {
        napi_status status;
        napi_value fn;

        status = napi_create_function(env, "listCertificates", NAPI_AUTO_LENGTH, listCertificates, nullptr, &fn);
        if (status != napi_ok) return nullptr;

        status = napi_set_named_property(env, exports, "listCertificates", fn);
        if (status != napi_ok) return nullptr;
        return exports;
    }

    NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
}