/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins-ab-tester
 * Created on: 25 нояб. 2020 г.
 *
 * lsp-plugins-ab-tester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-plugins-ab-tester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-plugins-ab-tester. If not, see <https://www.gnu.org/licenses/>.
 */

#include <lsp-plug.in/common/debug.h>
#include <lsp-plug.in/plug-fw/ui.h>
#include <lsp-plug.in/plug-fw/meta/func.h>
#include <lsp-plug.in/stdlib/stdio.h>
#include <lsp-plug.in/stdlib/string.h>

#include <private/plugins/ab_tester.h>
#include <private/ui/ab_tester.h>

namespace lsp
{
    namespace plugui
    {
        //---------------------------------------------------------------------
        // Plugin UI factory
        static const meta::plugin_t *plugin_uis[] =
        {
            &meta::ab_tester_x2_mono,
            &meta::ab_tester_x4_mono,
            &meta::ab_tester_x8_mono,
            &meta::ab_tester_x2_stereo,
            &meta::ab_tester_x4_stereo,
            &meta::ab_tester_x8_stereo
        };

        static ui::Module *ui_factory(const meta::plugin_t *meta)
        {
            return new ab_tester_ui(meta);
        }

        static ui::Factory factory(ui_factory, plugin_uis, 6);

        //---------------------------------------------------------------------
        static const char *KVT_SHUFFLE_INDICES = "/shuffle_indices";

        //---------------------------------------------------------------------
        // A/B tester UI
        ab_tester_ui::ab_tester_ui(const meta::plugin_t *meta):
            ui::Module(meta)
        {
            nInChannels     = 0;
            nOutChannels    = 0;

            for (const meta::port_t *port = meta->ports; ((port != NULL) && (port->id != NULL)); ++port)
            {
                if (meta::is_audio_in_port(port))
                    ++nInChannels;
                else if (meta::is_audio_out_port(port))
                    ++nOutChannels;
            }

            pSelector       = NULL;
            pReset          = NULL;
            pShuffle        = NULL;
            pBlindTest      = NULL;

            wBlindGrid      = NULL;
            wBlindVoid      = NULL;
            wBlindSelector  = NULL;
            wSelectAll      = NULL;
            wSelectNone     = NULL;
        }

        ab_tester_ui::~ab_tester_ui()
        {
        }

        void ab_tester_ui::destroy()
        {
            ui::Module::destroy();

            // Delete channels
            for (size_t i=0, n=vChannels.size(); i<n; ++i)
            {
                channel_t *c = vChannels.uget(i);
                if (c != NULL)
                    delete c;
            }
            vChannels.flush();
        }

        ab_tester_ui::channel_t *ab_tester_ui::create_channel(size_t channel_id)
        {
            channel_t *c = new channel_t;
            if (c == NULL)
                return NULL;

            LSPString id;
            tk::Registry *reg = pWrapper->controller()->widgets();
            c->nIndex           = channel_id + 1;
            c->nRandom          = 0;

            // Bind rating buttons
            for (size_t i=meta::ab_tester::RATE_MIN; i<=meta::ab_tester::RATE_MAX; i += meta::ab_tester::RATE_STEP)
            {
                // Find button widget for rating
                for (size_t j=0; j<2; ++j)
                {
                    id.fmt_ascii("%s_%d_%d",
                        (j == 0) ? "rating" : "bte_rating",
                        int(c->nIndex), int(i));
                    tk::Button *btn = reg->get<tk::Button>(&id);
                    if (btn != NULL)
                    {
                        c->vRating[j].vButtons.add(btn);
                        btn->slots()->bind(tk::SLOT_CHANGE, slot_rating_button_change, c);
                    }
                }
            }
            id.fmt_ascii("rate_%d", int(c->nIndex));
            c->pRating  = pWrapper->port(&id);
            if (c->pRating != NULL)
                c->pRating->bind(this);

            id.fmt_ascii("bte_%d", int(c->nIndex));
            c->pEnable = pWrapper->port(&id);

            id.fmt_ascii("channel_label_%d", int(c->nIndex));
            c->wName            = reg->get<tk::Edit>(&id);
            if (c->wName != NULL)
            {
                c->wName->text()->set("lists.ab_tester.instance");
                c->wName->text()->params()->set_int("id", int(c->nIndex));

                c->wName->slots()->bind(tk::SLOT_CHANGE, slot_channel_name_updated, c);
            }
            c->bNameChanged     = false;

            id.fmt_ascii("bte_label_%d", int(c->nIndex));
            c->wBlindLabel      = reg->get<tk::Label>(&id);
            id.fmt_ascii("bte_rating_%d", int(c->nIndex));
            c->wBlindRating     = reg->find(&id);
            id.fmt_ascii("bte_selector_%d", int(c->nIndex));
            c->wBlindSelector   = reg->find(&id);

            return c;
        }

