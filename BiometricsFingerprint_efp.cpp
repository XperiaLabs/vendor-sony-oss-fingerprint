#include "BiometricsFingerprint_efp.h"
#include "FormatException.hpp"

#define LOG_TAG "FPC ET"
#include <log/log.h>

namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_1 {
namespace implementation {

BiometricsFingerprint_efp::BiometricsFingerprint_efp() : loops(reinterpret_cast<uint64_t>(this)) {
    QSEEKeymasterTrustlet keymaster;
    mMasterKey = keymaster.GetKey();

    int rc = loops.Prepare();
    if (rc)
        throw FormatException("Prepare failed with rc = %d", rc);

    rc = loops.SetMasterKey(mMasterKey);
    if (rc)
        throw FormatException("SetMasterKey failed with rc = %d", rc);

    mAuthenticatorId = loops.GetRand64();
}

Return<uint64_t> BiometricsFingerprint_efp::setNotify(const sp<IBiometricsFingerprintClientCallback> &clientCallback) {
    loops.SetNotify(clientCallback);
    // This is here because HAL 2.1 doesn't have a way to propagate a
    // unique token for its driver. Subsequent versions should send a unique
    // token for each call to setNotify(). This is fine as long as there's only
    // one fingerprint device on the platform.
    return reinterpret_cast<uint64_t>(this);
}

Return<uint64_t> BiometricsFingerprint_efp::preEnroll() {
    // TODO: Original service aborts+retries on failure.
    auto challenge = loops.GetChallenge();
    ALOGI("%s: Generated enroll challenge %lu", __func__, challenge);
    return challenge;
}

Return<RequestStatus> BiometricsFingerprint_efp::enroll(const hidl_array<uint8_t, 69> &hat, uint32_t gid, uint32_t timeoutSec) {
    if (gid != mGid) {
        ALOGE("Cannot enroll finger for different gid! Caller needs to update storePath first with setActiveGroup()!");
        return RequestStatus::SYS_EINVAL;
    }

    const auto h = reinterpret_cast<const hw_auth_token_t *>(hat.data());
    if (!h) {
        // This seems to happen when locking the device while enrolling.
        // It is unknown why this function is called again.
        ALOGE("%s: authentication token is unset!", __func__);
        return RequestStatus::SYS_EINVAL;
    }

    ALOGI("Starting enroll for challenge %lu", h->challenge);
    return loops.Enroll(*h, timeoutSec) ? RequestStatus::SYS_EINVAL : RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint_efp::postEnroll() {
    ALOGI("%s: clearing challenge", __func__);
    // TODO: Original service aborts+retries on failure.
    return loops.ClearChallenge() ? RequestStatus::SYS_UNKNOWN : RequestStatus::SYS_OK;
}

Return<uint64_t> BiometricsFingerprint_efp::getAuthenticatorId() {
    return mAuthenticatorId;
}

Return<RequestStatus> BiometricsFingerprint_efp::cancel() {
    ALOGI("Cancel requested");
    bool success = loops.Cancel();
    return success ? RequestStatus::SYS_OK : RequestStatus::SYS_UNKNOWN;
}

Return<RequestStatus> BiometricsFingerprint_efp::enumerate() {
    return loops.Enumerate() ? RequestStatus::SYS_UNKNOWN : RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint_efp::remove(uint32_t gid, uint32_t fid) {
    ALOGI("%s: gid = %d, fid = %d", __func__, gid, fid);
    if (gid != mGid) {
        ALOGE("Change group and userpath through setActiveGroup first!");
        return RequestStatus::SYS_EINVAL;
    }
    return loops.RemoveFinger(fid) ? RequestStatus::SYS_EINVAL : RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint_efp::setActiveGroup(uint32_t gid, const hidl_string &storePath) {
    ALOGI("%s: gid = %u, path = %s", __func__, gid, storePath.c_str());
    mGid = gid;
    int rc = loops.SetUserDataPath(mGid, storePath.c_str());
    return rc ? RequestStatus::SYS_EINVAL : RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint_efp::authenticate(uint64_t operationId, uint32_t gid) {
    ALOGE("%s not implemented!", __func__);
    return RequestStatus::SYS_OK;
}

}  // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
