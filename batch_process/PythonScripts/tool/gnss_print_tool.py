# =================================================================
# Responsible for outputting key information and status of the solution process to the terminal in real-time
# Copyright (C) The GNSS Center, Wuhan University & Chinese Academy of Surveying and mapping
# =================================================================
def PrintGDD(string, printtype):
    """
    2022-03-27 : * Screen print style     by Chang Chuntao -> Version : 1.00
    2022-04-30 : * Add nothing    by Chang Chuntao -> Version : 1.12
    """
    if printtype == "input":
        print("  -  " + string)
    elif printtype == "normal":
        print("  *  " + string)
    elif printtype == "fail":
        print("  x  " + string)
    elif printtype == "warning":
        print("  #  " + string)
    elif printtype == "important":
        print(" *** " + string)
    elif printtype == "nothing":
        print("     " + string)