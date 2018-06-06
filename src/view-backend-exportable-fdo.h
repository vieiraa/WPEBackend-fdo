/*
 * Copyright (C) 2018 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <cassert>
#include "wpe-fdo/view-backend-exportable.h"
#include <gio/gio.h>
#include <vector>

class ViewBackend;

class ClientBundleBase {
public:
    ClientBundleBase(void* _data, ViewBackend* _viewBackend, uint32_t _initialWidth, uint32_t _initialHeight)
        : data(_data)
        , viewBackend(_viewBackend)
        , initialWidth(_initialWidth)
        , initialHeight(_initialHeight)
    {
    }

    virtual void exportBuffer(struct wl_resource *bufferResource) = 0;
    virtual void exportBuffer(const struct linux_dmabuf_buffer *dmabuf_buffer) = 0;

    void* data;
    ViewBackend* viewBackend;
    uint32_t initialWidth;
    uint32_t initialHeight;
};

class ClientBundle : public ClientBundleBase {
public:
    ClientBundle(struct wpe_view_backend_exportable_fdo_client* _client, void* data, ViewBackend* viewBackend,
                 uint32_t initialWidth, uint32_t initialHeight)
        : ClientBundleBase(data, viewBackend, initialWidth, initialHeight)
        , client(_client)
    {
    }

    void exportBuffer(struct wl_resource *bufferResource) override
    {
        client->export_buffer_resource(data, bufferResource);
    }

    void exportBuffer(const struct linux_dmabuf_buffer *dmabuf_buffer)
    {
        assert(!"This interface doesn't support Linux DMA buffers");
    }

    struct wpe_view_backend_exportable_fdo_client* client;
};


class ViewBackend : public WS::ExportableClient {
public:
    ViewBackend(ClientBundleBase* clientBundle, struct wpe_view_backend* backend);
    ~ViewBackend();

    void initialize();
    int clientFd();
    void frameCallback(struct wl_resource* callbackResource) override;
    void exportBufferResource(struct wl_resource* bufferResource) override;
    void exportLinuxDmabuf(const struct linux_dmabuf_buffer *dmabuf_buffer) override;
    void dispatchFrameCallback();
    void releaseBuffer(struct wl_resource* buffer_resource);

private:
    static gboolean s_socketCallback(GSocket*, GIOCondition, gpointer);

    uint32_t m_id { 0 };

    ClientBundleBase* m_clientBundle;
    struct wpe_view_backend* m_backend;

    std::vector<struct wl_resource*> m_callbackResources;

    GSocket* m_socket;
    GSource* m_source;
    int m_clientFd { -1 };
};

extern "C" {

struct wpe_view_backend_exportable_fdo;

struct wpe_view_backend_exportable_fdo*
wpe_view_backend_exportable_fdo_new(ClientBundleBase *clientBundle);

ClientBundleBase*
wpe_view_backend_exportable_fdo_get_client_bundle(struct wpe_view_backend_exportable_fdo* exportable);

}
