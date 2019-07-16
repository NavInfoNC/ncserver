#!/usr/bin/python 

import sys

def generate_config(program_name):
    logFormat = R'$template logFormat, "[%timegenerated%] [%hostname%] [{\"server\":\"%syslogtag:R:[A-Za-z]\+\[[0-9]\+\]--end%\",\"level\":\"%syslogseverity-text%\",\"log\":\"[%rawmsg%]\"}]\n"'
    fileFormat = "$template logFormat \"/var/log/%s/%%syslogtag:R:[A-Za-z]\\+--end%%-%%$year%%%%$month%%%%$day%%%%$hour%%.log\"" % (program_name)
    filter = ":syslogtag, ereregex, \"%s\\[[0-9]+\\]\" ?fileFormat;logFormat" % (program_name)
    fileName = "/etc/rsyslog.d/%s.conf" % (program_name)
    fileHandler = open(fileName, "w")
    fileHandler.write(logFormat + '\n')
    fileHandler.write(fileFormat + '\n')
    fileHandler.write(filter + '\n')
    fileHandler.close()

def print_help():
    print("./generate_config.py $server_name")

def main():
    if len(sys.argv) != 2:
        print("parameter error")
        print_help()
        return -1
    generate_config(sys.argv[1]);
    return 0

if __name__ == "__main__":
    sys.exit(main())
