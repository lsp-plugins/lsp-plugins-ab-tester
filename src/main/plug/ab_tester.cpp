/*
 * Copyright (C) 2023 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2023 Vladimir Sadovnikov <sadko4u@gmail.com>
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

#include <lsp-plug.in/common/alloc.h>
#include <lsp-plug.in/common/debug.h>
#include <lsp-plug.in/dsp/dsp.h>
#include <lsp-plug.in/dsp-units/units.h>
#include <lsp-plug.in/plug-fw/meta/func.h>

#include <private/plugins/ab_tester.h>

/* The size of temporary buffer for audio processing */
#define BUFFER_SIZE         0x400U

namespace lsp
{
    static plug::IPort *TRACE_PORT(plug::IPort *p)
    {
        lsp_trace("  port id=%s", (p)->metadata()->id);
        return p;
    }

    namespace plugins
    {
        //---------------------------------------------------------------------
        // Plugin factory
        static const meta::plugin_t *plugins[] =
        {
            &meta::ab_tester_x2_mono,
            &meta::ab_tester_x4_mono,
            &meta::ab_tester_x8_mono,
            &meta::ab_tester_x2_stereo,
            &meta::ab_tester_x4_stereo,
            &meta::ab_tester_x8_stereo
        };

        static plug::Module *plugin_factory(const meta::plugin_t *meta)
        {
            return new ab_tester(meta);
        }

        static plug::Factory factory(plugin_factory, plugins, 6);

        //---------------------------------------------------------------------
        // Implementation
        ab_tester::ab_tester(const meta::plugin_t *meta):
            Module(meta)
        {
            vInChannels     = NULL;
            vOutChannels    = NULL;
            nInChannels     = 0;
            nOutChannels    = 0;
            vTmp            = NULL;

            bBlindTest      = false;
            bMono           = false;
            nSelector       = 0;

            pBlindTest      = NULL;
            pMono           = NULL;
            pChannelSel     = NULL;

            pData           = NULL;

            for (const meta::port_t *port = meta->ports; ((port != NULL) && (port->id != NULL)); ++port)
            {
                if (meta::is_audio_in_port(port))
                    ++nInChannels;
                else if (meta::is_audio_out_port(port))
                    ++nOutChannels;
            }
        }

        ab_tester::~ab_tester()
        {
            do_destroy();
        }

        void ab_tester::init(plug::IWrapper *wrapper, plug::IPort **ports)
        {
            // Call parent class for initialization
            Module::init(wrapper, ports);

            // Estimate allocation size
            size_t szof_in_channel      = align_size(sizeof(in_channel_t) * nInChannels, DEFAULT_ALIGN);
            size_t szof_out_channel     = align_size(sizeof(out_channel_t) * nOutChannels, DEFAULT_ALIGN);
            size_t szof_buffers         = BUFFER_SIZE * sizeof(float);
            size_t alloc                = szof_in_channel + szof_out_channel + szof_buffers;

            // Allocate data
            uint8_t *ptr                = alloc_aligned<uint8_t>(pData, alloc, DEFAULT_ALIGN);
            if (ptr == NULL)
                return;

            // Input channels
            vInChannels                 = reinterpret_cast<in_channel_t *>(ptr);
            ptr                        += szof_in_channel;

            // Output channels
            vOutChannels                = reinterpret_cast<out_channel_t *>(ptr);
            ptr                        += szof_out_channel;

            // Temporary buffer
            vTmp                        = reinterpret_cast<float *>(ptr);
            ptr                        += szof_buffers;

            // Initialize input channels
            for (size_t i=0; i<nInChannels; ++i)
            {
                in_channel_t *c     = &vInChannels[i];

                c->sBypass.construct();
                c->vIn              = NULL;

                c->fOldGain         = GAIN_AMP_0_DB;
                c->fGain            = GAIN_AMP_0_DB;

                c->pIn              = NULL;
                c->pGain            = NULL;
                c->pInMeter         = NULL;
            }

            // Initialize output channels
            for (size_t i=0; i<nOutChannels; ++i)
            {
                out_channel_t *c    = &vOutChannels[i];

                c->vOut             = NULL;
                c->pOut             = NULL;
            }

            // Bind ports
            lsp_trace("Binding ports");
            size_t port_id      = 0;

            // Output ports
            for (size_t i=0; i<nOutChannels; ++i)
                vOutChannels[i].pOut        = TRACE_PORT(ports[port_id++]);

            // Bind global ports
            TRACE_PORT(ports[port_id++]); // Reset rating
            pBlindTest          = TRACE_PORT(ports[port_id++]); // Blind test enable
            TRACE_PORT(ports[port_id++]); // Re-shuffle
            pChannelSel         = TRACE_PORT(ports[port_id++]); // Channel selector
            if (nOutChannels > 1)
                pMono               = TRACE_PORT(ports[port_id++]);

            // Input ports
            size_t num_inputs   = nInChannels / nOutChannels;
            for (size_t i=0; i<nInChannels; i += nOutChannels)
            {
                if (nOutChannels == 1)
                {
                    in_channel_t *c     = &vInChannels[i];
                    c->pIn              = TRACE_PORT(ports[port_id++]);
                    c->pGain            = TRACE_PORT(ports[port_id++]);
                    c->pInMeter         = TRACE_PORT(ports[port_id++]);
                }
                else
                {
                    in_channel_t *l     = &vInChannels[i];
                    in_channel_t *r     = &vInChannels[i+1];
                    l->pIn              = TRACE_PORT(ports[port_id++]);
                    r->pIn              = TRACE_PORT(ports[port_id++]);
                    l->pGain            = TRACE_PORT(ports[port_id++]);
                    r->pGain            = l->pGain;
                    l->pInMeter         = TRACE_PORT(ports[port_id++]);
                    r->pInMeter         = TRACE_PORT(ports[port_id++]);
                }

                // Skip blind test input switch
                if (num_inputs > 2)
                    TRACE_PORT(ports[port_id++]);
                // Skip rate value
                TRACE_PORT(ports[port_id++]);
            }
        }

