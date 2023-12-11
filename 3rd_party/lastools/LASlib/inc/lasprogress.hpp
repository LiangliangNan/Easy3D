/*
===============================================================================

  FILE:  lasprogress.hpp

  CONTENTS:

    Simple text progress bar.

  PROGRAMMERS:

    Jean-Romain Roussel

  COPYRIGHT:

    (c) 2023, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    26 September 2023 -- created for lascopcindex

===============================================================================
*/

#ifndef LAS_PROGRESS_HPP
#define LAS_PROGRESS_HPP

#include "mydefs.hpp"
#include "lasdefinitions.hpp"

#define LASPROGRESSSYMBOL "=================================================="

class LASprogress
{
public:
    LASprogress(const LASreader* lasreader) : LASprogress((U64)lasreader->npoints)
    { 
    };

    LASprogress(U64 nmax)
    {
        reset();
        set_total(nmax);
        set_resolution(0.1f); // update the bar every 0.1%
        set_width(30);
        set_display(true);
    };

    LASprogress &operator++(int)
    {
        current++;
        compute_percentage();
        return *this;
    };

    inline void update(const LASreader* lasreader) 
    { 
        update((U64)lasreader->p_count); 
    };

    inline void update(U64 current) 
    { 
        this->current = current; 
        compute_percentage();
    };

    inline void reset()
    {
        percentage = 0;
        previous_percentage = F32_MIN;
        current = 0;
    };

    inline void set_width(I32 width) 
    { 
        if (width < 0) this->width = 30;
        else if (width > 50) this->width = 50;
        else this->width = width;
    };

    inline void set_display(BOOL display) 
    { 
        this->display = display; 
    };

    inline void set_resolution(F32 resolution) 
    { 
        this->resolution = (resolution < 0) ? 0.1f/100.0f : resolution/100.0f;
    };

    inline void set_total(U64 nmax)
    {
        total = nmax;
    };

    inline F32 get_progress() 
    { 
        return percentage*100.0f; 
    };

    inline void done()
    {
        current = total;
        percentage = 1.0f;
        previous_percentage = F32_MIN;
        print();
        if (display) fprintf(stderr, "\n");
        reset();
    };

    void print()
    {
        if (display && (F32)(percentage - previous_percentage) >= resolution)
        {
            previous_percentage = percentage;
            I32 completed = (I32)(width * percentage);
            I32 remaining = (I32)(width - completed);
            fprintf(stderr, "[%.*s%*s] %.1lf%%\r", completed, LASPROGRESSSYMBOL, remaining, "", percentage * 100);
            fflush(stderr);
        }
    };

private:
    inline void compute_percentage() 
    { 
        percentage = (F32)((F64)current / (F64)total);
    };

private:
    U64 total;
    U64 current;
    F32 resolution;
    F32 percentage;
    F32 previous_percentage;
    I32 width;
    BOOL display;
};

#endif