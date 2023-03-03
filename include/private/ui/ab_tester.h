/*
 * Copyright (C) 2023 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2023 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins-ab-tester
 * Created on: 3 мар. 2023 г.
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

#ifndef PRIVATE_UI_AB_TESTER_H_
#define PRIVATE_UI_AB_TESTER_H_

#include <lsp-plug.in/plug-fw/ui.h>
#include <lsp-plug.in/lltl/parray.h>

namespace lsp
{
    namespace plugui
    {
        /**
         * A/B Tester plugin series with Blind option
         */
        class ab_tester_ui: public ui::Module, public ui::IPortListener
        {
            protected:
                typedef struct rating_t
                {
                    lltl::parray<tk::Button>    vButtons;
                    ui::IPort                  *pPort;
                } rating_t;

                typedef struct channel_t
                {
                    rating_t                    sRating;
                } channel_t;

            protected:
                size_t                      nInChannels;
                size_t                      nOutChannels;
                lltl::parray<channel_t>     vChannels;

            protected:
                channel_t          *create_channel(size_t channel_id);

                void                update_rating(rating_t *rate);

            protected:
                static status_t     slot_rating_button_change(tk::Widget *sender, void *ptr, void *data);

            public:
                explicit ab_tester_ui(const meta::plugin_t *meta);
                virtual ~ab_tester_ui() override;

                virtual void        destroy() override;

            public:
                virtual status_t    post_init() override;

                virtual void        notify(ui::IPort *port) override;
        };
    } /* namespace plugui */
} /* namespace lsp */


#endif /* PRIVATE_UI_AB_TESTER_H_ */
