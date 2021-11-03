/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "group_auth_manager.h"
#include "common_defs.h"
#include "dev_auth_module_manager.h"
#include "device_auth_defines.h"
#include "hc_log.h"
#include "session_manager.h"

static int32_t GetModuleTypeFromPayload(const CJson *authParams)
{
    int32_t authForm = AUTH_FORM_INVALID_TYPE;
    if (GetIntFromJson(authParams, FIELD_AUTH_FORM, &authForm) != HC_SUCCESS) {
        LOGE("[GetModuleTypeFromPayload], Failed to get authForm!");
        return HC_ERR_JSON_GET;
    }
    if (authForm == AUTH_FORM_ACCOUNT_UNRELATED) {
        return DAS_MODULE;
    } else if ((authForm == AUTH_FORM_IDENTICAL_ACCOUNT) || (authForm == AUTH_FORM_ACROSS_ACCOUNT)) {
        return TCIS_MODULE;
    }
    LOGE("Invalid authForm for repeated payload check!");
    return INVALID_MODULE_TYPE;
}

int32_t GetAuthState(int64_t authReqId, const char *groupId, const char *peerUdid, uint8_t *out, uint32_t *outLen)
{
    (void)authReqId;
    (void)groupId;
    (void)peerUdid;
    (void)out;
    (void)outLen;
    LOGE("Lite group auth do not support func: %s!", __func__);
    return HC_ERR_NOT_SUPPORT;
}

void InformDeviceDisconnection(const char *udid)
{
    (void)udid;
    LOGE("Lite group auth do not support func: %s!", __func__);
    return;
}

bool IsTrustedDevice(const char *udid)
{
    (void)udid;
    LOGE("Lite group auth do not support func: %s!", __func__);
    return false;
}

int32_t QueryTrustedDeviceNum(void)
{
    LOGE("Lite group auth do not support func: %s!", __func__);
    return HC_ERR_NOT_SUPPORT;
}

void DoAuthDevice(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL, can't start lite-auth device!");
        return;
    }
    AuthDeviceTask *realTask = (AuthDeviceTask *)task;
    int32_t result = CreateSession(realTask->authReqId, TYPE_CLIENT_AUTH_SESSION_LITE,
        realTask->authParams, realTask->callback);
    if (result != HC_SUCCESS) {
        LOGE("Failed to create lite auth session for auth device!");
        if ((result != HC_ERR_CREATE_SESSION_FAIL) && (realTask->callback != NULL) &&
            (realTask->callback->onError != NULL)) {
            LOGE("[DoAuthDevice] Begin invoke onError by group auth manager lite!");
            realTask->callback->onError(realTask->authReqId, AUTH_FORM_INVALID_TYPE, result, NULL);
            LOGE("[DoAuthDevice] End invoke onError by group auth manager lite!");
        }
    }
}

void DoProcessAuthData(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL, can't process lite-auth data!");
        return;
    }
    AuthDeviceTask *realTask = (AuthDeviceTask *)task;
    int32_t res;
    if (IsRequestExist(realTask->authReqId)) {
        res = ProcessSession(realTask->authReqId, AUTH_TYPE, realTask->authParams);
        if (res != HC_SUCCESS) {
            DestroySession(realTask->authReqId);
        }
        return;
    }
    res = CheckMsgRepeatability(realTask->authParams, GetModuleTypeFromPayload(realTask->authParams));
    if (res != HC_SUCCESS) {
        LOGD("Caller inputs repeated payload, so we will ignore it.");
        return;
    }
    res = CreateSession(realTask->authReqId, TYPE_SERVER_AUTH_SESSION_LITE, realTask->authParams,
        realTask->callback);
    if (res != HC_SUCCESS) {
        LOGE("Failed to create lite auth session for process data!");
        if ((res != HC_ERR_CREATE_SESSION_FAIL) && (realTask->callback != NULL) &&
            (realTask->callback->onError != NULL)) {
            LOGE("Begin invoke onError by group auth manager lite!");
            realTask->callback->onError(realTask->authReqId, AUTH_FORM_INVALID_TYPE, res, NULL);
            LOGE("End invoke onError by group auth manager lite!");
        }
    }
}