# =================================================================
# Program description:
# The script is mainly used to manage the data pool and support the
# data input of different programs. Therefore, the specified data
# is identified and named in the header file, and all operations
# in the subsequent program are subject to the agreed format and name.
# =================================================================

import configparser
import os
import logging

# ===================================================================
# ini format file is as follows:
# [section_name] A node
# key_name = "value" A value in a node
# ===================================================================
class t_iniCfg(object):  # meter
    def __init__(self, pathIni):
        """
        The class that stores IonoMoni configuration file information in XML format
        """
        if os.path.exists(pathIni):
            self.path = pathIni
            self.config = configparser.ConfigParser()
            self.config.read(pathIni,"utf-8")
            logging.info(f"Read the path : {pathIni}")
        else:
            self.path = ""
            logging.error(f"Check your path {pathIni}")

    def getValue(self, sect_name, key_name):
        """
        Gets the value of a key and throws an exception if it is empty
        """
        result = ""
        if False == self.config.has_section(sect_name):
            result = ""
        if False == self.config.has_option(sect_name, key_name):
            result = ""
        
        # Determines whether the content is empty
        result =  self.config.get(sect_name, key_name)
        if result == "":
            raise Exception
        
        # Determine whether it is multiple characters
        resultList = result.split("~")
        if len(resultList) == 1:
            return resultList[0]
        else:
            return resultList
