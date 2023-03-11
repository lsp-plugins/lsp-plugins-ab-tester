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
                } rating_t;

                typedef struct channel_t
                {
                    rating_t                    vRating[2];     // Rating indicator, blnd rating indicator
                    size_t                      nIndex;         // Absolute index of the channel
                    int                         nRandom;        // Random number for shuffling

                    tk::Edit                   *wName;          // Edit that holds channel name
                    tk::Label                  *wBlindLabel;    // Blind label marker
                    tk::Widget                 *wBlindRating;   // Blind rating container
                    tk::Widget                 *wBlindSelector; // Blind selector

                    bool                        bNameChanged;   // Indicator that channel name has changed

                    ui::IPort                  *pEnable;        // Enable blind test
                    ui::IPort                  *pRating;        // Rating port
                } channel_t;

            protected:
                size_t                      nInChannels;
                size_t                      nOutChannels;

                ui::IPort                  *pSelector;          // Blind channel selector
                ui::IPort                  *pReset;             // Reset port
                ui::IPort                  *pShuffle;           // Shuffle port
                ui::IPort                  *pBlindTest;         // Blind test

                tk::Grid                   *wBlindGrid;         // Grid with blind test widgets
                tk::Widget                 *wBlindVoid;         // Blind void padding widget
                tk::Widget                 *wBlindSelector;     // Blind selector widget
                tk::Button                 *wSelectAll;         // Select all channels button
                tk::Button                 *wSelectNone;        // Select none channels button

                lltl::parray<channel_t>     vChannels;          // List of channels
                lltl::parray<channel_t>     vShuffled;          // Shuffled channels

            protected:
                static ssize_t      cmp_channels(const channel_t *a, const channel_t *b);

            protected:
                channel_t          *create_channel(size_t channel_id);
                void                set_channel_name(core::KVTStorage *kvt, int id, const char *name);
                void                sync_channel_names(core::KVTStorage *kvt);

                void                update_rating(channel_t *ch);
                void                reset_ratings();
                void                blind_test_enable();
                void                shuffle_data();
                void                update_blind_grid();
                void                select_updated(tk::Button *btn);

            protected:
                static status_t     slot_rating_button_change(tk::Widget *sender, void *ptr, void *data);
                static status_t     slot_channel_name_updated(tk::Widget *sender, void *ptr, void *data);
                static status_t     slot_select_updated(tk::Widget *sender, void *ptr, void *data);

            public:
                explicit ab_tester_ui(const meta::plugin_t *meta);
                virtual ~ab_tester_ui() override;

                virtual void        destroy() override;

            public:
                virtual status_t    post_init() override;

                virtual void        notify(ui::IPort *port) override;

                virtual void        idle() override;

                virtual void        kvt_changed(core::KVTStorage *kvt, const char *id, const core::kvt_param_t *value) override;

                virtual status_t    reset_settings() override;
        };
    } /* namespace plugui */
} /* namespace lsp */


#endif /* PRIVATE_UI_AB_TESTER_H_ */
