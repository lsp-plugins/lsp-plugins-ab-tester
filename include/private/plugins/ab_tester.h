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

#ifndef PRIVATE_PLUGINS_AB_TESTER_H_
#define PRIVATE_PLUGINS_AB_TESTER_H_

#include <lsp-plug.in/dsp-units/ctl/Bypass.h>
#include <lsp-plug.in/plug-fw/plug.h>
#include <private/meta/ab_tester.h>

namespace lsp
{
    namespace plugins
    {
        /**
         * A/B test plugin with Blind test option
         */
        class ab_tester: public plug::Module
        {
            private:
                ab_tester & operator = (const ab_tester &);
                ab_tester (const ab_tester &);

            protected:
                typedef struct in_channel_t
                {
                    dspu::Bypass        sBypass;    // Bypass
                    float              *vIn;        // Input data
                    float              *vRet;       // Return data
                    float               fOldGain;   // Old gain value
                    float               fGain;      // Input gain

                    plug::IPort        *pIn;        // Input data
                    plug::IPort        *pRet;       // Return data
                    plug::IPort        *pGain;      // Input gain
                    plug::IPort        *pInMeter;   // Input level meter
                } in_channel_t;

                typedef struct out_channel_t
                {
                    float              *vOut;       // Output data
                    plug::IPort        *pOut;       // Output data port
                } out_channel_t;

            protected:
                in_channel_t       *vInChannels;    // Input channels
                out_channel_t      *vOutChannels;   // Output channels
                size_t              nInChannels;    // Number of input channels
                size_t              nOutChannels;   // Number of output channels
                float              *vTmp;           // Temporary buffer
                bool                bBlindTest;     // Blind test mode
                bool                bMono;          // Mono listen mode
                size_t              nSelector;      // Selector

                plug::IPort        *pChannelSel;    // Channel selector
                plug::IPort        *pBlindTest;     // Blind test switch
                plug::IPort        *pMono;          // Mono switch

                uint8_t            *pData;          // All allocated data

            protected:
                void                do_destroy();

            public:
                explicit ab_tester(const meta::plugin_t *meta);
                virtual ~ab_tester();

                virtual void        init(plug::IWrapper *wrapper, plug::IPort **ports);
                void                destroy();

            public:
                virtual void        update_sample_rate(long sr);
                virtual void        update_settings();
                virtual void        process(size_t samples);
                virtual void        dump(dspu::IStateDumper *v) const;
        };

    } /* namespace plugins */
} /* namespace lsp */


#endif /* PRIVATE_PLUGINS_AB_TESTER_H_ */

