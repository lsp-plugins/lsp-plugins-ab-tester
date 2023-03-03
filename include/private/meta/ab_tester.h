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

#ifndef PRIVATE_META_AB_TESTER_H_
#define PRIVATE_META_AB_TESTER_H_

#include <lsp-plug.in/plug-fw/meta/types.h>
#include <lsp-plug.in/plug-fw/const.h>

namespace lsp
{
    //-------------------------------------------------------------------------
    // Plugin metadata
    namespace meta
    {
        typedef struct ab_tester
        {
            static constexpr size_t RATE_MIN            = 1;
            static constexpr size_t RATE_MAX            = 10;
            static constexpr size_t RATE_DFL            = 1;
            static constexpr size_t RATE_STEP           = 1;
        } ab_tester;

        // Plugin type metadata
        extern const plugin_t ab_tester_x2_mono;
        extern const plugin_t ab_tester_x4_mono;
        extern const plugin_t ab_tester_x8_mono;
        extern const plugin_t ab_tester_x2_stereo;
        extern const plugin_t ab_tester_x4_stereo;
        extern const plugin_t ab_tester_x8_stereo;

    } /* namespace meta */
} /* namespace lsp */

#endif /* PRIVATE_META_AB_TESTER_H_ */
