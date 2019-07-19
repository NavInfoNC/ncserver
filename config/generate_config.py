#!/usr/bin/python 

import sys
import argparse

def generate_config(program_name):
    logFormat = R'$template logFormat, "[%timegenerated%] - %hostname% - %syslogtag:R:[A-Za-z_-]\+\[[0-9]\+\]--end% - %rawmsg%\n"'
    fileFormat = "$template fileFormat, \"/var/log/%s/%%syslogtag:R:[A-Za-z_-]\\+--end%%-%%$year%%%%$month%%%%$day%%%%$hour%%.log\"" % (program_name)
    filter = ":syslogtag, ereregex, \"%s\\[[0-9]+\\]\" ?fileFormat;logFormat" % (program_name)
    fileName = "/etc/rsyslog.d/%s.conf" % (program_name)
    fileHandler = open(fileName, "w")
    fileHandler.write(logFormat + '\n')
    fileHandler.write(fileFormat + '\n')
    fileHandler.write(filter + '\n')
    fileHandler.close()

def main():
    parser = argparse.ArgumentParser(description="Generte rsyslog.d config file (/etc/rsyslog.d/SERVER_NAME.conf) by server name")
    parser.add_argument("server_name", metavar='SERVER_NAME', help="assign the server name")
    try:
        args = parser.parse_args()
    except:
        sys.exit(1)
    generate_config(args.server_name);
    return 0

if __name__ == "__main__":
    sys.exit(main())