        status_t ab_tester_ui::post_init()
        {
            status_t res = ui::Module::post_init();
            if (res != STATUS_OK)
                return res;

            // Create input channels
            size_t stereo_channels = nInChannels / nOutChannels;
            for (size_t i=0; i<stereo_channels; ++i)
            {
                channel_t *c = create_channel(i);
                if (c == NULL)
                    return STATUS_NO_MEM;
                if (!vChannels.add(c))
                {
                    delete c;
                    return STATUS_NO_MEM;
                }
            }

            tk::Registry *reg = pWrapper->controller()->widgets();

            // Bind events
            pSelector               = pWrapper->port("sel");

            pReset                  = pWrapper->port("rst");
            if (pReset != NULL)
                pReset->bind(this);

            pShuffle                = pWrapper->port("shuf");
            if (pShuffle != NULL)
                pShuffle->bind(this);

            pBlindTest              = pWrapper->port("bte");
            if (pBlindTest != NULL)
                pBlindTest->bind(this);

            wBlindGrid              = reg->get<tk::Grid>("bte_grid");
            wBlindVoid              = reg->find("bte_void");
            wBlindSelector          = reg->find("bte_selector_0");

            wSelectAll              = reg->get<tk::Button>("select_all");
            if (wSelectAll != NULL)
                wSelectAll->slots()->bind(tk::SLOT_CHANGE, slot_select_updated, this);
            wSelectNone             = reg->get<tk::Button>("select_none");
            if (wSelectNone != NULL)
                wSelectNone->slots()->bind(tk::SLOT_CHANGE, slot_select_updated, this);

            return STATUS_OK;
        }

        void ab_tester_ui::notify(ui::IPort *port)
        {
            if (port == pBlindTest)
            {
                if (pBlindTest->value() >= 0.5f)
                    blind_test_enable();
            }

            if (port == pReset)
            {
                if (pReset->value() >= 0.5f)
                    reset_ratings();
            }

            if (port == pShuffle)
            {
                if (pShuffle->value() >= 0.5f)
                    shuffle_data();
            }

            for (size_t i=0, n=vChannels.size(); i<n; ++i)
            {
                channel_t *c = vChannels.uget(i);
                if (c == NULL)
                    continue;

                if (c->pRating == port)
                    update_rating(c);
            }
        }

        void ab_tester_ui::update_rating(channel_t *ch)
        {
            if (ch->pRating == NULL)
                return;

            size_t value = ch->pRating->value();

            for (size_t j=0; j<2; ++j)
            {
                rating_t *r = &ch->vRating[j];
                size_t max  = meta::ab_tester::RATE_MIN;

                for (size_t i=0, n=r->vButtons.size(); i<n; ++i, max += meta::ab_tester::RATE_STEP)
                {
                    tk::Button *btn = r->vButtons.uget(i);
                    if (btn == NULL)
                        continue;

                    btn->down()->set(max <= value);
                }
            }
        }

        status_t ab_tester_ui::slot_rating_button_change(tk::Widget *sender, void *ptr, void *data)
        {
            tk::Button *btn = tk::widget_cast<tk::Button>(sender);
            if (btn == NULL)
                return STATUS_OK;

            channel_t *c = static_cast<channel_t *>(ptr);
            if (c->pRating == NULL)
                return STATUS_OK;

            // Update port value
            for (size_t j=0; j<2; ++j)
            {
                rating_t *r = &c->vRating[j];
                size_t max  = meta::ab_tester::RATE_MIN;

                for (size_t i=0, n=r->vButtons.size(); i<n; ++i, max +=  meta::ab_tester::RATE_STEP)
                {
                    tk::Button *rate_btn = r->vButtons.uget(i);
                    if (btn == rate_btn)
                    {
                        c->pRating->set_value(max);
                        c->pRating->notify_all();
                        break;
                    }
                }
            }

            return STATUS_OK;
        }

