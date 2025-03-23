/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * profile.c -- For timing and event counting.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: profile.c,v $
 * Revision 1.7  2005/06/22 03:10:59  arthchan2003
 * 1, Fixed doxygen documentation, 2, Added  keyword.
 *
 * Revision 1.3  2005/03/30 01:22:48  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 11-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added ptmr_init().
 * 
 * 19-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32) && !defined(__SYMBIAN32__)
# include <windows.h>
# ifndef _WIN32_WCE
#  include <time.h>
# endif
#elif defined(HAVE_UNISTD_H) /* I know this, this is Unix... */
# include <unistd.h>
# include <sys/time.h>
# include <sys/resource.h>
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4996)
#endif

#include "sphinxbase/profile.h"
#include "sphinxbase/err.h"
#include "sphinxbase/ckd_alloc.h"

#if defined(_WIN32_WCE) || defined(_WIN32_WP)
DWORD unlink(const char *filename)
{
	WCHAR *wfilename;
	DWORD rv;
	size_t len;

	len = mbstowcs(NULL, filename, 0);
	wfilename = ckd_calloc(len+1, sizeof(*wfilename));
	mbstowcs(wfilename, filename, len);
	rv = DeleteFileW(wfilename);
	ckd_free(wfilename);

	return rv;
}
#endif

pctr_t *
pctr_new(char *nm)
{
    pctr_t *pc;

    pc = ckd_calloc(1, sizeof(pctr_t));
    pc->name = ckd_salloc(nm);
    pc->count = 0;

    return pc;
}

void
pctr_reset(pctr_t * ctr)
{
    ctr->count = 0;
}

void
pctr_increment(pctr_t * ctr, int32 inc)
{
    ctr->count += inc;
}

void
pctr_print(FILE * fp, pctr_t * ctr)
{
    fprintf(fp, "CTR:");
    fprintf(fp, "[%d %s]", ctr->count, ctr->name);
}

void
pctr_free(pctr_t * pc)
{
    if (pc) {
        if (pc->name)
            ckd_free(pc->name);
    }
    ckd_free(pc);
}

static float64
make_sec(clock_t t)
{
    return (float64)t / CLOCKS_PER_SEC;
}

void
ptmr_start(ptmr_t * tm)
{
    tm->start_cpu = make_sec(clock());
    tm->start_elapsed = make_sec(clock());
}

void
ptmr_stop(ptmr_t * tm)
{
    float64 dt_cpu, dt_elapsed;
    
    dt_cpu = make_sec(clock()) - tm->start_cpu;
    dt_elapsed = make_sec(clock()) - tm->start_elapsed;
    
    tm->t_cpu = dt_cpu;
    tm->t_elapsed = dt_elapsed;
    tm->t_tot_cpu += dt_cpu;
    tm->t_tot_elapsed += dt_elapsed;
}

void
ptmr_reset(ptmr_t * tm)
{
    tm->t_cpu = 0.0;
    tm->t_elapsed = 0.0;
}

void
ptmr_init(ptmr_t * tm)
{
    tm->t_cpu = 0.0;
    tm->t_elapsed = 0.0;
    tm->t_tot_cpu = 0.0;
    tm->t_tot_elapsed = 0.0;
    tm->start_cpu = 0.0;
    tm->start_elapsed = 0.0;
}

void
ptmr_reset_all(ptmr_t * tm)
{
    while (tm->name != NULL) {
        ptmr_reset(tm);
        ++tm;
    }
}

void
ptmr_print_all(FILE * fp, ptmr_t * tm, float64 norm)
{
    while (tm->name != NULL) {
        fprintf(fp, "%s: %.2f\n", tm->name, tm->t_cpu / norm);
        ++tm;
    }
}

int32
host_pclk(int32 dummy)
{
    return 0;
}

int32
host_endian(void)
{
    return 1; // Assume little-endian
}
