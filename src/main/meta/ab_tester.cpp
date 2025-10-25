/*
 * Copyright (C) 2025 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2025 Vladimir Sadovnikov <sadko4u@gmail.com>
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

#include <lsp-plug.in/plug-fw/meta/ports.h>
#include <lsp-plug.in/shared/meta/developers.h>
#include <private/meta/ab_tester.h>

#define LSP_PLUGINS_AB_TESTER_VERSION_MAJOR       1
#define LSP_PLUGINS_AB_TESTER_VERSION_MINOR       0
#define LSP_PLUGINS_AB_TESTER_VERSION_MICRO       19

#define LSP_PLUGINS_AB_TESTER_VERSION  \
    LSP_MODULE_VERSION( \
        LSP_PLUGINS_AB_TESTER_VERSION_MAJOR, \
        LSP_PLUGINS_AB_TESTER_VERSION_MINOR, \
        LSP_PLUGINS_AB_TESTER_VERSION_MICRO  \
    )

namespace lsp
{
    namespace meta
    {
        //-------------------------------------------------------------------------
        // Plugin metadata

        #define BLIND_SWITCH(id, label, alias, enable) \
            SWITCH("bte" id, "Blind test enable " label, "Test on" alias, enable), \

        #define NO_BLIND_SWITCH(id, label, alias, enable)

        #define ABTEST_MONO_CHANNEL(id, label, alias, blind_switch, bte) \
            AUDIO_INPUT("in" id, "Audio input " label), \
            OPT_RETURN_MONO("ret" id, "rin" id, "Audio return " label), \
            AMP_GAIN100("g" id, "Input gain " label, "In gain" alias, 1.0), \
            METER_GAIN("ism" id, "Input signal meter " label, GAIN_AMP_P_48_DB), \
            blind_switch(id, label, alias, bte) \
            INT_CONTROL("rate" id, "Channel blind test rate " label, "Rate" alias, U_NONE, meta::ab_tester::RATE)

        #define ABTEST_STEREO_CHANNEL(id, label, alias, blind_switch, bte) \
            AUDIO_INPUT("in" id "l", "Audio input " label " Left"), \
            AUDIO_INPUT("in" id "r", "Audio input " label " Right"), \
            OPT_RETURN_STEREO("ret" id, "rin" id, "Audio return " label), \
            AMP_GAIN100("g" id, "Input gain " label, "In gain" alias, 1.0), \
            METER_GAIN("ism" id "l", "Input signal meter " label " Left", GAIN_AMP_P_48_DB), \
            METER_GAIN("ism" id "r", "Input signal meter " label " Right", GAIN_AMP_P_48_DB), \
            blind_switch(id, label, alias, bte) \
            INT_CONTROL("rate" id, "Channel blind test rate " label, "Rate" alias, U_NONE, meta::ab_tester::RATE) \

        #define ABTEST_GLOBAL(max_sel) \
            TRIGGER("rst", "Reset channel rating", "Reset"), \
            SWITCH("bte", "Blind test enable", "Blind test", 0.0), \
            TRIGGER("shuf", "Re-shuffle channels", "Shuffle"), \
            INT_CONTROL_ALL("sel", "Channel selector", "Channel selector", U_NONE, 0, max_sel, 0, 1)

        #define ABTEST_MONO_SWITCH \
            SWITCH("mono", "Mono switch", "Mono", 0.0f)

        static const port_t ab_tester_x2_mono_ports[] =
        {
            AUDIO_OUTPUT_MONO,
            ABTEST_GLOBAL(3),
            ABTEST_MONO_CHANNEL("_1", "1", " 1", NO_BLIND_SWITCH, 1.0),
            ABTEST_MONO_CHANNEL("_2", "2", " 2", NO_BLIND_SWITCH, 1.0),
            PORTS_END
        };

        static const port_t ab_tester_x4_mono_ports[] =
        {
            AUDIO_OUTPUT_MONO,
            ABTEST_GLOBAL(5),
            ABTEST_MONO_CHANNEL("_1", "1", " 1", BLIND_SWITCH, 1.0),
            ABTEST_MONO_CHANNEL("_2", "2", " 2", BLIND_SWITCH, 1.0),
            ABTEST_MONO_CHANNEL("_3", "3", " 3", BLIND_SWITCH, 0.0),
            ABTEST_MONO_CHANNEL("_4", "4", " 4", BLIND_SWITCH, 0.0),
            PORTS_END
        };

        static const port_t ab_tester_x8_mono_ports[] =
        {
            AUDIO_OUTPUT_MONO,
            ABTEST_GLOBAL(9),
            ABTEST_MONO_CHANNEL("_1", "1", " 1", BLIND_SWITCH, 1.0),
            ABTEST_MONO_CHANNEL("_2", "2", " 2", BLIND_SWITCH, 1.0),
            ABTEST_MONO_CHANNEL("_3", "3", " 3", BLIND_SWITCH, 0.0),
            ABTEST_MONO_CHANNEL("_4", "4", " 4", BLIND_SWITCH, 0.0),
            ABTEST_MONO_CHANNEL("_5", "5", " 5", BLIND_SWITCH, 0.0),
            ABTEST_MONO_CHANNEL("_6", "6", " 6", BLIND_SWITCH, 0.0),
            ABTEST_MONO_CHANNEL("_7", "7", " 7", BLIND_SWITCH, 0.0),
            ABTEST_MONO_CHANNEL("_8", "8", " 8", BLIND_SWITCH, 0.0),
            PORTS_END
        };

        static const port_t ab_tester_x2_stereo_ports[] =
        {
            AUDIO_OUTPUT_STEREO,
            ABTEST_GLOBAL(3),
            ABTEST_MONO_SWITCH,
            ABTEST_STEREO_CHANNEL("_1", "1", " 1", NO_BLIND_SWITCH, 1.0),
            ABTEST_STEREO_CHANNEL("_2", "2", " 2", NO_BLIND_SWITCH, 1.0),
            PORTS_END
        };

        static const port_t ab_tester_x4_stereo_ports[] =
        {
            AUDIO_OUTPUT_STEREO,
            ABTEST_GLOBAL(5),
            ABTEST_MONO_SWITCH,
            ABTEST_STEREO_CHANNEL("_1", "1", " 1", BLIND_SWITCH, 1.0),
            ABTEST_STEREO_CHANNEL("_2", "2", " 2", BLIND_SWITCH, 1.0),
            ABTEST_STEREO_CHANNEL("_3", "3", " 3", BLIND_SWITCH, 0.0),
            ABTEST_STEREO_CHANNEL("_4", "4", " 4", BLIND_SWITCH, 0.0),
            PORTS_END
        };

        static const port_t ab_tester_x8_stereo_ports[] =
        {
            AUDIO_OUTPUT_STEREO,
            ABTEST_GLOBAL(9),
            ABTEST_MONO_SWITCH,
            ABTEST_STEREO_CHANNEL("_1", "1", " 1", BLIND_SWITCH, 1.0),
            ABTEST_STEREO_CHANNEL("_2", "2", " 2", BLIND_SWITCH, 1.0),
            ABTEST_STEREO_CHANNEL("_3", "3", " 3", BLIND_SWITCH, 0.0),
            ABTEST_STEREO_CHANNEL("_4", "4", " 4", BLIND_SWITCH, 0.0),
            ABTEST_STEREO_CHANNEL("_5", "5", " 5", BLIND_SWITCH, 0.0),
            ABTEST_STEREO_CHANNEL("_6", "6", " 6", BLIND_SWITCH, 0.0),
            ABTEST_STEREO_CHANNEL("_7", "7", " 7", BLIND_SWITCH, 0.0),
            ABTEST_STEREO_CHANNEL("_8", "8", " 8", BLIND_SWITCH, 0.0),
            PORTS_END
        };

        #define ABTEST_MONO_GROUP(i) \
            { "abtest_in" #i, "A/B Test input " #i,     GRP_MONO,       PGF_IN,    ab_tester_pg_mono_ ## i ##_ports        }

        #define ABTEST_STEREO_GROUP(i) \
            { "abtest_in" #i, "A/B Test input " #i,     GRP_STEREO,     PGF_IN,    ab_tester_pg_stereo_ ## i ##_ports      }

        #define ABTEST_GROUP_PORTS(i) \
            MONO_PORT_GROUP_PORT(ab_tester_pg_mono_ ## i, "in_" #i); \
            STEREO_PORT_GROUP_PORTS(ab_tester_pg_stereo_ ## i, "in_" #i "l", "in_" #i "r"); \

        ABTEST_GROUP_PORTS(1);
        ABTEST_GROUP_PORTS(2);
        ABTEST_GROUP_PORTS(3);
        ABTEST_GROUP_PORTS(4);
        ABTEST_GROUP_PORTS(5);
        ABTEST_GROUP_PORTS(6);
        ABTEST_GROUP_PORTS(7);
        ABTEST_GROUP_PORTS(8);

        static const port_group_t ab_tester_x2_mono_port_groups[] =
        {
            MAIN_MONO_OUT_PORT_GROUP,
            ABTEST_MONO_GROUP(1),
            ABTEST_MONO_GROUP(2),
            PORT_GROUPS_END
        };

        static const port_group_t ab_tester_x4_mono_port_groups[] =
        {
            MAIN_MONO_OUT_PORT_GROUP,
            ABTEST_MONO_GROUP(1),
            ABTEST_MONO_GROUP(2),
            ABTEST_MONO_GROUP(3),
            ABTEST_MONO_GROUP(4),
            PORT_GROUPS_END
        };

        static const port_group_t ab_tester_x8_mono_port_groups[] =
        {
            MAIN_MONO_OUT_PORT_GROUP,
            ABTEST_MONO_GROUP(1),
            ABTEST_MONO_GROUP(2),
            ABTEST_MONO_GROUP(3),
            ABTEST_MONO_GROUP(4),
            ABTEST_MONO_GROUP(5),
            ABTEST_MONO_GROUP(6),
            ABTEST_MONO_GROUP(7),
            ABTEST_MONO_GROUP(8),
            PORT_GROUPS_END
        };

        static const port_group_t ab_tester_x2_stereo_port_groups[] =
        {
            MAIN_STEREO_OUT_PORT_GROUP,
            ABTEST_STEREO_GROUP(1),
            ABTEST_STEREO_GROUP(2),
            PORT_GROUPS_END
        };

        static const port_group_t ab_tester_x4_stereo_port_groups[] =
        {
            MAIN_STEREO_OUT_PORT_GROUP,
            ABTEST_STEREO_GROUP(1),
            ABTEST_STEREO_GROUP(2),
            ABTEST_STEREO_GROUP(3),
            ABTEST_STEREO_GROUP(4),
            PORT_GROUPS_END
        };

        static const port_group_t ab_tester_x8_stereo_port_groups[] =
        {
            MAIN_STEREO_OUT_PORT_GROUP,
            ABTEST_STEREO_GROUP(1),
            ABTEST_STEREO_GROUP(2),
            ABTEST_STEREO_GROUP(3),
            ABTEST_STEREO_GROUP(4),
            ABTEST_STEREO_GROUP(5),
            ABTEST_STEREO_GROUP(6),
            ABTEST_STEREO_GROUP(7),
            ABTEST_STEREO_GROUP(8),
            PORT_GROUPS_END
        };

        static const int plugin_classes[]       = { C_UTILITY, -1 };
        static const int clap_features_mono[]   = { CF_AUDIO_EFFECT, CF_UTILITY, CF_MONO, -1 };
        static const int clap_features_stereo[] = { CF_AUDIO_EFFECT, CF_UTILITY, CF_STEREO, -1 };

        const meta::bundle_t ab_tester_bundle =
        {
            "ab_tester",
            "A/B Test Plugin",
            B_UTILITIES,
            "EFf4VqvMUXM",
            "Plugin that allows to perform quick A/B test with blind option"
        };

        const plugin_t ab_tester_x2_mono =
        {
            "A/B Prüfer x2 Mono",
            "A/B Tester x2 Mono",
            "A/B Tester x2 Mono",
            "ABP2M",
            &developers::v_sadovnikov,
            "ab_tester_x2_mono",
            {
                LSP_LV2_URI("ab_tester_x2_mono"),
                LSP_LV2UI_URI("ab_tester_x2_mono"),
                "ab2m",
                LSP_VST3_UID("abp2m   ab2m"),
                LSP_VST3UI_UID("abp2m   ab2m"),
                LSP_LADSPA_AB_TESTER_BASE + 0,
                LSP_LADSPA_URI("ab_tester_x2_mono"),
                LSP_CLAP_URI("ab_tester_x2_mono"),
                LSP_GST_UID("ab_tester_x2_mono"),
            },
            LSP_PLUGINS_AB_TESTER_VERSION,
            plugin_classes,
            clap_features_mono,
            E_DUMP_STATE | E_KVT_SYNC,
            ab_tester_x2_mono_ports,
            "util/ab_tester.xml",
            NULL,
            ab_tester_x2_mono_port_groups,
            &ab_tester_bundle
        };

        const plugin_t ab_tester_x4_mono =
        {
            "A/B Prüfer x4 Mono",
            "A/B Tester x4 Mono",
            "A/B Tester x4 Mono",
            "ABP4M",
            &developers::v_sadovnikov,
            "ab_tester_x4_mono",
            {
                LSP_LV2_URI("ab_tester_x4_mono"),
                LSP_LV2UI_URI("ab_tester_x4_mono"),
                "ab4m",
                LSP_VST3_UID("abp4m   ab4m"),
                LSP_VST3UI_UID("abp4m   ab4m"),
                LSP_LADSPA_AB_TESTER_BASE + 1,
                LSP_LADSPA_URI("ab_tester_x4_mono"),
                LSP_CLAP_URI("ab_tester_x4_mono"),
                LSP_GST_UID("ab_tester_x4_mono"),
            },
            LSP_PLUGINS_AB_TESTER_VERSION,
            plugin_classes,
            clap_features_mono,
            E_DUMP_STATE | E_KVT_SYNC,
            ab_tester_x4_mono_ports,
            "util/ab_tester.xml",
            NULL,
            ab_tester_x4_mono_port_groups,
            &ab_tester_bundle
        };

        const plugin_t ab_tester_x8_mono =
        {
            "A/B Prüfer x8 Mono",
            "A/B Tester x8 Mono",
            "A/B Tester x8 Mono",
            "ABP8M",
            &developers::v_sadovnikov,
            "ab_tester_x8_mono",
            {
                LSP_LV2_URI("ab_tester_x8_mono"),
                LSP_LV2UI_URI("ab_tester_x8_mono"),
                "ab8m",
                LSP_VST3_UID("abp8m   ab8m"),
                LSP_VST3UI_UID("abp8m   ab8m"),
                LSP_LADSPA_AB_TESTER_BASE + 2,
                LSP_LADSPA_URI("ab_tester_x8_mono"),
                LSP_CLAP_URI("ab_tester_x8_mono"),
                LSP_GST_UID("ab_tester_x8_mono"),
            },
            LSP_PLUGINS_AB_TESTER_VERSION,
            plugin_classes,
            clap_features_mono,
            E_DUMP_STATE | E_KVT_SYNC,
            ab_tester_x8_mono_ports,
            "util/ab_tester.xml",
            NULL,
            ab_tester_x8_mono_port_groups,
            &ab_tester_bundle
        };

        const plugin_t ab_tester_x2_stereo =
        {
            "A/B Prüfer x2 Stereo",
            "A/B Tester x2 Stereo",
            "A/B Tester x2 Stereo",
            "ABP2S",
            &developers::v_sadovnikov,
            "ab_tester_x2_stereo",
            {
                LSP_LV2_URI("ab_tester_x2_stereo"),
                LSP_LV2UI_URI("ab_tester_x2_stereo"),
                "ab2s",
                LSP_VST3_UID("abp2s   ab2s"),
                LSP_VST3UI_UID("abp2s   ab2s"),
                LSP_LADSPA_AB_TESTER_BASE + 3,
                LSP_LADSPA_URI("ab_tester_x2_stereo"),
                LSP_CLAP_URI("ab_tester_x2_stereo"),
                LSP_GST_UID("ab_tester_x2_stereo"),
            },
            LSP_PLUGINS_AB_TESTER_VERSION,
            plugin_classes,
            clap_features_stereo,
            E_DUMP_STATE | E_KVT_SYNC,
            ab_tester_x2_stereo_ports,
            "util/ab_tester.xml",
            NULL,
            ab_tester_x2_stereo_port_groups,
            &ab_tester_bundle
        };

        const plugin_t ab_tester_x4_stereo =
        {
            "A/B Prüfer x4 Stereo",
            "A/B Tester x4 Stereo",
            "A/B Tester x4 Stereo",
            "ABP4S",
            &developers::v_sadovnikov,
            "ab_tester_x4_stereo",
            {
                LSP_LV2_URI("ab_tester_x4_stereo"),
                LSP_LV2UI_URI("ab_tester_x4_stereo"),
                "ab4s",
                LSP_VST3_UID("abp4s   ab4s"),
                LSP_VST3UI_UID("abp4s   ab4s"),
                LSP_LADSPA_AB_TESTER_BASE + 4,
                LSP_LADSPA_URI("ab_tester_x4_stereo"),
                LSP_CLAP_URI("ab_tester_x4_stereo"),
                LSP_GST_UID("ab_tester_x4_stereo"),
            },
            LSP_PLUGINS_AB_TESTER_VERSION,
            plugin_classes,
            clap_features_stereo,
            E_DUMP_STATE | E_KVT_SYNC,
            ab_tester_x4_stereo_ports,
            "util/ab_tester.xml",
            NULL,
            ab_tester_x4_stereo_port_groups,
            &ab_tester_bundle
        };

        const plugin_t ab_tester_x8_stereo =
        {
            "A/B Prüfer x8 Stereo",
            "A/B Tester x8 Stereo",
            "A/B Tester x8 Stereo",
            "ABP8S",
            &developers::v_sadovnikov,
            "ab_tester_x8_stereo",
            {
                LSP_LV2_URI("ab_tester_x8_stereo"),
                LSP_LV2UI_URI("ab_tester_x8_stereo"),
                "ab8s",
                LSP_VST3_UID("abp8s   ab8s"),
                LSP_VST3UI_UID("abp8s   ab8s"),
                LSP_LADSPA_AB_TESTER_BASE + 5,
                LSP_LADSPA_URI("ab_tester_x8_stereo"),
                LSP_CLAP_URI("ab_tester_x8_stereo"),
                LSP_GST_UID("ab_tester_x8_stereo"),
            },
            LSP_PLUGINS_AB_TESTER_VERSION,
            plugin_classes,
            clap_features_stereo,
            E_DUMP_STATE | E_KVT_SYNC,
            ab_tester_x8_stereo_ports,
            "util/ab_tester.xml",
            NULL,
            ab_tester_x8_stereo_port_groups,
            &ab_tester_bundle
        };

    } /* namespace meta */
} /* namespace lsp */



