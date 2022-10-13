import { listCertificates } from "../build/Release/macrootcas_native.node"

let cachedCertificates: string[] | null = null

export type ListTrustedCertificatesOptions = {
    /** 
     * Bypass cache mechanism of mac-root-cas.
     * NOTE: currently native module MAY have memory leaks, so you shouldn't call this automatically. only use this when user request.
     */
    wontCache?: boolean
}

/**
 * List certificates that trusted by system administrator/user.
 * it will fetched from macOS Keychain.
 * NOTE: System bundled CAs will not listed.
 * @returns Array of certificate string (encoded as PEM format).
 */
export function listTrustedCertificates(options?: ListTrustedCertificatesOptions) {
    let certs = cachedCertificates
    if (certs == null || options?.wontCache === true) {
        certs = listCertificates()
        if (options?.wontCache !== true) {
            cachedCertificates = certs
        }
    }
    return [...certs]
}

/**
 * Clear (in-memory) cache of mac-root-cas.
 * NOTE: currently native module MAY have memory leaks, so you shouldn't call this automatically. only use this when user request.
 */
export function purgeCacheTrustedCertificates() {
    cachedCertificates = null
}

export function installCertificatesToCurrentInstance(options?: ListTrustedCertificatesOptions) {
    // TODO: replace with more better way
    const NativeSecureContext = (process as any).binding("crypto").SecureContext
    const old_addRootCerts = NativeSecureContext.prototype.addRootCerts
    NativeSecureContext.prototype.addRootCerts = function(...args: any[]) {
        var ret = old_addRootCerts.apply(this, args)
        for (const cert of listTrustedCertificates(options)) {
            this.addCACert(cert)
        }
        return ret
    }
}