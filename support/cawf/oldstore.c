From forthdv!stephan Mon Feb  8 17:27:32 1993
Path: forthdv!stephan
From: stephan@forthdv.rmt.sub.org (Stephan Forth)
Newsgroups: comp.os.coherent
Distribution: world
Subject: cc problem(?)
Keywords: 
Summary: 
X-Newsreader: TIN [version 1.1 PL8]
Message-ID: <930207186@forthdv.rmt.sub.org>
Organization: Forthdv
Date: Sun,  7 Feb 93 09:34:42 GMT
Lines: 40


Hi networld,

I do have a problem with the cc. I must say first, that I am not a very
experiensed C programer under UNIX. The following piece of code
produces an error under Coherent cc and compiles fine under DOS Quick-C:

-------------piece of code-------------------

struct scale Scale[] = {		/* scaling factors */
	{ 'i',	(240.0)		 	},
	{ 'c',	((240.0 * 50.0)/127.0)	},
	{ 'P',	(240.0/6.0)		},
	{ 'p',	(240.0/72.0)		},
	{ 'u',  (1.0)                   },
	{ 'm',  (1.0)                   },
	{ 'n',  (1.0)                   },
	{ 'v',  (1.0)                   },
	{ '\0',	(0.0)			}
};
--------------------end of code--------------------------------

The compiler brings out the following error message:

cc -f -DUNIX -DSTDLIB   -c store.c -o store.o
147: store.c: initializer too complex
148: store.c: initializer too complex
149: store.c: initializer too complex


Does anybody has an idea?? The code is part of the CAWF Text formatter
which I am trying to port. It supports a big subset of nroff commands
and supports national characters (Coherents nroff doesn't do this)

Regards Stephan
--
------------
Stephan Forth                     stephan@forthdv.rmt.sub.org
Am Schinnergraben 30              
6500 Mainz 42, Germany            voice: +49 6131 504273