        status_t ab_tester_ui::slot_channel_name_updated(tk::Widget *sender, void *ptr, void *data)
        {
            channel_t *c    = static_cast<channel_t *>(ptr);
            c->bNameChanged = true;

            return STATUS_OK;
        }

        void ab_tester_ui::set_channel_name(core::KVTStorage *kvt, int id, const char *name)
        {
            char kvt_name[0x80];
            core::kvt_param_t kparam;

            // Submit new value to KVT
            snprintf(kvt_name, sizeof(kvt_name), "/channel/%d/name", id);
            kparam.type     = core::KVT_STRING;
            kparam.str      = name;
            lsp_trace("%s = %s", kvt_name, kparam.str);
            kvt->put(kvt_name, &kparam, core::KVT_RX);
            wrapper()->kvt_notify_write(kvt, kvt_name, &kparam);
        }

        void ab_tester_ui::idle()
        {
            // Scan the list of instrument names for changes
            size_t changes = 0;
            for (size_t i=0, n=vChannels.size(); i<n; ++i)
            {
                channel_t *c = vChannels.uget(i);
                if ((c->wName != NULL) && (c->bNameChanged))
                    ++changes;
            }

            // Apply instrument names to KVT
            if (changes > 0)
            {
                core::KVTStorage *kvt = wrapper()->kvt_lock();
                if (kvt != NULL)
                {
                    sync_channel_names(kvt);
                    wrapper()->kvt_release();
                }
            }
        }

        void ab_tester_ui::kvt_changed(core::KVTStorage *kvt, const char *id, const core::kvt_param_t *value)
        {
            if ((value->type == core::KVT_STRING) && (::strstr(id, "/channel/") == id))
            {
                id += ::strlen("/channel/");

                char *endptr = NULL;
                errno = 0;
                long index = ::strtol(id, &endptr, 10);

                // Valid object number?
                if ((errno == 0) && (!::strcmp(endptr, "/name")) && (index > 0))
                {
                    for (size_t i=0, n=vChannels.size(); i<n; ++i)
                    {
                        channel_t *c = vChannels.uget(i);
                        if ((c->wName == NULL) || (c->nIndex != size_t(index)))
                            continue;

                        c->wName->text()->set_raw(value->str);
                        c->bNameChanged = false;
                    }
                }
            }
            else if ((value->type == core::KVT_UINT32) && (strcmp(id, KVT_SHUFFLE_INDICES) == 0))
            {
                // Update shuffle state
                vShuffled.clear();
                uint32_t shuffle_data = value->u32;
                for (size_t i=0; i<8; ++i)
                {
                    size_t idx      = (shuffle_data >> (4 * i)) & 0xf;
                    if (!(idx & 0x8))
                        continue;
                    idx            &= 0x7;

                    // Commit value to shuffled array
                    channel_t *c    = vChannels.get(idx);
                    if (c == NULL)
                        continue;
                    if (vShuffled.contains(c))
                        continue;
                    vShuffled.add(c);
                }

                // Upate grid
                update_blind_grid();
            }
        }

        status_t ab_tester_ui::reset_settings()
        {
            core::KVTStorage *kvt = wrapper()->kvt_lock();
            if (kvt != NULL)
            {
                // Reset all names for all instruments
                for (size_t i=0, n=vChannels.size(); i<n; ++i)
                {
                    channel_t *c = vChannels.uget(i);
                    if (c->wName == NULL)
                        continue;

                    c->wName->text()->set("lists.ab_tester.instance");
                    c->wName->text()->params()->set_int("id", int(c->nIndex));
                    c->bNameChanged  = true;
                }

                sync_channel_names(kvt);
                wrapper()->kvt_release();
            }

            return ui::Module::reset_settings();
        }

        void ab_tester_ui::sync_channel_names(core::KVTStorage *kvt)
        {
            LSPString value;

            for (size_t i=0, n=vChannels.size(); i<n; ++i)
            {
                channel_t *c = vChannels.uget(i);
                if ((c->wName == NULL) || (!c->bNameChanged))
                    continue;

                // Obtain the new instrument name
                if (c->wName->text()->format(&value) != STATUS_OK)
                    continue;

                // Submit new value to KVT
                set_channel_name(kvt, c->nIndex, value.get_utf8());
            }
        }

