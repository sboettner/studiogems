/*
 * Studio Gems DISTRHO Plugins
 * Copyright (C) 2022 Stefan T. Boettner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the LICENSE file.
 */

#ifndef DISTRHO_PLUGIN_OPAL_H_INCLUDED
#define DISTRHO_PLUGIN_OPAL_H_INCLUDED

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class DistrhoPluginOpal : public Plugin
{
public:
    DistrhoPluginOpal();
    ~DistrhoPluginOpal() override;

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override
    {
        return "OpalChorus";
    }

    const char* getDescription() const override
    {
        return "A chorus plugin";
    }

    const char* getMaker() const noexcept override
    {
        return "Stefan T. Boettner";
    }

    const char* getHomePage() const override
    {
        return "https://...";
    }

    const char* getLicense() const noexcept override
    {
        return "GPL v3+";
    }

    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('S', 'G', 'O', 'p');
    }

    // -------------------------------------------------------------------
    // Init

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override;
    void initParameter(uint32_t index, Parameter& parameter) override;

    // -------------------------------------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void  setParameterValue(uint32_t index, float value) override;

    // -------------------------------------------------------------------
    // Process

    void activate() override;
    void deactivate() override;
    void run(const float**, float** outputs, uint32_t frames) override;

    // -------------------------------------------------------------------

private:

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistrhoPluginOpal)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // DISTRHO_PLUGIN_OPAL_H_INCLUDED
