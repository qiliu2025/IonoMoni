#ifndef GCFG_PPP_H
#define GCFG_PPP_H

#include <string>
#include <iostream>
#include <signal.h>

#include "gall/gallprec.h"
#include "gall/gallpcv.h"
#include "gall/gallobs.h"
#include "gall/gallotl.h"
#include "gall/gallbias.h"
#include "gproc/gpppflt.h"
#include "gproc/gpreproc.h"
#include "gio/gio.h"
#include "gio/gfile.h"
#include "gio/grtlog.h"
#include "spdlog/spdlog.h"
#include "gutils/gtime.h"
#include "gutils/gtypeconv.h"
#include "gdata/gdata.h"
#include "gcoders/gcoder.h"
#include "gcoders/rinexo.h"
#include "gcoders/rinexc.h"
#include "gcoders/rinexn.h"
#include "gcoders/biasinex.h"
#include "gcoders/biabernese.h"
#include "gcoders/sp3.h"
#include "gcoders/atx.h"
#include "gcoders/blq.h"
#include "gmodels/gpcv.h"
#include "gmodels/gbancroft.h"
#include "gset/gsetgen.h"
#include "gset/gsetinp.h"
#include "gset/gsetout.h"
#include "gset/gsetproc.h"
#include "gset/gsetgnss.h"
#include "gset/gsetflt.h"
#include "gset/gsetrec.h"
#include "gproc/gpvtflt.h"
#include "gdata/gifcb.h"
#include "gcoders/upd.h"
#include "gcoders/sp3.h"
#include "gcoders/rinexo.h"
#include "gcoders/rinexn.h"
#include "gcoders/rinexc.h"
#include "gcoders/poleut1.h"
#include "gcoders/ifcb.h"
#include "gcoders/gcoder.h"
#include "gcoders/dvpteph405.h"
#include "gcoders/blq.h"
#include "gcoders/biasinex.h"
#include "gcoders/biabernese.h"
#include "gcoders/atx.h"
#include "gdata/gnavde.h"

using namespace std;
using namespace pugi;

namespace gnut
{

    class t_gcfg_ppp : public t_gsetgen,
        public t_gsetinp,
        public t_gsetout,
        public t_gsetgnss,
        public t_gsetproc,
        public t_gsetflt,
        public t_gsetrec,
        public t_gsetamb
    {
    public:
        /** Default constructor. */
        t_gcfg_ppp();

        /** Default destructor. */
        ~t_gcfg_ppp();

        /** Check all configuration settings. */
        void check();

        /** Print help for all configuration settings. */
        void help();

        /** Read custom XML field, e.g., <function>XXX</function> */
        string comment(const string& key)
        {
            _gmutex.lock();
            string val = _doc.child("config").child("gen").child(key.c_str()).child_value();
            _gmutex.unlock();
            return trim(val);
        }

        /** Support passing XML file path directly (not argc/argv method) */
        void arg(const string& xmlfile, bool xml = true, bool log = true);

        /** Support traditional argc/argv parsing for backward compatibility */
        void arg(int argc, char** argv, bool xml, bool log);

        /** Initialize internal logger */
        void init_logger();

        /** Provide logger interface for external modules */
        std::shared_ptr<spdlog::logger> logger();

    protected:
    private:
        t_grtlog _grtlog;
    };

} // namespace gnut

#endif
