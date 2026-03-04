#include "decode_obs.h"
#include "gcoders/rinexo.h"
#include "gio/gfile.h"
#include <iostream>
using namespace std;
using namespace gnut;

namespace gnut {

    t_gallobs* decode_obs(t_gcfg_ppp& gset, shared_ptr<spdlog::logger> my_logger)
    {
        my_logger->info("Start decoding observation files...");

        t_gallobs* gobs = new t_gallobs();
        gobs->gset(&gset);
        gobs->spdlog(my_logger);

        multimap<IFMT, string> inputs = gset.inputs_all();
        int obs_count = 0;  

        for (auto it = inputs.begin(); it != inputs.end(); ++it)
        {
            IFMT fmt = it->first;
            string filename = it->second;

            if (fmt != IFMT::RINEXO_INP) continue;

            my_logger->info("Decoding observation file: {}", filename);

            t_rinexo* decoder = new t_rinexo(&gset);
            decoder->path(filename);
            decoder->add_data("ID0", gobs);      // Associate output container
            decoder->spdlog(my_logger);          // Pass logger for consistent logging

            t_gfile* file_reader = new t_gfile(my_logger);
            file_reader->path(filename);
            file_reader->coder(decoder);
            file_reader->run_read();             // Perform file reading and decoding

            delete decoder;
            delete file_reader;

            obs_count++;
        }

        my_logger->info("Number of successfully decoded observation files: {}", obs_count);

        return gobs;
    }

}
