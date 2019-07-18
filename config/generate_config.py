#!/usr/bin/python 

import sys
import argparse

def generate_config(program_name):
    logFormat = R'$template logFormat, "[%timegenerated%] - %hostname% - %syslogtag:R:[A-Za-z]\+\[[0-9]\+\]--end% - [%rawmsg%]\n"'
    fileFormat = "$template fileFormat, \"/var/log/%s/%%syslogtag:R:[A-Za-z]\\+--end%%-%%$year%%%%$month%%%%$day%%%%$hour%%.log\"" % (program_name)
    filter = ":syslogtag, ereregex, \"%s\\[[0-9]+\\]\" ?fileFormat;logFormat" % (program_name)
    fileName = "/etc/rsyslog.d/%s.conf" % (program_name)
    fileHandler = open(fileName, "w")
    fileHandler.write(logFormat + '\n')
    fileHandler.write(fileFormat + '\n')
    fileHandler.write(filter + '\n')
    fileHandler.close()

def main():
    parser = argparse.ArgumentParser(usage="./generate_config.py $SERVER_NAME", description="Generte rsyslog.d config file by server name")
    parser.add_argument("SERVER_NAME", help="assign the server name", type=str)
    try:
        args = parser.parse_args()
    except:
        sys.exit(1)
    generate_config(args.SERVER_NAME);
    return 0

if __name__ == "__main__":
    sys.exit(main())
