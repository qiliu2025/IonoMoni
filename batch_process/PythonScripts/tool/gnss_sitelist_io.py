# -*- coding: utf-8 -*-
# ===================================================================
# sitelist file reading interface
# ===================================================================

import logging
import os
import re

# get log
logger = logging.getLogger()



########################################################################
# read  sitelist as list
########################################################################


def readSitelist(fileName):
    """
    读取sitelist文件，并存储为list
    """
    siteList = set()
    try:
        # file mode
        if os.path.isfile(fileName):
            fr = open(fileName, "r")
            lines = fr.readlines()
            fr.close()

            for line in lines:
                tmpList = line.strip().split()
                for site in tmpList:
                    if len(site) != 4:
                        continue
                    else:
                        siteList.add(site.lower())
        # string mode
        else:
            tmpList = str(fileName).lower().split()
            for site in tmpList:
                if len(site) != 4:
                    continue
                else:
                    siteList.add(site.lower())
    except:
        logging.error("Can not open sitelist file : " + fileName)
        exit()
    siteList = list(set(siteList))
    siteList.sort()
    return siteList