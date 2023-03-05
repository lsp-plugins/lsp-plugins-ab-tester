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

            // Bind rating buttons
            for (size_t i=meta::ab_tester::RATE_MIN; i<=meta::ab_tester::RATE_MAX; i += meta::ab_tester::RATE_STEP)
            {
                // Find button widget
                id.fmt_ascii("rating_%d_%d", int(c->nIndex), int(i));
                tk::Button *btn = reg->get<tk::Button>(&id);
                if (btn != NULL)
                {
                    c->sRating.vButtons.add(btn);
                    btn->slots()->bind(tk::SLOT_CHANGE, slot_rating_button_change, &c->sRating);
                }
            }
            id.fmt_ascii("rate_%d", int(c->nIndex));
            c->sRating.pPort    = pWrapper->port(&id);
            if (c->sRating.pPort != NULL)
                c->sRating.pPort->bind(this);

            id.fmt_ascii("channel_label_%d", int(c->nIndex));
            c->wName            = reg->get<tk::Edit>(&id);
            if (c->wName != NULL)
            {
                c->wName->text()->set("lists.ab_tester.instance");
                c->wName->text()->params()->set_int("id", int(c->nIndex));

                c->wName->slots()->bind(tk::SLOT_CHANGE, slot_channel_name_updated, c);
            }
            c->bNameChanged     = false;

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

            // TODO


            return STATUS_OK;
        }

        void ab_tester_ui::notify(ui::IPort *port)
        {
            for (size_t i=0, n=vChannels.size(); i<n; ++i)
            {
                channel_t *c = vChannels.uget(i);
                if (c == NULL)
                    continue;

                if (c->sRating.pPort == port)
                    update_rating(&c->sRating);
            }
        }

        void ab_tester_ui::update_rating(rating_t *rate)
        {
            if (rate->pPort == NULL)
                return;

            size_t value = rate->pPort->value();
            size_t max = meta::ab_tester::RATE_MIN;
            for (size_t i=0, n=rate->vButtons.size(); i<n; ++i, max +=  meta::ab_tester::RATE_STEP)
            {
                tk::Button *btn = rate->vButtons.uget(i);
                if (btn == NULL)
                    continue;

                btn->down()->set(max <= value);
            }
        }

        status_t ab_tester_ui::slot_rating_button_change(tk::Widget *sender, void *ptr, void *data)
        {
            tk::Button *btn = tk::widget_cast<tk::Button>(sender);
            if (btn == NULL)
                return STATUS_OK;

            rating_t *rate = static_cast<rating_t *>(ptr);
            if (rate->pPort == NULL)
                return STATUS_OK;

            // Update port value
            size_t max = meta::ab_tester::RATE_MIN;
            for (size_t i=0, n=rate->vButtons.size(); i<n; ++i, max +=  meta::ab_tester::RATE_STEP)
            {
                tk::Button *rate_btn = rate->vButtons.uget(i);
                if (btn == rate_btn)
                {
                    rate->pPort->set_value(max);
                    rate->pPort->notify_all();
                    break;
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

    } /* namespace plugui */
} /* namespace lsp */


