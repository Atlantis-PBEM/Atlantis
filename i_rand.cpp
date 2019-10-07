// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 1995-1999 Geoff Dunbar
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program, in the file license.txt. If not, write
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// See the Atlantis Project web page for details:
// http://www.prankster.com/project
//
// END A3HEADER
//
// I grabbed this code from:
// http://ourworld.compuserve.com/homepages/bob_jenkins/isaacafa.htm
//
// It appears to be free for any use.
//
/*
------------------------------------------------------------------------------
rand.c: By Bob Jenkins.  My random number generator, ISAAC.
MODIFIED:
	960327: Creation (addition of randinit, really)
	970719: use context, not global variables, for internal state
	980324: added main (ifdef'ed out), also rearranged randinit()
------------------------------------------------------------------------------
*/
#ifndef STANDARD
#include "i_std.h"
#endif
#ifndef RAND
#include "i_rand.h"
#endif


#define ind(mm,x)	(*(ub4 *)((ub1 *)(mm) + ((x) & ((RANDSIZ-1)<<2))))
#define rngstep(mix,a,b,mm,m,m2,r,x) \
{ \
	x = *m; \
	a = (a^(mix)) + *(m2++); \
	*(m++) = y = ind(mm,x) + a + b; \
	*(r++) = b = ind(mm,y>>RANDSIZL) + x; \
}

void isaac(randctx *ctx)
{
	ub4 a,b,x,y,*m,*mm,*m2,*r,*mend;
	mm=ctx->randmem; r=ctx->randrsl;
	a = ctx->randa; b = ctx->randb + (++ctx->randc);
	for (m = mm, mend = m2 = m+(RANDSIZ/2); m<mend; )
	{
		rngstep( a<<13, a, b, mm, m, m2, r, x);
		rngstep( a>>6 , a, b, mm, m, m2, r, x);
		rngstep( a<<2 , a, b, mm, m, m2, r, x);
		rngstep( a>>16, a, b, mm, m, m2, r, x);
	}
	for (m2 = mm; m2<mend; )
	{
		rngstep( a<<13, a, b, mm, m, m2, r, x);
		rngstep( a>>6 , a, b, mm, m, m2, r, x);
		rngstep( a<<2 , a, b, mm, m, m2, r, x);
		rngstep( a>>16, a, b, mm, m, m2, r, x);
	}
	ctx->randb = b; ctx->randa = a;
}


#define mix(a,b,c,d,e,f,g,h) \
{ \
	a^=b<<11; d+=a; b+=c; \
	b^=c>>2;  e+=b; c+=d; \
	c^=d<<8;  f+=c; d+=e; \
	d^=e>>16; g+=d; e+=f; \
	e^=f<<10; h+=e; f+=g; \
	f^=g>>4;  a+=f; g+=h; \
	g^=h<<8;  b+=g; h+=a; \
	h^=a>>9;  c+=h; a+=b; \
}

/* if (flag==TRUE), then use the contents of randrsl[] to initialize mm[]. */
void randinit(randctx *ctx, word flag)
{
	word i;
	ub4 a,b,c,d,e,f,g,h;
	ub4 *m,*r;
	ctx->randa = ctx->randb = ctx->randc = 0;
	m=ctx->randmem;
	r=ctx->randrsl;
	a=b=c=d=e=f=g=h=0x9e3779b9;	/* the golden ratio */

	for (i=0; i<4; ++i)		/* scramble it */
	{
		mix(a,b,c,d,e,f,g,h);
	}

	if (flag) 
	{
		/* initialize using the contents of r[] as the seed */
		for (i=0; i<RANDSIZ; i+=8)
		{
			a+=r[i  ]; b+=r[i+1]; c+=r[i+2]; d+=r[i+3];
			e+=r[i+4]; f+=r[i+5]; g+=r[i+6]; h+=r[i+7];
			mix(a,b,c,d,e,f,g,h);
			m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
			m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
		}
		/* do a second pass to make all of the seed affect all of m */
		for (i=0; i<RANDSIZ; i+=8)
		{
			a+=m[i  ]; b+=m[i+1]; c+=m[i+2]; d+=m[i+3];
			e+=m[i+4]; f+=m[i+5]; g+=m[i+6]; h+=m[i+7];
			mix(a,b,c,d,e,f,g,h);
			m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
			m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
		}
	}
	else
	{
		/* fill in mm[] with messy stuff */
		for (i=0; i<RANDSIZ; i+=8)
		{
			mix(a,b,c,d,e,f,g,h);
			m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
			m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
		}
	}

	isaac(ctx);		/* fill in the first set of results */
	ctx->randcnt=RANDSIZ;	/* prepare to use the first set of results */
}
