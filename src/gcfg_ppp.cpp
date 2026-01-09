#include <iomanip>
#include <sstream>

#include "gcfg_ppp.h"

using namespace std;
using namespace pugi;

namespace gnut
{

    t_gcfg_ppp::t_gcfg_ppp()
        : t_gsetbase(),
        t_gsetgen(),
        t_gsetinp(),
        t_gsetout(),
        t_gsetgnss(),
        t_gsetproc(),
        t_gsetflt(),
        t_gsetrec(),
        t_gsetamb()
    {
        _IFMT_supported.insert(IFMT::RINEXO_INP);
        _IFMT_supported.insert(IFMT::RINEXC_INP);
        _IFMT_supported.insert(IFMT::RINEXN_INP);
        _IFMT_supported.insert(IFMT::ATX_INP);
        _IFMT_supported.insert(IFMT::BLQ_INP);
        _IFMT_supported.insert(IFMT::SP3_INP);
        _IFMT_supported.insert(IFMT::BIAS_INP);
        _IFMT_supported.insert(IFMT::BIASINEX_INP);
        _IFMT_supported.insert(IFMT::DE_INP);
        _IFMT_supported.insert(IFMT::EOP_INP);
        _IFMT_supported.insert(IFMT::LEAPSECOND_INP);
        _IFMT_supported.insert(IFMT::UPD_INP);
        _IFMT_supported.insert(IFMT::IFCB_INP);

        _OFMT_supported.insert(LOG_OUT);
        _OFMT_supported.insert(PPP_OUT);
        _OFMT_supported.insert(FLT_OUT);
        _OFMT_supported.insert(OFMT::TXT_OUT);
    }

    t_gcfg_ppp::~t_gcfg_ppp()
    {
    }

    void t_gcfg_ppp::check()
    {
        t_gsetgen::check();
        t_gsetinp::check();
        t_gsetout::check();
        t_gsetrec::check();
        t_gsetflt::check();
        t_gsetproc::check();
        t_gsetgnss::check();
        t_gsetamb::check();
    }

    void t_gcfg_ppp::help()
    {
        t_gsetbase::help_header();
        t_gsetgen::help();
        t_gsetinp::help();
        t_gsetout::help();
        t_gsetrec::help();
        t_gsetflt::help();
        t_gsetproc::help();
        t_gsetgnss::help();
        t_gsetamb::help();
        t_gsetbase::help_footer();
        t_gsetamb::help_footer();
    }

    void t_gcfg_ppp::arg(int argc, char** argv, bool xml, bool log)
    {
        t_gsetbase::arg(argc, argv, xml, log);
    }

    void t_gcfg_ppp::arg(const std::string& xmlfile, bool xml, bool log)
    {
        if (xmlfile.empty()) {
            spdlog::error("[gcfg_ppp::arg] XML file path is empty!");
            throw std::runtime_error("XML path is empty!");
        }

        if (read(xmlfile) != 0) {
            spdlog::error("[gcfg_ppp::arg] Failed to read XML file!");
            throw std::runtime_error("Failed to read XML!");
        }

        int argc = 2;
        const char* argv[2] = { "IonoMoni.exe", xmlfile.c_str() };
        t_gsetbase::arg(argc, const_cast<char**>(argv), xml, log);
    }

} // namespace gnut