        void ab_tester::destroy()
        {
            Module::destroy();
            do_destroy();
        }

        void ab_tester::do_destroy()
        {
            if (pData != NULL)
            {
                free_aligned(pData);
                pData       = NULL;
            }
        }

        void ab_tester::update_sample_rate(long sr)
        {
            for (size_t i=0; i<nInChannels; ++i)
            {
                in_channel_t *c     = &vInChannels[i];
                c->sBypass.init(sr);
            }
        }

        void ab_tester::update_settings()
        {
            bBlindTest      = pBlindTest->value() >= 0.5f;
            bMono           = (pMono != NULL) ? pMono->value() >= 0.5f : false;
            nSelector       = lsp_max(0.0f, pChannelSel->value());

            lsp_trace("selector = %d", int(nSelector));

            for (size_t i=0; i<nInChannels; ++i)
            {
                in_channel_t *c     = &vInChannels[i];
                c->fOldGain         = c->fGain;
                c->fGain            = c->pGain->value();
                size_t chan_id      = (i / nOutChannels) + 1;

                c->sBypass.set_bypass(nSelector != chan_id);
            }
        }

        void ab_tester::process(size_t samples)
        {
            // Bind input and output buffers
            for (size_t i=0; i<nInChannels; ++i)
            {
                in_channel_t *c     = &vInChannels[i];
                c->vIn              = c->pIn->buffer<float>();
            }
            for (size_t i=0; i<nOutChannels; ++i)
            {
                out_channel_t *c    = &vOutChannels[i];
                c->vOut             = c->pOut->buffer<float>();
                dsp::fill_zero(c->vOut, samples);
            }

            // Main processing loop
            for (size_t offset=0; offset<samples; )
            {
                size_t block        = lsp_min(samples - offset, BUFFER_SIZE);

                // Process input channels
                for (size_t i=0; i<nInChannels; ++i)
                {
                    in_channel_t *in     = &vInChannels[i];
                    out_channel_t *out   = &vOutChannels[i % nOutChannels];

                    dsp::lramp2(vTmp, in->vIn, in->fOldGain, in->fGain, block);
                    in->fOldGain        = in->fGain;
                    float level         = (bBlindTest) ? 0.0f : dsp::abs_max(vTmp, block);
                    in->sBypass.process(vTmp, NULL, vTmp, block);
                    in->pInMeter->set_value(level);

                    // Add input channel to output
                    dsp::add2(out->vOut, vTmp, block);
                }

                // Mono switch
                if ((nOutChannels > 1) && (bMono))
                {
                    float *l        = vOutChannels[0].vOut;
                    float *r        = vOutChannels[1].vOut;
                    dsp::lr_to_mid(l, l, r, block);
                    dsp::copy(r, l, block);
                }

                // Update pointers
                offset             += block;
                for (size_t i=0; i<nInChannels; ++i)
                    vInChannels[i].vIn     += block;
                for (size_t i=0; i<nOutChannels; ++i)
                    vOutChannels[i].vOut   += block;
            }
        }

        void ab_tester::dump(dspu::IStateDumper *v) const
        {
            v->begin_array("vInChannels", vInChannels, nInChannels);
            for (size_t i=0; i<nInChannels; ++i)
            {
                in_channel_t *in     = &vInChannels[i];

                v->begin_object(in, sizeof(in_channel_t));
                {
                    v->write_object(&in->sBypass);
                    v->write("vIn", in->vIn);
                    v->write("fOldGain", in->fOldGain);
                    v->write("fGain", in->fGain);
                    v->write("pIn", in->pIn);
                    v->write("pGain", in->pGain);
                    v->write("pInMeter", in->pInMeter);
                }
                v->end_object();
            }
            v->end_array();

            v->begin_array("vOutChannels", vOutChannels, nOutChannels);
            for (size_t i=0; i<nOutChannels; ++i)
            {
                out_channel_t *out  = &vOutChannels[i];

                v->begin_object(out, sizeof(out_channel_t));
                {
                    v->write("vOut", out->vOut);
                    v->write("pOut", out->pOut);
                }
                v->end_object();
            }
            v->end_array();

            v->write("nInChannels", nInChannels);
            v->write("nOutChannels", nOutChannels);
            v->write("vTmp", vTmp);
            v->write("bBlindTest", bBlindTest);
            v->write("bMono", bMono);
            v->write("nSelector", nSelector);
            v->write("pChannelSel", pChannelSel);
            v->write("pBlindTest", pBlindTest);
            v->write("bMono", bMono);
            v->write("nSelector", nSelector);
            v->write("pChannelSel", pChannelSel);
            v->write("pBlindTest", pBlindTest);
            v->write("pMono", pMono);
            v->write("pData", pData);
        }

    } /* namespace plugins */
} /* namespace lsp */