        void ab_tester_ui::reset_ratings()
        {
            for (size_t i=0, n=vChannels.size(); i<n; ++i)
            {
                channel_t *c = vChannels.uget(i);
                if ((c == NULL) || (c->pRating == NULL))
                    continue;

                c->pRating->set_default();
                c->pRating->notify_all();
            }
        }

        ssize_t ab_tester_ui::cmp_channels(const channel_t *a, const channel_t *b)
        {
            if (a->nRandom == b->nRandom)
                return 0;

            return (a->nRandom < b->nRandom) ? -1 : 1;
        }

        void ab_tester_ui::shuffle_data()
        {
            // Re-shuffle channels
            for (size_t i=0, n=vShuffled.size(); i<n; ++i)
            {
                channel_t *c    = vShuffled.uget(i);
                if (c == NULL)
                    continue;

                c->nRandom      = rand();
            }
            vShuffled.qsort(cmp_channels);

            // Clear blind test selector
            if (pSelector != NULL)
            {
                pSelector->set_value(0);
                pSelector->notify_all();
            }

            // Store shuffle state
            uint32_t shuffle_data = 0;
            for (size_t i=0, n=vShuffled.size(); i<n; ++i)
            {
                channel_t *c    = vShuffled.uget(i);
                if (c == NULL)
                    continue;

                shuffle_data   |= (((c->nIndex-1) & 0x7) | 0x8) << (4 * i);
            }

            core::KVTStorage *kvt = pWrapper->kvt_lock();
            if (kvt != NULL)
            {
                lsp_finally { pWrapper->kvt_release(); };

                core::kvt_param_t kparam;
                kparam.type     = core::KVT_UINT32;
                kparam.u32      = shuffle_data;

                lsp_trace("%s = 0x%x", KVT_SHUFFLE_INDICES, int(kparam.u32));
                kvt->put(KVT_SHUFFLE_INDICES, &kparam, core::KVT_RX);
                wrapper()->kvt_notify_write(kvt, KVT_SHUFFLE_INDICES, &kparam);
            }
            else
                update_blind_grid();
        }

        void ab_tester_ui::update_blind_grid()
        {
            // Update grid
            if (wBlindGrid == NULL)
                return;

            wBlindGrid->remove_all();
            wBlindGrid->add(wBlindVoid, 1, 2);
            wBlindGrid->add(wBlindSelector);

            for (size_t i=0, n=vShuffled.size(); i<n; ++i)
            {
                channel_t *c    = vShuffled.uget(i);
                if (c == NULL)
                    continue;

                c->wBlindLabel->text()->params()->set_int("id", i + 1);
                wBlindGrid->add(c->wBlindLabel);
                wBlindGrid->add(c->wBlindRating);
                wBlindGrid->add(c->wBlindSelector);
            }
        }

        void ab_tester_ui::blind_test_enable()
        {
            // Form list of ports for shuffling
            vShuffled.clear();
            for (size_t i=0, n=vChannels.size(); i<n; ++i)
            {
                channel_t *c = vChannels.uget(i);
                if (c == NULL)
                    continue;
                bool enabled = (c->pEnable != NULL) ? c->pEnable->value() >= 0.5f : true;
                if (enabled)
                {
                    if (!vShuffled.add(c))
                        return;
                }
            }
            if (vShuffled.size() < 2)
            {
                pBlindTest->set_value(0.0f);
                pBlindTest->notify_all();
                return;
            }

            reset_ratings();
            shuffle_data();
        }

        status_t ab_tester_ui::slot_select_updated(tk::Widget *sender, void *ptr, void *data)
        {
            tk::Button *btn = tk::widget_cast<tk::Button>(sender);
            if (btn == NULL)
                return STATUS_OK;

            ab_tester_ui *this_ = static_cast<ab_tester_ui *>(ptr);
            if (ptr == NULL)
                return STATUS_OK;

            this_->select_updated(btn);

            return STATUS_OK;
        }

        void ab_tester_ui::select_updated(tk::Button *btn)
        {
            // Handle only button down case
            if (!btn->down()->get())
                return;

            float select = (btn == wSelectAll) ? 1.0f : 0.0f;

            for (size_t i=0, n=vChannels.size(); i<n; ++i)
            {
                channel_t *c = vChannels.uget(i);
                if (c->pEnable == NULL)
                    continue;

                c->pEnable->set_value(select);
                c->pEnable->notify_all();
            }
        }

    } /* namespace plugui */
} /* namespace lsp */


